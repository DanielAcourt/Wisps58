#!/usr/bin/env python3
"""
Sovereign Framework: Local Art Asset Synchronizer & Auto-Discovery Engine
Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3.

Parses asset_manifest.json to synchronize external art assets bidirectionally between local asset vault
and Unreal Engine Content directories (e.g. Content/Assets/External/).
Automatically discovers new subdirectories inside local_vault_root and updates asset_manifest.json.

CRITICAL ARCHITECTURAL SAFETY RULE:
Level files (.umap) and World Partition external actor/object metadata (__ExternalActors__, __ExternalObjects__)
MUST NEVER be synced by this tool. Level files and World Partition metadata belong strictly in project Git tracking
under Content/ (e.g. Content/Maps/ or Content/Levels/) to prevent Unreal Engine from generating duplicate actor GUIDs
and level package collisions on launch.
"""

import os
import sys
import json
import shutil
import argparse
from pathlib import Path

# Directories and file extensions that MUST NOT be synced
IGNORED_DIR_NAMES = {
    "__externalactors__",
    "__externalobjects__",
    ".git",
    ".vs",
    "intermediate",
    "saved",
    "binaries",
    "deriveddatacache",
    "build"
}

IGNORED_FILE_EXTENSIONS = {
    ".umap",         # Unreal Level maps (must stay in Git under Content/Maps/ or Content/Levels/)
    ".umap.bak",     # Level backups
    ".uasset.bak",   # Asset backups
    ".tmp",          # Temporary files
    ".log"           # Log files
}


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

    # Search one subfolder level
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


