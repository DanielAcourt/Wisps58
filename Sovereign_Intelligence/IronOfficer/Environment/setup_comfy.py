# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Iron Officer: ComfyUI Standalone Setup Script
---------------------------------------------
Automates Python virtual environment creation with Python 3.10-3.12 compatibility enforcement,
PyTorch CUDA installation pinned to 2.4.1+cu124, comfy_kitchen schema type hint patching, dependency resolution,
and directory structure initialization.
"""

import os
import sys
import shutil
import subprocess
from pathlib import Path

def find_compatible_python() -> str:
    """
    Finds a Python 3.10, 3.11, or 3.12 executable on the system.
    PyTorch does NOT support Python 3.13 / 3.14 wheels yet.
    """
    if sys.version_info >= (3, 10) and sys.version_info < (3, 13):
        return sys.executable

    # Check Windows 'py' launcher for Python 3.11 or 3.12
    for py_ver in ["-3.11", "-3.12", "-3.10"]:
        try:
            res = subprocess.run(["py", py_ver, "-c", "import sys; print(sys.executable)"], capture_output=True, text=True)
            if res.returncode == 0 and res.stdout.strip():
                executable = res.stdout.strip()
                if os.path.exists(executable):
                    return executable
        except Exception:
            pass

    # Check common Windows Python installation directories
    local_app_data = os.environ.get("LocalAppData", "")
    program_files = os.environ.get("ProgramFiles", r"C:\Program Files")
    candidates = [
        os.path.join(local_app_data, r"Programs\Python\Python311\python.exe"),
        os.path.join(local_app_data, r"Programs\Python\Python312\python.exe"),
        os.path.join(program_files, r"Python311\python.exe"),
        os.path.join(program_files, r"Python312\python.exe"),
        r"C:\Python311\python.exe",
        r"C:\Python312\python.exe",
    ]

    for cand in candidates:
        if cand and os.path.exists(cand):
            return cand

    return None

def find_comfy_directory() -> Path:
    """Locates the ComfyUI installation directory efficiently without deep filesystem sweeps."""
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent.parent

    candidate_paths = [
        Path("C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master/ComfyUI-master"),
        Path("C:/Users/Danie/Unreal Projects/2027/repo/comfy/ComfyUI-master"),
        repo_root.parent.parent / "comfy" / "ComfyUI-master" / "ComfyUI-master",
        repo_root.parent.parent / "comfy" / "ComfyUI-master",
        repo_root.parent / "comfy" / "ComfyUI-master" / "ComfyUI-master",
        repo_root.parent / "comfy" / "ComfyUI-master",
        repo_root / "comfy" / "ComfyUI-master" / "ComfyUI-master",
        repo_root / "comfy" / "ComfyUI-master",
        repo_root / "comfy",
    ]

    for path in candidate_paths:
        if (path / "main.py").exists():
            return path.resolve()

    # Shallow check inside sibling comfy folder if present
    for search_base in [repo_root.parent.parent / "comfy", repo_root.parent / "comfy", repo_root / "comfy"]:
        if search_base.exists():
            for root, dirs, files in os.walk(str(search_base)):
                dirs[:] = [d for d in dirs if d not in ('.git', 'venv', 'Intermediate', 'Binaries', 'Saved', '__pycache__')]
                if "main.py" in files:
                    return Path(root).resolve()

    return None

def patch_comfy_kitchen_if_needed(venv_dir: Path):
    """
    Force-reinstalls a clean copy of comfy-kitchen to wipe any corrupted site-packages edits,
    then applies minimal surgical type hint fixes ONLY to files containing `@torch.library.custom_op`.
    """
    if os.name == "nt":
        venv_python = venv_dir / "Scripts" / "python.exe"
    else:
        venv_python = venv_dir / "bin" / "python"

    # Step 1: Force clean reinstall of comfy-kitchen to ensure zero corrupted files
    try:
        print("[07 CLEANUP] Resetting comfy-kitchen to pristine package state...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "--force-reinstall", "--no-deps", "comfy-kitchen"], capture_output=True, text=True)
    except Exception as e:
        print(f"[07 WARNING] Could not reinstall comfy-kitchen: {e}")

    # Step 2: Surgically patch ONLY custom_op files
    site_packages_list = [
        venv_dir / "Lib" / "site-packages",
        venv_dir / "lib" / f"python{sys.version_info.major}.{sys.version_info.minor}" / "site-packages"
    ]

    for site_packages in site_packages_list:
        ck_dir = site_packages / "comfy_kitchen"
        if ck_dir.exists():
            for py_file in ck_dir.glob("**/*.py"):
                try:
                    content = py_file.read_text(encoding="utf-8")
                    # STRICT FILTER: Only touch files that register PyTorch custom_ops
                    if "@torch.library.custom_op" not in content and "custom_op(" not in content:
                        continue

                    lines = content.splitlines(keepends=True)
                    new_lines = []
                    b_modified = False

                    for line in lines:
                        # Replace list[...] with typing.List[...] in custom_op signature lines
                        if "list[" in line or "| None" in line or "None |" in line:
                            line = line.replace("list[", "typing.List[")
                            line = line.replace("sequence[", "typing.Sequence[")
                            line = line.replace("| None", "| None") # Keep union if not custom_op
                            b_modified = True
                        new_lines.append(line)

                    if b_modified:
                        # Add `import typing` after `from __future__` if typing. is referenced
                        final_content = "".join(new_lines)
                        if "typing." in final_content and "import typing" not in final_content:
                            split_lines = final_content.splitlines(keepends=True)
                            future_idx = -1
                            for idx, l in enumerate(split_lines):
                                if "from __future__ import" in l:
                                    future_idx = idx

                            if future_idx >= 0:
                                split_lines.insert(future_idx + 1, "import typing\n")
                            else:
                                split_lines.insert(0, "import typing\n")
                            final_content = "".join(split_lines)

                        py_file.write_text(final_content, encoding="utf-8")
                        print(f"[07 PATCH SUCCESS] Surgically patched custom_op schema in {py_file.name}")
                except Exception as e:
                    print(f"[07 WARNING] Could not patch {py_file.name}: {e}")

def main():
    print("=" * 70)
    print("[07] Iron Officer: Standalone ComfyUI Setup Manager")
    print("=" * 70)

    # Diagnostic Python Environment Inspection
    is_64bit = sys.maxsize > 2**32
    py_ver = sys.version.split()[0]
    print(f"[07] Host Python Executable: {sys.executable}")
    print(f"[07] Host Python Version:    {py_ver} ({'64-bit' if is_64bit else '32-bit WARNING'})")

    # Python Version Compatibility Guard
    target_py = sys.executable
    if sys.version_info >= (3, 13):
        print("\n" + "!" * 70)
        print(f"[07 WARNING] Python {py_ver} detected!")
        print("PyTorch and CUDA do NOT support Python 3.13 or 3.14 wheels yet.")
        print("Searching system for Python 3.10, 3.11, or 3.12...")
        print("!" * 70)

        compat_py = find_compatible_python()
        if compat_py:
            print(f"[07 AUTO-SWITCH] Found compatible Python: {compat_py}")
            target_py = compat_py
        else:
            print("\n[07 CRITICAL ERROR] No compatible Python version (3.10 - 3.12) found on system!")
            print("PyTorch requires Python 3.10, 3.11, or 3.12.")
            print("\nTo fix this, run this command in Command Prompt:")
            print("    winget install Python.Python.3.11")
            print("\nThen re-run setup_comfy_standalone.bat!")
            sys.exit(1)

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

    # Step 1: Check if venv was created with incompatible Python version
    b_recreate_venv = False
    if venv_python.exists():
        v_check = subprocess.run([str(venv_python), "-c", "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"], capture_output=True, text=True)
        if v_check.returncode == 0:
            v_ver = v_check.stdout.strip()
            if v_ver.startswith("3.14") or v_ver.startswith("3.13"):
                print(f"\n[07 PURGE] Existing venv was built with Python {v_ver} (incompatible with PyTorch CUDA).")
                print(f"[07 PURGE] Forcefully purging {venv_dir} to rebuild with {target_py}...")
                b_recreate_venv = True

    if not venv_python.exists() or b_recreate_venv:
        if b_recreate_venv and venv_dir.exists():
            try:
                if os.name == "nt":
                    subprocess.run(f'rmdir /s /q "{venv_dir}"', shell=True)
                else:
                    shutil.rmtree(str(venv_dir), ignore_errors=True)
            except Exception as e:
                print(f"[07 WARNING] Could not completely remove venv folder: {e}")

        print(f"\n[07] Creating dedicated Python Virtual Environment at {venv_dir} using {target_py}...")
        try:
            subprocess.run([target_py, "-m", "venv", str(venv_dir)], check=True)
            print("[07] Virtual Environment created successfully.")
        except subprocess.CalledProcessError as e:
            print(f"[07 ERROR] Failed to create virtual environment: {e}")
            sys.exit(1)
    else:
        print(f"\n[07] Existing Virtual Environment detected at {venv_dir}")

    # Step 2: Upgrade Pip, Setuptools, Wheel inside venv
    try:
        print("\n[07] Upgrading pip, setuptools, wheel inside venv...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"[07 WARNING] Pip upgrade step had issues: {e}")

    # Step 3: Check if PyTorch with CUDA is ALREADY working
    print("\n[07] Checking PyTorch and CUDA installation...")
    test_cmd = [str(venv_python), "-c", "import torch, sys; sys.exit(0 if torch.cuda.is_available() else 1)"]
    test_res = subprocess.run(test_cmd)

    if test_res.returncode == 0:
        print("[07 SUCCESS] PyTorch with CUDA is already installed and verified!")
        cuda_torch_ok = True
    else:
        print("[07 NOTICE] PyTorch CUDA not verified. Preparing clean CUDA PyTorch wheel installation...")
        subprocess.run([str(venv_python), "-m", "pip", "uninstall", "-y", "torch", "torchvision", "torchaudio", "intel-openmp"])

        pytorch_targets = [
            ([str(venv_python), "-m", "pip", "install", "torch==2.4.1+cu124", "torchvision==0.19.1+cu124", "torchaudio==2.4.1+cu124", "--index-url", "https://download.pytorch.org/whl/cu124"], "CUDA 12.4 PyTorch 2.4.1 (Stable ComfyUI)"),
            ([str(venv_python), "-m", "pip", "install", "torch==2.5.1+cu124", "torchvision==0.20.1+cu124", "torchaudio==2.5.1+cu124", "--index-url", "https://download.pytorch.org/whl/cu124"], "CUDA 12.4 PyTorch 2.5.1"),
            ([str(venv_python), "-m", "pip", "install", "torch", "torchvision", "torchaudio", "--index-url", "https://download.pytorch.org/whl/cu124"], "CUDA 12.4 Latest Index"),
            ([str(venv_python), "-m", "pip", "install", "torch", "torchvision", "torchaudio", "--index-url", "https://download.pytorch.org/whl/cu121"], "CUDA 12.1 Index"),
        ]

        cuda_torch_ok = False
        for cmd, label in pytorch_targets:
            print(f"\n[07] Installing PyTorch via {label}...")
            res = subprocess.run(cmd)
            if res.returncode == 0:
                test_res = subprocess.run(test_cmd)
                if test_res.returncode == 0:
                    print(f"\n[07 SUCCESS] PyTorch CUDA acceleration verified via {label}!")
                    cuda_torch_ok = True
                    break
                else:
                    print(f" -> {label} finished but torch.cuda.is_available() returned False. Purging and trying next index...")
                    subprocess.run([str(venv_python), "-m", "pip", "uninstall", "-y", "torch", "torchvision", "torchaudio"])

        if not cuda_torch_ok:
            print("\n[07 ERROR] All PyTorch CUDA wheel installations failed.")
            print("Action Required: Please verify NVIDIA GPU drivers are installed and 64-bit Python 3.10/3.11/3.12 is in use.")
            sys.exit(1)

    # Step 5: Install ComfyUI dependencies
    req_file = comfy_dir / "requirements.txt"
    if req_file.exists():
        print(f"\n[07] Installing dependencies from {req_file}...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "-r", str(req_file)])
    else:
        print("\n[07] Installing standard PyTorch/ComfyUI packages...")
        subprocess.run([str(venv_python), "-m", "pip", "install", "pillow", "pyyaml", "safetensors", "scipy", "tqdm", "psutil", "requests"])

    # Step 6: Apply comfy_kitchen type hint schema patch
    patch_comfy_kitchen_if_needed(venv_dir)

    # Step 7: Ensure Model Directories Exist
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
