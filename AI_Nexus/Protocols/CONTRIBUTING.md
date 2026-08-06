// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2025-05-22
# Contributing Guidelines - Multi-Agent Development Workflow

## Overview
This project uses a **multi-agent development model** where AI agents and human developers work on specific subsystems independently. This document ensures coordination without context pollution.

## Agent Assignment & Responsibilities

### SaveSystem (Agent Focus: Error Handling & Unit Testing)
- **Owner:** SaveSystem Agent (Copilot / Claude)
- **Files:**
  - `Public/SaveSystem/*`
  - `Private/SaveSystem/*`
- **Responsibilities:**
  - Unit test coverage for save/load pipeline
  - Null safety hardening
  - Registry consistency validation
  - JSON serialization robustness
  - Soft reference failure resilience
- **Off-Limits:** Do not modify spawn behavior or entity initialization logic

### Entities (Reserved for Future Agent)
- **Files:**
  - `Public/Entities/*`
  - `Private/Entities/*`
- **Current Status:** Documentation phase only

### Components (Reserved for Future Agent)
- **Files:**
  - `Public/Components/*`
  - `Private/Components/*`

### Interaction System (Reserved for Future Agent)
- **Files:**
  - `Public/Interaction/*`
  - `Private/Interaction/*`

## Development Workflow

### Before Starting Work
1. **Check `_AGENT_CONTEXT.md`** at repo root for latest status
2. **Identify your assigned module** and read its documentation
3. **Do NOT explore outside your module** unless explicitly needed
4. **Report blockers** via Git tags or issues, not context pollution

### While Working
- **Preserve existing interfaces**—don't refactor other agents' code
- **Document contracts**—if you add a new parameter, document why
- **Test isolation**—your unit tests should not depend on other modules
- **Fail safely**—defensive programming preferred (log + return null, not assert)

### When Committing
- **Tag your work:** `git tag "saveSystem-tests-complete"`
- **Include checklist** of what was hardened
- **Link to unit tests** in commit message

## Error Handling Standards (SaveSystem)

### Soft Reference Failure Strategy (RESILIENT APPROACH)
**Philosophy:** Spawn entities with default values when species data fails to load.  
**Rationale:** This is a playtest branch exploring runtime exception handling for unknown classes (PSTAs math framework pending).

```cpp
// ✅ GOOD: Resilient with detailed logging
if (!Row->SpeciesVesselData.IsNull())
{
    USovereignSpeciesData* SpeciesData = Row->SpeciesVesselData.LoadSynchronous();
    if (SpeciesData == nullptr)
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("SaveSystem: Failed to load SpeciesVesselData for GUID=%s. Spawning with defaults."), 
            *Data.MyGUID.ToString());
        // Entity spawns anyway with default values from FallbackUnknownClass
    }
    else
    {
        PostSpawnInitialize(SpeciesData, Data.MotherID, Data.FatherID);
    }
}
else
{
    // No species data assigned (expected for non-biological entities)
    UE_LOG(LogTemp, Verbose, TEXT("SaveSystem: Entity %s has no SpeciesVesselData (non-biological?)"), 
        *Data.MyGUID.ToString());
}

// ❌ AVOID: Silent failure
if (!Row->SpeciesVesselData.IsNull())
{
    return Row->SpeciesVesselData.LoadSynchronous();  // Could return null silently
}

// ❌ AVOID: Assertions in shipped code
check(SpeciesData != nullptr);  // Will crash on missing data!
```

### Null/Pointer Safety (General)
```cpp
// ✅ GOOD: Defensive with logging
if (!LoadedSuitcase) 
{
    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: LoadedSuitcase is null, aborting load"));
    return false;
}

// ✅ GOOD: Validate actors before dereferencing
for (const FEntitySaveData& Data : LoadedSuitcase->SavedActors)
{
    AActor* TargetActor = Registry->FindActor(Data.MyGUID);
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Verbose, TEXT("SaveSystem: Entity %s not found in registry, will spawn"), 
            *Data.MyGUID.ToString());
        // Proceed with spawn
    }
}

// ❌ AVOID: Chained null dereferences
Registry->FindActor(GUID)->GetComponent()->DoSomething();
```

### Registry Consistency
```cpp
// ✅ GOOD: Validate registry state after operations
PurgeUnearnedEntities(LoadedSuitcase);
if (Registry->GetTrackedActorsAsList().Num() != LoadedSuitcase->SavedActors.Num())
{
    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Registry/SaveData mismatch: %d tracked vs %d saved"), 
        Registry->GetTrackedActorsAsList().Num(),
        LoadedSuitcase->SavedActors.Num());
    // This is non-fatal; logging for debugging
}

// ✅ GOOD: Verify consistency after purge
for (AActor* Actor : Registry->GetTrackedActorsAsList())
{
    if (!LoadedSuitcase->SavedActors.ContainsByPredicate([&](const FEntitySaveData& S) 
        { return S.MyGUID == Actor->FindComponentByClass<USovereignSaveableEntityComponent>()->EntityID; }))
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Found tracked actor not in save file: %s"), *Actor->GetName());
    }
}
```

