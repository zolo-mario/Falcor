<#
.SYNOPSIS
    Configure and build Falcor (Visual Studio 2022 presets only).

.PARAMETER Preset
    CMake configure preset: windows-vs2022 | windows-vs2022-ci
    Default: windows-vs2022

.PARAMETER Config
    Build configuration: Debug or Release.
    Default: Debug

.PARAMETER Target
    Optional build target (e.g. Karma, Niagara, GBuffer, FalcorTest).
    Omit to build everything.

.EXAMPLE
    .\build.ps1
    .\build.ps1 -Target Karma
    .\build.ps1 -Preset windows-vs2022-ci -Target Niagara
    .\build.ps1 -Config Release
    .\build.ps1 -Target FalcorTest -- --parallel 8
#>

param(
    [ValidateSet("windows-vs2022", "windows-vs2022-ci")]
    [string]$Preset = "windows-vs2022",

    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [string]$Target = "",

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ScriptDir = $PSScriptRoot
$CmakeExe = Join-Path $ScriptDir "tools\.packman\cmake\bin\cmake.exe"

Push-Location $ScriptDir

$cfgOut = @(& $CmakeExe --preset $Preset "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" 2>&1)
if ($LASTEXITCODE -ne 0) {
    $cfgOut | Where-Object { $_ -match "error|failed|Error C|fatal|CMake Error" } |
        ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host
    Pop-Location; exit $LASTEXITCODE
}

$buildArgs = @("--build", "build/$Preset", "--config", $Config)
if ($Target) { $buildArgs += @("--target", $Target) }
$buildArgs += $ExtraArgs

$buildOut = @(& $CmakeExe $buildArgs 2>&1)
$code = $LASTEXITCODE
$buildOut | Where-Object { $_ -match "error|failed|Error C|fatal" } |
    ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host

Pop-Location
exit $code
