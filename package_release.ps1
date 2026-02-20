$ErrorActionPreference = "Stop"

# Paths
$projectRoot = "D:\flow5\flow5"
$releaseDir = "$projectRoot\release"
$appDir = "$projectRoot\build-qt2-mingw\fl5-app"
$qtBin = "D:\Qt2\6.10.1\mingw_64\bin"
$mingwBin = "D:\Qt2\Tools\mingw1310_64\bin"
$occtBin = "D:\flow5\OCCT\build-mingw-dll\win64\gcc\bin"
$openblasBin = "D:\flow5\OpenBLAS\build\lib"
$xfoilDir = "$projectRoot\build-qt2-mingw\XFoil-lib"
$libDir = "$projectRoot\build-qt2-mingw\fl5-lib"

Write-Host "Creating release directory at $releaseDir"
if (Test-Path $releaseDir) { Remove-Item -Recurse -Force $releaseDir }
New-Item -ItemType Directory -Path $releaseDir | Out-Null

# ---------------------------------------------------------
# 1. Copy Application Binaries
# ---------------------------------------------------------
Write-Host "Copying core application files..."
if (-not (Test-Path "$appDir\flow5.exe")) { Write-Error "flow5.exe not found!" }
Copy-Item "$appDir\flow5.exe" -Destination $releaseDir

if (-not (Test-Path "$libDir\fl5-lib.dll")) { Write-Error "fl5-lib.dll not found!" }
Copy-Item "$libDir\fl5-lib.dll" -Destination $releaseDir

if (-not (Test-Path "$xfoilDir\XFoil1.dll")) { Write-Error "XFoil1.dll not found!" }
Copy-Item "$xfoilDir\XFoil1.dll" -Destination $releaseDir

# ---------------------------------------------------------
# 2. Copy OpenBLAS
# ---------------------------------------------------------
Write-Host "Copying OpenBLAS..."
if (-not (Test-Path "$openblasBin\libopenblas.dll")) { Write-Error "libopenblas.dll not found!" }
Copy-Item "$openblasBin\libopenblas.dll" -Destination $releaseDir

# ---------------------------------------------------------
# 3. Copy OCCT DLLs
# ---------------------------------------------------------
Write-Host "Copying OpenCASCADE libraries..."
$occtLibs = Get-ChildItem -Path "$occtBin\libTK*.dll"
if ($occtLibs.Count -eq 0) { Write-Error "No OCCT DLLs found in $occtBin" }
$occtLibs | Copy-Item -Destination $releaseDir

# ---------------------------------------------------------
# 4. Copy MinGW specific DLLs
# ---------------------------------------------------------
Write-Host "Copying MinGW runtime libraries..."
# Fortran and Quadmath are often missed by windeployqt
Copy-Item "$mingwBin\libgfortran-5.dll" -Destination $releaseDir
Copy-Item "$mingwBin\libquadmath-0.dll" -Destination $releaseDir

# ---------------------------------------------------------
# 5. Run windeployqt
# ---------------------------------------------------------
Write-Host "Running windeployqt..."
# Add MinGW bin to PATH temporarily for windeployqt to find libraries
$env:PATH = "$mingwBin;$env:PATH"
& "$qtBin\windeployqt.exe" --release --compiler-runtime --force --no-translations "$releaseDir\flow5.exe"

# ---------------------------------------------------------
# 6. Copy App Resources
# ---------------------------------------------------------
Write-Host "Copying resources..."

# Translations
$transDir = "$releaseDir\translations"
New-Item -ItemType Directory -Path $transDir | Out-Null
if (Test-Path "$projectRoot\fl5-app\translations\*.qm") {
    Get-ChildItem "$projectRoot\fl5-app\translations\*.qm" | Copy-Item -Destination $transDir
}

# Documentation/Meta
$docDir = "$releaseDir\doc"
New-Item -ItemType Directory -Path $docDir | Out-Null
$imgDir = "$docDir\images"
New-Item -ItemType Directory -Path $imgDir | Out-Null

Copy-Item "$projectRoot\meta\doc\releasenotes.html" -Destination $docDir
Copy-Item "$projectRoot\meta\doc\style.css" -Destination $docDir
Copy-Item "$projectRoot\meta\doc\images\flow5.png" -Destination $imgDir

Write-Host "Release Content Prepared."
Write-Host "Creating Zip archive..."

Compress-Archive -Path "$releaseDir\*" -DestinationPath "$projectRoot\flow5_release.zip" -Force

Write-Host "---------------------------------------------------"
Write-Host "DEPLOYMENT COMPLETE"
Write-Host "Folder: $releaseDir"
Write-Host "Archive: $projectRoot\flow5_release.zip"
Write-Host "---------------------------------------------------"
