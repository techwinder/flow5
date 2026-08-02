# Flow5 AppImage packaging

This directory turns an existing Flow5 Linux release directory into one
versioned `x86_64` AppImage. It deliberately starts from the maintainer's
complete AppDir-style payload: it does not build Flow5, Qt, OpenCascade, Gmsh,
or the numerical runtime, and it does not assemble application dependencies.

The finalizer validates the supplied payload, stages a private copy, adds the
AppImage metadata, checks that the staged executable's loader dependencies are
closed, and publishes an AppImage with a SHA-256 checksum. The input directory
is never modified.

## Prerequisites

Run the finalizer on an `x86_64` Linux host with:

- Bash, GNU coreutils, findutils, awk, `file`, and binutils (`readelf`);
- glibc's `ldd`;
- a pinned `appimagetool` release;
- `desktop-file-validate` from desktop-file-utils (recommended); and
- writable `TMPDIR` space for roughly twice the AppDir size. The staged copy
  and artifact must not exhaust a small tmpfs-backed `/tmp`.

## Obtain and verify appimagetool

Download `appimagetool-x86_64.AppImage` from a numbered release on the
[official appimagetool release page](https://github.com/AppImage/appimagetool/releases).
Never use an unpinned continuous build for a release artifact. For example,
pin release `1.9.1` and verify the digest published for its x86-64 asset:

```bash
curl -fLO \
  https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
printf '%s  %s\n' \
  'ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0' \
  appimagetool-x86_64.AppImage | sha256sum --check
```

The finalizer can repeat that check before use:

```bash
export APPIMAGETOOL_SHA256='ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0'
```

The script never downloads `appimagetool`. If the packaging host cannot use
FUSE to run it, set `APPIMAGE_EXTRACT_AND_RUN=1`.

## Required input layout

The first argument must be the existing, complete Flow5 AppDir. At minimum it
must contain:

```text
AppRun                         link or executable resolving inside the AppDir
flow5                          x86-64 ELF executable
flow5.png                      root application icon
qt.conf                        Plugins points to plugins/
lib/                           bundled application and third-party libraries
plugins/platforms/libqxcb.so   bundled Qt XCB platform plugin
translations/                  compiled flow5_*.qm translations
```

The `flow5` executable must have `$ORIGIN/lib` in its RPATH or RUNPATH and no
absolute library directory. Private Flow5 libraries are auto-detected from the
executable's `DT_NEEDED` entries and must exist in `lib/`. The v7.56 executable
links `libXFoil.so.1` and `libflow5-lib.so.1`; the current source tree also
links `libflow5-io-lib.so.1`.

The input binary, not the packaging host, determines glibc and libstdc++
compatibility. Produce the payload on the oldest distribution the release
should support. Repackaging cannot lower an executable's ABI requirements.

## What the finalizer changes in its staged copy

- Replaces a symbolic-link `AppRun` with a wrapper that clears
  `LD_LIBRARY_PATH` and `LD_PRELOAD` before starting Flow5. With `DT_RUNPATH`,
  a host XFLR5 `libXFoil.so.1` on the loader path could otherwise silently
  shadow the bundled library. A custom AppRun program is preserved with a
  warning.
- Installs the desktop entry and icon under `usr/share` for AppImage desktop
  integration while keeping the required root metadata.
- Adds the repository `LICENSE`.
- Normalises staged payload modes with `chmod -R a+rX`.

The finalizer checks symbolic-link containment, the executable runpath,
desktop metadata, the Qt XCB platform plugin, and every auto-detected private
library. It then runs `ldd -r` on the staged executable and rejects missing
libraries, undefined symbols, or a private library that resolves outside the
staged `lib/` directory.

## Build the AppImage

From the repository root, run:

```bash
APPIMAGETOOL=/absolute/path/to/appimagetool-x86_64.AppImage \
APPIMAGETOOL_SHA256=ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0 \
  ./packaging/appimage/finalize-appdir.sh \
  /absolute/path/to/flow5_v7.56_linux \
  7.56
```

An optional third argument selects the output directory. The default is
`dist/` under the repository root:

```text
dist/Flow5-7.56-x86_64.AppImage
dist/Flow5-7.56-x86_64.AppImage.sha256
```

The checksum file contains the relative AppImage filename, so both files can
be moved together and verified from their new directory:

```bash
cd dist
sha256sum --check Flow5-7.56-x86_64.AppImage.sha256
```

## Troubleshooting

### FUSE is unavailable

Set `APPIMAGE_EXTRACT_AND_RUN=1` when invoking `appimagetool`. End users can
launch the finished artifact with `--appimage-extract-and-run`.

### Missing bundled library

Return to the dependency-deployment step and add the reported runtime to the
AppDir. Do not work around the error with a host `LD_LIBRARY_PATH`.

### Absolute runpath reported

Relink `flow5` with `$ORIGIN/lib` and remove host or build-tree library paths.
An absolute path is not portable and can select an incompatible system XFoil.

Before publishing, test the artifact on a clean installation of the oldest
supported distribution. Also review redistribution obligations for every
bundled third-party library; the finalizer cannot make that determination.
