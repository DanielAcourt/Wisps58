# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Iron Officer: ComfyUI Headless API Server Launcher
--------------------------------------------------
Launches ComfyUI in headless API server mode listening on 127.0.0.1:8188.
Supports --verify-only flag for diagnostic environment checks.
"""

import os
import sys
import subprocess
from pathlib import Path
from setup_comfy import find_comfy_directory

def main():
    b_verify_only = "--verify-only" in sys.argv

    print("=" * 70)
    print(f"[07] Iron Officer: {'ComfyUI Diagnostic Audit' if b_verify_only else 'Launching Headless ComfyUI API Server'}")
    print("=" * 70)

    comfy_dir = find_comfy_directory()
    if not comfy_dir:
        print("\n[07 ERROR] Could not find ComfyUI main.py!")
        print("Please run setup_comfy_standalone.bat first.")
        sys.exit(1)

    venv_dir = comfy_dir / "venv"
    if os.name == "nt":
        venv_python = venv_dir / "Scripts" / "python.exe"
    else:
        venv_python = venv_dir / "bin" / "python"

    if not venv_python.exists():
        print(f"\n[07 ERROR] Virtual Environment python not found at {venv_python}!")
        print("[07 ACTION REQUIRED] Run 'setup_comfy_standalone.bat' first to build the environment!")
        sys.exit(1)

    print(f"\n[07] ComfyUI Directory: {comfy_dir}")
    print(f"[07] Virtual Environment Python: {venv_python}")

    # Test PyTorch & CUDA
    print("\n[07] Testing PyTorch and CUDA acceleration...")
    test_res = subprocess.run([
        str(venv_python), "-c",
        "import torch; print(f'[07] PyTorch Version: {torch.__version__} | CUDA Available: {torch.cuda.is_available()}'); print(f'[07] GPU Device: {torch.cuda.get_device_name(0)}') if torch.cuda.is_available() else print('[07 WARNING] CUDA GPU not detected!')"
    ])

    if test_res.returncode != 0:
        print("\n[07 ERROR] PyTorch test failed. Please re-run setup_comfy_standalone.bat!")
        sys.exit(1)

    if b_verify_only:
        print("\n[07 SUCCESS] Diagnostic verification complete. Virtual environment and PyTorch are operational!")
        sys.exit(0)

    print("\n" + "=" * 70)
    print("[07] Starting ComfyUI backend server listening on http://127.0.0.1:8188...")
    print("[07] Keep this window running in the background for image generation.")
    print("=" * 70 + "\n")

    os.chdir(comfy_dir)
    try:
        subprocess.run([str(venv_python), "main.py", "--listen", "127.0.0.1", "--port", "8188", "--highvram"])
    except KeyboardInterrupt:
        print("\n[07] ComfyUI server stopped by user.")

if __name__ == "__main__":
    main()
