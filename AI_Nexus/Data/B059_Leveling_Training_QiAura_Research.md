// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-15
# Sovereign Framework Research Report: Ticket B-059
## Autonomous Entity Leveling, Stat Training & Environmental Qi Aura Architecture

**Node:** `07` (Researcher)
**Ticket ID:** `B-059`
**GDD Spec:** `AI_Nexus/Docs/15_Metabolism_Training_and_Qi_Aura_GDD.md`
**Status:** In Research / Pre-Implementation Architectural Blueprint
**Date:** October 15, 2026

---

## 1. Executive Summary & Vision

Ticket **B-059** defines the **Autonomous Entity Leveling, Stat Training & Environmental Qi Aura Architecture** within the Sovereign Framework. Derived from *GDD 15: Metabolism, Training, and Qi Aura Systems*, this architecture introduces a simulation-driven, autonomous training and cultivation ecosystem applicable across all entity types—from flora and fauna to humanoid vessels, dragons, and spirit wisps.

The primary architectural goal is to **decouple core stat storage from progression and cultivation execution**. Core attribute state resides inside `USovereignAttributeComponent`, while training focus, stat experience accumulation, and level-up rollover evaluation are encapsulated within `USovereignLevelComponent`. Simultaneously, `USovereignQiComponent` is expanded to process low-frequency environmental Qi aura gathering via the framework's heartbeat pipeline (`OnSovereignHeartbeat`), eliminating per-frame physics tick overhead.

---

## 2. Summary of Required Changes

### A. C++ Component & Data Architecture

1. **New Component: `USovereignLevelComponent`**
   - **Role**: Houses active entity training state machines, stat experience accumulators, and level-up rollover triggers.
   - **Attachment Strategy**: Attaches directly to `USovereignAttributeComponent` (or is instantiated alongside it). `USovereignAttributeComponent` serves as the core anchor component that determines presence of downstream specialized components (`USovereignQiComponent`, `USovereignBioComponent`, `USovereignLevelComponent`).
   - **Training Focus Enum (`ESovereignTrainingFocus`)**:
     - `None` (Idle)
     - `Qi` (Qi Cultivation)
     - `Strength` (Strength Training)
     - `Dexterity` (Agility & Coordination)
     - `Constitution` (Endurance & Hardening)
     - `Intelligence` (Study & Arcana)
     - `Wisdom` (Meditation & Insight)
     - `Charisma` (Presence & Aura)
     - `Luck` (Fortune Seeking)
   - **XP Accumulators**: Stores continuous `double`/`float` experience totals (`StrengthXP`, `DexterityXP`, `ConstitutionXP`, `IntelligenceXP`, `WisdomXP`, `CharismaXP`, `LuckXP`).

2. **`USovereignAttributeComponent` Integration & Precision XP Rollover**
   - **Attribute Increment Logic**: When `USovereignLevelComponent` accumulates experience reaching a whole number threshold (e.g., `StrengthXP` increases from `0.99` to `1.01`), `USovereignAttributeComponent` increments the underlying base stat (e.g., `BaseStrength` increases from 10 to 11).
   - **Continuous Retention (No XP Zeroing)**: The experience accumulator retains the full continuous float value (`1.01` rather than resetting to `0.0`). This ensures zero loss of fractional experience gains, maintaining mathematical precision for scaling leveling curves and audit logs.

3. **`USovereignQiComponent` Low-Frequency Environmental Qi Aura Gathering**
   - **Heartbeat Execution (`OnSovereignHeartbeat`)**: Replaces per-frame `TickComponent` polling with low-frequency heartbeat spatial queries (`GetWorld()->OverlapMultiByChannel` or actor sphere sweeps).
   - **Aura Emitter & Receiver Properties**:
     - `QiAuraRadius` (e.g., 500.0 units)
     - `bIsQiSource` (boolean flag)
     - `QiGive` (emission rate per heartbeat)
     - `QiTake` (absorption capacity per heartbeat)
   - **Distance Falloff Formula**:
     $$\text{DistanceFactor } D = \text{FMath::Clamp}\left(1.0 - \frac{\text{Distance}}{\text{QiAuraRadius}}, 0.0, 1.0\right)$$
     $$\text{QiGain} = \min(\text{Source.QiGive}, \text{Self.QiTake}) \times D \times \text{HeartbeatInterval}$$
   - **Qi Cap Auto-Update Loop**: When `CurrentQi >= MaxQiCapacity` during Qi cultivation (`TrainingFocus == Qi`), `MaxQiCapacity` increments by `+1` (or formula tier), `CurrentQi` wraps around preserving residual energy, and `TotalQiAccumulated` records lifetime progress while driving `Tiredness` and `QiEssence` depletion in `USovereignBioComponent`.

