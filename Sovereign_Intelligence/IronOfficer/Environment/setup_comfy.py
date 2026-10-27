# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Iron Officer: ComfyUI Standalone Setup Script
---------------------------------------------
Automates Python virtual environment creation, PyTorch CUDA installation with multi-index
fallback (cu124 -> cu121 -> PyPI), dependency resolution, and directory structure initialization.
"""

import os
import sys
import subprocess
from pathlib import Path

def find_comfy_directory() -> Path:
    """Locates the ComfyUI installation directory efficiently without deep filesystem sweeps."""
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent.parent

    candidate_paths = [
        Path("C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master/ComfyUI-master"),
        repo_root / "comfy" / "ComfyUI-master" / "ComfyUI-master",
        Path("C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master"),
        repo_root / "comfy" / "ComfyUI-master",
        repo_root / "comfy",
    ]

    for path in candidate_paths:
        if (path / "main.py").exists():
            return path.resolve()

    # Shallow check inside repo_root/comfy folder if present
    comfy_base = repo_root / "comfy"
    if comfy_base.exists():
        for root, dirs, files in os.walk(str(comfy_base)):
            dirs[:] = [d for d in dirs if d not in ('.git', 'venv', 'Intermediate', 'Binaries', 'Saved', '__pycache__')]
            if "main.py" in files:
                return Path(root).resolve()

    return None

def main():
    print("=" * 70)
    print("[07] Iron Officer: Standalone ComfyUI Setup Manager")
    print("=" * 70)

    # Diagnostic Python Environment Inspection
    is_64bit = sys.maxsize > 2**32
    py_ver = sys.version.split()[0]
    print(f"[07] Host Python Executable: {sys.executable}")
    print(f"[07] Host Python Version:    {py_ver} ({'64-bit' if is_64bit else '32-bit WARNING'})")

    if not is_64bit:
        print("\n[07 WARNING] 32-bit Python detected! PyTorch requires 64-bit Python.")
        print("Please install 64-bit Python 3.10 / 3.11 / 3.12 from python.org!")

    comfy_dir = find_comfy_directory()
    if not comfy_dir:
        print("\n[07 ERROR] Could not find ComfyUI main.py!")
        print("Looked in standard paths:")
        print("  - C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master/ComfyUI-master")
        print("  - C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master")
        print("\nPlease ensure ComfyUI is extracted under the 'comfy/' folder.")
        sys.exit(1)

    print(f"\n[07] Target ComfyUI Directory: {comfy_dir}")

    # Virtual Environment Path
    venv_dir = comfy_dir / "venv"
    if os.name == "nt":
        venv_python = venv_dir / "Scripts" / "python.exe"
    else:
        venv_python = venv_dir / "bin" / "python"

    # Step 1: Create Virtual Environment if missing
    if not venv_python.exists():
        print(f"\n[07] Creating dedicated Python Virtual Environment at {venv_dir}...")
        try:
            subprocess.run([sys.executable, "-m", "venv", str(venv_dir)], check=True)
            print("[07] Virtual Environment created successfully.")
        except subprocess.CalledProcessError as e:
            print(f"[07 ERROR] Failed to create virtual environment: {e}")
            sys.exit(1)
    else:
        print(f"\n[07] Existing Virtual Environment detected at {venv_dir}")

    # Step 2: Upgrade Pip, Setuptools, Wheel
    try:
        print("\n[07] Upgrading pip, setuptools, wheel...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"[07 WARNING] Pip upgrade step had issues: {e}")

    # Step 3: Install PyTorch with multi-index fallback
    print("\n[07] Installing PyTorch with CUDA acceleration...")
    pytorch_targets = [
        ("https://download.pytorch.org/whl/cu124", "CUDA 12.4 Index"),
        ("https://download.pytorch.org/whl/cu121", "CUDA 12.1 Index"),
        ("https://download.pytorch.org/whl/cu118", "CUDA 11.8 Index"),
        (None, "Standard PyPI Index")
    ]

    torch_installed = False
    for index_url, index_label in pytorch_targets:
        print(f" -> Attempting PyTorch installation via {index_label}...")
        cmd = [str(venv_python), "-m", "pip", "install", "torch", "torchvision", "torchaudio"]
        if index_url:
            cmd.extend(["--index-url", index_url])

        res = subprocess.run(cmd)
        if res.returncode == 0:
            print(f"[07 SUCCESS] PyTorch installed successfully via {index_label}!")
            torch_installed = True
            break
        else:
            print(f" -> {index_label} installation returned exit code {res.returncode}. Trying next index...")

    if not torch_installed:
        print("\n[07 ERROR] All PyTorch installation attempts failed.")
        print("Diagnostic Tip: If using Python 3.13 or 32-bit Python, please install 64-bit Python 3.11 or 3.12.")
        sys.exit(1)

    # Step 4: Install ComfyUI dependencies
    req_file = comfy_dir / "requirements.txt"
    if req_file.exists():
        print(f"\n[07] Installing dependencies from {req_file}...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "-r", str(req_file)])
    else:
        print("\n[07] Installing standard PyTorch/ComfyUI packages...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "pillow", "pyyaml", "safetensors", "scipy", "tqdm", "psutil", "requests"])

    # Step 5: Ensure Model Directories Exist
    print("\n[07] Ensuring model subdirectories exist...")
    model_dirs = ["unet", "vae", "clip", "checkpoints", "loras", "controlnet"]
    models_base = comfy_dir / "models"
    models_base.mkdir(parents=True, exist_ok=True)

    for sub in model_dirs:
        (models_base / sub).mkdir(parents=True, exist_ok=True)

    (comfy_dir / "output").mkdir(parents=True, exist_ok=True)
    (comfy_dir / "input").mkdir(parents=True, exist_ok=True)

    print("\n" + "=" * 70)
    print("[07 SUCCESS] Standalone ComfyUI Environment set up successfully!")
    print("Next steps:")
    print("  1. Run 'download_flux_models.bat' to download model weights.")
    print("  2. Run 'run_comfy_api.bat' to start the ComfyUI API server on port 8188.")
    print("=" * 70 + "\n")

if __name__ == "__main__":
    main()
