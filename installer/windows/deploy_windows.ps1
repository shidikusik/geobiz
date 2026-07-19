# SPDX-License-Identifier: MIT
#
# deploy_windows.ps1 — stage the Qt runtime next to the built executable and
# produce the installer + portable ZIP.
#
# Prerequisites: a Qt 6 (MSVC) kit with WebEngine, CMake, NSIS, and windeployqt
# on PATH. Run from the repo root, e.g.:
#
#   pwsh installer/windows/deploy_windows.ps1 -BuildDir build -Config Release
#
param(
    [string]$BuildDir = "build",
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

# 1) Configure + build.
cmake -S . -B $BuildDir -G "Ninja" -DCMAKE_BUILD_TYPE=$Config
cmake --build $BuildDir --config $Config

# 2) Locate the freshly built executable.
$exe = Join-Path $BuildDir "GeoBizUzbekistan.exe"
if (-not (Test-Path $exe)) {
    $exe = Get-ChildItem -Path $BuildDir -Recurse -Filter "GeoBizUzbekistan.exe" |
        Select-Object -First 1 -ExpandProperty FullName
}
if (-not $exe) { throw "GeoBizUzbekistan.exe not found under $BuildDir" }

# 3) Deploy the Qt runtime (DLLs, QML, WebEngine process, translations).
windeployqt --qmldir qml --release --compiler-runtime $exe

# 4) Build the NSIS installer and portable ZIP via CPack.
Push-Location $BuildDir
cpack -G "NSIS;ZIP"
Pop-Location

Write-Host "Done. Installer and ZIP are in $BuildDir." -ForegroundColor Green
