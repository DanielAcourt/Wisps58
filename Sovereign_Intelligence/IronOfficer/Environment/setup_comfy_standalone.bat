@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Environment Setup Script
:: Target: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master

title [07] ComfyUI Standalone Setup

echo =======================================================================
echo [07] Iron Officer: Setting up Standalone ComfyUI Environment
echo =======================================================================

:: 1. Resolve Repo Root & ComfyUI Target Directory
set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\..\..\"

:: Check nested path first, then single level, then explicit fallback
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
    echo [07 ERROR] ComfyUI directory not found!
    echo Target expected at: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\main.py
    pause
    exit /b 1
)

echo [07] Found ComfyUI at: "%COMFY_DIR%"
cd /d "%COMFY_DIR%"

:: 2. Resolve Python Launcher Command (Priority: python -> py -> python3)
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :FOUND_PY)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :FOUND_PY)
python3 --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python3" & goto :FOUND_PY)

for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%D\python.exe" (set "PY_CMD="%%D\python.exe"" & goto :FOUND_PY)
)

echo [07 ERROR] Python launcher not found. Please install Python 3.11+.
pause
exit /b 1

:FOUND_PY
echo [07] Using Python command: %PY_CMD%

:: 3. Create Dedicated Virtual Environment
if not exist "venv\Scripts\activate.bat" (
    echo [07] Creating dedicated Python Virtual Environment (venv)...
    %PY_CMD% -m venv venv
    if %errorlevel% NEQ 0 (
        echo [07 ERROR] Failed to create virtual environment.
        pause
        exit /b 1
    )
) else (
    echo [07] Existing venv detected.
)

:: 4. Activate Virtual Environment
call venv\Scripts\activate.bat

:: 5. Upgrade pip & Install Core PyTorch with CUDA 12.4
echo [07] Upgrading pip, setuptools, wheel...
python -m pip install --upgrade pip setuptools wheel

echo [07] Installing PyTorch 2.4+ with CUDA 12.4 acceleration...
python -m pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu124

echo [07] Installing ComfyUI core dependencies...
if exist "requirements.txt" (
    python -m pip install -r requirements.txt
) else (
    echo [07 WARNING] requirements.txt not found in %COMFY_DIR%. Installing standard packages...
    python -m pip install pillow pyyaml safetensors scipy tqdm psutil requests
)

:: 6. Ensure Required Model Subdirectories Exist
echo [07] Creating target model subdirectories...
if not exist "models\unet" mkdir "models\unet"
if not exist "models\vae" mkdir "models\vae"
if not exist "models\clip" mkdir "models\clip"
if not exist "models\checkpoints" mkdir "models\checkpoints"
if not exist "models\loras" mkdir "models\loras"
if not exist "models\controlnet" mkdir "models\controlnet"
if not exist "output" mkdir "output"
if not exist "input" mkdir "input"

echo.
echo =======================================================================
echo [07 SUCCESS] Standalone ComfyUI Environment configured successfully!
echo =======================================================================
echo.
echo Next steps:
echo   1. Run 'download_flux_models.bat' to download FLUX.1 [dev] FP8 models.
echo   2. Run 'run_comfy_api.bat' to launch the headless API server on port 8188.
echo.
pause
