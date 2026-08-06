# SAVESYSTEM SETUP GUIDE (The Sovereign Way)

## ⛩️ The Philosophy
The SaveSystem is the "Bridge of Continuity." If an entity has no Soul (`USovereignSaveableEntityComponent`), it cannot cross the bridge. If the bridge is not configured in the project "Vessel" (`.uproject`), no one can cross.

---

## 🛠️ Step 1: Project Integration
To enable the SaveSystem's testing and hardening features, ensure your `.uproject` file contains the `WispCPP7VRTests` module:

```json
{
    "Name": "WispCPP7VRTests",
    "Type": "DeveloperTool",
    "LoadingPhase": "PreDefault",
    "TargetAllowList": ["Editor"]
}
```

---

## 🧬 Step 2: Entity Preparation
Every Actor you wish to save MUST have the `USovereignSaveableEntityComponent` attached.
1. Open the Actor Blueprint.
2. Add Component > `SovereignSaveableEntity`.
3. The `EntityID` (GUID) will be generated automatically upon birth.

---

## 🧪 Step 3: Running Automation Tests
The SaveSystem is guarded by 15+ automated checks.
1. Open Unreal Editor.
2. Go to **Tools > Automation**.
3. Search for "SaveSystem".
4. Select all tests and click **Start Tests**.

**Expected Result:** All tests should return ✅ (Green). If any return ❌ (Red), consult the `SaveSystem_Error_Audit.md`.

---

## 💾 Step 4: JSON vs. Binary
The `USaveManager` supports two storage modes:
*   **JSON (Default):** Best for debugging. Files are human-readable and located in `/Saved/SaveGames/`.
*   **Binary (.sav):** Best for performance. Standard Unreal SaveGame format.

You can toggle this in the `USaveManager` settings within the Editor or via the `bAsJson` parameter in C++ calls.

---

## ⚠️ The Sovereign Rule
> "Never inherit scale, never ignore nulls."

When modifying the SaveSystem, always use `FAttachmentTransformRules::SnapToTargetNotIncludingScale` for children and ALWAYS validate pointers before calling `CaptureFullEntityState()`.
