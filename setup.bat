:: Thin wrapper for cmd / legacy scripts. Prefer .\setup.ps1 in PowerShell.
@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0setup.ps1" %*
exit /b %errorlevel%