def resolve_project_path(path_str, project_root):
    """Resolves path_str flexibly against CWD and project_root without duplicating paths."""
    p = Path(path_str)
    if p.is_absolute():
        return p

    proj_path = Path(project_root).resolve()

    # Strip duplicate project folder prefix if present
    if p.parts and p.parts[0] == proj_path.name:
        p = Path(*p.parts[1:])

    candidate = (proj_path / p).resolve()
    return candidate


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
    Excludes special system folders like Levels, __ExternalActors__, __ExternalObjects__.
    """
    vault_path = resolve_project_path(vault_root, project_root)

    if not vault_path.exists() or not vault_path.is_dir():
        return False

    existing_sources = {p.get("source_path") for p in manifest_data.get("asset_packages", []) if "source_path" in p}
    discovered_any = False

    for item in vault_path.iterdir():
        if item.is_dir():
            folder_name = item.name
            folder_lower = folder_name.lower()

            # Skip ignored directories
            if folder_lower in IGNORED_DIR_NAMES or folder_name.startswith("."):
                print(f"[AUTO-DISCOVERY] Skipping restricted folder '{folder_name}'.")
                continue

            if folder_name not in existing_sources:
                package_id = f"art_vault_{folder_lower.replace(' ', '_')}"
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


def ensure_suggested_vault_folders(vault_path, packages):
    """Auto-creates suggested vault package folders if they do not exist."""
    for pkg in packages:
        source_rel = pkg.get("source_path", "")
        if source_rel:
            pkg_src = vault_path / source_rel
            if not pkg_src.exists():
                try:
                    os.makedirs(pkg_src, exist_ok=True)
                    print(f"[INIT] Auto-created suggested vault directory: {pkg_src}")
                except Exception as e:
                    print(f"[WARNING] Could not create suggested folder '{pkg_src}': {e}")


def is_file_ignored(file_path):
    """
    Returns True if file or any parent folder in file_path should be excluded from sync.
    """
    path_obj = Path(file_path)
    file_name_lower = path_obj.name.lower()

    # Check extension
    for ext in IGNORED_FILE_EXTENSIONS:
        if file_name_lower.endswith(ext):
            return True

    # Check directory parts
    for part in path_obj.parts:
        if part.lower() in IGNORED_DIR_NAMES:
            return True

    return False


def sync_directory_pair(source_dir, target_dir, copy_mode=True):
    """
    Synchronizes files bidirectionally between source_dir and target_dir based on timestamps.
    Copies newer or missing files from source -> target AND target -> source (auto-vault backup).
    """
    os.makedirs(source_dir, exist_ok=True)
    os.makedirs(target_dir, exist_ok=True)

    synced_forward = 0
    synced_reverse = 0
    skipped_count = 0

    # 1. Forward Sync: Source (Vault) -> Target (Content/Assets/External)
    if source_dir.is_dir():
        for root, dirs, files in os.walk(source_dir):
            dirs[:] = [d for d in dirs if d.lower() not in IGNORED_DIR_NAMES and not d.startswith(".")]
            rel_dir = Path(root).relative_to(source_dir)
            dest_dir = target_dir / rel_dir

            for file_name in files:
                src_file = Path(root) / file_name
                if is_file_ignored(src_file):
                    skipped_count += 1
                    continue

                os.makedirs(dest_dir, exist_ok=True)
                dest_file = dest_dir / file_name

                if not dest_file.exists() or src_file.stat().st_mtime > dest_file.stat().st_mtime:
                    if copy_mode:
                        shutil.copy2(src_file, dest_file)
                    else:
                        if dest_file.exists():
                            dest_file.unlink()
                        os.symlink(src_file.resolve(), dest_file)
                    synced_forward += 1

    # 2. Reverse Sync: Target (Content/Assets/External) -> Source (Vault)
    if target_dir.is_dir():
        for root, dirs, files in os.walk(target_dir):
            dirs[:] = [d for d in dirs if d.lower() not in IGNORED_DIR_NAMES and not d.startswith(".")]
            rel_dir = Path(root).relative_to(target_dir)
            dest_dir = source_dir / rel_dir

            for file_name in files:
                src_file = Path(root) / file_name
                if is_file_ignored(src_file):
                    skipped_count += 1
                    continue

                os.makedirs(dest_dir, exist_ok=True)
                dest_file = dest_dir / file_name

                if not dest_file.exists() or src_file.stat().st_mtime > dest_file.stat().st_mtime:
                    if copy_mode:
                        shutil.copy2(src_file, dest_file)
                    else:
                        if dest_file.exists():
                            dest_file.unlink()
                        os.symlink(src_file.resolve(), dest_file)
                    synced_reverse += 1

    return synced_forward, synced_reverse, skipped_count


def sync_package(vault_root, package, project_root, copy_mode=True):
    """Syncs an individual asset package bidirectionally between vault and target destination."""
    package_id = package.get("package_id", "unknown")
    name = package.get("name", package_id)
    source_rel = package.get("source_path", "")
    target_rel = package.get("target_destination", "")
    enabled = package.get("enabled", True)

    if not enabled:
        print(f"[SKIP] Package '{name}' ({package_id}) is disabled in manifest.")
        return True

    candidate_vault_src = resolve_project_path(Path(vault_root) / source_rel, project_root)
    candidate_direct_src = resolve_project_path(source_rel, project_root)

    if candidate_vault_src.exists() or not candidate_direct_src.exists():
        source_path = candidate_vault_src
    else:
        source_path = candidate_direct_src

    target_path = resolve_project_path(target_rel, project_root)

    print(f"\n[SYNC] Processing Package: {name}")
    print(f"       Vault Source : {source_path}")
    print(f"       Engine Target: {target_path}")

    if source_path.resolve() == target_path.resolve():
        print(f"[NOTICE] Package source and target are identical directory ({source_path.resolve()}).")
        return True

    forward, reverse, skipped = sync_directory_pair(source_path, target_path, copy_mode=copy_mode)

    status_msg = f"[SUCCESS] Package '{name}' synced ("
    details = []
    if forward > 0:
        details.append(f"{forward} copied to engine")
    if reverse > 0:
        details.append(f"{reverse} backed up to vault")
    if forward == 0 and reverse == 0:
        details.append("0 files updated, up to date")
    if skipped > 0:
        details.append(f"{skipped} restricted files skipped")

    status_msg += ", ".join(details) + ")."
    print(status_msg)
    return True


def run_sync(manifest_path="asset_manifest.json", vault_override=None, copy_mode=True):
    """Executes the full bidirectional asset synchronization process."""
    project_name, project_root = detect_uproject()

    manifest = load_manifest(manifest_path)
    if not manifest:
        return {"status": "error", "message": "Failed to load manifest."}

    if manifest.get("project_name") == "auto":
        manifest["project_name"] = project_name

    vault_root = vault_override or manifest.get("local_vault_root", "Content/ArtVault")
    vault_path = resolve_project_path(vault_root, project_root)

    os.makedirs(vault_path, exist_ok=True)

    packages = manifest.get("asset_packages", [])

    ensure_suggested_vault_folders(vault_path, packages)
    auto_discover_packages(manifest_path, manifest, vault_root, project_root)

    print(f"=== Sovereign Framework Bidirectional Art Asset Sync ===")
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
    parser = argparse.ArgumentParser(description="Synchronize local art assets bidirectionally into Unreal Engine Content directory.")
    parser.add_argument("--manifest", default="asset_manifest.json", help="Path to asset_manifest.json")
    parser.add_argument("--vault", default=None, help="Override local asset vault root folder")
    parser.add_argument("--symlink", action="store_true", help="Use symlinks instead of copying files")

    args = parser.parse_args()
    run_sync(manifest_path=args.manifest, vault_override=args.vault, copy_mode=not args.symlink)
