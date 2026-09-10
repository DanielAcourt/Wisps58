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
LogPython: Sovereign Art Sync Plugin initialized. 'execute_1click_sync()' and 'sovereign_art_sync_plugin' are now ready in Output Log.
```

---

## 💻 Method 1: Running Sync from UE Output Log Console

In Unreal Engine 5, open **Bottom Bar -> Output Log** and set the command bar input mode to **Cmd (Python)** (bottom left dropdown).

### Option A: Direct Function Call
Because `init_unreal.py` pre-imports convenience functions on startup, type:
```python
execute_1click_sync()
```

### Option B: Module One-Liner
If typing a full Python import block in the single-line Output Log input, use a semicolon (`;`) on a single line (do **not** add leading spaces/indentation):
```python
import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()
```

---

## 🎨 Method 2: Creating an Editor Utility Widget (EUW)

You can build a custom in-editor UI panel with a **1-Click Sync** button inside Unreal Engine:

### Step 1: Create the Widget Asset
1. In the Content Browser, **Right-Click -> Editor Utilities -> Editor Utility Widget**.
2. Name it `EUW_SovereignArtSync`.
3. Double-click `EUW_SovereignArtSync` to open the Design Palette.

### Step 2: Design the UI Layout
1. Drag a **Vertical Box** into the Canvas Panel.
2. Drag a **Button** into the Vertical Box, name it `Btn_SyncAssets`.
3. Drag a **Text** block onto the Button and set text to `"1-Click Sync Art Assets"`.
4. Drag a **Multi-Line Editable Text Box** below the Button, name it `LogOutput`.

### Step 3: Wire the Blueprint Graph
1. Select `Btn_SyncAssets` and click **On Clicked** in the Details panel.
2. In the Event Graph, add the node **Execute Python Command** (from `Editor Scripting Utilities`).
   * *Note on Node Return Values:* Standard `Execute Python Command` returns a `bool` (success/fail) and logs to Unreal's **Output Log** tab. If you want the log output as a string pin directly in Blueprint to bind to a UI Text Box, use **Execute Python Command Advanced**!
3. Set **Python Command** string to:
   ```text
   execute_1click_sync()
   ```
   *(Note: Do NOT type `python` before `execute_1click_sync()`, and ensure there are no leading spaces or blank lines!)*

### Step 4: Run the Widget
1. Right-click `EUW_SovereignArtSync` in the Content Browser.
2. Click **Run Editor Utility Widget**.
3. Dock the tab next to your Content Browser or Viewport, and click **1-Click Sync Art Assets** anytime!

---

## ☀️ Day-to-Day Artist Workflow

1. **Drop New Assets:** Copy `.uasset` folders (e.g. `Meshes`, `Textures`) into `Content/ArtVault/`.
2. **1-Click Sync:** Click the **1-Click Sync** button in your Editor Utility Widget or type `execute_1click_sync()` in the Output Log.
3. **Save in UE5:** Import FBX or edit materials in UE5 and press `Ctrl + S`. Running **1-Click Sync** automatically backs up updated `.uassets` to `Content/ArtVault/`.

---

## ❓ Troubleshooting Common Output Log Errors

### 1. `NameError: name 'sovereign_art_sync_plugin' is not defined`
* **Cause:** The plugin was placed in `Content/Plugins/` instead of `Plugins/`, or Unreal Engine was not restarted after enabling the Python Script Plugin.
* **Fix:** Move the plugin to `[YourProjectRoot]/Plugins/SovereignArtSync/` and restart Unreal Engine 5. Verify that the startup message `LogPython: Sovereign Art Sync Plugin initialized.` appears in the log.

### 2. `IndentationError: unexpected indent`
* **Cause:** Typing multi-line Python with leading spaces or indentation into Unreal's single-line command bar.
* **Fix:** Simply type:
  ```python
  execute_1click_sync()
  ```

### 3. Advanced Node Output Returns `None`
* **Cause:** Typing `import sovereign_art_sync_plugin; ...` inside the Advanced Python node forces Python into statement execution mode, which yields `None`.
* **Fix:** Leave out the `import` statement and type **only** `execute_1click_sync()`. Because `init_unreal.py` pre-loads the function on startup, typing `execute_1click_sync()` directly evaluates as an expression and returns the full log string to Blueprint!
