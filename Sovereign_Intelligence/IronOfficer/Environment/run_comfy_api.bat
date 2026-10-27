@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Headless API Launcher

title [07] ComfyUI Headless API Server (GTX 5090)

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Launching Headless ComfyUI API Server
echo =======================================================================
echo.

python run_comfy.py
if %errorlevel% NEQ 0 (
    py run_comfy.py
)

echo.
pause
