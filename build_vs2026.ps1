# Thin wrapper - delegates to build.ps1 -Preset windows-vs2026
# Usage: .\build_vs2026.ps1 [-Config Debug|Release] [-Target <name>]

param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ForwardArgs = @()
if ($ExtraArgs) {
    $ForwardArgs = @($ExtraArgs | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
}

& "$PSScriptRoot\build.ps1" -Preset windows-vs2026 @ForwardArgs
exit $LASTEXITCODE
