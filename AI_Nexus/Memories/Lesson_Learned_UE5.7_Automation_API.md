# Lesson Learned: UE 5.7 Automation API Changes

## 📅 Date: 2026-05-13
## 🛠 Technical Note: BlackBox Tests Build Errors

### ❌ Problem
Build errors in `SovereignBlackBoxTests.cpp`:
*   `FAutomationEditorCommonUtils::CreateNewWorld()` - not found
*   `FAutomationEditorCommonUtils::DisposeWorld()` - not found

### 🔍 Root Cause
UE 5.7 deprecated/removed these utility functions from the automation testing API. These were helper functions that abstracted world creation/destruction, but Epic Games removed them in favor of direct engine API calls.

### 💡 Why We Can't Use `CreateNewWorld()`
1.  **API Deprecation** - The function was removed from `FAutomationEditorCommonUtils` class (checked in `AutomationEditorCommon.h`).
2.  **Different Testing Paradigm** - UE 5.7 shifted from convenience wrappers to explicit engine management.

### ✅ Solution
Replace the deprecated helpers with direct UE 5.7 APIs:

**OLD (doesn't work in UE 5.7):**
```cpp
World = FAutomationEditorCommonUtils::CreateNewWorld();
// ... tests ...
FAutomationEditorCommonUtils::DisposeWorld(World);
```

**NEW (UE 5.7 compatible):**
```cpp
// Creation
World = NewObject<UWorld>();
World->WorldType = EWorldType::Editor;
FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
WorldContext.SetCurrentWorld(World);

// Cleanup
GEngine->DestroyWorldContext(World);
World->DestroyWorld(true);
```

### 📈 Comparison
| Aspect | Old API | New API |
| :--- | :--- | :--- |
| **Creation** | Single function call | Manual object creation + context setup |
| **Management** | Hidden by utility | Explicit engine context management |
| **Cleanup** | Single function call | Explicit context destroy + world destroy |
| **Control** | Less explicit | More control, clearer intent |

### 🚀 Why This Matters
*   **Version compatibility** - We're targeting UE 5.7, which expects modern API usage.
*   **No vendor lock-in** - Direct engine APIs are more stable long-term than utility wrappers.
*   **Better debugging** - Explicit code is easier to troubleshoot.
