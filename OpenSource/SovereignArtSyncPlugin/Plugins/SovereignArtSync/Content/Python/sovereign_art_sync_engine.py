#!/usr/bin/env python3
"""
sovereign-art-sync: Core Sync Engine for Unreal Engine 5 Plugin
Copyright (c) 2026 Daniel Acourt. Version 1.0.0. Licensed under the MIT License.
"""

import os
import sys
import json
import shutil
from pathlib import Path

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
    ".umap",         # Level maps
    ".umap.bak",     # Level backups
    ".uasset.bak",   # Asset backups
    ".tmp",          # Temporary files
    ".log"           # Log files
}


def detect_uproject(root_dir="."):
    """Scans for .uproject files to resolve project context."""
    # 1. Try Unreal Engine Editor API if available
    try:
        import unreal
        proj_file = unreal.Paths.get_project_file_path()
        if proj_file:
            proj_path = Path(proj_file).resolve()
            if proj_path.suffix == ".uproject":
                return proj_path.stem, proj_path.parent

        proj_dir = unreal.Paths.project_dir()
        if proj_dir:
            proj_path = Path(proj_dir).resolve()
            uprojects = list(proj_path.glob("*.uproject"))
            if uprojects:
                return uprojects[0].stem, proj_path
            return proj_path.name, proj_path
    except Exception:
        pass

    # 2. Fallback to filesystem search
    root_path = Path(root_dir).resolve()

    uproject_files = list(root_path.glob("*.uproject"))
    if uproject_files:
        return uproject_files[0].stem, root_path

    for child in root_path.iterdir():
        if child.is_dir():
            child_uprojects = list(child.glob("*.uproject"))
            if child_uprojects:
                return child_uprojects[0].stem, child

    parent_path = root_path.parent
    parent_uprojects = list(parent_path.glob("*.uproject"))
    if parent_uprojects:
        return parent_uprojects[0].stem, parent_path

    return "UnrealProject", root_path


def resolve_project_path(path_str, project_root):
    """Resolves relative or absolute path against project_root."""
    p = Path(path_str)
    if p.is_absolute():
        return p

    proj_path = Path(project_root).resolve()
    if p.parts and p.parts[0] == proj_path.name:
        p = Path(*p.parts[1:])

    return (proj_path / p).resolve()


def load_manifest(manifest_path="asset_manifest.json"):
    """Loads and validates asset manifest JSON."""
    manifest_file = Path(manifest_path)
    if not manifest_file.exists():
        return None

    try:
        with open(manifest_file, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return None


def auto_discover_packages(manifest_path, manifest_data, vault_root, project_root):
    """Auto-registers unregistered vault subdirectories into manifest."""
    vault_path = resolve_project_path(vault_root, project_root)
    if not vault_path.exists() or not vault_path.is_dir():
        return False

    existing_sources = {p.get("source_path") for p in manifest_data.get("asset_packages", []) if "source_path" in p}
    discovered_any = False

    for item in vault_path.iterdir():
        if item.is_dir():
            folder_name = item.name
            folder_lower = folder_name.lower()

            if folder_lower in IGNORED_DIR_NAMES or folder_name.startswith("."):
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

    if discovered_any:
        try:
            with open(manifest_path, "w", encoding="utf-8") as f:
                json.dump(manifest_data, f, indent=2)
        except Exception:
            pass

    return discovered_any


def is_file_ignored(file_path):
    """Checks whether a file should be excluded from sync."""
    path_obj = Path(file_path)
    file_name_lower = path_obj.name.lower()

    for ext in IGNORED_FILE_EXTENSIONS:
        if file_name_lower.endswith(ext):
            return True

    for part in path_obj.parts:
        if part.lower() in IGNORED_DIR_NAMES:
            return True

    return False


def sync_directory_pair(source_dir, target_dir, copy_mode=True, dry_run=False):
    """Synchronizes files bidirectionally based on timestamps."""
    if not dry_run:
        os.makedirs(source_dir, exist_ok=True)
        os.makedirs(target_dir, exist_ok=True)

    synced_forward = 0
    synced_reverse = 0
    skipped_count = 0

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

                dest_file = dest_dir / file_name

                if not dest_file.exists() or src_file.stat().st_mtime > dest_file.stat().st_mtime:
                    if not dry_run:
                        os.makedirs(dest_dir, exist_ok=True)
                        if copy_mode:
                            shutil.copy2(src_file, dest_file)
                            synced_forward += 1
                        else:
                            try:
                                if dest_file.exists():
                                    dest_file.unlink()
                                os.symlink(src_file.resolve(), dest_file)
                                synced_forward += 1
                            except OSError:
                                pass
                    else:
                        synced_forward += 1

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

                dest_file = dest_dir / file_name

                if not dest_file.exists() or src_file.stat().st_mtime > dest_file.stat().st_mtime:
                    if not dry_run:
                        os.makedirs(dest_dir, exist_ok=True)
                        if copy_mode:
                            shutil.copy2(src_file, dest_file)
                            synced_reverse += 1
                        else:
                            try:
                                if dest_file.exists():
                                    dest_file.unlink()
                                os.symlink(src_file.resolve(), dest_file)
                                synced_reverse += 1
                            except OSError:
                                pass
                    else:
                        synced_reverse += 1

    return synced_forward, synced_reverse, skipped_count


def run_sync(manifest_path="asset_manifest.json", vault_override=None, copy_mode=True, dry_run=False):
    """Executes full synchronization process and returns detailed result dict."""
    project_name, project_root = detect_uproject()

    resolved_manifest = resolve_project_path(manifest_path, project_root)
    manifest = load_manifest(resolved_manifest)
    if not manifest:
        return {"status": "error", "message": f"Failed to load manifest at {resolved_manifest}"}

    vault_root = vault_override or manifest.get("local_vault_root", "Content/ArtVault")
    vault_path = resolve_project_path(vault_root, project_root)

    if not dry_run:
        os.makedirs(vault_path, exist_ok=True)

    packages = manifest.get("asset_packages", [])
    auto_discover_packages(resolved_manifest, manifest, vault_root, project_root)

    results = []
    total_forward = 0
    total_reverse = 0

    for pkg in packages:
        source_rel = pkg.get("source_path", "")
        target_rel = pkg.get("target_destination", "")

        src_path = resolve_project_path(Path(vault_path) / source_rel, project_root)
        tgt_path = resolve_project_path(target_rel, project_root)

        fwd, rev, skip = sync_directory_pair(src_path, tgt_path, copy_mode=copy_mode, dry_run=dry_run)
        total_forward += fwd
        total_reverse += rev

        results.append({
            "package": pkg.get("name", pkg.get("package_id")),
            "forward": fwd,
            "reverse": rev,
            "skipped": skip
        })

    return {
        "status": "success",
        "project_name": project_name,
        "total_packages": len(packages),
        "total_forward": total_forward,
        "total_reverse": total_reverse,
        "dry_run": dry_run,
        "package_results": results
    }
