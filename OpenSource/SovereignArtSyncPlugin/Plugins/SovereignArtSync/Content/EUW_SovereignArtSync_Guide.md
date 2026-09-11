# EUW_SovereignArtSync: Editor Utility Widget Setup Guide

The `SovereignArtSync` plugin provides an in-editor toolbar panel and Editor Utility Widget for 1-click local asset synchronization in Unreal Engine 5.

---

## 🎨 ARTIST WORKFLOW: IMPORTING RAW PNG & FBX FILES

When artists drag raw `.png` textures or `.fbx` models into Unreal Engine:

1. **Import:** Drag PNG/FBX files into Unreal Engine's Content Browser.
2. **Save All (`Ctrl + S`):** Press **`Ctrl + S`** (or **File -> Save All**). Unreal Engine holds newly imported assets in editor memory until saved to disk as compiled `.uasset` binary files.
3. **1-Click Sync:** Click the **1-Click Sync** button in your Editor Utility Widget! The sync engine detects the saved `.uassets` on disk and backs them up bidirectionally into `Content/ArtVault/`.

---

## ⚡ RECOMMENDED BLUEPRINT GRAPH SETUP FOR UI TEXT BOXES

To execute 1-click sync AND immediately display the summary report inside your Editor Utility Widget Text Box:

### Node 1: Execute Python Command
* **Node:** `Execute Python Command` (from `Editor Scripting Utilities`)
* **Python Command Input String:**
  ```text
  import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()
  ```

### Node 2: Set Text (Log Output Text Box)
* **Node:** `Execute Python Command Advanced` (or `get_last_sync_log()`)
* **Python Command Input String for Advanced Node:**
  ```text
  get_last_sync_log()
  ```
  *(Connect the **Log Output** or **Command Result** string pin directly to your UI Multi-Line Text Box **Set Text** pin!)*

---

## 🛑 IMPORTANT SYNTAX WARNINGS

1. **DO NOT add the word `python` at the start of the command!**
   - ❌ WRONG: `python execute_1click_sync()`  *(Causes SyntaxError: invalid syntax)*
   - ✅ CORRECT: `import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()`

2. **DO NOT add leading spaces, tabs, or blank lines!**
   - ❌ WRONG:
     ```text

        execute_1click_sync()
     ```
     *(Causes IndentationError: unexpected indent)*
   - ✅ CORRECT: `import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()`
