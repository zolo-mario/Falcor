<#
.SYNOPSIS
    Fetch Falcor dependencies and optionally configure a CMake solution.

.PARAMETER Command
    Optional first argument:
      (omit) — full setup with preset windows-vs2022
      ci     — preset windows-vs2022-ci
      deps   — submodules + packman + .vscode only (no CMake)

.PARAMETER Preset
    CMake configure preset (default: windows-vs2022). Ignored when -DepsOnly is set.
    Overridden by positional argument 'ci'.

.PARAMETER DepsOnly
    Only update git submodules, pull packman dependencies, and copy VS Code defaults.
    Same as: .\setup.ps1 deps

.EXAMPLE
    .\setup.ps1
    .\setup.ps1 ci
    .\setup.ps1 deps
    .\setup.ps1 -Preset windows-vs2022-ci
    .\setup.ps1 -DepsOnly
#>

param(
    [Parameter(Position = 0)]
    [string]$Command,

    [ValidateSet("windows-vs2022", "windows-vs2022-ci")]
    [string]$Preset = "windows-vs2022",

    [switch]$DepsOnly
)

$ErrorActionPreference = "Stop"

if (-not [string]::IsNullOrWhiteSpace($Command)) {
    switch ($Command) {
        'deps' { $DepsOnly = $true }
        'ci' { $Preset = "windows-vs2022-ci" }
        default {
            Write-Error "Unknown argument: '$Command'. Use 'ci', 'deps', or omit."
            exit 1
        }
    }
}

$Root = $PSScriptRoot
$CmakeExe = Join-Path $Root "tools\.packman\cmake\bin\cmake.exe"

Write-Host "Updating git submodules ..."
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "git not found on PATH. Initialize submodules manually and re-run."
    exit 1
}
& git -C $Root submodule sync --recursive
& git -C $Root submodule update --init --recursive
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Fetching dependencies ..."
$Packman = Join-Path $Root "tools\packman\packman.cmd"
& cmd /c $Packman pull --platform windows-x86_64 "$Root\dependencies.xml"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to fetch packman dependencies!"
    exit $LASTEXITCODE
}

$VscodePath = Join-Path $Root ".vscode"
if (-not (Test-Path $VscodePath)) {
    Write-Host "Setting up VS Code workspace ..."
    New-Item -ItemType Directory -Path $VscodePath | Out-Null
    Copy-Item -Path "$Root\.vscode-default\*" -Destination $VscodePath -Recurse -Force
}

if ($DepsOnly) {
    Write-Host "Setup complete (dependencies only)."
    exit 0
}

Write-Host "Configuring solution (preset: $Preset) ..."
& $CmakeExe --preset $Preset "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to configure solution!"
    exit $LASTEXITCODE
}

Write-Host "Setup complete."
