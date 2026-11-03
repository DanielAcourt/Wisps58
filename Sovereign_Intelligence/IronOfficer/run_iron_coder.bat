@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: Iron Coder 1-Click Task Launcher
:: Runs autonomous multi-turn software engineering tasks locally on RTX 5090.

setlocal enabledelayedexpansion
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo ============================================================
echo   IRON CODER: Autonomous Local Engineering Task Driver
echo   Hardware Target: RTX 5090 (32GB VRAM Local Inference)
echo ============================================================

:: 1. Python Resolution Logic
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :FOUND)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :FOUND)
python3 --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python3" & goto :FOUND)

for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%D\python.exe" (set "PY_CMD="%%D\python.exe"" & goto :FOUND)
)

echo [ERROR] Python not found. Please install Python 3.10+.
pause
exit /b

:FOUND
:: 2. Check if bridge is listening on port 8000
netstat -ano | findstr :8000 | findstr LISTENING >nul
if %errorlevel% NEQ 0 (
    echo [07 WARNING] Iron Officer Bridge is not listening on port 8000.
    echo [07] Starting bridge in background...
    start "Iron Officer Bridge" run_bridge.bat
    timeout /t 3 /nobreak >nul
)

:: 3. Prompt Intake
set "TASK_PROMPT=%~1"
if "%TASK_PROMPT%"=="" (
    echo.
    set /p "TASK_PROMPT=Enter your coding task or directive: "
)

if "%TASK_PROMPT%"=="" (
    echo [ERROR] No task provided. Exiting.
    pause
    exit /b
)

echo.
echo [07] Launching Iron Coder multi-turn execution loop...
echo [07] Task: "%TASK_PROMPT%"
echo ------------------------------------------------------------
%PY_CMD% iron_coder.py --prompt "%TASK_PROMPT%" --turns 10

echo.
echo ============================================================
echo   Task Execution Completed.
echo ============================================================
pause
