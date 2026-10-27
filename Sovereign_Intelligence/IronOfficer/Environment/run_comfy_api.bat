@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Headless API Launcher

title [07] ComfyUI Headless API Server (GTX 5090)

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Launching Headless ComfyUI API Server
echo =======================================================================
echo.

:: Priority 1: Windows 'py' launcher with Python 3.11 / 3.12 (PyTorch compatible)
py -3.11 --version >nul 2>&1
if %errorlevel% EQU 0 (
    echo [07] Launching server using Python 3.11...
    py -3.11 run_comfy.py
    goto :END
)

py -3.12 --version >nul 2>&1
if %errorlevel% EQU 0 (
    echo [07] Launching server using Python 3.12...
    py -3.12 run_comfy.py
    goto :END
)

python run_comfy.py

:END
echo.
pause
