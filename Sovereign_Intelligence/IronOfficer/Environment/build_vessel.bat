@echo off
:: Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
:: 07 Architect: Chat Vessel Build Script (PyInstaller Wrapper)
:: Purpose: Generates a standalone .EXE for the Chat Vessel to ensure portability.

echo [07] Iron Officer: Building Chat Vessel Executable...

:: 1. Ensure core dependencies are present
python -m pip install pyinstaller requests

:: 2. Execute PyInstaller Build
:: --onefile: Bundles all dependencies into a single binary.
:: --name: Sets the output identity to 'IronOfficer_Chat'.
:: --distpath: Forces the binary to output in the current directory for easy access.
python -m PyInstaller --onefile --name IronOfficer_Chat --distpath . vessel.py

:: 3. Post-Build Cleanup (Optional: remove temporary build folders)
:: rd /s /q build
:: del /f /q IronOfficer_Chat.spec

echo.
echo [07] Build Complete. 'IronOfficer_Chat.exe' is ready in the Environment folder.
echo [07] Note: Ensure 'run_bridge.bat' is active before launching the chat.
pause
