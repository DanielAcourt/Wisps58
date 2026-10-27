@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Headless API Launcher
:: Server Endpoint: http://127.0.0.1:8188

title [07] ComfyUI Headless API Server (GTX 5090)

echo =======================================================================
echo [07] Iron Officer: Launching Standalone Headless ComfyUI API Server
echo [07] Listening on: http://127.0.0.1:8188
echo =======================================================================

:: 1. Resolve Target ComfyUI Directory
set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\..\..\"

set "COMFY_DIR="
if exist "%REPO_ROOT%\comfy\ComfyUI-master\main.py" (
    set "COMFY_DIR=%REPO_ROOT%\comfy\ComfyUI-master"
) else if exist "C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\main.py" (
    set "COMFY_DIR=C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master"
) else (
    echo [07 ERROR] ComfyUI directory not found!
    pause
    exit /b 1
)

cd /d "%COMFY_DIR%"

:: 2. Activate Dedicated Virtual Environment
if exist "venv\Scripts\activate.bat" (
    call venv\Scripts\activate.bat
) else (
    echo [07 WARNING] Virtual environment (venv) not found. Attempting global python execution...
    echo Tip: Run 'setup_comfy_standalone.bat' first to initialize the environment.
)

:: 3. Launch ComfyUI Server
echo [07] Starting ComfyUI backend server...
python main.py --listen 127.0.0.1 --port 8188 --highvram

if %errorlevel% NEQ 0 (
    echo.
    echo [07 ERROR] ComfyUI server exited with error code %errorlevel%.
)

pause
