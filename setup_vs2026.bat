:: Thin wrapper - delegates to tools/dev/setup.ps1
:: Usage: setup_vs2026.bat [ci]

@echo off
set PRESET=windows-vs2026
if "%~1"=="ci" set PRESET=windows-vs2026-ci

powershell -ExecutionPolicy Bypass -File "%~dp0tools\dev\setup.ps1" -Preset %PRESET%
exit /b %errorlevel%
