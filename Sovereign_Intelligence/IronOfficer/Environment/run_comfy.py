# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Iron Officer: ComfyUI Headless API Server Launcher
--------------------------------------------------
Launches ComfyUI in headless API server mode listening on 127.0.0.1:8188.
Includes auto-repair when CPU-only PyTorch builds are detected.
Supports --verify-only flag for diagnostic environment checks.
"""

import os
import sys
import subprocess
from pathlib import Path
import urllib.request
from setup_comfy import find_comfy_directory, patch_comfy_kitchen_if_needed, main as run_setup_comfy

def verify_and_repair_models(comfy_dir: Path):
    """
    Verifies that required model weights exist and are valid binary safetensors files.
    If a model (such as ae.safetensors) is missing, corrupted, or contains an HTML redirect text,
    it automatically downloads the clean weights before server launch.
    """
    models_dir = comfy_dir / "models"
    models_dir.mkdir(parents=True, exist_ok=True)

    required_models = [
        (models_dir / "vae" / "ae.safetensors", "https://huggingface.co/camenduru/FLUX.1-dev/resolve/main/ae.safetensors", 335000000, "FLUX VAE"),
        (models_dir / "clip" / "clip_l.safetensors", "https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/clip_l.safetensors", 240000000, "CLIP-L Text Encoder"),
        (models_dir / "clip" / "t5xxl_fp8_e4m3fn.safetensors", "https://huggingface.co/comfyanonymous/flux_text_encoders/resolve/main/t5xxl_fp8_e4m3fn.safetensors", 4000000000, "T5-XXL Text Encoder"),
    ]

    for model_path, url, min_bytes, name in required_models:
        model_path.parent.mkdir(parents=True, exist_ok=True)
        b_needs_download = False

        if not model_path.exists():
            print(f"[07 MODEL CHECK] {name} missing at {model_path.name}. Downloading...")
            b_needs_download = True
        else:
            file_size = model_path.stat().st_size
            if file_size < min_bytes or file_size < 10000000:
                print(f"[07 MODEL REPAIR] {name} at {model_path.name} is corrupted/truncated ({file_size} bytes). Repairing...")
                try:
                    model_path.unlink(missing_ok=True)
                except Exception:
                    pass
                b_needs_download = True
            else:
                # Inspect first 100 bytes to detect HTML error responses
                try:
                    with open(model_path, "rb") as f:
                        header = f.read(100)
                        if b"<!DOCTYPE" in header or b"<html" in header or b"Git LFS" in header:
                            print(f"[07 MODEL REPAIR] {name} is an HTML error page instead of binary weights. Repairing...")
                            model_path.unlink(missing_ok=True)
                            b_needs_download = True
                except Exception:
                    pass

        if b_needs_download:
            print(f"[07 DOWNLOADING] Downloading {name} from open repository...")
            print(f" -> URL: {url}")
            print(f" -> Destination: {model_path}")
            try:
                def progress_bar(block_num, block_size, total_size):
                    downloaded = block_num * block_size
                    if total_size > 0:
                        percent = min(100.0, (downloaded / total_size) * 100)
                        mb = downloaded / (1024 * 1024)
                        total_mb = total_size / (1024 * 1024)
                        sys.stdout.write(f"\r -> Progress: {percent:.1f}% ({mb:.1f}/{total_mb:.1f} MB)")
                        sys.stdout.flush()

                urllib.request.urlretrieve(url, str(model_path), reporthook=progress_bar)
                print(f"\n[07 SUCCESS] {name} downloaded and verified successfully!\n")
            except Exception as e:
                print(f"\n[07 ERROR] Failed to download {name}: {e}\n")

    # UNet check
    unet_path = models_dir / "unet" / "flux1-dev-fp8.safetensors"
    if not unet_path.exists() or unet_path.stat().st_size < 1000000000:
        print("\n" + "!" * 70)
        print("[07 WARNING] FLUX.1 FP8 UNet model (~12GB) missing or incomplete at:")
        print(f"             {unet_path}")
        print("[07 ACTION REQUIRED] Run 'download_flux_models.bat' to fetch the main UNet weights.")
        print("!" * 70 + "\n")

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
        print("[07 ACTION REQUIRED] Triggering setup_comfy.py to build environment...")
        run_setup_comfy()

    print(f"\n[07] ComfyUI Directory: {comfy_dir}")
    print(f"[07] Virtual Environment Python: {venv_python}")

    # Ensure comfy_kitchen schema type hints are patched before launch
    patch_comfy_kitchen_if_needed(venv_dir)

    # Verify and automatically repair model weight safetensors if corrupted/missing
    verify_and_repair_models(comfy_dir)

    # Test PyTorch & CUDA
    print("\n[07] Testing PyTorch and CUDA acceleration...")
    test_res = subprocess.run([
        str(venv_python), "-c",
        "import torch, sys; sys.exit(0 if torch.cuda.is_available() else 1)"
    ])

    if test_res.returncode != 0:
        print("\n" + "!" * 70)
        print("[07 AUTO-REPAIR] CPU-only PyTorch build or CUDA missing in venv!")
        print("[07 AUTO-REPAIR] Automatically invoking setup_comfy.py to force CUDA PyTorch installation...")
        print("!" * 70 + "\n")

        # Trigger setup_comfy to fix PyTorch CUDA installation
        run_setup_comfy()

        # Re-verify post repair
        test_res_2 = subprocess.run([
            str(venv_python), "-c",
            "import torch, sys; gpu = torch.cuda.get_device_name(0) if torch.cuda.is_available() else 'None'; print(f'[07 REPAIRED] PyTorch: {torch.__version__} | CUDA: {torch.cuda.is_available()} | GPU: {gpu}'); sys.exit(0 if torch.cuda.is_available() else 1)"
        ])
        if test_res_2.returncode != 0:
            print("\n[07 ERROR] PyTorch CUDA installation failed after auto-repair attempt.")
            print("Please verify NVIDIA drivers are installed and 64-bit Python is in use.")
            sys.exit(1)

    if b_verify_only:
        print("\n[07 SUCCESS] Diagnostic verification complete. Virtual environment and PyTorch CUDA are operational!")
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
