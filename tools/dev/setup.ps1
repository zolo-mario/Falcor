<#
.SYNOPSIS
    Fetch Falcor dependencies and configure a CMake solution.

.PARAMETER Preset
    CMake configure preset. Allowed values:
      windows-vs2022 | windows-vs2022-ci | windows-vs2026 | windows-vs2026-ci
    Default: windows-vs2026

.EXAMPLE
    # First-time setup for VS2026 (default)
    .\tools\dev\setup.ps1

    # Explicit VS2022
    .\tools\dev\setup.ps1 -Preset windows-vs2022

    # CI mode (enables header validation, disables PCH)
    .\tools\dev\setup.ps1 -Preset windows-vs2026-ci
#>

param(
    [ValidateSet("windows-vs2022", "windows-vs2022-ci", "windows-vs2026", "windows-vs2026-ci")]
    [string]$Preset = "windows-vs2026"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path "$PSScriptRoot\..\.."

# Pick cmake based on preset family
if ($Preset -like "*vs2026*") {
    $CmakeExe = Join-Path $Root "tools\cmake-4.2.3\bin\cmake.exe"
} else {
    $CmakeExe = Join-Path $Root "tools\.packman\cmake\bin\cmake.exe"
}

# ---------------------------------------------------------------------------
# 1. Git submodules
# ---------------------------------------------------------------------------
Write-Host "Updating git submodules ..."
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "git not found on PATH. Initialize submodules manually and re-run."
    exit 1
}
& git -C $Root submodule sync --recursive
& git -C $Root submodule update --init --recursive
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ---------------------------------------------------------------------------
# 2. Packman dependencies
# ---------------------------------------------------------------------------
Write-Host "Fetching dependencies ..."
$Packman = Join-Path $Root "tools\packman\packman.cmd"
& cmd /c $Packman pull --platform windows-x86_64 "$Root\dependencies.xml"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to fetch packman dependencies!"
    exit $LASTEXITCODE
}

# ---------------------------------------------------------------------------
# 3. VS Code workspace (first-time only)
# ---------------------------------------------------------------------------
$VscodePath = Join-Path $Root ".vscode"
if (-not (Test-Path $VscodePath)) {
    Write-Host "Setting up VS Code workspace ..."
    New-Item -ItemType Directory -Path $VscodePath | Out-Null
    Copy-Item -Path "$Root\.vscode-default\*" -Destination $VscodePath -Recurse -Force
}

# ---------------------------------------------------------------------------
# 4. CMake configure
# ---------------------------------------------------------------------------
Write-Host "Configuring solution (preset: $Preset) ..."
& $CmakeExe --preset $Preset "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to configure solution!"
    exit $LASTEXITCODE
}

Write-Host "Setup complete."
