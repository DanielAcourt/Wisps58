@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Automated Model Downloader for FLUX.1 [dev] FP8
:: Target: C:\Users\Danie\Unreal Projects\2027\repo\comfy\ComfyUI-master\ComfyUI-master\models

title [07] ComfyUI FLUX.1 Model Downloader

echo =======================================================================
echo [07] Iron Officer: Automated FLUX.1 [dev] FP8 Model Weights Downloader
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

:: Model 1: FLUX.1 [dev] FP8 UNet (~12 GB)
set "FLUX_UNET_FILE=models\unet\flux1-dev-fp8.safetensors"
set "FLUX_UNET_URL=https://huggingface.co/Comfy-Org/flux1-dev/resolve/main/flux1-dev-fp8.safetensors"

if exist "%FLUX_UNET_FILE%" (
    echo [07 SKIP] %FLUX_UNET_FILE% already exists.
) else (
    echo [07 DOWNLOADING] FLUX.1 [dev] FP8 UNet model...
    echo URL: %FLUX_UNET_URL%
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object System.Net.WebClient).DownloadFile('%FLUX_UNET_URL%', '%FLUX_UNET_FILE%')"
    if %errorlevel% NEQ 0 (
        echo [07 WARNING] Automatic download failed. You can manually download FLUX.1 dev FP8 from:
        echo %FLUX_UNET_URL%
        echo and save to: %COMFY_DIR%\%FLUX_UNET_FILE%
    )
)

echo.

:: Model 2: FLUX VAE (~335 MB)
set "FLUX_VAE_FILE=models\vae\ae.safetensors"
set "FLUX_VAE_URL=https://huggingface.co/black-forest-labs/FLUX.1-dev/resolve/main/vae/diffusion_pytorch_model.safetensors"

if exist "%FLUX_VAE_FILE%" (
    echo [07 SKIP] %FLUX_VAE_FILE% already exists.
) else (
    echo [07 DOWNLOADING] FLUX VAE model (ae.safetensors)...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object System.Net.WebClient).DownloadFile('%FLUX_VAE_URL%', '%FLUX_VAE_FILE%')"
)

echo.

:: Model 3: CLIP-L Text Encoder (~246 MB)
set "CLIP_L_FILE=models\clip\clip_l.safetensors"
set "CLIP_L_URL=https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/clip_l.safetensors"

if exist "%CLIP_L_FILE%" (
    echo [07 SKIP] %CLIP_L_FILE% already exists.
) else (
    echo [07 DOWNLOADING] CLIP-L Text Encoder...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object System.Net.WebClient).DownloadFile('%CLIP_L_URL%', '%CLIP_L_FILE%')"
)

echo.

:: Model 4: T5-XXL FP8 Text Encoder (~4.9 GB)
set "T5_XXL_FILE=models\clip\t5xxl_fp8_e4m3fn.safetensors"
set "T5_XXL_URL=https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/t5xxl_fp8_e4m3fn.safetensors"

if exist "%T5_XXL_FILE%" (
    echo [07 SKIP] %T5_XXL_FILE% already exists.
) else (
    echo [07 DOWNLOADING] T5-XXL FP8 Text Encoder...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object System.Net.WebClient).DownloadFile('%T5_XXL_URL%', '%T5_XXL_FILE%')"
)

echo.
echo =======================================================================
echo [07 SUCCESS] All required FLUX.1 [dev] model weights downloaded/verified!
echo =======================================================================
echo.
pause
