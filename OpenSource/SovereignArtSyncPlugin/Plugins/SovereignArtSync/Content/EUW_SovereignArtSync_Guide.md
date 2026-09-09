# EUW_SovereignArtSync: Editor Utility Widget Setup Guide

The `SovereignArtSync` plugin provides an in-editor toolbar panel and Editor Utility Widget for 1-click local asset synchronization in Unreal Engine 5.

---

## ⚡ EXACT COPY-PASTE BLUEPRINT COMMAND STRINGS

In your Editor Utility Widget Blueprint, connect the **On Clicked** event of your button to the **Execute Python Command** node (from `Editor Scripting Utilities`).

### String Input Pin for "Execute Python Command" Node:

```text
execute_1click_sync()
```

*(Copy and paste the line above directly into the **Python Command** string input pin of the **Execute Python Command** node!)*

---

## 🛑 IMPORTANT SYNTAX WARNINGS

1. **DO NOT add the word `python` at the start of the string!**
   - ❌ WRONG: `python execute_1click_sync()`  *(Causes SyntaxError: invalid syntax)*
   - ✅ CORRECT: `execute_1click_sync()`

2. **DO NOT add leading spaces, tabs, or blank lines!**
   - ❌ WRONG:
     ```text

        execute_1click_sync()
     ```
     *(Causes IndentationError: unexpected indent)*
   - ✅ CORRECT: `execute_1click_sync()`  *(No spaces or newlines before the text!)*

---

## 🎨 Dry Run & Auto-Discover String Pin:

```text
execute_dry_run_autodiscover()
```
