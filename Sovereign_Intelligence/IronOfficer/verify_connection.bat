@echo off
:: Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
:: 07 Architect: Iron Officer Connection Tester (Self-Healing)
:: Purpose: Verifies the 'Social Pillar' (Connectivity) between the Vessel and the Bridge.

echo [07] Initializing Connection Test...
echo [07] Path: %~dp0

set "BASE_DIR=%~dp0"
cd /d "%BASE_DIR%"

:: 1. Python Discovery
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :FOUND)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :FOUND)

if "%PY_CMD%"=="" (
    echo [ERROR] Python not found.
    pause
    exit /b
)

:FOUND
:: 2. Ensure testing modules are present
%PY_CMD% -c "import requests" >nul 2>&1
if %errorlevel% NEQ 0 (
    echo [07] Missing 'requests' module. Installing...
    %PY_CMD% -m pip install requests
)

:: 3. Run the Social Sync Test
%PY_CMD% test_connection.py

pause
