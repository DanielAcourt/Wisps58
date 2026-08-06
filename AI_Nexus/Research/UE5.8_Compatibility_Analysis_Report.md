// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.8. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-04
# UE 5.8.1 Compatibility Analysis: FJsonObject::Values Iteration Audit

## 🏛️ Executive Summary
During the transition from Unreal Engine 5.7.3 to 5.8.1, Epic Games refactored `FJsonObject::Values` to optimize memory and performance during heavy JSON operations. In Unreal 5.7.3 and below, `FJsonObject::Values` was a standard `TMap<FString, TSharedPtr<FJsonValue>>`. In Unreal 5.8.1, `Values` uses a custom string-type optimization wrapper: `TMap<FJsonObject::FStringType, TSharedPtr<FJsonValue>>` (utilizing `UE::FSharedString` under the hood).

Because range-based `for` loops (e.g. `for (auto& Elem : JsonObject->Values)`) evaluate `Elem.Key` as this new `FJsonObject::FStringType` instead of raw `FString`, all implicit conversions and pointer dereferences (`*Elem.Key`) fail.

This document serves as a comprehensive compatibility audit of the `WispsCPPVR` codebase, mapping every instance of `FJsonObject::Values` iteration, evaluating its compatibility status under UE 5.8.1, and identifying manual changes to ensure a zero-warning build.

---

## 🔬 Audit Matrix & Status

| File Path | Code Location | Iteration Target | Compatibility Status | Recommended Refactor / Audit Note |
| :--- | :--- | :--- | :---: | :--- |
| `SovereignQiComponent.cpp` | Line 153-157 | `(*ResObj)->Values` | **⚠️ COMPILER FAIL** | Currently using `*Elem.Key` inside `FName(...)`. Must wrap with `FString(Elem.Key)` first. |
| `SovereignJsonUtils.cpp` | Line 92-96 | `JsonObject->Values` | **⚠️ COMPILER FAIL** (Commented Out) | Currently inside a block comment. If uncommented, `Entry.Key` will fail inside `OutMap.Add(...)`. Must wrap with `FString(Entry.Key)`. |
| `SovereignDiagnosticBroker.cpp` | Line 28-40 | `TruthObj->Values` | **✅ SECURE** | Already refactored. Uses `FString KeyStr = FString(Elem.Key);` explicitly. |
| `SovereignSaveableEntityComponent.cpp` | Line 214-217 | `(*TagsObj)->Values` | **✅ SECURE** | Already refactored. Uses `FString(Elem.Key)` explicitly inside the map addition. |
| `SovereignSaveableEntityComponent.cpp` | Line 235-247 | `JsonData->Values` | **✅ SECURE** | Already refactored. Uses `FString(Elem.Key)` explicitly inside the map additions. |

---

## 🔎 Detailed Code Inspections & Verification

### 1. USovereignQiComponent Load Pipeline (COMPILER FAIL)
* **Target File:** `WispsCPPVR/Source/WispCPP7VR/Private/Components/SovereignQiComponent.cpp`
* **Active Code:**
  ```cpp
  const TSharedPtr<FJsonObject>* ResObj;
  if ((*QiObj)->TryGetObjectField(TEXT("Resonance"), ResObj))
  {
      ElementalResonance.Empty();
      for (auto& Elem : (*ResObj)->Values)
      {
          ElementalResonance.Add(FName(*Elem.Key), (float)Elem.Value->AsNumber());
      }
  }
  ```
* **Hazard:** Attempting to dereference `Elem.Key` with the pointer operator (`*Elem.Key`) will fail because `FJsonObject::FStringType` does not support explicit dereferencing to `const TCHAR*` expected by `FName`.
* **Resolution:** Wrap explicitly into a local `FString` container to enforce the container conversion:
  ```cpp
  for (auto& Elem : (*ResObj)->Values)
  {
      FString KeyStr = FString(Elem.Key);
      ElementalResonance.Add(FName(*KeyStr), (float)Elem.Value->AsNumber());
  }
  ```

### 2. USovereignJsonUtils Map Parser (COMPILER FAIL - Commented Out)
* **Target File:** `WispsCPPVR/Source/WispCPP7VR/Private/SaveSystem/SovereignJsonUtils.cpp`
* **Active Code (Commented):**
  ```cpp
  TMap<FString, FString> USovereignJsonUtils::JsonObjectToMap(TSharedPtr<FJsonObject> JsonObject)
  {
      TMap<FString, FString> OutMap;
      if (JsonObject.IsValid())
      {
          for (auto& Entry : JsonObject->Values)
          {
              OutMap.Add(Entry.Key, Entry.Value->AsString());
          }
      }
      return OutMap;
  }
  ```
* **Hazard:** `Entry.Key` is passed directly as the `TMap<FString, FString>` key argument. This triggers implicit constructor failure since there is no implicit cast from `UE::FSharedString` to `FString`.
* **Resolution:** Declare an explicit `FString` instance:
  ```cpp
  for (auto& Entry : JsonObject->Values)
  {
      FString KeyStr = FString(Entry.Key);
      OutMap.Add(KeyStr, Entry.Value->AsString());
  }
  ```

### 3. UDiagnosticBroker Load Pipeline (SECURE)
* **Target File:** `WispsCPPVR/Source/WispCPP7VR/Private/Entities/SovereignDiagnosticBroker.cpp`
* **Active Code:**
  ```cpp
  for (auto& Elem : TruthObj->Values)
  {
      // Explicitly convert Elem.Key to FString for Unreal 5.8 compatibility
      FString KeyStr = FString(Elem.Key);
      ...
  }
  ```
* **Verification:** This loop is completely secure. It properly implements the `FString` wrapping pattern, resolving all compatibility problems for Unreal 5.8.1.

### 4. USovereignSaveableEntityComponent Restoration (SECURE)
* **Target File:** `WispsCPPVR/Source/WispCPP7VR/Private/Entities/SovereignSaveableEntityComponent.cpp`
* **Active Code A (UnknownTags):**
  ```cpp
  for (auto& Elem : (*TagsObj)->Values)
  {
      UnknownMetaTags.Add(FString(Elem.Key), Elem.Value->AsString());
  }
  ```
* **Active Code B (Flat Owner Data):**
  ```cpp
  for (const auto& Elem : JsonData->Values)
  {
      if (Elem.Value.IsValid())
      {
          if (Elem.Value->Type == EJson::String)
          {
              FlatData.Add(FString(Elem.Key), Elem.Value->AsString());
          }
          ...
      }
  }
  ```
* **Verification:** Highly secure and clean. Both loops explicitly invoke the `FString` constructor on `Elem.Key`, guaranteeing safe compilation and linear execution.

---

## 🏛️ SSoT Architectural Verification
All non-JSON collections (e.g. `RawData` in `USovereignBlackBoxComponent` or `ValuesByKey` in `SovereignBlackBoxExporter`) were validated to confirm that they do not iterate over `FJsonObject::Values` directly, but rather process standard `TMap<FString, FString>` collections, making them natively immune to the 5.8.1 engine refactor.

This audit concludes that once `USovereignQiComponent` is updated to wrap its resonance keys explicitly (and optionally the commented section in `USovereignJsonUtils`), the `WispsCPPVR` persistence system will be 100% compliant with Unreal Engine 5.8.1.
