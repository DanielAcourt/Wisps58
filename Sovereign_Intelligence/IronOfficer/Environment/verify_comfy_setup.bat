@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
:: 07 Architecture: Diagnostic Verification Audit for Standalone ComfyUI Engine

title [07] ComfyUI Setup Diagnostic Audit

cd /d "%~dp0"

echo =======================================================================
echo [07] Iron Officer: Standalone ComfyUI Environment Diagnostic Audit
echo =======================================================================
echo.

python -c "from setup_comfy import find_comfy_directory; import os, sys; d = find_comfy_directory(); print(f'[07] Found ComfyUI: {d}') if d else print('[07 FAILED] ComfyUI directory not found!'); v = (d / 'venv') if d else None; print(f'[07] Virtual Environment: {v} (Exists: {v.exists()})') if v else None"

echo.
python run_comfy.py --verify-only 2>nul

echo.
pause
