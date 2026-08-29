#!/usr/bin/env python3
"""
Sovereign Framework: Local Art Asset Synchronizer
Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3.

Parses asset_manifest.json to synchronize external art assets from a local asset vault
into the Unreal Engine Content directory (e.g. WispsCPPVR/Content/Assets/External/).
"""

import os
import sys
import json
import shutil
import argparse
from pathlib import Path


def load_manifest(manifest_path="asset_manifest.json"):
    """Loads and validates the asset manifest file."""
    manifest_file = Path(manifest_path)
    if not manifest_file.exists():
        print(f"[ERROR] Manifest file not found: {manifest_path}")
        return None

    try:
        with open(manifest_file, "r", encoding="utf-8") as f:
            data = json.load(f)
            return data
    except Exception as e:
        print(f"[ERROR] Failed to parse manifest JSON: {e}")
        return None


def sync_package(vault_root, package, copy_mode=True):
    """Syncs an individual asset package from local vault to target destination."""
    package_id = package.get("package_id", "unknown")
    name = package.get("name", package_id)
    source_rel = package.get("source_path", "")
    target_rel = package.get("target_destination", "")
    enabled = package.get("enabled", True)

    if not enabled:
        print(f"[SKIP] Package '{name}' ({package_id}) is disabled in manifest.")
        return True

    # Resolve source path flexible lookup
    candidate_vault_src = Path(vault_root) / source_rel
    candidate_direct_src = Path(source_rel)

    if candidate_vault_src.exists():
        source_path = candidate_vault_src
    elif candidate_direct_src.exists():
        source_path = candidate_direct_src
    else:
        source_path = candidate_vault_src

    target_path = Path(target_rel)

    print(f"\n[SYNC] Processing Package: {name}")
    print(f"       Source: {source_path}")
    print(f"       Target: {target_path}")

    if not source_path.exists():
        print(f"[WARNING] Local vault source path does not exist: {source_path}")
        print("          Please place asset files into the vault directory or update asset_manifest.json.")
        return False

    # Prevent copying folder onto itself
    if source_path.resolve() == target_path.resolve():
        print(f"[NOTICE] Package source and target are the identical directory ({source_path.resolve()}).")
        print("         Assets are already in place in the target directory.")
        return True

    os.makedirs(target_path, exist_ok=True)

    synced_count = 0
    if source_path.is_dir():
        for root, dirs, files in os.walk(source_path):
            rel_dir = Path(root).relative_to(source_path)
            dest_dir = target_path / rel_dir
            os.makedirs(dest_dir, exist_ok=True)

            for file_name in files:
                src_file = Path(root) / file_name
                dest_file = dest_dir / file_name

                # Copy if missing or modified time differs
                if not dest_file.exists() or src_file.stat().st_mtime > dest_file.stat().st_mtime:
                    if copy_mode:
                        shutil.copy2(src_file, dest_file)
                    else:
                        if dest_file.exists():
                            dest_file.unlink()
                        os.symlink(src_file.resolve(), dest_file)
                    synced_count += 1

    print(f"[SUCCESS] Package '{name}' synced ({synced_count} files updated).")
    return True


def run_sync(manifest_path="asset_manifest.json", vault_override=None, copy_mode=True):
    """Executes the full asset synchronization process."""
    manifest = load_manifest(manifest_path)
    if not manifest:
        return {"status": "error", "message": "Failed to load manifest."}

    vault_root = vault_override or manifest.get("local_vault_root", "WispsCPPVR/Content/ArtVault")
    packages = manifest.get("asset_packages", [])

    print(f"=== Sovereign Local Art Asset Sync ===")
    print(f"Manifest Version: {manifest.get('manifest_version', '1.0.0')}")
    print(f"Vault Root: {vault_root}")
    print(f"Total Registered Packages: {len(packages)}\n")

    # Ensure vault root directory exists as a local placeholder
    os.makedirs(vault_root, exist_ok=True)

    success_count = 0
    for pkg in packages:
        if sync_package(vault_root, pkg, copy_mode=copy_mode):
            success_count += 1

    summary = {
        "status": "success",
        "total_packages": len(packages),
        "synced_packages": success_count,
        "vault_root": str(vault_root)
    }
    print(f"\n=== Sync Complete: {success_count}/{len(packages)} packages processed successfully. ===")
    return summary


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Synchronize local art assets into Unreal Engine Content directory.")
    parser.add_argument("--manifest", default="asset_manifest.json", help="Path to asset_manifest.json")
    parser.add_argument("--vault", default=None, help="Override local asset vault root folder")
    parser.add_argument("--symlink", action="store_true", help="Use symlinks instead of copying files")

    args = parser.parse_args()
    run_sync(manifest_path=args.manifest, vault_override=args.vault, copy_mode=not args.symlink)
