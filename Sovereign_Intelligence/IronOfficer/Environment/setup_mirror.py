# Copyright (c) 2013-2025 Daniel Acourt. Version 0.36.3.1MABackup.
"""
Iron Officer: Mirror Nexus Setup
Creates a lean, clean local Nexus mirror for the Iron Officer's environment.
"""

import os
import shutil

# Paths relative to repo root
SOURCE_NEXUS = "AI_Nexus"
TARGET_ENV = "Sovereign_Intelligence/IronOfficer/Environment/Mirror_Nexus"

def setup_mirror():
    print(f"[07] Initializing Mirror Nexus at: {TARGET_ENV}")

    # Core directories to mirror
    subdirs = ["Admin", "Protocols", "Identity", "Docs"]

    if not os.path.exists(TARGET_ENV):
        os.makedirs(TARGET_ENV)

    for folder in subdirs:
        src = os.path.join(SOURCE_NEXUS, folder)
        dst = os.path.join(TARGET_ENV, folder)

        if os.path.exists(src):
            print(f"[07] Mirroring {folder}...")
            if os.path.exists(dst):
                shutil.rmtree(dst)
            shutil.copytree(src, dst)
        else:
            print(f"[07 WARNING] Source {src} not found.")

    # Create a local README for the mirror
    with open(os.path.join(TARGET_ENV, "README.md"), "w") as f:
        f.write("# Sovereign Iron Officer: Mirror Nexus\n\n")
        f.write("This is a local, lean mirror of the AI_Nexus. It provides the Iron Officer with immediate local access to core protocols and identity markers.\n")
        f.write("Sync Status: NOMINAL\n")

    print("[07] Mirror Nexus Setup Complete.")

if __name__ == "__main__":
    setup_mirror()
