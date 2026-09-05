# sovereign-art-sync: Day-to-Day User Guide & Developer Workflow
**Version:** 1.0.1
**License:** MIT License

---

## 🏛️ Executive Summary & Strategy

In Unreal Engine 5 development, binary art assets (`.uasset`, high-resolution textures, skeletal meshes, audio banks) quickly swell repository size into tens or hundreds of gigabytes. Storing binary assets directly inside a Git repository causes severe performance issues (`git clone`/`git pull` slowdowns, merge conflicts on binary files, and repository bloat).

`sovereign-art-sync` enforces a strict **Separation of Intent**:
1. **Git Source Repository = Blueprint / DNA / Levels**: Stores C++ source files, Python scripts, configuration `.ini` files, JSON schemas, level map layout files (`.umap`), and lightweight stub blueprints.
2. **Local Asset Vault = Physical Flesh**: Stores heavy `.uasset` static meshes, materials, textures, and audio files locally outside Git control (`Content/ArtVault/` or external drives).
3. **Asset Manifest (`asset_manifest.json`) + Sync Engine (`sync_art_assets.py` / `.bat`) = The Bridge**: Programmatically maps, auto-discovers, and synchronizes external art assets into `Content/Assets/External/`.

---

## ⚠️ CRITICAL ARCHITECTURAL RULE: WHERE TO SAVE LEVELS (`.umap`)

> **NEVER SAVE LEVEL MAPS (`.umap`) INSIDE `Content/ArtVault/` OR `Content/Assets/External/`!**

### Why?
In Unreal Engine 5 (especially with World Partition enabled), saving a level map inside `Content/ArtVault/` causes UE5 to generate level metadata and actor data files (`__ExternalActors__` and `__ExternalObjects__`). If these are placed in the vault and synced into `Content/Assets/External/`:
1. Unreal Engine will detect duplicate level packages at runtime (`/Game/ArtVault/...` vs `/Game/Assets/External/...`).
2. Unreal Engine will re-assign new unique GUIDs to all actors every single time you open the project.
3. Your Git repository will flood with dozens of untracked `.uasset` files under `__ExternalActors__`.

### Correct Level Placement:
* **Always save levels in `Content/Maps/` or `Content/Levels/`** directly under project `Content/` (e.g. `Content/Maps/TestLevel.umap`).
* `sync_art_assets.py` automatically ignores `.umap` files, `__ExternalActors__`, and `__ExternalObjects__` as a safety guardrail.

---

## ☀️ Day-to-Day Developer & Artist Workflow (Step-by-Step)

Here is the exact step-by-step process when adding assets, building levels, and committing code:

### Step 1: Adding New Art Assets to the Vault
- Acquire or export 3D meshes, textures, materials, or audio banks.
- Drop them into `Content/ArtVault/` (e.g. `Content/ArtVault/Meshes/`, `Content/ArtVault/Textures/`, `Content/ArtVault/Props/`).
- Double-click **`sync_art_assets.bat`** in your project root.
  - *What happens under the hood?* The auto-discovery engine detects any new subfolder, updates `asset_manifest.json`, and copies/syncs the binary art files into `Content/Assets/External/ArtVault/`.

### Step 2: Opening Unreal Engine 5 & Placing Assets
- Launch **Unreal Engine 5**.
- In the Content Browser, navigate to `Content -> Assets -> External -> ArtVault/`.
- Drag your Static Mesh into the 3D viewport.
- Assign materials or textures to the mesh in the Details Panel.
- Position, rotate, and scale objects to design your level.

### Step 3: Saving the Level (`.umap` & `__ExternalActors__`)
- Click **File -> Save Current Level** into **`Content/Maps/MainLevel.umap`** (NOT inside `ArtVault/`!).
- Unreal Engine records the 3D Spatial Transform (Location, Rotation, Scale) and Asset Reference path strings (`"/Game/Assets/External/ArtVault/Meshes/MyMesh.MyMesh"`) inside the level map file (`.umap`) and `Content/__ExternalActors__/`.

### Step 4: Committing Code & Levels to Git
- Open terminal or your Git client.
- **Notice what Git tracks:**
  - `Content/Maps/MainLevel.umap` (Tracked - level layout and object placements).
  - `Content/__ExternalActors__/` (Tracked - World Partition actor metadata for `MainLevel`).
  - `asset_manifest.json` (Tracked - lists active package mappings).
  - C++ source files, blueprints, and configs (Tracked).
- **Notice what Git IGNORES:**
  - Heavy `.uasset` binary meshes and textures in `Content/Assets/External/` (Ignored).
  - Raw vault files in `Content/ArtVault/` (Ignored).
  - Autosaves and build files (Ignored).
- Commit your changes:
  ```bash
  git add .
  git commit -m "build: designed level layout in MainLevel"
  ```

---

## 🔄 How Teammates Recreate the Game Environment

1. **Pull Code**: Teammates run `git pull`. They receive level files (`.umap`), C++ logic, and `asset_manifest.json`.
2. **Populate Local Vault**: They place asset files into their local `Content/ArtVault/` folder and double-click `sync_art_assets.bat`.
3. **Launch UE5**: Open Unreal Engine 5. UE5 opens `MainLevel.umap`, resolves `/Game/Assets/External/ArtVault/Meshes/MyMesh`, and renders the full 3D level environment instantly!
