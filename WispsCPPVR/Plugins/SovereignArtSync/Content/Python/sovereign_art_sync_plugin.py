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


def execute_1click_sync(manifest_path="asset_manifest.json", vault_override=None):
    """
    Called by Editor Utility Widget / Toolbar for 1-click synchronization.
    Returns formatted log string for UI display.
    """
    res = engine.run_sync(manifest_path=manifest_path, vault_override=vault_override, copy_mode=True, dry_run=False)

    if res.get("status") == "error":
        return f"[ERROR] {res.get('message')}"

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
    try:
        import unreal
        unreal.log(output)
    except ImportError:
        print(output)

    return output


def execute_dry_run_autodiscover(manifest_path="asset_manifest.json", vault_override=None):
    """
    Called by Editor Utility Widget to dry run and detect new packages without making disk writes.
    Returns formatted log string.
    """
    res = engine.run_sync(manifest_path=manifest_path, vault_override=vault_override, copy_mode=True, dry_run=True)

    if res.get("status") == "error":
        return f"[ERROR] {res.get('message')}"

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
    try:
        import unreal
        unreal.log(output)
    except ImportError:
        print(output)

    return output


if __name__ == "__main__":
    print(execute_1click_sync())