4. **DataTable & CSV Schema Expansion (`FSovereignItemRow` & `DT_SovereignSpeciesTemplate.csv`)**
   - Expand `FSovereignItemRow` in `WispsCPPVR/Source/WispCPP7VR/Public/DataTables/SovereignDataTypes.h` with explicit item/action XP reward fields:
     - `StrengthXP` (`double`)
     - `DexterityXP` (`double`)
     - `ConstitutionXP` (`double`)
     - `IntelligenceXP` (`double`)
     - `WisdomXP` (`double`)
     - `CharismaXP` (`double`)
     - `LuckXP` (`double`)
   - Consumables (e.g., Spirit Herbs, Dragon Meat) and actions grant immediate or over-time attribute XP to the consumer's `USovereignLevelComponent`.

5. **Python Importer Suite & Editor Utility Widget (EUW) Integration**
   - Update `import_species_data_assets.py` to parse item XP columns from `DT_SovereignSpeciesTemplate.csv`.
   - Ensure reflection-based EUW import tools (`EUW_SovereignCSVImporter`) map XP fields cleanly into generated `USovereignSpeciesData` Primary DataAssets and DataTables without schema mismatches.

6. **Save System & JSON Serialization (`USovereignSaveableEntityComponent` & Brokers)**
   - `USovereignLevelComponent` implements `ISovereignBrokerInterface`, serializing active training focus and attribute XP state under the `"Sovereign.Level"` JSON namespace key.
   - `USovereignQiComponent` expands its `"Sovereign.Qi"` JSON serialization to persist `QiAuraRadius`, `bIsQiSource`, `QiGive`, and `QiTake` values across world saves and vessel transitions.

---

## 3. Gap Analysis

| Architectural Dimension | Current Codebase State | B-059 Target Spec (GDD 15) | Identified Gap | Remediation Strategy |
|:---|:---|:---|:---|:---|
| **Leveling Component** | Non-existent in `Public/Components/`. Stat experience lacks a dedicated component container. | Standalone `USovereignLevelComponent` managing `ESovereignTrainingFocus`, XP accumulators, and level-up loops. | Missing component implementation. | Create `USovereignLevelComponent` deriving from `USovereignBaseComponent`. |
| **Component Attachment Paradigm** | Specialized components instantiated directly on `ASovereignLivingEntity` / `ASovereignBaseCharacter`. | Leveling capability attached directly to `USovereignAttributeComponent`, treating Attributes as the core entity anchor. | Leveling logic decoupled from entity base classes rather than anchored to `USovereignAttributeComponent`. | Design `USovereignLevelComponent` to pair with `USovereignAttributeComponent` as a child/sibling component attached upon attribute initialization. |
| **Stat XP Rollover Math** | `SyncStatsFromEntity()` calculates attributes as `BaseStat + FloorToInt(StatExperience)`. | Fractional XP accumulated continuously (e.g., `0.99` + `0.02` = `1.01`), triggering base stat increment while retaining `1.01`. | Potential loss or improper zeroing of leftover XP if whole-number truncation resets accumulators. | Enforce continuous double/float retention on XP accumulators without zeroing residual remainders upon level increment. |
| **Item CSV Schema** | `FSovereignItemRow` contains D&D base attributes and bio rates, but no item XP reward fields. | `FSovereignItemRow` and `DT_SovereignSpeciesTemplate.csv` include `StrengthXP` through `LuckXP`. | Missing XP fields in C++ struct and CSV headers. | Add 7 XP double properties to `FSovereignItemRow` and update CSV parser/importers. |
| **Qi Gathering Execution** | `USovereignQiComponent::ProcessQiFlow()` executes on per-frame `TickComponent`. | Environmental Qi aura gathering executes on low-frequency `OnSovereignHeartbeat` spatial queries. | Per-frame physics/distance calculation creates CPU overhead when scaled across large herds. | Migrate spatial aura queries strictly to `OnSovereignHeartbeat` using `GetWorld()->OverlapMultiByChannel` or actor sweeps. |
| **Persistence & Save System** | Bio and Qi components serialize to JSON; Leveling state is unpersisted. | Full JSON serialization of `ESovereignTrainingFocus` and stat XP under `"Sovereign.Level"`. | Unsaved leveling progress across session saves/loads. | Implement `ISovereignBrokerInterface` on `USovereignLevelComponent` for JSON save/load. |

---

## 4. SWOT Analysis

