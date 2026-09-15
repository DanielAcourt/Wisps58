# Sovereign Framework: Local Art Asset Management & Git Strategy SOP
**Version:** 37.0.0
**Copyright:** (c) 2013-2026 Daniel Acourt. Licensed under GPLv3.
**Last Updated:** August 2026

---

## 🏛️ Executive Summary & Architectural Rationale

In game development using Unreal Engine 5, binary art assets (`.uasset`, `.umap`, high-resolution textures, skeletal meshes, audio banks) rapidly swell repository size into tens or hundreds of gigabytes. Storing these binary assets directly inside a Git repository leads to severe performance degradation (`git clone`/`git pull` slowdowns, merge conflicts on binary files, and repository bloat).

The **Sovereign Framework Local Art Asset Strategy** enforces a strict **Separation of Intent**:
1. **Git Source Code Repository = The DNA / Blueprint**: Stores C++ source files, Python scripts, configuration `.ini`s, JSON schemas, level files (`.umap`), and lightweight stub blueprints (< 50 MB total).
2. **Local Asset Vault = The Physical Flesh**: Stores heavy `.uasset` / `.umap` art files outside Git control (e.g., `WispsCPPVR/Content/ArtVault/` or external drive `D:/Sovereign_Art_Vault/`).
3. **Asset Manifest (`asset_manifest.json`) + Sync Script (`sync_art_assets.py` / `sync_art_assets.bat` / `Plugins/SovereignArtSync`) = The Bridge**: Programmatically maps, auto-discovers, and synchronizes external art assets into `WispsCPPVR/Content/Assets/External/`.

---

## 📁 Repository Exclusion Rules (`.gitignore`)

The following directory patterns are ignored by Git to prevent binary art files, temporary backup files, and Visual Studio/Unreal build outputs from bloating the code repository:

```gitignore
# External Art Assets & Local Vault directories
WispsCPPVR/Content/Assets/External/
WispsCPPVR/Content/ArtVault/
WispsCPPVR/Content/LevelPrototyping/External/
*.uasset.bak
*.umap.bak

# Visual Studio & C++ IDE Transient Files
.vs/
*.vsidx
*.VC.db*
v17/
v18/
.vsconfig

# Unreal Engine Generated & Intermediate Folders
Binaries/
DerivedDataCache/
Intermediate/
Saved/
Build/
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
      "package_id": "art_vault_meshes",
      "name": "Art Vault Static & Skeletal Meshes",
      "version": "1.0.0",
      "source_path": "Meshes",
      "target_destination": "WispsCPPVR/Content/Assets/External/ArtVault/Meshes",
      "enabled": true,
      "description": "3D meshes stored in local ArtVault/Meshes."
    },
    {
      "package_id": "art_vault_textures",
      "name": "Art Vault High-Res Textures",
      "version": "1.0.0",
      "source_path": "Textures",
      "target_destination": "WispsCPPVR/Content/Assets/External/ArtVault/Textures",
      "enabled": true,
      "description": "Texture maps stored in local ArtVault/Textures."
    },
    {
      "package_id": "art_vault_materials",
      "name": "Art Vault Materials & Shader Instances",
      "version": "1.0.0",
      "source_path": "Materials",
      "target_destination": "WispsCPPVR/Content/Assets/External/ArtVault/Materials",
      "enabled": true,
      "description": "Materials and shaders stored in local ArtVault/Materials."
    }
  ]
}
```

---

## ☀️ Day-to-Day Developer & Artist Workflow (Step-by-Step)

Here is exactly what happens when you create a level, import art assets, and run synchronization day-to-day:

### 1. Adding Pre-Compiled Art Assets to the Vault
- Drop `.uasset` folders into `WispsCPPVR/Content/ArtVault/` (or subfolders like `Meshes`, `Textures`, `Materials`, `Props`).
- Trigger sync via **`sync_art_assets.bat`** in terminal or via **EUW Panel** in Unreal Engine (`execute_artist_friendly_sync()`).
  - *What happens under the hood?* Auto-discovery detects new subfolders, registers them in `asset_manifest.json`, and synchronizes files into `WispsCPPVR/Content/Assets/External/ArtVault/`.

### 2. Importing Raw PNG / FBX Files in Unreal Engine
- Drag raw `.png` textures or `.fbx` models into Unreal Engine's Content Browser.
- Run `execute_artist_friendly_sync()` via your **Editor Utility Widget (EUW)** button:
  - **Automatic Pre-Save Hook:** The plugin automatically calls `unreal.EditorLoadingAndSavingUtils.save_dirty_packages()` to commit newly imported assets as binary `.uasset` files on disk.
  - **Vault Backup:** The saved `.uasset` files are backed up bidirectionally to `Content/ArtVault/`.
  - **UI Status Display:** The widget immediately reports:
    > `"Successfully added 3 object(s) to the Art Vault and updated 0 engine asset(s)."`

### 3. Assigning Assets & Saving Levels (`.umap` & `__ExternalActors__`)
- Open **Unreal Engine 5.8**.
- In Content Browser, go to `Content -> Assets -> External -> ArtVault/`.
- Drag Static Meshes into the viewport and assign materials.
- Click **File -> Save Current Level** (`Ctrl + S`).
- Unreal Engine records the 3D spatial transforms and asset path references inside the level map file (`.umap`) and `__ExternalActors__` metadata folder.

### 4. Committing Code & Levels to Git
- Run `git status` in terminal or source control GUI.
- **Notice what Git sees:**
  - `TestWorld.umap` (Tracked - level layout and object placements).
  - `asset_manifest.json` (Tracked - active package mappings).
  - C++ source files, blueprints, and configs (Tracked).
- **Notice what Git IGNORES:**
  - Heavy `.uasset` binary meshes, materials, and textures in `Content/Assets/External/` (Ignored).
  - Raw vault files in `Content/ArtVault/` (Ignored).
  - Autosaves, `.bak` files, `.vsidx`, and `Intermediate/` (Ignored).
- Run `git add .` and `git commit -m "build: designed new level layout in TestWorld"`.

---

## 🔄 How Another Developer or Machine Recreates the Game Environment

1. **Clone/Pull Code**: The teammate runs `git pull`. They receive the `.umap` level file, C++ logic, and `asset_manifest.json`.
2. **Sync Local Vault**: They place the `ArtVault` folder into their local machine and run `sync_art_assets.bat` or `execute_artist_friendly_sync()`.
3. **Launch UE 5.8**: Launch the project. Unreal Engine opens `TestWorld.umap`, resolves the local asset path `/Game/Assets/External/ArtVault/Meshes/MyMesh`, and renders the full 3D environment instantly!
