#!/usr/bin/env python3
"""
sovereign-art-sync: Unreal Editor Bridge & EUW Interface
Copyright (c) 2026 Daniel Acourt. Version 1.0.0. Licensed under the MIT License.
"""

import sys
from pathlib import Path

# Add current folder to sys.path
current_dir = Path(__file__).resolve().parent
if str(current_dir) not in sys.path:
    sys.path.insert(0, str(current_dir))

import sovereign_art_sync_engine as engine

LAST_SYNC_LOG = "No sync operation executed yet."


def save_dirty_packages_before_sync():
    """
    Saves all unsaved (dirty) packages in Unreal Editor before synchronizing.
    Guarantees that binary files on disk reflect the active editor state.
    """
    try:
        import unreal
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(show_dialog_if_dirty=False, save_map_packages=False)
        unreal.log("🏛️ Sovereign Art Sync: Auto-saved dirty asset packages prior to sync.")
    except Exception as e:
        pass


def get_last_sync_log():
    """Returns the latest sync status report string for UI display."""
    global LAST_SYNC_LOG
    return LAST_SYNC_LOG


def execute_1click_sync(manifest_path="asset_manifest.json", vault_override=None, auto_save=True):
    """
    Called by Editor Utility Widget / Toolbar for 1-click synchronization.
    Saves dirty packages, runs sync, and returns formatted log string for UI display.
    """
    global LAST_SYNC_LOG
    if auto_save:
        save_dirty_packages_before_sync()

    res = engine.run_sync(manifest_path=manifest_path, vault_override=vault_override, copy_mode=True, dry_run=False)

    if res.get("status") == "error":
        output = f"[ERROR] {res.get('message')}"
    else:
        logs = [
            f"=== Sovereign Art Sync (1-Click Sync Complete) ===",
            f"Project Name   : {res.get('project_name')}",
            f"Packages Synced: {res.get('total_packages')}",
            f"Engine Copies  : {res.get('total_forward')} files",
            f"Vault Backups  : {res.get('total_reverse')} files\n"
        ]

        for pkg in res.get("package_results", []):
            logs.append(f"• {pkg['package']}: {pkg['forward']} copied to engine, {pkg['reverse']} backed up to vault.")

        output = "\n".join(logs)

    LAST_SYNC_LOG = output

    # Always print to sys.stdout so 'Execute Python Command Advanced' node captures log output in Blueprint
    print(output)
    try:
        import unreal
        if res.get("status") == "error":
            unreal.log_error(output)
        else:
            unreal.log(output)
    except ImportError:
        pass

    return output


def execute_artist_friendly_sync(manifest_path="asset_manifest.json", vault_override=None):
    """
    Artist-focused execution interface for Editor Utility Widgets (EUW).
    Saves unsaved packages, executes synchronization, and returns a concise single-line summary string:
    'Successfully added X object(s) to the Art Vault and updated Y engine asset(s).'
    """
    save_dirty_packages_before_sync()

    res = engine.run_sync(manifest_path=manifest_path, vault_override=vault_override, copy_mode=True, dry_run=False)

    if res.get("status") == "error":
        output = f"[ERROR] Sync Failed: {res.get('message')}"
    else:
        fwd = res.get("total_forward", 0)
        rev = res.get("total_reverse", 0)
        output = f"Successfully added {rev} object(s) to the Art Vault and updated {fwd} engine asset(s)."

    global LAST_SYNC_LOG
    LAST_SYNC_LOG = output

    print(output)
    try:
        import unreal
        if res.get("status") == "error":
            unreal.log_error(output)
        else:
            unreal.log(output)
    except ImportError:
        pass

    return output


def execute_dry_run_autodiscover(manifest_path="asset_manifest.json", vault_override=None):
    """
    Called by Editor Utility Widget to dry run and detect new packages without making disk writes.
    Returns formatted log string.
    """
    global LAST_SYNC_LOG
    res = engine.run_sync(manifest_path=manifest_path, vault_override=vault_override, copy_mode=True, dry_run=True)

    if res.get("status") == "error":
        output = f"[ERROR] {res.get('message')}"
    else:
        logs = [
            f"=== Sovereign Art Sync (Dry-Run & Auto-Discovery) ===",
            f"Project Name   : {res.get('project_name')}",
            f"Packages Found : {res.get('total_packages')}",
            f"Pending Copies : {res.get('total_forward')} files",
            f"Pending Backups: {res.get('total_reverse')} files\n"
        ]

        for pkg in res.get("package_results", []):
            logs.append(f"• {pkg['package']}: {pkg['forward']} pending forward, {pkg['reverse']} pending reverse.")

        output = "\n".join(logs)

    LAST_SYNC_LOG = output

    # Always print to sys.stdout so 'Execute Python Command Advanced' node captures log output in Blueprint
    print(output)
    try:
        import unreal
        if res.get("status") == "error":
            unreal.log_error(output)
        else:
            unreal.log(output)
    except ImportError:
        pass

    return output


if __name__ == "__main__":
    print(execute_artist_friendly_sync())
