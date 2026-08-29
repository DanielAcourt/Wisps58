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
3. **Asset Manifest (`asset_manifest.json`) + Sync Script (`sync_art_assets.py` / `sync_art_assets.bat`) = The Bridge**: Programmatically maps, auto-discovers, and synchronizes external art assets into `WispsCPPVR/Content/Assets/External/`.

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

## ☀️ Day-to-Day Developer Workflow (Step-by-Step)

Here is exactly what happens when you create a level and add art assets day-to-day:

### 1. Adding New Art Assets to the Vault
- You acquire or export a 3D mesh, texture, or material.
- Drop it into `WispsCPPVR/Content/ArtVault/` (or a subfolder inside it, like `Meshes`, `Textures`, `Materials`, or `Props`).
- Double-click **`sync_art_assets.bat`** in the repository root.
  - *What happens under the hood?* The auto-discovery script detects any new subfolder, updates `asset_manifest.json`, and copies/syncs the files into `WispsCPPVR/Content/Assets/External/ArtVault/`.

### 2. Opening Unreal Engine & Assigning Assets in the Viewport
- Open **Unreal Engine 5.8**.
- In the Content Browser, go to `Content -> Assets -> External -> ArtVault/`.
- Drag your Static Mesh into the 3D viewport (spawning a `StaticMeshActor`).
- Assign materials/textures to the mesh component in the Details Panel.
- Position, scale, and rotate your objects to design your level.

### 3. Saving the Level (`.umap` & `__ExternalActors__`)
- Click **File -> Save Current Level** (e.g. `WispsCPPVR/Content/TestWorld.umap`).
- Unreal Engine records the **3D Spatial Transform** (Location, Rotation, Scale) and **Asset References** (e.g., path reference string `"/Game/Assets/External/ArtVault/Meshes/MyMesh.MyMesh"`) inside the level map file (`.umap`) and `__ExternalActors__` folder.

### 4. Committing Code & Levels to Git
- Run `git status` in terminal or source control GUI.
- **Notice what Git sees:**
  - `TestWorld.umap` (Tracked - level layout and object placements).
  - `asset_manifest.json` (Tracked - lists active art package mappings).
  - C++ source files, blueprints, and configs (Tracked).
- **Notice what Git IGNORES:**
  - Heavy `.uasset` binary meshes, materials, and textures in `Content/Assets/External/` (Ignored).
  - Raw vault files in `Content/ArtVault/` (Ignored).
  - Autosaves, `.bak` files, `.vsidx`, and `Intermediate/` (Ignored).
- Run `git add .` and `git commit -m "build: designed new level layout in TestWorld"`.

---

## 🔄 How Another Developer or Machine Recreates the Game Environment

1. **Clone/Pull Code**: The teammate runs `git pull`. They receive the `.umap` level file, C++ logic, and `asset_manifest.json`.
2. **Sync Local Vault**: They place the `ArtVault` folder into their local machine (or point `asset_manifest.json` to their external drive) and double-click `sync_art_assets.bat`.
3. **Launch UE 5.8**: Launch the project. Unreal Engine opens `TestWorld.umap`, resolves the local asset path `/Game/Assets/External/ArtVault/Meshes/MyMesh`, and renders the full 3D environment instantly!
