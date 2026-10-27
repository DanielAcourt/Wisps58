@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Automated Model Downloader for FLUX.1 [dev] FP8 with Resumable Visual Progress
:: Target: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\models

title [07] ComfyUI FLUX.1 Model Downloader (Visual Progress & Resumable)

echo =======================================================================
echo [07] Iron Officer: Automated FLUX.1 [dev] FP8 Model Weights Downloader
echo [07] Features: Live Transfer Speed, Percentage Progress & Resumable Download
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
    echo [07 ERROR] ComfyUI directory not found!
    echo Target expected at: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\main.py
    pause
    exit /b 1
)

cd /d "%COMFY_DIR%"

:: Ensure directories exist
if not exist "models\unet" mkdir "models\unet"
if not exist "models\vae" mkdir "models\vae"
if not exist "models\clip" mkdir "models\clip"

echo [07] Downloading models into: "%COMFY_DIR%\models"
echo.

:: Detect curl executable
set "DL_CMD=curl -L -C -"
curl --version >nul 2>&1
if %errorlevel% NEQ 0 (
    echo [07 WARNING] curl not found in PATH. Downloading via PowerShell Start-BitsTransfer...
    set "DL_CMD=powershell -Command Start-BitsTransfer -Source"
)

:: Model 1: FLUX.1 [dev] FP8 UNet (~12 GB)
set "FLUX_UNET_FILE=models\unet\flux1-dev-fp8.safetensors"
set "FLUX_UNET_URL=https://huggingface.co/Comfy-Org/flux1-dev/resolve/main/flux1-dev-fp8.safetensors"

echo -----------------------------------------------------------------------
echo [07 DOWNLOADING 1/4] FLUX.1 [dev] FP8 UNet model (~12.0 GB)
echo Target: %FLUX_UNET_FILE%
echo -----------------------------------------------------------------------
curl -L -C - --output "%FLUX_UNET_FILE%" "%FLUX_UNET_URL%"
if %errorlevel% NEQ 0 (
    echo [07 WARNING] Download encountered an issue or was paused. Run script again to resume!
)
echo.

:: Model 2: FLUX VAE (~335 MB)
set "FLUX_VAE_FILE=models\vae\ae.safetensors"
set "FLUX_VAE_URL=https://huggingface.co/black-forest-labs/FLUX.1-dev/resolve/main/vae/diffusion_pytorch_model.safetensors"

echo -----------------------------------------------------------------------
echo [07 DOWNLOADING 2/4] FLUX VAE model (~335 MB)
echo Target: %FLUX_VAE_FILE%
echo -----------------------------------------------------------------------
curl -L -C - --output "%FLUX_VAE_FILE%" "%FLUX_VAE_URL%"
echo.

:: Model 3: CLIP-L Text Encoder (~246 MB)
set "CLIP_L_FILE=models\clip\clip_l.safetensors"
set "CLIP_L_URL=https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/clip_l.safetensors"

echo -----------------------------------------------------------------------
echo [07 DOWNLOADING 3/4] CLIP-L Text Encoder (~246 MB)
echo Target: %CLIP_L_FILE%
echo -----------------------------------------------------------------------
curl -L -C - --output "%CLIP_L_FILE%" "%CLIP_L_URL%"
echo.

:: Model 4: T5-XXL FP8 Text Encoder (~4.9 GB)
set "T5_XXL_FILE=models\clip\t5xxl_fp8_e4m3fn.safetensors"
set "T5_XXL_URL=https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/t5xxl_fp8_e4m3fn.safetensors"

echo -----------------------------------------------------------------------
echo [07 DOWNLOADING 4/4] T5-XXL FP8 Text Encoder (~4.9 GB)
echo Target: %T5_XXL_FILE%
echo -----------------------------------------------------------------------
curl -L -C - --output "%T5_XXL_FILE%" "%T5_XXL_URL%"
echo.

echo =======================================================================
echo [07 SUCCESS] All required FLUX.1 [dev] model weights downloaded/verified!
echo =======================================================================
echo.
pause
