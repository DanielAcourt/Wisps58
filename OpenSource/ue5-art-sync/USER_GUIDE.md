# sovereign-art-sync: Day-to-Day User Guide & Developer Workflow
**Version:** 1.0.1
**License:** MIT License

---

## 🏛️ Executive Summary & Strategy

In Unreal Engine 5 development, binary art assets (`.uasset`, high-resolution textures, skeletal meshes, audio banks) quickly swell repository size into tens or hundreds of gigabytes. Storing binary assets directly inside a Git repository causes severe performance issues (`git clone`/`git pull` slowdowns, merge conflicts on binary files, and repository bloat).

`sovereign-art-sync` enforces a strict **Separation of Intent**:
1. **Git Source Repository = Blueprint / DNA / Levels**: Stores C++ source files, Python scripts, configuration `.ini` files, JSON schemas, level map layout files (`.umap`), and lightweight stub blueprints.
2. **Local Asset Vault = Physical Flesh**: Stores heavy `.uasset` static meshes, materials, textures, and audio files locally outside Git control (`Content/ArtVault/` or external drives).
3. **Asset Manifest (`asset_manifest.json`) + Sync Engine (`sync_art_assets.py` / `.bat`) = The Bridge**: Programmatically maps, auto-discovers, and synchronizes external art assets bidirectionally into `Content/Assets/External/`.

---

## 🎨 Blender, 3D DCC & FBX Import Pipeline

When modeling assets in **Blender**, Maya, or 3ds Max:

### 1. Source FBX vs. Compiled Unreal `.uasset`
* **Raw DCC Export (`.fbx`)**: Blender exports raw 3D geometry as `.fbx` (e.g. `DragonEgg.fbx`). Unreal Engine cannot render `.fbx` files directly in a 3D level viewport.
* **Unreal Binary Asset (`.uasset`)**: When you import `DragonEgg.fbx` into Unreal Engine, Unreal Engine converts it into a binary `.uasset` static mesh (`DragonEgg.uasset`).
* **In-Memory vs. Disk Save**: When imported in UE5, the asset is held in memory until you press **`Ctrl + S`** (Save All) in Unreal Engine, writing `DragonEgg.uasset` to disk under `Content/Assets/External/ArtVault/Meshes/`.

### 2. Automatic Bidirectional Vault Backup
* Once saved in Unreal Engine, running **`sync_art_assets.bat`** automatically detects any new or updated `.uasset` files inside `Content/Assets/External/ArtVault/` and backs them up into your local vault folder (`Content/ArtVault/Meshes/`).
* You do **not** need to manually copy `.uasset` files between directories—the bidirectional sync engine manages this automatically in both directions.

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
- Acquire or export 3D meshes (`.fbx`/`.uasset`), textures, materials, or audio banks.
- Drop them into `Content/ArtVault/` (e.g. `Content/ArtVault/Meshes/`, `Content/ArtVault/Textures/`, `Content/ArtVault/Props/`).
- Double-click **`sync_art_assets.bat`** in your project root.
  - *What happens under the hood?* The auto-discovery engine detects any new subfolder, updates `asset_manifest.json`, and copies/syncs the binary art files into `Content/Assets/External/ArtVault/`.

### Step 2: Opening Unreal Engine 5 & Placing Assets
- Launch **Unreal Engine 5**.
- In the Content Browser, navigate to `Content -> Assets -> External -> ArtVault/`.
- If importing raw `.fbx` files, drag them into the Content Browser and hit **Save All (`Ctrl + S`)**.
- Drag your Static Mesh (e.g. `DragonEgg.uasset`) into the 3D viewport.
- Assign materials or textures to the mesh in the Details Panel.
- Position, rotate, and scale objects to design your level.

### Step 3: Saving the Level (`.umap` & `__ExternalActors__`)
- Click **File -> Save Current Level** into **`Content/Maps/MainLevel.umap`** (NOT inside `ArtVault/`!).
- Unreal Engine records the 3D Spatial Transform (Location, Rotation, Scale) and Asset Reference path strings (`"/Game/Assets/External/ArtVault/Meshes/DragonEgg.DragonEgg"`) inside the level map file (`.umap`) and `Content/__ExternalActors__/`.

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

1. **In Git**: Teammates run `git pull`. They receive level layout files (`MainLevel.umap`), C++ logic, and `asset_manifest.json` (which records where the Dragon Egg is placed in the 3D room).
2. **In Vault**: Teammates place `DragonEgg.uasset` (or its raw `.fbx`) into their local `Content/ArtVault/Meshes/` folder and double-click `sync_art_assets.bat`.
3. **In UE5**: Open Unreal Engine 5. UE5 opens `MainLevel.umap`, resolves the path `/Game/Assets/External/ArtVault/Meshes/DragonEgg`, and renders the egg perfectly in the level!

---

## 🧪 Automated Testing & Continuous Integration (CI)

To ensure sync logic stability across Python versions and OS environments, run the standalone test suite:

```bash
python -m unittest discover -s tests -p "test_*.py"
```

### Test Coverage:
* **Mock Project Setup**: Simulates temporary project roots and `.uproject` detection.
* **Manifest Parsing**: Tests JSON loading, error handling, and vault auto-registration.
* **Safety Rules**: Verifies that `.umap`, World Partition metadata (`__ExternalActors__`, `__ExternalObjects__`), backup `.bak` files, and temporary logs are never synced into Content.
* **Sync Verification**: Asserts forward (vault -> engine) and reverse (engine -> vault) timestamp copy behavior and symlink support.
