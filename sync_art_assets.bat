@echo off
:: Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3.
:: Sovereign Framework: One-Click Local Art Asset Synchronizer

echo ===================================================
echo [07] Sovereign Framework: Local Art Asset Sync
echo ===================================================

:: Navigate to repository root
cd /d "%~dp0"

:: Resolve Python executable (priority: python -> py -> python3)
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :RUN)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :RUN)
python3 --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python3" & goto :RUN)

:: Fallback: Search common Windows installation paths
for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%D\python.exe" (set "PY_CMD="%%D\python.exe"" & goto :RUN)
)

echo [ERROR] Python not found. Please ensure Python 3.10+ is installed and added to PATH.
pause
exit /b 1

:RUN
echo [07] Executing sync_art_assets.py...
echo.
%PY_CMD% WispsCPPVR/Content/Python/sync_art_assets.py %*

echo.
echo ===================================================
echo [07] Sync Execution Finished.
echo ===================================================
pause
