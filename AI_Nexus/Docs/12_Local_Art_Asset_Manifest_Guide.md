# Sovereign Framework: Local Art Asset Management & Git Strategy SOP
**Version:** 37.0.0
**Copyright:** (c) 2013-2026 Daniel Acourt. Licensed under GPLv3.
**Last Updated:** August 2026

---

## 🏛️ Executive Summary & Architectural Rationale

In game development using Unreal Engine 5, binary art assets (`.uasset`, `.umap`, high-resolution textures, skeletal meshes, audio banks) rapidly swell repository size into tens or hundreds of gigabytes. Storing these binary assets directly inside a Git repository leads to severe performance degradation (`git clone`/`git pull` slowdowns, merge conflicts on binary files, and repository bloat).

The **Sovereign Framework Local Art Asset Strategy** resolves this by enforcing a strict **Separation of Intent**:
1. **Git Source Code Repository = The DNA / Blueprint**: Stores C++ source files, Python scripts, configuration `.ini`s, JSON schemas, and lightweight C++ stub blueprints (< 50 MB total).
2. **Local Asset Vault = The Physical Flesh**: Stores heavy `.uasset` / `.umap` art files outside Git control (e.g., `WispsCPPVR/Content/ArtVault/` or external drive `D:/Sovereign_Art_Vault/`).
3. **Asset Manifest (`asset_manifest.json`) + Sync Script (`sync_art_assets.py`) = The Bridge**: Programmatically maps, validates, and synchronizes external art assets into `WispsCPPVR/Content/Assets/External/`.

---

## 📁 Repository Exclusion Rules (`.gitignore`)

The following directory patterns are ignored by Git to prevent binary art files from bloating the code repository:

```gitignore
# External Art Assets & Local Vault directories
WispsCPPVR/Content/Assets/External/
WispsCPPVR/Content/ArtVault/
WispsCPPVR/Content/LevelPrototyping/External/
*.uasset.bak
*.umap.bak
```

---

## 📜 Asset Manifest Schema (`asset_manifest.json`)

`asset_manifest.json` located at the root of the repository tracks all external art asset packages required by the project:

```json
{
  "manifest_version": "1.0.0",
  "project_name": "WispsCPPVR",
  "description": "Local Art Asset Manifest mapping external binary asset packages to local Unreal Engine Content directories.",
  "local_vault_root": "WispsCPPVR/Content/ArtVault",
  "asset_packages": [
    {
      "package_id": "env_prototypes_v1",
      "name": "Environment Prototypes Package",
      "version": "1.0.0",
      "source_path": "Environment_Prototypes",
      "target_destination": "WispsCPPVR/Content/Assets/External/Environment",
      "enabled": true,
      "description": "High-fidelity environment meshes and textures kept outside Git source control."
    },
    {
      "package_id": "character_models_v1",
      "name": "Character Models & Animations Package",
      "version": "1.0.0",
      "source_path": "Characters",
      "target_destination": "WispsCPPVR/Content/Assets/External/Characters",
      "enabled": true,
      "description": "3D character models, rigs, and external animation sequences."
    }
  ]
}
```

---

## 🛠️ Usage & Operations Guide

### 1. Synchronizing Local Art Assets (One-Click Batch Script or CLI)
Simply double-click `sync_art_assets.bat` in the repository root directory!

Or run via command line:
```bash
# Standard copy mode using default manifest and vault root
python WispsCPPVR/Content/Python/sync_art_assets.py

# Specify custom asset vault location (e.g. external SSD)
python WispsCPPVR/Content/Python/sync_art_assets.py --vault "D:/Sovereign_Art_Vault"

# Fast symbolic link mode (requires Administrator / Developer Mode on Windows)
python WispsCPPVR/Content/Python/sync_art_assets.py --symlink
```

### 2. Synchronizing via Unreal Engine Python / MCP Subsystem
Inside Unreal Engine 5.8 or via the Python MCP Bridge:

```python
import sovereign_mcp_tools
result = sovereign_mcp_tools.sync_local_art_assets("asset_manifest.json")
```

---

## 🔄 How Another Developer or Machine Recreates the Game Environment

1. **Clone Code**: The developer clones the Git repository (`git clone ...`). They instantly receive all C++ classes, Python bridges, logic, and configurations.
2. **Obtain Art Vault**: Provide the `ArtVault` folder (via local transfer, USB drive, or network share) containing the `.uasset` folders.
3. **Execute Sync Script**: Run `python WispsCPPVR/Content/Python/sync_art_assets.py`.
4. **Launch UE 5.8**: Launch `WispsCPPVR.uproject`. Unreal Engine recognizes the synced assets in `Content/Assets/External/` without any missing asset errors or broken references.
