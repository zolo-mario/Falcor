@echo off
setlocal
set CMAKE_EXE=%~dp0tools\.packman\cmake\bin\cmake.exe
%CMAKE_EXE% --preset windows-vs2022 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 --log-level=WARNING
%CMAKE_EXE% --build build/windows-vs2022 --config Debug %*
