@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Headless API Launcher
:: Server Endpoint: http://127.0.0.1:8188

title [07] ComfyUI Headless API Server (GTX 5090)

echo =======================================================================
echo [07] Iron Officer: Launching Standalone Headless ComfyUI API Server
echo =======================================================================

:: 1. Resolve Target ComfyUI Directory
set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\..\..\"

set "COMFY_DIR="
if exist "%REPO_ROOT%\comfy\ComfyUI-master\ComfyUI-master\main.py" (
    set "COMFY_DIR=%REPO_ROOT%\comfy\ComfyUI-master\ComfyUI-master"
) else if exist "C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\main.py" (
    set "COMFY_DIR=C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master"
) else if exist "%REPO_ROOT%\comfy\ComfyUI-master\main.py" (
    set "COMFY_DIR=%REPO_ROOT%\comfy\ComfyUI-master"
) else if exist "C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\main.py" (
    set "COMFY_DIR=C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master"
) else (
    echo [07 ERROR] ComfyUI directory or main.py not found!
    echo Looked in:
    echo  - %REPO_ROOT%\comfy\ComfyUI-master\ComfyUI-master\main.py
    echo  - C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\main.py
    echo.
    echo Please ensure ComfyUI is downloaded and extracted at that location.
    echo.
    pause
    exit /b 1
)

echo [07] Found ComfyUI directory: "%COMFY_DIR%"
cd /d "%COMFY_DIR%"

:: 2. Check and Activate Dedicated Virtual Environment
if exist "venv\Scripts\activate.bat" (
    echo [07] Activating dedicated Python virtual environment (venv)...
    call venv\Scripts\activate.bat
) else (
    echo [07 WARNING] Dedicated virtual environment (venv) not found in "%COMFY_DIR%".
    echo [07 ACTION REQUIRED] Please run 'setup_comfy_standalone.bat' first to initialize PyTorch and dependencies!
    echo.
    pause
    exit /b 1
)

:: 3. Verify PyTorch & CUDA availability
echo [07] Testing PyTorch and CUDA runtime environment...
python -c "import torch; print(f'[07] PyTorch Version: {torch.__version__} | CUDA Available: {torch.cuda.is_available()}')" 2>nul
if %errorlevel% NEQ 0 (
    echo [07 ERROR] PyTorch is not properly installed in the virtual environment.
    echo [07 ACTION REQUIRED] Run 'setup_comfy_standalone.bat' to install PyTorch with CUDA 12.4 support!
    echo.
    pause
    exit /b 1
)

:: 4. Launch ComfyUI Server
echo.
echo [07] Starting ComfyUI server listening on http://127.0.0.1:8188...
echo [07] Press Ctrl+C in this window to stop the server when finished.
echo =======================================================================
echo.

python main.py --listen 127.0.0.1 --port 8188 --highvram

if %errorlevel% NEQ 0 (
    echo.
    echo [07 ERROR] ComfyUI server exited with error code %errorlevel%.
)

echo.
echo [07] ComfyUI server process has ended.
pause
