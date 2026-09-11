# Sovereign Art Sync Plugin: User & Developer Guide
**Version:** 1.0.0
**License:** MIT License

---

## 🏛️ Executive Strategy

In Unreal Engine 5, binary art assets quickly swell repository sizes to tens or hundreds of gigabytes. `SovereignArtSync` enforces strict separation between lightweight source control (blueprints, C++, config files, level layouts) and local binary asset vaults (`Content/ArtVault/`).

---

## 📦 Plugin Installation & Setup

### 1. Copy Plugin Directory

> ⚠️ **CRITICAL INSTALLATION WARNING:**
> The plugin folder **MUST** be placed directly inside `[YourProjectRoot]/Plugins/SovereignArtSync/`.
> **DO NOT** place it inside `Content/Plugins/`! Placement inside `Content/` prevents Unreal Engine from loading `init_unreal.py` on startup.

```text
[YourUE5ProjectRoot]/          # (e.g. WispsCPPVR/)
├── YourGame.uproject          # (e.g. WispsCPPVR.uproject)
├── Plugins/                   # <-- Direct child of Project Root
│   └── SovereignArtSync/
│       ├── SovereignArtSync.uplugin
│       ├── Content/
│       │   ├── Python/
│       │   │   ├── init_unreal.py
│       │   │   ├── sovereign_art_sync_engine.py
│       │   │   └── sovereign_art_sync_plugin.py
│       │   └── asset_manifest.json
│       └── Resources/
│           └── Icon128.png
```

### 2. Enable Plugins in UE5
1. Launch **Unreal Engine 5**.
2. Go to **Edit -> Plugins**.
3. Search for **Python Script Plugin** and **Editor Scripting Utilities** — ensure both are **Enabled**.
4. Search for **Sovereign Art Sync** — ensure it is **Enabled**.
5. Restart Unreal Engine 5!

Upon startup, you will see the confirmation log in the Output Log:
```text
LogPython: Sovereign Art Sync Plugin initialized. 'execute_artist_friendly_sync()' and 'execute_1click_sync()' are ready in Python builtins.
```

---

## 🎨 Method 1: Editor Utility Widget (EUW) - Recommended for Artists

You can build a custom in-editor UI panel with a **1-Click Sync** button inside Unreal Engine that automatically saves unsaved editor assets and provides crisp single-line status reporting.

### Step 1: Create the Widget Asset
1. In the Content Browser, **Right-Click -> Editor Utilities -> Editor Utility Widget**.
2. Name it `EUW_SovereignArtSync`.
3. Double-click `EUW_SovereignArtSync` to open the Design Palette.

### Step 2: Design the UI Layout
1. Drag a **Vertical Box** into the Canvas Panel.
2. Drag a **Button** into the Vertical Box, name it `Btn_SyncAssets`.
3. Drag a **Text** block onto the Button and set text to `"1-Click Sync Art Assets"`.
4. Drag a **Text** or **Multi-Line Editable Text Box** below the Button, name it `Txt_StatusOutput`.

### Step 3: Wire the Blueprint Graph (Crisp Artist Feedback & Pre-Save)
1. Select `Btn_SyncAssets` and click **On Clicked** in the Details panel.
2. In the Event Graph, add the node **Execute Python Command Advanced** (from `Editor Scripting Utilities`).
3. Set **Python Command** string pin to:
   ```text
   execute_artist_friendly_sync()
   ```
   *(Note: Do NOT type `import` statements or `python` prefix! `init_unreal.py` automatically binds `execute_artist_friendly_sync()` into Python builtins on editor launch.)*
4. Connect the **Log Output** (or Command Result) string pin from **Execute Python Command Advanced** directly to a **Set Text** node targeting `Txt_StatusOutput`!
5. **How it Works:** On button click, `execute_artist_friendly_sync()` automatically runs `unreal.EditorLoadingAndSavingUtils.save_dirty_packages()` to save any newly imported/edited textures or materials to disk first, then executes the vault sync, returning a clean single-line status:
   > `"Successfully added 4 object(s) to the Art Vault and updated 2 engine asset(s)."`

---

## 💻 Method 2: Running Sync from UE Output Log Console

In Unreal Engine 5, open **Bottom Bar -> Output Log** and set the command bar input mode to **Cmd (Python)** (bottom left dropdown).

### Option A: Artist Summary Execution
```python
execute_artist_friendly_sync()
```

### Option B: Detailed Diagnostic Log Execution
```python
execute_1click_sync()
```

### Option C: Dry-Run Auto-Discovery
```python
execute_dry_run_autodiscover()
```

---

## ☀️ Day-to-Day Artist Workflow

### 1. Dropping Pre-compiled `.uasset` Folders into Vault
1. Copy `.uasset` folders (e.g. `Meshes`, `Textures`, `Materials`) into `WispsCPPVR/Content/ArtVault/`.
2. Click **1-Click Sync** in your Editor Utility Widget.

### 2. Importing Raw PNG / FBX Files in Unreal Engine
1. **Import:** Drag raw `.png` textures or `.fbx` models into the Unreal Engine Content Browser.
2. **1-Click Sync:** Click **1-Click Sync** in the EUW panel.
   - The plugin automatically calls `save_dirty_packages()` to commit newly imported assets as binary `.uasset` files on disk.
   - The plugin backs up the saved `.uasset` files bidirectionally to `Content/ArtVault/` and displays:
     > `"Successfully added 2 object(s) to the Art Vault and updated 0 engine asset(s)."`

---

## ❓ Troubleshooting Common Output Log & EUW Behavior

### 1. Advanced Node Returns `None` or Empty Output Pin
* **Cause:** Typing `import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_artist_friendly_sync()` inside `Execute Python Command Advanced` triggers Python `exec` mode, which returns `None` to Blueprint.
* **Fix:** Leave out the `import` statement and set the command string to strictly `execute_artist_friendly_sync()`. Because `init_unreal.py` pre-loads the function into builtins, single expression evaluation returns the output string directly to your Blueprint pin.

### 2. Unsaved Assets Not Synced
* **Cause:** Running sync when imported assets are dirty (unsaved) in editor RAM means disk binaries do not yet exist.
* **Fix:** Use `execute_artist_friendly_sync()`. It automatically invokes `save_dirty_packages()` prior to scanning disk folders.

### 3. `NameError: name 'execute_artist_friendly_sync' is not defined`
* **Cause:** The plugin was placed in `Content/Plugins/` instead of `Plugins/`, or Unreal Engine was not restarted after enabling the Python Script Plugin.
* **Fix:** Move the plugin to `[YourProjectRoot]/Plugins/SovereignArtSync/` and restart Unreal Engine 5.
