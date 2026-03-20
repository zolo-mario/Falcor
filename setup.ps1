<#
.SYNOPSIS
    Root entry: clone/setup Falcor (delegates to tools/dev/setup.ps1).

.PARAMETER Command
    Optional first argument:
      (omit) — full setup with preset windows-vs2022
      ci     — preset windows-vs2022-ci
      deps   — submodules + packman + .vscode only (no CMake)

.EXAMPLE
    .\setup.ps1
    .\setup.ps1 ci
    .\setup.ps1 deps
#>

param(
    [Parameter(Position = 0)]
    [string]$Command
)

$ErrorActionPreference = "Stop"
$DevSetup = Join-Path $PSScriptRoot "tools\dev\setup.ps1"

if (-not (Test-Path $DevSetup)) {
    Write-Error "Missing $DevSetup"
    exit 1
}

if ([string]::IsNullOrWhiteSpace($Command)) {
    & $DevSetup
    exit $LASTEXITCODE
}

switch ($Command) {
    'deps' { & $DevSetup -DepsOnly }
    'ci' { & $DevSetup -Preset windows-vs2022-ci }
    default {
        Write-Error "Unknown argument: '$Command'. Use 'ci', 'deps', or omit."
        exit 1
    }
}
exit $LASTEXITCODE
