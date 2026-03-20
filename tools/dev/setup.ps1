<#
.SYNOPSIS
    Fetch Falcor dependencies and configure a CMake solution.

.PARAMETER Preset
    CMake configure preset. Allowed values:
      windows-vs2022 | windows-vs2022-ci
    Default: windows-vs2022

.EXAMPLE
    # First-time setup (default)
    .\tools\dev\setup.ps1

    # CI mode (enables header validation, disables PCH)
    .\tools\dev\setup.ps1 -Preset windows-vs2022-ci
#>

param(
    [ValidateSet("windows-vs2022", "windows-vs2022-ci")]
    [string]$Preset = "windows-vs2022"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path "$PSScriptRoot\..\.."
$CmakeExe = Join-Path $Root "tools\.packman\cmake\bin\cmake.exe"

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
