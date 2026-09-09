#!/usr/bin/env python3
"""
Fab Marketplace Plugin Bundler Script for Sovereign Art Sync
Copyright (c) 2026 Daniel Acourt. Version 1.0.0.
"""

import os
import sys
import zipfile
from pathlib import Path

def create_fab_package():
    script_dir = Path(__file__).resolve().parent
    output_zip = script_dir / "SovereignArtSync_v1.0.0_Fab.zip"

    files_to_include = [
        "README.md",
        "USER_GUIDE.md",
        "LICENSE",
        "Fab_Marketplace_Metadata.json"
    ]

    plugin_dir = script_dir / "Plugins" / "SovereignArtSync"

    if not plugin_dir.exists():
        print(f"[ERROR] Plugin directory not found: {plugin_dir}")
        return False

    with zipfile.ZipFile(output_zip, "w", zipfile.ZIP_DEFLATED) as zipf:
        # Add root documentation files
        for fname in files_to_include:
            fpath = script_dir / fname
            if fpath.exists():
                zipf.write(fpath, arcname=fname)

        # Add Plugins/SovereignArtSync directory recursively
        for root, dirs, files in os.walk(plugin_dir):
            for file_name in files:
                full_path = Path(root) / file_name
                rel_path = full_path.relative_to(script_dir)
                zipf.write(full_path, arcname=str(rel_path))

    print(f"[SUCCESS] Created Fab Marketplace zip bundle: {output_zip}")
    print(f"          Size: {output_zip.stat().st_size} bytes")
    return True

if __name__ == "__main__":
    create_fab_package()
