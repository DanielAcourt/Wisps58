# EUW_SovereignArtSync: Editor Utility Widget Setup Guide

The `SovereignArtSync` plugin provides an in-editor toolbar panel and Editor Utility Widget for 1-click local asset synchronization in Unreal Engine 5.

---

## ⚡ EXACT COPY-PASTE BLUEPRINT COMMAND STRINGS

In your Editor Utility Widget Blueprint, connect the **On Clicked** event of your button to the **Execute Python Command** or **Execute Python Command Advanced** node (from `Editor Scripting Utilities`).

### Recommended Input String for Blueprint Node:

```text
import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()
```

*(Copy and paste the exact line above directly into the **Python Command** string input pin!)*

---

## 💡 Standard vs. Advanced Blueprint Nodes Explained

Unreal Engine provides two Python execution Blueprint nodes:

### Option A: Standard `Execute Python Command` Node (Default & Simplest)
* **Return Value Pin:** `Return Value` (Boolean `bool`).
* **Where logs appear:** Writes the complete sync report directly into Unreal Engine's **Output Log** tab under the **`LogPython`** category.

### Option B: `Execute Python Command Advanced` Node (For UI Text Boxes)
* **Return Value Pin:** `Command Result` / `Log Output` (String `string`).
* **Where logs appear:** Captures `sys.stdout` and hands the text string directly to your Blueprint node so you can connect it into a **Set Text** or **Print String** node!

---

## 🛑 IMPORTANT SYNTAX WARNINGS

1. **DO NOT add the word `python` at the start of the string!**
   - ❌ WRONG: `python execute_1click_sync()`  *(Causes SyntaxError: invalid syntax)*
   - ✅ CORRECT: `import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()`

2. **DO NOT add leading spaces, tabs, or blank lines!**
   - ❌ WRONG:
     ```text

        execute_1click_sync()
     ```
     *(Causes IndentationError: unexpected indent)*
   - ✅ CORRECT: `import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_1click_sync()`

---

## 🎨 Dry Run & Auto-Discover String Pin:

```text
import sovereign_art_sync_plugin; sovereign_art_sync_plugin.execute_dry_run_autodiscover()
```
