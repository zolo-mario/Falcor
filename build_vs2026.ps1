# Usage: .\build_vs2026.ps1 [--target <name>]

param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ScriptDir = $PSScriptRoot
$CmakeExe = Join-Path $ScriptDir "tools\cmake-4.2.3\bin\cmake.exe"

Push-Location $ScriptDir

# Configure - capture and only show errors
$cfgOut = @(& $CmakeExe --preset windows-vs2026 "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" 2>&1)
if ($LASTEXITCODE -ne 0) {
    $cfgOut | ForEach-Object { "$_" } | Where-Object { $_ -match "error|failed|Error C|fatal|CMake Error" } | ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host
    Pop-Location; exit $LASTEXITCODE
}

# Build - capture and only show errors
$buildArgs = @("--build", "build/windows-vs2026", "--config", "Debug") + $ExtraArgs
$buildOut = @(& $CmakeExe $buildArgs 2>&1)
$code = $LASTEXITCODE
$buildOut | ForEach-Object { "$_" } | Where-Object { $_ -match "error|failed|Error C|fatal" } | ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host
Pop-Location
exit $code