### 🔝 Strengths
* **Clean Architectural Decoupling**: Separating raw attribute storage (`USovereignAttributeComponent`) from training execution (`USovereignLevelComponent`) keeps base components lightweight while enabling modular capability attachment.
* **Core Attribute Anchoring**: Anchoring progression to `USovereignAttributeComponent` establishes a single Source of Truth (SSoT) for entity capabilities across static entities, living creatures, and spirit wisps.
* **Performance-Optimized Heartbeat Pipeline**: Leveraging `OnSovereignHeartbeat` for spatial Qi aura gathering prevents frame-rate dips and CPU physics bottlenecks even in dense simulations with hundreds of entities.
* **Mathematical Precision**: Retaining exact continuous XP values (e.g. `1.01`) during level-up threshold crossing prevents experience loss, enabling high-fidelity leveling curves and exact audit logging.

### 📉 Weaknesses
* **Component Memory Overhead**: Adding `USovereignLevelComponent` increases component allocation footprint per actor if attached indiscriminately to non-growing static entities.
* **Heartbeat Query Clumping**: If hundreds of entities trigger `OnSovereignHeartbeat` spatial overlap queries in the exact same frame tick, a micro-stutter could occur unless heartbeat execution is staggered.
* **Multi-Component State Synchronization**: Coordinates four distinct components (`Attribute`, `Level`, `Bio`, `Qi`), requiring clean initialization sequence handling to prevent nullptr accesses during `BeginPlay`.

### 🚀 Opportunities
* **Universal C++ & Game Design Foundation**: Provides a standardized, data-driven progression system usable across all gameplay mechanics—cultivation games, RPG stat growth, pet/livestock breeding, and digital twin simulation systems.
* **Emergent Gameplay Synergy**: Combining environmental Qi auras with metabolic depletion enables complex sandbox emergent behaviors (e.g., AI creatures seeking out Qi-dense ancient trees or rare spirit herbs when exhausted or training).
* **Seamless Tooling & EUW Integration**: Extends the Sovereign Framework's Editor Utility Widget suite, enabling game designers to tune item XP values directly in CSV templates and auto-import them with 1-Click execution.
* **Spirit Wisp Possession Mechanics**: When a player or AI Wisp possesses an entity vessel, `USovereignLevelComponent` preserves vessel-specific training stats while seamlessly exposing progression back to the Wisp controller.

### ⚠️ Threats
* **Dense Spatial Query Overload**: In extreme scenarios (e.g. 500+ Qi sources overlapping in a tight arena), `OverlapMultiByChannel` calls during heartbeats could still introduce spatial index contention if not distance-capped or throttled.
* **Save File Schema Drift**: Unsaved or legacy entity state representations from earlier sprint iterations could fail to deserialize if `"Sovereign.Level"` JSON fields are expected without fallback defaults.

---

## 5. Strategic Impact & Game Design Leverage

### A. Universal C++ Functionality Standard
By making `USovereignAttributeComponent` the core anchor for `USovereignLevelComponent`, the Sovereign Framework establishes a single, universal progression model:
1. **Entities with Attributes** automatically gain access to stat leveling, XP tracking, and cultivation focus.
2. **Entities with Bio Components** feed metabolic energy (`Hunger`, `Fatigue`) directly into the leveling engine, creating a tight loop between survival and growth.
3. **Entities with Qi Components** participate in atmospheric energy exchange, enabling both active training (`Qi Cultivation`) and passive environmental harvesting (`Qi Aura Gathering`).

### B. Game Design & Simulation Leverage
- **Emergent Ecosystems**: Plants radiate Qi, herbivores eat plants to gain `ConstitutionXP` and `QiEssence`, carnivores hunt herbivores to gain `StrengthXP`, and cultivators meditate near Qi-rich flora to accelerate breakthrough loops.
- **Data-Driven Balance**: Designers control the entire progression curve, item yields, and species limits strictly through `DT_SovereignSpeciesTemplate.csv` and Unreal Engine Primary DataAssets (`USovereignSpeciesData`), requiring zero code changes for balance iteration.
- **Cross-Domain Reusability**: The same C++ progression architecture that powers fantasy cultivation in Unreal Engine can power real-world digital twin hardware aging, stress analysis, and battery/charge degradation logic in maritime and industrial simulation applications.

---

## 6. Recommendations & Implementation Roadmap

1. **Phase 1 (C++ Foundation)**: Implement `USovereignLevelComponent`, `ESovereignTrainingFocus`, and continuous XP rollover logic attached to `USovereignAttributeComponent`.
2. **Phase 2 (Qi Aura Heartbeat)**: Integrate heartbeat-driven `OverlapMultiByChannel` spatial queries into `USovereignQiComponent` with distance falloff.
3. **Phase 3 (Data Schema & Importer)**: Expand `FSovereignItemRow` with item XP reward fields and update Python importer scripts (`import_species_data_assets.py`).
4. **Phase 4 (Persistence & Validation)**: Add JSON serialization under `"Sovereign.Level"` and verify state restoration across save/load cycles.
