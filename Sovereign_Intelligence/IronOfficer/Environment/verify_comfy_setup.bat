@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Diagnostic Verification Audit for Standalone ComfyUI Engine

title [07] ComfyUI Setup Diagnostic Audit

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Standalone ComfyUI Environment Diagnostic Audit
echo =======================================================================
echo.

py -3.11 --version >nul 2>&1
if %errorlevel% EQU 0 (
    py -3.11 run_comfy.py --verify-only
    goto :END
)

py -3.12 --version >nul 2>&1
if %errorlevel% EQU 0 (
    py -3.12 run_comfy.py --verify-only
    goto :END
)

python run_comfy.py --verify-only

:END
echo.
pause
