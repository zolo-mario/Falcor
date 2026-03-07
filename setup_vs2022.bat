:: Thin wrapper - delegates to tools/dev/setup.ps1
:: Usage: setup_vs2022.bat [ci]

@echo off
set PRESET=windows-vs2022
if "%~1"=="ci" set PRESET=windows-vs2022-ci

powershell -ExecutionPolicy Bypass -File "%~dp0tools\dev\setup.ps1" -Preset %PRESET%
exit /b %errorlevel%
