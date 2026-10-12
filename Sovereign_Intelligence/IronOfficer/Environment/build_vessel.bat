@echo off
:: Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
:: 07 Architect: Chat Vessel Build Script (PyInstaller Wrapper)
:: Purpose: Generates a standalone .EXE for the Chat Vessel to ensure portability.

echo [07] Iron Officer: Building Chat Vessel Executable...

:: 1. Resolve Python Launcher Command (Priority: python -> py -> python3)
set "PY_CMD="
python --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python" & goto :FOUND_PY)
py --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=py" & goto :FOUND_PY)
python3 --version >nul 2>&1
if %errorlevel% EQU 0 (set "PY_CMD=python3" & goto :FOUND_PY)

for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%D\python.exe" (set "PY_CMD="%%D\python.exe"" & goto :FOUND_PY)
)

echo [07 ERROR] Python not found. Please install Python 3.10+.
pause
exit /b

:FOUND_PY
:: 2. Ensure core dependencies are present
echo [07] Verifying PyInstaller and Requests dependencies...
%PY_CMD% -m pip install pyinstaller requests

:: 3. Execute PyInstaller Build
:: --onefile: Bundles all dependencies into a single binary.
:: --name: Sets the output identity to 'IronOfficer_Chat'.
:: --distpath: Forces the binary to output in the current directory for easy access.
%PY_CMD% -m PyInstaller --onefile --name IronOfficer_Chat --distpath . vessel.py

:: 3. Post-Build Cleanup (Optional: remove temporary build folders)
:: rd /s /q build
:: del /f /q IronOfficer_Chat.spec

echo.
echo [07] Build Complete. 'IronOfficer_Chat.exe' is ready in the Environment folder.
echo [07] Note: Ensure 'run_bridge.bat' is active before launching the chat.
pause
