# Sovereign Art Sync Plugin: User & Developer Guide
**Version:** 1.0.0
**License:** MIT License

---

## 🏛️ Executive Strategy

In Unreal Engine 5, binary art assets quickly swell repository sizes to tens or hundreds of gigabytes. `SovereignArtSync` enforces strict separation between lightweight source control (blueprints, C++, config files, level layouts) and local binary asset vaults (`Content/ArtVault/`).

---

## 🛠️ In-Editor Workflow

### 1. Dropping New Assets
1. Export or copy `.uasset` folders into `Content/ArtVault/` (e.g. `Content/ArtVault/Meshes/`, `Content/ArtVault/Textures/`).
2. Inside Unreal Engine 5, run **1-Click Sync** from the Editor Utility Widget or Output Log console:
   ```python
   import sovereign_art_sync_plugin
   sovereign_art_sync_plugin.execute_1click_sync()
   ```
3. Assets instantly appear in `Content -> Assets -> External -> ArtVault/` inside the Content Browser.

### 2. Auto-Discovery
The sync engine scans `Content/ArtVault/` for unregistered folders and automatically appends them to `asset_manifest.json`.

### 3. Bidirectional Saving
When you edit materials or save imported static meshes in Unreal Engine (`Ctrl + S`), running **1-Click Sync** backs up the modified `.uasset` files from `Content/Assets/External/` back into your local `Content/ArtVault/`.
