# Sovereign Art Sync (Unreal Engine 5 Plugin)

> **Fab Marketplace Ready UE5 Plugin: Local Art Asset Synchronizer & Auto-Discovery Engine**

`SovereignArtSync` is an Unreal Engine 5 editor plugin that decouples heavy binary `.uasset` files (static meshes, high-resolution textures, materials, audio) from Git source control. It keeps your Git repository under 50 MB while keeping art assets stored locally in `Content/ArtVault/`.

---

## 🚀 Key Features

* **In-Editor Utility Widget (EUW):** 1-Click Sync toolbar panel inside Unreal Engine 5.
* **Zero Repository Bloat:** Stores heavy binary assets in local vaults or external storage drives.
* **Auto-Discovery Engine:** Automatically detects newly dropped asset folders in `Content/ArtVault/` and updates `asset_manifest.json`.
* **PythonScriptPlugin & EditorScriptingUtilities Integration:** Native integration with UE5's embedded Python interpreter.
* **Bidirectional Syncing:** Syncs assets to `Content/Assets/External/` and automatically backs up in-editor saved assets to the vault.
* **Safety Rules:** Strictly excludes level maps (`.umap`), World Partition metadata (`__ExternalActors__`, `__ExternalObjects__`), and backups.

---

## 📁 Plugin Installation

1. Copy the `Plugins/SovereignArtSync/` directory into your project's `Plugins/` folder:
   ```
   [YourUE5ProjectRoot]/
   ├── YourGame.uproject
   ├── Plugins/
   │   └── SovereignArtSync/
   ```
2. Enable **Python Script Plugin** and **Editor Scripting Utilities** in `Edit -> Plugins`.
3. Restart Unreal Engine 5!

---

## 🛠️ Usage

### Option 1: Output Log Console (Cmd Python)
Type directly into the UE Output Log console (Cmd Python mode):
```python
execute_1click_sync()
```
Or as a 1-liner:
```python
import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()
```

### Option 2: Editor Utility Widget (EUW)
Wire a button in your Editor Utility Widget using the **Execute Python Command** node:
```python
import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()
```
Read [USER_GUIDE.md](USER_GUIDE.md) for complete step-by-step EUW setup instructions.

---

## 📜 License

Licensed under the [MIT License](LICENSE).
