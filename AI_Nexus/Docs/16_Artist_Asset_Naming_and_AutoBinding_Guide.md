// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# Sovereign Framework Artist & Content Creator Guide
## 16: Automated Art Asset Naming Conventions & DataAsset Auto-Binding Protocol

---

## 1. Overview & Purpose

The **Sovereign Framework** features an automated 1-Click asset discovery and binding engine. When adding new species, plants, items, or creatures to **WispsGame**, artists and technical artists do **not** need to manually type asset paths into CSV files or manually select soft pointers in Unreal Editor Primary DataAssets.

By adhering to standard **Epic Games naming conventions**, artists simply drop 3D meshes, textures, and material instances into the local Art Vault (`Content/ArtVault/`). The automated importer (`import_species_data_assets.py`) scans the synced content directory (`/Game/Assets/External/`) and automatically connects 3D meshes and materials to their respective species growth stage arrays (`GrowthStages[0..N]`).

---

## 2. Standard Naming Conventions

All art assets placed in the Art Vault must follow the exact prefix and indexing structure corresponding to the `SpeciesTag` in `DT_SovereignSpeciesTemplate.csv` (e.g. `SpeciesTag = Species.Plant.AppleTree` -> Species Name = `AppleTree`).

### 2.1 File Prefix Rules

| Asset Type | Prefix | Target Component | Example File Name |
| :--- | :--- | :--- | :--- |
| **Static Mesh** (Environment, Flora, Rocks, Items) | `SM_` | `UStaticMeshComponent` | `SM_AppleTree_0.uasset` |
| **Skeletal Mesh** (Humanoids, Animals, Dragons) | `SK_` | `USkeletalMeshComponent` | `SK_SilverDragon_0.uasset` |
| **Material Instance / Material** | `MI_` or `M_` | Material Slot 0 | `MI_AppleTree_0.uasset` |

### 2.2 Growth Stage Indexing (`_0` to `_N`)

The number suffix at the end of the filename represents the **Growth Stage Index** (0-indexed):
- Stage 0 = Seed / Hatchling / Base State (`_0`)
- Stage 1 = Sprout / Juvenile (`_1`)
- Stage 2 = Mature Plant / Adult (`_2`)
- Stage 3 = Ancient / Elder (`_3`)

**Examples**:
- `SM_AppleTree_0.uasset` -> Static Mesh for AppleTree Stage 0
- `SM_AppleTree_1.uasset` -> Static Mesh for AppleTree Stage 1
- `MI_AppleTree_0.uasset` -> Material for AppleTree Stage 0
- `SK_SilverDragon_0.uasset` -> Skeletal Mesh for SilverDragon Stage 0 (Hatchling)
- `SK_SilverDragon_3.uasset` -> Skeletal Mesh for SilverDragon Stage 3 (Adult Dragon)

---

## 3. Artist Directory Workflow & Art Vault Sync

```text
[Local Artist Vault]                      [Unreal Engine Project]
Content/ArtVault/                    -->  WispsCPPVR/Content/Assets/External/
  ├── SM_AppleTree_0.uasset                 ├── SM_AppleTree_0.uasset
  ├── SM_AppleTree_1.uasset                 ├── SM_AppleTree_1.uasset
  ├── MI_AppleTree_0.uasset                 ├── MI_AppleTree_0.uasset
  └── MI_AppleTree_1.uasset                 └── MI_AppleTree_1.uasset
```

1. **Model & Material Creation**: Create 3D models and materials in your preferred 3D DCC software (Blender, Maya, ZBrush, Substance Painter).
2. **Export to Art Vault**: Export `.fbx` models and textures into the local `Content/ArtVault/` folder using the naming convention described above.
3. **Execute 1-Click Sync**: Run `sync_art_assets.bat` from the repository root (or press **Execute 1-Click Art Sync** inside Unreal Editor toolbar). The assets are instantly synced into `Content/Assets/External/`.

---

## 4. Using the 1-Click Importer with the Art Toggle (`bAutoBindArtAssets`)

When importing CSV template updates via the **Editor Utility Widget (`EUW_SovereignCSVImporter`)** or Python console, artists and designers have full control over whether art assets are auto-bound:

```text
+-------------------------------------------------------------+
|               EUW_SovereignCSVImporter                      |
+-------------------------------------------------------------+
| CSV Path:    [ DT_SovereignSpeciesTemplate.csv          [...] ]|
| Output Dir:  [ /Game/Data/SpeciesAssets/              [...] ]|
|                                                             |
| [X] Overwrite Existing Assets                               |
| [X] Auto-Bind Art Assets (bAutoBindArtAssets)               |  <-- TOGGLE
|                                                             |
|                      [ 1-CLICK IMPORT ]                     |
+-------------------------------------------------------------+
```

### 4.1 Importer Options

- **`bAutoBindArtAssets = True` (Default for Artists)**:
  - The Python importer parses `GrowthStageCount` (e.g. `StageCount = 4`).
  - For each stage `i` from `0` to `StageCount - 1`:
    - Python searches `/Game/Assets/External/` for `SM_<SpeciesName>_<i>` or `SK_<SpeciesName>_<i>`.
    - Python searches `/Game/Assets/External/` for `MI_<SpeciesName>_<i>` or `M_<SpeciesName>_<i>`.
  - Automatically populates `GrowthStages[i].StageMesh`, `GrowthStages[i].StageSkeletalMesh`, and `GrowthStages[i].StageMaterial` soft object pointers!

- **`bAutoBindArtAssets = False` (Data-Only Mode for Systems Programmers)**:
  - Disables art auto-discovery. Updates species stats (Strength, Dexterity, Metabolic Rates, Gameplay Tags) without altering existing in-engine mesh or material assignments.

---

## 5. Python Console & Command Line Execution

Artists can also trigger the importer directly from the Unreal Engine Python console or Output Log:

```python
# Standard Import with Art Auto-Binding Enabled
execute_importer_ui(
    csv_path="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv",
    b_overwrite=True,
    b_auto_bind_art=True
)

# Data-Only Import (Skipping Art Auto-Binding)
execute_importer_ui(
    csv_path="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv",
    b_overwrite=True,
    b_auto_bind_art=False
)
```

---

## 6. Summary Checklist for Artists

- [ ] Species entry exists in `DT_SovereignSpeciesTemplate.csv` (e.g., `SpeciesTag = Species.Plant.Daisy`, `GrowthStageCount = 3`).
- [ ] Meshes are named `SM_Daisy_0`, `SM_Daisy_1`, `SM_Daisy_2` (or `SK_` for characters).
- [ ] Materials are named `MI_Daisy_0`, `MI_Daisy_1`, `MI_Daisy_2`.
- [ ] Assets are saved into `Content/ArtVault/` and synced via `sync_art_assets.bat`.
- [ ] Run **1-Click Import** with `bAutoBindArtAssets = True`.
- [ ] Open the generated `USovereignSpeciesData` asset in `/Game/Data/SpeciesAssets/` to verify stage meshes and materials are automatically bound!
