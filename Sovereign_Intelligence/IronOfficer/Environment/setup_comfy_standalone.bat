@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Environment Setup Script

title [07] ComfyUI Standalone Setup

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Launching ComfyUI Setup Manager
echo =======================================================================
echo.

:: Priority 1: Windows 'py' launcher with Python 3.11 / 3.12 (PyTorch compatible)
py -3.11 --version >nul 2>&1
if %errorlevel% EQU 0 (
    echo [07] Executing setup using Python 3.11...
    py -3.11 setup_comfy.py
    goto :END
)

py -3.12 --version >nul 2>&1
if %errorlevel% EQU 0 (
    echo [07] Executing setup using Python 3.12...
    py -3.12 setup_comfy.py
    goto :END
)

python setup_comfy.py

:END
echo.
pause
