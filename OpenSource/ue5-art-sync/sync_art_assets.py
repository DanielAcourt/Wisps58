#!/usr/bin/env python3
"""
sovereign-art-sync: Local Art Asset Synchronizer & Auto-Discovery Engine for Unreal Engine 5
Copyright (c) 2026 Daniel Acourt. Version 1.0.0. Licensed under the MIT License.

Parses asset_manifest.json to synchronize external art assets from a local asset vault
into the Unreal Engine Content directory (e.g. Content/Assets/External/).
Automatically detects .uproject names and auto-discovers new subdirectories inside local_vault_root.
"""

import os
import sys
import json
import shutil
import argparse
from pathlib import Path


def detect_uproject(root_dir="."):
    """
    Scans for .uproject files in root_dir or immediate child directories to detect project context.
    Returns (project_name, project_root_path).
    """
    root_path = Path(root_dir).resolve()

    # Search current directory
    uproject_files = list(root_path.glob("*.uproject"))
    if uproject_files:
        project_file = uproject_files[0]
        return project_file.stem, root_path

    # Search one subfolder level (e.g., if tool placed in repo root containing MyGame/MyGame.uproject)
    for child in root_path.iterdir():
        if child.is_dir():
            child_uprojects = list(child.glob("*.uproject"))
            if child_uprojects:
                return child_uprojects[0].stem, child

    # Search parent directory
    parent_path = root_path.parent
    parent_uprojects = list(parent_path.glob("*.uproject"))
    if parent_uprojects:
        return parent_uprojects[0].stem, parent_path

    # Fallback default
    return "UnrealProject", root_path


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


def auto_discover_packages(manifest_path, manifest_data, vault_root, project_root):
    """
    Scans local_vault_root for unregistered subdirectories and automatically
    adds them to asset_manifest.json to eliminate manual JSON editing.
    """
    vault_path = Path(vault_root)
    if not vault_path.is_absolute():
        vault_path = Path(project_root) / vault_path

    if not vault_path.exists() or not vault_path.is_dir():
        return False

    existing_sources = {p.get("source_path") for p in manifest_data.get("asset_packages", []) if "source_path" in p}
    discovered_any = False

    for item in vault_path.iterdir():
        if item.is_dir():
            folder_name = item.name
            if folder_name not in existing_sources and not folder_name.startswith("."):
                package_id = f"art_vault_{folder_name.lower().replace(' ', '_')}"
                target_dest = f"Content/Assets/External/ArtVault/{folder_name}"

                new_pkg = {
                    "package_id": package_id,
                    "name": f"Art Vault {folder_name}",
                    "version": "1.0.0",
                    "source_path": folder_name,
                    "target_destination": target_dest,
                    "enabled": True,
                    "description": f"Auto-discovered package from local vault folder {folder_name}."
                }
                manifest_data.setdefault("asset_packages", []).append(new_pkg)
                existing_sources.add(folder_name)
                discovered_any = True
                print(f"[AUTO-DISCOVERY] Detected new folder '{folder_name}'. Auto-registered into manifest.")

    if discovered_any:
        try:
            with open(manifest_path, "w", encoding="utf-8") as f:
                json.dump(manifest_data, f, indent=2)
            print(f"[AUTO-DISCOVERY] Updated '{manifest_path}' with newly discovered packages.")
        except Exception as e:
            print(f"[ERROR] Failed to save auto-discovered packages to manifest: {e}")

    return discovered_any


def sync_package(vault_root, package, project_root, copy_mode=True):
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
    if not candidate_vault_src.is_absolute():
        candidate_vault_src = Path(project_root) / candidate_vault_src

    candidate_direct_src = Path(source_rel)
    if not candidate_direct_src.is_absolute():
        candidate_direct_src = Path(project_root) / candidate_direct_src

    if candidate_vault_src.exists():
        source_path = candidate_vault_src
    elif candidate_direct_src.exists():
        source_path = candidate_direct_src
    else:
        source_path = candidate_vault_src

    target_path = Path(target_rel)
    if not target_path.is_absolute():
        target_path = Path(project_root) / target_path

    print(f"\n[SYNC] Processing Package: {name}")
    print(f"       Source: {source_path}")
    print(f"       Target: {target_path}")

    if not source_path.exists():
        print(f"[WARNING] Local vault source path does not exist: {source_path}")
        print("          Please place asset files into the vault directory or update asset_manifest.json.")
        return False

    # Prevent copying folder onto itself
    if source_path.resolve() == target_path.resolve():
        print(f"[NOTICE] Package source and target are identical directory ({source_path.resolve()}).")
        print("         Assets are already in place in target directory.")
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
    project_name, project_root = detect_uproject()

    manifest = load_manifest(manifest_path)
    if not manifest:
        return {"status": "error", "message": "Failed to load manifest."}

    # Dynamic project name update in manifest if set to auto
    if manifest.get("project_name") == "auto":
        manifest["project_name"] = project_name

    vault_root = vault_override or manifest.get("local_vault_root", "Content/ArtVault")
    vault_path = Path(vault_root)
    if not vault_path.is_absolute():
        vault_path = Path(project_root) / vault_path

    # Ensure vault root directory exists as a local placeholder
    os.makedirs(vault_path, exist_ok=True)

    # Perform auto-discovery of new subdirectories
    auto_discover_packages(manifest_path, manifest, vault_root, project_root)

    packages = manifest.get("asset_packages", [])

    print(f"=== Sovereign Art Sync (sovereign-art-sync) ===")
    print(f"Project Detected: {project_name} ({project_root})")
    print(f"Manifest Version: {manifest.get('manifest_version', '1.0.0')}")
    print(f"Vault Root: {vault_path}")
    print(f"Total Registered Packages: {len(packages)}\n")

    success_count = 0
    for pkg in packages:
        if sync_package(vault_path, pkg, project_root, copy_mode=copy_mode):
            success_count += 1

    summary = {
        "status": "success",
        "project_name": project_name,
        "total_packages": len(packages),
        "synced_packages": success_count,
        "vault_root": str(vault_path)
    }
    print(f"\n=== Sync Complete: {success_count}/{len(packages)} packages processed successfully. ===")
    return summary


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="sovereign-art-sync: Synchronize local art assets into Unreal Engine Content directory.")
    parser.add_argument("--manifest", default="asset_manifest.json", help="Path to asset_manifest.json")
    parser.add_argument("--vault", default=None, help="Override local asset vault root folder")
    parser.add_argument("--symlink", action="store_true", help="Use symlinks instead of copying files")

    args = parser.parse_args()
    run_sync(manifest_path=args.manifest, vault_override=args.vault, copy_mode=not args.symlink)
