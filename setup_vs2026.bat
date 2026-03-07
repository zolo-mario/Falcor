: This script sets up a Visual Studio 2026 solution.

@echo off
setlocal

set PRESET=windows-vs2026
if "%~1"=="ci" set PRESET=windows-vs2026-ci

set CMAKE_EXE=%~dp0tools\cmake-4.2.3\bin\cmake.exe

: Fetch dependencies (includes cmake 4.2.3 via github-cmake remote).
call %~dp0\setup.bat
if errorlevel 1 exit /b 1

: Configure solution by running cmake.
echo Configuring Visual Studio 2026 solution ...
"%CMAKE_EXE%" --preset %PRESET% -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 (
    echo Failed to configure solution!
    exit /b 1
)

: Success.
exit /b 0
