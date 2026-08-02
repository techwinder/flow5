#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

private_libraries=()

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
repo_root="$(cd -- "$script_dir/../.." && pwd -P)"
desktop_source="$script_dir/flow5.desktop"
work_dir=""
apprun_hardened=0

log() {
    printf '[flow5-appimage] %s\n' "$*"
}

warn() {
    printf '[flow5-appimage] WARNING: %s\n' "$*" >&2
}

die() {
    printf '[flow5-appimage] ERROR: %s\n' "$*" >&2
    exit 1
}

usage() {
    cat <<'EOF'
Usage:
  APPIMAGETOOL=/absolute/path/to/appimagetool-x86_64.AppImage \
    finalize-appdir.sh INPUT_APPDIR VERSION [OUTPUT_DIRECTORY]

Environment:
  APPIMAGETOOL           appimagetool path. If unset, known names are searched
                         on PATH.
  APPIMAGETOOL_SHA256    Optional expected SHA-256 for appimagetool.
  TMPDIR                 Staging location. Packaging needs roughly twice the
                         AppDir size and must not run out of space on a small
                         tmpfs /tmp.
EOF
}

cleanup() {
    if [[ -n "$work_dir" && -d "$work_dir" ]]; then
        rm -rf -- "$work_dir"
    fi
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

resolve_appimagetool() {
    local candidate=""

    if [[ -n "${APPIMAGETOOL:-}" ]]; then
        candidate="$APPIMAGETOOL"
    else
        local name
        for name in \
            appimagetool-x86_64.AppImage \
            appimagetool-x86_64 \
            appimagetool; do
            if candidate="$(command -v "$name" 2>/dev/null)"; then
                break
            fi
            candidate=""
        done
    fi

    [[ -n "$candidate" ]] || die \
        "appimagetool was not found; set APPIMAGETOOL or install it on PATH"
    [[ -f "$candidate" ]] || die "appimagetool is not a file: $candidate"
    [[ -x "$candidate" ]] || die "appimagetool is not executable: $candidate"
    realpath -e -- "$candidate"
}

has_library() {
    # Pathname expansion is intentional: the second argument is a library glob.
    # shellcheck disable=SC2206
    local matches=("$1"/$2)
    [[ -e "${matches[0]}" ]]
}

# Read Flow5's own libraries from the executable instead of hard-coding them.
# Each one must be bundled and resolve inside the image; libXFoil.so.1 in
# particular is also shipped by XFLR5.
detect_private_libraries() {
    local executable="$1"

    mapfile -t private_libraries < <(readelf -d "$executable" 2>/dev/null |
        sed -nE 's/.*\(NEEDED\).*Shared library: \[(lib(XFoil|flow5)[^]]*)\].*/\1/p')

    (( ${#private_libraries[@]} > 0 )) || die \
        "flow5 declares no private Flow5 library in DT_NEEDED"
    printf '%s\n' "${private_libraries[@]}" | grep -q '^libXFoil\.' || die \
        "flow5 does not link a private libXFoil; it would load an XFLR5 system library"

    local joined=""
    joined="$(printf '%s, ' "${private_libraries[@]}")"
    log "Private Flow5 libraries: ${joined%, }"
}

validate_appdir_input() {
    local appdir="$1"
    local apprun_target=""
    local name

    [[ -f "$appdir/flow5" ]] || die "Input AppDir is missing flow5"
    [[ -x "$appdir/flow5" ]] || die "Input AppDir flow5 is not executable"
    [[ -e "$appdir/AppRun" ]] || die \
        "Input AppDir is missing AppRun or AppRun is a broken link"
    [[ -x "$appdir/AppRun" ]] || die "Input AppDir AppRun is not executable"

    apprun_target="$(realpath -e -- "$appdir/AppRun")"
    case "$apprun_target" in
        "$appdir"/*) ;;
        *) die "AppRun resolves outside the input AppDir: $apprun_target" ;;
    esac

    for name in lib plugins translations; do
        [[ -d "$appdir/$name" ]] || die \
            "Input AppDir is missing the $name/ directory"
    done

    [[ -f "$appdir/qt.conf" ]] || die "Input AppDir is missing qt.conf"
    [[ -r "$appdir/flow5.png" ]] || die \
        "Input AppDir is missing the root Flow5 icon: flow5.png"

    has_library "$appdir/translations" 'flow5_*.qm' || die \
        "Input AppDir translations/ has no Flow5 .qm translations"

    grep -Eqi '^[[:space:]]*Plugins[[:space:]]*=[[:space:]]*(\./)?plugins/?[[:space:]]*$' \
        "$appdir/qt.conf" || die \
        "qt.conf must point Plugins at the bundled plugins/ directory"

    local elf_header=""
    elf_header="$(readelf -h "$appdir/flow5" 2>/dev/null)" || die \
        "flow5 is not a readable ELF executable"
    if ! grep -Eq 'Class:[[:space:]]+ELF64' <<<"$elf_header"; then
        die "flow5 is not a 64-bit ELF executable"
    fi
    if ! grep -Eq 'Machine:[[:space:]]+Advanced Micro Devices X86-64' \
        <<<"$elf_header"; then
        die "flow5 is not an x86-64 ELF executable"
    fi

    detect_private_libraries "$appdir/flow5"
}

validate_runpath() {
    local executable="$1"
    local runpaths=""
    local entry=""
    local found_origin=0

    runpaths="$(readelf -d "$executable" 2>/dev/null | sed -nE \
        's/^.*Library (rpath|runpath): \[([^]]*)\].*$/\2/p')"
    [[ -n "$runpaths" ]] || die "flow5 has no ELF RPATH or RUNPATH"

    while IFS= read -r entry; do
        case "$entry" in
            "\$ORIGIN/lib"|"\${ORIGIN}/lib") found_origin=1 ;;
            /*) die "flow5 contains an absolute build-host runpath: $entry" ;;
        esac
    done < <(tr ':' '\n' <<<"$runpaths")

    (( found_origin == 1 )) || die \
        "flow5 RPATH/RUNPATH does not contain \$ORIGIN/lib"

    # DT_RPATH outranks LD_LIBRARY_PATH; DT_RUNPATH does not. The generated
    # AppRun clears the variable, so warn only when a custom AppRun was kept.
    if (( apprun_hardened == 0 )) &&
       ! readelf -d "$executable" 2>/dev/null | grep -q '(RPATH)'; then
        warn "flow5 uses DT_RUNPATH and AppRun does not clear LD_LIBRARY_PATH; a host XFLR5 installation can override \$ORIGIN/lib at run time"
    fi
}

validate_loader_closure() {
    local appdir="$1"
    local ldd_output=""
    local library=""
    local ldd_line=""
    local resolved=""

    if ! ldd_output="$(ldd -r "$appdir/flow5" 2>&1)"; then
        printf '%s\n' "$ldd_output" >&2
        die "ldd -r failed for the staged flow5 executable"
    fi
    if grep -Eq 'not found|undefined symbol' <<<"$ldd_output"; then
        printf '%s\n' "$ldd_output" >&2
        die "Staged AppDir has an unresolved library or symbol"
    fi
    for library in "${private_libraries[@]}"; do
        ldd_line="$(grep -E "^[[:space:]]*${library//./\\.}[[:space:]]+=>" \
            <<<"$ldd_output" | head -n 1 || true)"
        [[ -n "$ldd_line" ]] || die \
            "$library is not a declared dependency of staged flow5"
        resolved="${ldd_line#*=> }"
        resolved="${resolved%% (*}"
        case "$resolved" in
            "$appdir"/lib/*) ;;
            *) die "$library resolved outside the staged AppDir: $resolved" ;;
        esac
    done
    log "Staged AppDir dynamic-link validation: passed"
}

validate_staged_payload() {
    local appdir="$1"
    local link=""
    local link_target=""
    local library=""

    while IFS= read -r -d '' link; do
        link_target="$(realpath -e -- "$link")" || die \
            "Staged AppDir contains a broken symbolic link: ${link#"$appdir"/}"
        case "$link_target" in
            "$appdir"/*) ;;
            *) die \
                "Staged symbolic link resolves outside the AppDir: ${link#"$appdir"/} -> $link_target" ;;
        esac
    done < <(find "$appdir" -type l -print0)

    validate_runpath "$appdir/flow5"

    for library in "${private_libraries[@]}"; do
        [[ -e "$appdir/lib/$library" ]] || die \
            "Missing required private library: lib/$library"
    done

    [[ -e "$appdir/plugins/platforms/libqxcb.so" ]] || die \
        "Missing Qt XCB platform plugin: plugins/platforms/libqxcb.so"

    # Qt 6.5 and newer load libxcb-cursor at runtime, outside the loader graph.
    has_library "$appdir/lib" 'libxcb-cursor.so*' || warn \
        "libxcb-cursor.so.0 is not bundled; Qt 6.5+ XCB startup depends on the host providing it"

    if grep -Eq '/usr/local|/home/|/tmp/|/var/tmp/' \
        "$appdir/flow5.desktop"; then
        die "flow5.desktop contains an absolute host or build-directory path"
    fi
    grep -Eq '^Exec=flow5[[:space:]]*$' "$appdir/flow5.desktop" || die \
        "flow5.desktop must contain Exec=flow5"
    grep -Eq '^Icon=flow5[[:space:]]*$' "$appdir/flow5.desktop" || die \
        "flow5.desktop must contain Icon=flow5"

    if command -v desktop-file-validate >/dev/null 2>&1; then
        desktop-file-validate "$appdir/flow5.desktop" || die \
            "desktop-file-validate rejected flow5.desktop"
        log "Desktop entry validation: passed"
    else
        warn "desktop-file-validate is unavailable; skipped optional validation"
    fi

    validate_loader_closure "$appdir"
}

# Replace a symlink AppRun with a wrapper that prevents an XFLR5 installation
# on LD_LIBRARY_PATH from shadowing Flow5's bundled libXFoil.
install_apprun() {
    local appdir="$1"

    if [[ -f "$appdir/AppRun" && ! -L "$appdir/AppRun" ]]; then
        chmod 0755 "$appdir/AppRun"
        warn "AppRun is a program rather than the expected symlink to flow5; keeping it and skipping LD_LIBRARY_PATH hardening"
        return
    fi

    # Delete first: writing to the symlink would truncate flow5 through it.
    rm -f -- "$appdir/AppRun"
    cat >"$appdir/AppRun" <<'EOF'
#!/bin/sh
# Generated by packaging/appimage/finalize-appdir.sh.
HERE="$(dirname "$(readlink -f "$0")")"
unset LD_LIBRARY_PATH LD_PRELOAD
exec "$HERE/flow5" "$@"
EOF
    chmod 0755 "$appdir/AppRun"
    apprun_hardened=1
}

# Desktop integration tools read the menu entry and icon from usr/share.
install_desktop_integration() {
    local appdir="$1"
    local dimensions=""
    local icon_width=""
    local icon_height=""

    install -Dm 0644 "$appdir/flow5.desktop" \
        "$appdir/usr/share/applications/flow5.desktop"

    dimensions="$(file -Lb "$appdir/flow5.png" |
        sed -nE 's/.*, ([0-9]+) x ([0-9]+),.*/\1\t\2/p')"
    IFS=$'\t' read -r icon_width icon_height <<<"$dimensions" || true
    if [[ -z "$icon_width" || "$icon_width" != "$icon_height" ]]; then
        warn "flow5.png is not a square PNG of readable size; installed the menu entry without a hicolor icon"
        return
    fi

    install -Dm 0644 "$appdir/flow5.png" \
        "$appdir/usr/share/icons/hicolor/${icon_width}x${icon_height}/apps/flow5.png"
    log "Desktop integration: usr/share menu entry and ${icon_width}x${icon_height} icon"
}

if (( $# < 2 || $# > 3 )); then
    usage >&2
    exit 2
fi

for command_name in awk file find ldd readelf realpath sha256sum; do
    require_command "$command_name"
done

input_argument="$1"
version="$2"
output_argument="${3:-$repo_root/dist}"

[[ "$version" =~ ^[0-9A-Za-z][0-9A-Za-z._+-]*$ ]] || die \
    "VERSION contains unsupported filename characters: $version"
[[ -d "$input_argument" ]] || die "Input AppDir is not a directory: $input_argument"

input_dir="$(realpath -e -- "$input_argument")"
output_dir="$(realpath -m -- "$output_argument")"

case "$output_dir/" in
    "$input_dir/"*) die "Output directory must not be inside the input AppDir" ;;
esac

if [[ -e "$output_dir" && ! -d "$output_dir" ]]; then
    die "Output path exists but is not a directory: $output_dir"
fi
mkdir -p -- "$output_dir"
output_dir="$(realpath -e -- "$output_dir")"

appimagetool="$(resolve_appimagetool)"
if [[ -n "${APPIMAGETOOL_SHA256:-}" ]]; then
    [[ "$APPIMAGETOOL_SHA256" =~ ^[0-9A-Fa-f]{64}$ ]] || die \
        "APPIMAGETOOL_SHA256 must contain exactly 64 hexadecimal characters"
    actual_tool_sha256="$(sha256sum "$appimagetool" | awk '{print $1}')"
    [[ "${actual_tool_sha256,,}" == "${APPIMAGETOOL_SHA256,,}" ]] || die \
        "appimagetool SHA-256 mismatch: expected $APPIMAGETOOL_SHA256, got $actual_tool_sha256"
    log "appimagetool SHA-256 verification: passed"
else
    warn "APPIMAGETOOL_SHA256 is unset; ensure the pinned tool was verified before release use"
fi

validate_appdir_input "$input_dir"

work_dir="$(mktemp -d -t flow5-appimage.XXXXXXXX)"
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
trap 'exit 129' HUP

staged_appdir="$work_dir/Flow5.AppDir"
mkdir -p "$staged_appdir"
cp -a -- "$input_dir/." "$staged_appdir/"

install -m 0644 "$desktop_source" "$staged_appdir/flow5.desktop"
install -m 0644 "$repo_root/LICENSE" "$staged_appdir/LICENSE"
chmod 0755 "$staged_appdir/flow5"
install_apprun "$staged_appdir"
install_desktop_integration "$staged_appdir"
# squashfs preserves modes, so make the staged payload readable to end users.
chmod -R a+rX -- "$staged_appdir"

validate_staged_payload "$staged_appdir"

artifact_name="Flow5-${version}-x86_64.AppImage"
checksum_name="${artifact_name}.sha256"
built_artifact="$work_dir/$artifact_name"

log "Creating $artifact_name"
ARCH=x86_64 "$appimagetool" "$staged_appdir" "$built_artifact"
[[ -s "$built_artifact" ]] || die "appimagetool did not create a non-empty artifact"
chmod 0755 "$built_artifact"

if ! file -Lb "$built_artifact" | grep -Eq 'ELF 64-bit LSB.*x86-64'; then
    die "Generated artifact is not an x86-64 ELF AppImage"
fi

(
    cd -- "$work_dir"
    sha256sum "$artifact_name" > "$checksum_name"
)

install -m 0755 "$built_artifact" "$output_dir/$artifact_name"
install -m 0644 "$work_dir/$checksum_name" "$output_dir/$checksum_name"
(
    cd -- "$output_dir"
    sha256sum --check "$checksum_name" >/dev/null
) || die "Published checksum verification failed"

artifact_size="$(stat --format='%s bytes' "$output_dir/$artifact_name")"

log "Packaging complete"
printf '  AppImage:  %s (%s)\n' "$output_dir/$artifact_name" "$artifact_size"
printf '  Checksum:  %s\n' "$output_dir/$checksum_name"
