@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Diagnostic Verification Audit for Standalone ComfyUI Engine
:: Target: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master

title [07] ComfyUI Setup Diagnostic Audit

echo =======================================================================
echo [07] Iron Officer: Standalone ComfyUI Environment Diagnostic Audit
echo =======================================================================
echo.

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
)

:: AUDIT CHECK 1: COMFYUI DIRECTORY
echo [AUDIT 1/5] Checking ComfyUI Directory...
if defined COMFY_DIR (
    echo   [OK] Found ComfyUI at: "%COMFY_DIR%"
) else (
    echo   [FAILED] ComfyUI main.py not found!
    echo   Action: Download/extract ComfyUI repository to comfy\ComfyUI-master\ComfyUI-master\
    goto :AUDIT_SUMMARY
)
echo.

cd /d "%COMFY_DIR%"

:: AUDIT CHECK 2: PYTHON VIRTUAL ENVIRONMENT
echo [AUDIT 2/5] Checking Dedicated Virtual Environment (venv)...
if exist "venv\Scripts\activate.bat" (
    echo   [OK] Virtual Environment exists.
    call venv\Scripts\activate.bat
) else (
    echo   [FAILED] Virtual environment missing!
    echo   Action: Run 'setup_comfy_standalone.bat' to build the venv environment.
    goto :AUDIT_SUMMARY
)
echo.

:: AUDIT CHECK 3: PYTORCH & CUDA ACCELERATION
echo [AUDIT 3/5] Testing PyTorch and GPU Acceleration...
python -c "import torch; print(f'  [OK] PyTorch Version: {torch.__version__}'); print(f'  [OK] CUDA Available: {torch.cuda.is_available()}'); print(f'  [OK] GPU Device: {torch.cuda.get_device_name(0)}') if torch.cuda.is_available() else print('  [WARNING] CUDA GPU not detected by PyTorch!')" 2>nul
if %errorlevel% NEQ 0 (
    echo   [FAILED] PyTorch import failed inside venv!
    echo   Action: Run 'setup_comfy_standalone.bat' to install PyTorch with CUDA 12.4.
)
echo.

:: AUDIT CHECK 4: MODEL WEIGHTS
echo [AUDIT 4/5] Checking FLUX.1 Model Weights...
set "MISSING_MODELS=0"

if exist "models\unet\flux1-dev-fp8.safetensors" (
    echo   [OK] UNet Model: models\unet\flux1-dev-fp8.safetensors
) else (
    echo   [MISSING] UNet Model: models\unet\flux1-dev-fp8.safetensors
    set "MISSING_MODELS=1"
)

if exist "models\vae\ae.safetensors" (
    echo   [OK] VAE Model:  models\vae\ae.safetensors
) else (
    echo   [MISSING] VAE Model:  models\vae\ae.safetensors
    set "MISSING_MODELS=1"
)

if exist "models\clip\clip_l.safetensors" (
    echo   [OK] CLIP-L:     models\clip\clip_l.safetensors
) else (
    echo   [MISSING] CLIP-L:     models\clip\clip_l.safetensors
    set "MISSING_MODELS=1"
)

if exist "models\clip\t5xxl_fp8_e4m3fn.safetensors" (
    echo   [OK] T5-XXL:     models\clip\t5xxl_fp8_e4m3fn.safetensors
) else (
    echo   [MISSING] T5-XXL:     models\clip\t5xxl_fp8_e4m3fn.safetensors
    set "MISSING_MODELS=1"
)

if %MISSING_MODELS% EQU 1 (
    echo   Action: Run 'download_flux_models.bat' to fetch missing weights!
)
echo.

:: AUDIT CHECK 5: LIVE API SERVER PROBE
echo [AUDIT 5/5] Probing Live API Server on http://127.0.0.1:8188...
powershell -Command "$r = Invoke-WebRequest -Uri 'http://127.0.0.1:8188/system_stats' -UseBasicParsing -TimeoutSec 2 -ErrorAction SilentlyContinue; if ($r.StatusCode -eq 200) { Write-Host '  [OK] ComfyUI API Server is ONLINE and accepting connections!' } else { Write-Host '  [OFFLINE] API Server not responding. Run run_comfy_api.bat to start.' }"
echo.

:AUDIT_SUMMARY
echo =======================================================================
echo [07 SUMMARY & QUICK-START GUIDE]
echo   1. If venv or PyTorch failed -> Run 'setup_comfy_standalone.bat'
echo   2. If models are missing      -> Run 'download_flux_models.bat'
echo   3. If server is offline       -> Run 'run_comfy_api.bat'
echo =======================================================================
echo.
pause
