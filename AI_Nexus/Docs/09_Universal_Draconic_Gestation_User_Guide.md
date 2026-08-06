// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.9. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-04
# USER GUIDE: UNIVERSAL DRACONIC GESTATION & PROGRESSION SYSTEM (B-035 / B-038)

## 🌌 Overview
This user guide details the technical and gameplay rules governing the **Universal Draconic Gestation, Nesting, and Live Progression Subsystem**. Located inside the `USovereignBioComponent` C++ module, this subsystem is responsible for handling female dragon lifecycles, internal egg formation, extradimensional nesting, and dynamic womb-power accumulation.

---

## 🛠️ 1. Core Gestation Properties
All draconic reproductive variables are contained within the `Bio` namespace of `USovereignBioComponent` and are fully serialized and replicated.

| Variable | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| `bGestationActive` | `bool` | True if a female dragon is currently carrying an egg. | `false` |
| `GestationProgress` | `float` | Progression of the egg's internal formation (0% to 100%+). | `0.0f` |
| `GestationRate` | `float` | Rate at which gestation progress increments per second of heartbeat. | `1.0f` |
| `bIsNestCreated` | `bool` | True if the dragon has established a spatially distorted nest. | `false` |
| `NestSpatiotemporalVolume` | `float` | The physical volume in cubic meters of the extradimensional nest. | `0.0f` |
| `EggFertilityState` | `FString` | Current status of the egg (e.g. `"None"`, `"ReadyToLay"`, `"Condensed_Tier_1"`). | `"None"` |

---

## 🧬 2. Live Progression Tick Logic
To prevent double-ticking component logic, general frame-based ticking is disabled on `USovereignBioComponent` (`PrimaryComponentTick.bCanEverTick = false`).
Instead, metabolism and draconic growth updates are driven solely by the **Sovereign Entity Heartbeat** (`OnSovereignHeartbeat` in `ASovereignBaseEntity`).

When `bGestationActive` is true during a heartbeat update:
1. `GestationProgress` increments:
   $$\Delta \text{Progress} = \text{GestationRate} \times \Delta t$$
2. **Threshold Completion:** Once `GestationProgress` reaches `100.0f`, if `EggFertilityState` is `"None"`, it transitions to `"ReadyToLay"`.
3. **Over-Gestation Womb Power:** The egg does **not** stop progressing or automatically lay. It continues to grow beyond 100.0f (e.g., reaching 120.0f or higher) inside the womb, accumulating continuous magical power until the dragon triggers the laying or condensation ritual.

---

## 🪺 3. Prestige Condensation Rituals
A female dragon can perform a 24-hour magical ritual inside her nest to condense a completed egg (`GestationProgress >= 100.0f`) to shrink its physical size while multiplying its future hatchling's attributes:
- Call `CondensePrestigeEgg()`.
- Shrinks `NestSpatiotemporalVolume` by 50% (spatial compression).
- Divides `GestationRate` by `1.5f` (representing the increased density and required magical focus for the next tier).
- Resets `GestationProgress` back to `0.0f` to restart the re-growth/condensation process.
- Sets `EggFertilityState` to `"Condensed_Tier_X"` (e.g., Tier 1, Tier 2).

---

## 🧪 4. C++ Automation Testing
The system's integrity is guaranteed through three comprehensive automation tests inside the Unreal Engine test runner (`SovereignBioComponentTests.cpp`):

### 1. `Sovereign.Bio.ComponentDefaults`
- Verifies that new components initialize gestation active as `false`, progress/volume as `0.0f`, rate as `1.0f`, and fertility as `"None"`.
- Confirms that frame-based ticking is disabled by default.

### 2. `Sovereign.Bio.ComponentSerialization`
- Verifies that all gestation, nesting, rate, and egg fertility fields serialize correctly to JSON (`OnSave`) and restore seamlessly on load (`OnLoad`).

### 3. `Sovereign.Bio.ComponentProgression`
- Verifies that gestation does not tick when inactive.
- Verifies that active gestation ticks up, transitions the egg to `"ReadyToLay"` at 100.0f, and continues to grow past 100.0f without limit to build womb-nourishment power.
