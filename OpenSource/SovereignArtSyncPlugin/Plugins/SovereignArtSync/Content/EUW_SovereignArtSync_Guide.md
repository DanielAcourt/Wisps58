# EUW_SovereignArtSync: Editor Utility Widget Setup Guide

The `SovereignArtSync` plugin provides an in-editor toolbar panel and Editor Utility Widget for 1-click local asset synchronization in Unreal Engine 5.

## Widget Layout & Blueprint Nodes

### 1. "1-Click Sync Assets" Button
* **On Clicked Event**:
  * Calls Python Command via `Execute Python Command`:
    ```python
    import sovereign_art_sync_plugin
    result_log = sovereign_art_sync_plugin.execute_1click_sync(manifest_path="asset_manifest.json")
    ```
  * Binds `result_log` output string to the UI Multi-Line Editable Text Log Box.

### 2. "Dry Run / Auto-Discover Packages" Button
* **On Clicked Event**:
  * Calls Python Command via `Execute Python Command`:
    ```python
    import sovereign_art_sync_plugin
    result_log = sovereign_art_sync_plugin.execute_dry_run_autodiscover(manifest_path="asset_manifest.json")
    ```
  * Binds `result_log` output string to the UI Log Box.

### 3. Path Inputs & Overrides
* **Manifest Path Input Text Box** (Default: `asset_manifest.json`)
* **Vault Root Path Input Text Box** (Default: `Content/ArtVault`)
