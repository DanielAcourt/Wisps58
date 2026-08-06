@echo off
:: Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
:: 07 Architect: Iron Officer One-Click Launcher (Self-Healing)
:: Purpose: Initializes the FastAPI Bridge, validates environment, and resolves local Nexus paths.

echo [07] Initializing Iron Officer Bridge...
echo [07] Hardware Target: GTX 5090

:: Get the directory where this batch file is located
set "IRON_OFFICER_DIR=%~dp0"
cd /d "%IRON_OFFICER_DIR%"

:: 1. Calculate the Nexus Path (Up two levels from IronOfficer/ to repo root)
set "REPO_ROOT=%~dp0..\.."
pushd "%REPO_ROOT%"
set "NEXUS_PATH=%cd%\AI_Nexus"
popd

:: 2. Python Resolution Logic (Priority: python -> py -> python3)
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :FOUND)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :FOUND)
python3 --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python3" & goto :FOUND)

:: Fallback: Search common Windows installation paths
for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%D\python.exe" (set "PY_CMD="%%D\python.exe"" & goto :FOUND)
)

echo [ERROR] Python not found. Please install Python 3.10+.
pause
exit /b

:FOUND
:: 3. Dependency Check & Sync
%PY_CMD% -c "import requests, fastapi, uvicorn" >nul 2>&1
if %errorlevel% NEQ 0 (
    echo [07] Missing dependencies detected. Installing now...
    %PY_CMD% -m pip install -r requirements.txt
)

:: 4. Port 8000 Conflict Resolution
netstat -ano | findstr :8000 | findstr LISTENING >nul
if %errorlevel% EQU 0 (
    echo [07 WARNING] Port 8000 is already in use.
    for /f "tokens=5" %%a in ('netstat -aon ^| findstr :8000 ^| findstr LISTENING') do set "PID=%%a"
    echo [07] Blocker PID: %PID%
    set /p choice="[07] Would you like to kill the blocking process? (y/n): "
    if /i "%choice%"=="y" (
        taskkill /F /PID %PID%
        echo [07] Process terminated. Continuing...
    )
)

:: 5. Ollama Persistence Check
set "OLLAMA_RUNNING=0"
tasklist /FI "IMAGENAME eq ollama.exe" 2>NUL | find /I /N "ollama.exe">NUL
if "%ERRORLEVEL%" EQU "0" set "OLLAMA_RUNNING=1"
if "%OLLAMA_RUNNING%" NEQ "1" (
    echo [07 CRITICAL] Ollama is not running. Please start it before proceeding.
    pause
)

:: 6. Launch the Sovereign Bridge
echo [07] Starting FastAPI Service...
%PY_CMD% bridge.py --nexus "%NEXUS_PATH%"

pause
