# Thin wrapper - delegates to build.ps1 -Preset windows-vs2022
# Usage: .\build_vs2022.ps1 [-Config Debug|Release] [-Target <name>]

param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ForwardArgs = @()
if ($ExtraArgs) {
    $ForwardArgs = @($ExtraArgs | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
}

& "$PSScriptRoot\build.ps1" -Preset windows-vs2022 @ForwardArgs
exit $LASTEXITCODE
