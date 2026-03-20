<#
.SYNOPSIS
    Configure and build Falcor.

.PARAMETER Preset
    CMake configure preset. Allowed values:
      windows-vs2022 | windows-vs2022-ci
    Default: windows-vs2022

.PARAMETER Config
    Build configuration: Debug or Release.
    Default: Debug

.PARAMETER Target
    Optional build target (e.g. Karma, Niagara, GBuffer, FalcorTest).
    Omit to build everything.

.EXAMPLE
    # Build all (VS2022, Debug)
    .\build.ps1

    # Build a specific target
    .\build.ps1 -Target Karma

    # Release build
    .\build.ps1 -Config Release

    # Pass extra cmake --build flags directly
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

# ---------------------------------------------------------------------------
# Configure (silent; print only errors)
# ---------------------------------------------------------------------------
$cfgOut = @(& $CmakeExe --preset $Preset "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" 2>&1)
if ($LASTEXITCODE -ne 0) {
    $cfgOut | Where-Object { $_ -match "error|failed|Error C|fatal|CMake Error" } |
        ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host
    Pop-Location; exit $LASTEXITCODE
}

# ---------------------------------------------------------------------------
# Build (silent; print only errors)
# ---------------------------------------------------------------------------
$buildArgs = @("--build", "build/$Preset", "--config", $Config)
if ($Target) { $buildArgs += @("--target", $Target) }
$buildArgs += $ExtraArgs

$buildOut = @(& $CmakeExe $buildArgs 2>&1)
$code = $LASTEXITCODE
$buildOut | Where-Object { $_ -match "error|failed|Error C|fatal" } |
    ForEach-Object { $_.Trim() } | Where-Object { $_ } | Write-Host

Pop-Location
exit $code
