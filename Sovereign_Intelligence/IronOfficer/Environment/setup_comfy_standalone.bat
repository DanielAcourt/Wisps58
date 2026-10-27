@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Standalone ComfyUI Environment Setup Script

title [07] ComfyUI Standalone Setup

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Launching ComfyUI Setup Manager
echo =======================================================================
echo.

python setup_comfy.py
if %errorlevel% NEQ 0 (
    py setup_comfy.py
)

echo.
pause
