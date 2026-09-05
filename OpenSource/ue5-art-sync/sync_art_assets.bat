@echo off
:: Copyright (c) 2026 Daniel Acourt. Version 1.0.0. Licensed under the MIT License.
:: sovereign-art-sync: One-Click Local Art Asset Synchronizer for Unreal Engine 5

echo ===================================================
echo sovereign-art-sync: Local Art Asset Sync
echo ===================================================

:: Navigate to script directory
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
echo Executing sync_art_assets.py...
echo.
%PY_CMD% sync_art_assets.py %*

echo.
echo ===================================================
echo Sync Execution Finished.
echo ===================================================
pause