## Unit Testing Requirements (SaveSystem)

### Test Categories
1. **Serialization Tests**
   - JSON to struct conversion
   - Struct to JSON conversion
   - Round-trip fidelity (save → load → compare)
   - Unicode/special characters in entity names

2. **Soft Reference Tests**
   - Valid species data loads correctly
   - Missing species data doesn't crash (resilient spawn)
   - Null species data logged appropriately
   - Non-biological entities with null species data load fine

3. **Registry Consistency Tests**
   - Registry state before/after save matches
   - Purge logic removes only untracked entities
   - Parent IDs correctly restored
   - GUIDs unique across save file

4. **Edge Cases**
   - Empty save file
   - Save file with single entity
   - Save file with 1000+ entities
   - Corrupted JSON (missing fields)
   - Entities with null GUIDs (invalid)
   - Circular parent references (should be impossible, but test it)

5. **Performance Baseline**
   - Time to save 100 entities (target: <1s)
   - Time to load 100 entities (target: <2s)
   - Memory usage of SaveGame object with 100 entities
   - JSON string size vs. binary size comparison

## Code Style (SaveSystem)

- **Logging:** Use `LogTemp` with descriptive messages; indicate severity (Warning/Error/Verbose)
- **Naming:** `bSomething` for bools, `SomeArray` for containers, `SpeciesData` for non-null data
- **Comments:** Explain WHY, not WHAT (code shows WHAT)
- **Line Length:** Max 120 characters
- **Indentation:** Tabs (UE5 standard)
- **Null checks:** Always validate pointers before dereferencing

## Known Limitations (Intentional Blindness)

This agent intentionally does NOT have access to:
- Other agents' work or conversations
- Full codebase context (only SaveSystem visible)
- PSTAs math framework (mentioned but not in this branch)
- External documentation (GitHub wiki, design docs)
- Git history or previous commits

**Why?** To test isolation and prevent hallucination. If you need context, ask explicitly.

## Questions or Blockers?

- **Technical Issue:** Create a GitHub issue with tag `saveSystem-blocker`
- **Need Context:** Ask in the PR comment (don't update `_AGENT_CONTEXT.md` yourself)
- **Found Bug Outside SaveSystem:** Document it, but don't fix it
- **Soft Reference Failure:** Use resilient approach (spawn with defaults, log warnings)

---

# SaveSystem Setup Guide

## 🔧 Initial Setup

Follow these steps to configure the SaveSystem for development:

1. **Update Unreal Engine** to the latest version (UE 5.8.1 or higher)
2. **Clone the repository** with all submodules:
   ```bash
   git clone --recurse-submodules <repo_url>
   ```
3. **Install Dependencies** using the Epic Games Launcher:
   - Navigate to the **Library** tab
   - Under **Engine Versions**, click **+ Install Engine Version** and select the latest version
   - Ensure **Starter Content** is selected
4. **Generate Visual Studio project files** (if using Windows):
   - Right-click the `.uproject` file
   - Select **Generate Visual Studio project files**
5. **Build the project** from Visual Studio or Xcode:
   - Open the solution file (`.sln`) in Visual Studio or Xcode
   - Build the solution (this may take some time)

## ⚙️ Configuration Settings

Set up the following configuration files:

- **DefaultEngine.ini**
  - Navigate to `Saved/Config/Windows/` (or your platform)
  - Open `DefaultEngine.ini` and ensure the following settings are present:
    ```ini
    [/Script/Engine.RendererSettings]
    r.SSR.MaxRoughness=0.5
    r.SSR.Quality=2
    ```

- **GameUserSettings.ini**
  - Navigate to `Saved/Config/Windows/` (or your platform)
  - Open `GameUserSettings.ini` and set your desired resolution and quality settings

## 🐞 Common Issues

### 1. Missing DLLs or Components
- **Symptoms:** Editor fails to start, missing module errors
- **Solution:** Verify that all required plugins are enabled in the Epic Games Launcher under **Manage Plugins**.

### 2. Shader Compilation Errors
- **Symptoms:** Shaders fail to compile, materials appear pink
- **Solution:** Delete the `DerivedDataCache` folder located in the project's root directory and restart the editor.

### 3. Asset Packaging Issues
- **Symptoms:** Assets not included in packaged build
- **Solution:** Ensure that all assets are saved and that there are no errors in the **Output Log** before packaging.

## 📚 Documentation References

- [Unreal Engine Documentation](https://docs.unrealengine.com/en-US/index.html)
- [SaveSystem Design Document](../Docs/SaveSystem_Design.md)
- [API Reference](../Docs/API_Reference.md)

---

## ⏰ Estimated Time to Complete Setup: 2 hours

---

## 🚀 Next Steps

1. Complete the initial setup and configuration
2. Review the SaveSystem design document
3. Begin implementing the Save and Load functionality