# Thin wrapper - delegates to build.ps1 -Preset windows-vs2026
# Usage: .\build_vs2026.ps1 [-Config Debug|Release] [-Target <name>]

param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

& "$PSScriptRoot\build.ps1" -Preset windows-vs2026 @ExtraArgs
exit $LASTEXITCODE
