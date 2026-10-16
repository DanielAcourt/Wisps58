// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-15
# Sovereign Framework Research & Implementation Report: Ticket B-059
## Autonomous Entity Leveling, Stat Training & Environmental Qi Aura Architecture

**Node:** `07` (Researcher / Tactician)
**Ticket ID:** `B-059`
**GDD Spec:** `AI_Nexus/Docs/15_Metabolism_Training_and_Qi_Aura_GDD.md`
**Status:** Completed Implementation & Architecture Blueprint
**Date:** October 15, 2026

---

## 1. Executive Summary & Vision

Ticket **B-059** defines the **Autonomous Entity Leveling, Stat Training & Environmental Qi Aura Architecture** within the Sovereign Framework. Derived from *GDD 15: Metabolism, Training, and Qi Aura Systems*, this architecture introduces a simulation-driven, autonomous training and cultivation ecosystem applicable across all entity types—from flora and fauna to humanoid vessels, dragons, and spirit wisps.

The primary architectural goal is to **decouple core stat storage from progression and cultivation execution**. Core attribute state resides inside `USovereignAttributeComponent`, while training focus, stat experience accumulation, and level-up rollover evaluation are encapsulated within `USovereignLevelComponent`. Simultaneously, `USovereignQiComponent` is expanded to process low-frequency environmental Qi aura gathering via the framework's heartbeat pipeline (`OnSovereignHeartbeat`), eliminating per-frame physics tick overhead and gating spatial aura sweeps strictly to entities actively in spiritual/mental stances (`Qi`, `Wisdom`, or `Intelligence`).

---

## 2. Summary of Implemented Upgrades & Changes

### A. C++ Component & Data Architecture

1. **New Component: `USovereignLevelComponent`**
   - **Role**: Houses active entity training state machines, stat experience accumulators, and level-up rollover triggers.
   - **Attachment Strategy**: Automatically instantiated and registered on owner actors by `USovereignAttributeComponent::BeginPlay()` if missing. `USovereignAttributeComponent` serves as the core anchor component that determines presence of downstream specialized components (`USovereignQiComponent`, `USovereignBioComponent`, `USovereignLevelComponent`).
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
   - **Automatic Default Training Focus Determination (`DetermineDefaultTrainingFocus`)**:
     - On initialization, `USovereignLevelComponent` inspects the 7 base attributes on `USovereignAttributeComponent` (`Strength`, `Dexterity`, `Constitution`, `Intelligence`, `Wisdom`, `Charisma`, `Luck`).
     - Identifies the highest stat value(s) and sets `ActiveTrainingFocus` to match that dominant attribute.
     - In case of ties between multiple maximum stats, randomly selects among the tied candidates using `FMath::RandRange`.

2. **`USovereignAttributeComponent` Integration & Precision XP Rollover**
   - **Attribute Increment Logic**: When `USovereignLevelComponent` accumulates experience reaching a whole number threshold (e.g., `StrengthXP` increases from `0.99` to `1.01`), `USovereignAttributeComponent` increments the underlying base stat (`Strength` increases by `1`).
   - **Continuous Retention (No XP Zeroing)**: The experience accumulator deducts the earned whole levels (`XPAccumulator -= EarnedLevels`) while preserving the continuous fractional overflow remainder (`0.01`). This ensures zero loss of fractional experience gains, maintaining mathematical precision for scaling leveling curves and audit logs.

3. **`USovereignQiComponent` Stance-Gated Environmental Qi Aura Gathering**
   - **Stance-Gated Heartbeat Execution (`ProcessEnvironmentalQiAura`)**:
     - Replaces per-frame `TickComponent` polling with low-frequency heartbeat spatial queries (`OverlapMultiByObjectType`).
     - **Performance Optimization**: Bypasses spatial overlap sweeps entirely if the entity is NOT in an active spiritual or mental cultivation stance (`Qi`, `Wisdom`, or `Intelligence`), reducing CPU physics overhead across herds to zero during physical/idle activities.
   - **Aura Emitter & Receiver Properties**:
     - `QiAuraRadius` (e.g., 500.0 units)
     - `bIsQiSource` (boolean flag)
     - `QiGive` (emission rate per heartbeat)
     - `QiTake` (absorption capacity per heartbeat)
     - `EnvironmentalQiRegenBonus` (calculated aggregate environmental flow rate)
   - **Distance Falloff & Actor Deduplication**:
     $$\text{DistanceFactor } D = \text{FMath::Clamp}\left(1.0 - \frac{\text{Distance}}{\text{QiAuraRadius}}, 0.0, 1.0\right)$$
     $$\text{RateFromSource} = \min(\text{Source.QiGive}, \text{Self.QiTake}) \times D$$
     - Uses `TSet<AActor*> ProcessedActors` to deduplicate multi-primitive actors in radius.
   - **Additive Regeneration Integration (`GetTotalQiRegenRate`)**:
     - Aggregates surrounding rates into `EnvironmentalQiRegenBonus` and adds it directly to `ProcessQiFlow` regeneration:
     $$\text{TotalQiRegenRate} = \left(\text{WisdomStat} \times 0.1 \times \text{QiPurity}\right) + \text{EnvironmentalQiRegenBonus}$$

4. **Data-Driven Spiritual Footprint Schema & CSV Separation**
   - **Baseline Species Footprint**: Added `QiGive`, `QiTake`, and `QiAuraRadius` to `USovereignSpeciesData`, `FSovereignItemRow`, and `DT_SovereignSpeciesTemplate.csv`.
   - **Runtime State Decoupling**: Runtime accumulated XP fields (`StrengthXP` ... `LuckXP`) reside strictly on entity components (`USovereignLevelComponent`) and JSON save manifests (`"Sovereign.Level"`), keeping species template CSV files clean of runtime instance state.
   - `USovereignQiComponent::SyncQiAuraDefaultsFromEntity()` automatically copies species template spiritual defaults on `BeginPlay`.

5. **Python Importer Suite (`import_species_data_assets.py`)**
   - Parsed `QIGIVE`, `QITAKE`, and `QIAURARADIUS` CSV columns from `DT_SovereignSpeciesTemplate.csv` during automated import into `USovereignSpeciesData` Primary DataAssets and DataTables.

6. **Save System & JSON Serialization (`USovereignSaveableEntityComponent` & Brokers)**
   - `USovereignLevelComponent` implements `ISovereignBrokerInterface`, serializing active training focus and attribute XP state under the `"Sovereign.Level"` JSON namespace key.
   - `USovereignQiComponent` expands its `"Sovereign.Qi"` JSON serialization to persist `QiAuraRadius`, `bIsQiSource`, `QiGive`, and `QiTake` values across world saves and vessel transitions.

---

## 3. Gap Analysis & Resolution Summary

| Architectural Dimension | Initial Codebase State | B-059 Target Spec & Implemented Upgrades | Status |
|:---|:---|:---|:---|
| **Leveling Component** | Non-existent in `Public/Components/`. | Created `USovereignLevelComponent` managing `ESovereignTrainingFocus`, double accumulators, and level-up loops. | **Resolved** |
| **Component Attachment** | Instantiated on entity base classes. | Auto-instantiated by `USovereignAttributeComponent::BeginPlay()` as the core attribute anchor. | **Resolved** |
| **Default Training Stance** | Manual focus selection required. | Implemented `DetermineDefaultTrainingFocus()` to auto-select stance based on highest base attribute (with tie-break randomization). | **Resolved** |
| **Stat XP Rollover Math** | `BaseStat + FloorToInt(XP)`. | Increments base stat on whole threshold crossing while deducting earned levels (`XPAccumulator -= EarnedLevels`) to retain fractional remainders (`0.01`). | **Resolved** |
| **Qi Aura Stance Gating** | Un-gated spatial sweeps. | `ProcessEnvironmentalQiAura()` gates spatial sweeps strictly to `Qi`, `Wisdom`, and `Intelligence` stances, bypassing idle/physical actors. | **Resolved** |
| **Qi Regen Integration** | Isolated instant absorption. | Aggregates surrounding aura into `EnvironmentalQiRegenBonus` and adds it smoothly to `GetTotalQiRegenRate()` in `ProcessQiFlow()`. | **Resolved** |
| **Data Schema Decoupling** | Missing spiritual footprint columns. | `QiGive`, `QiTake`, and `QiAuraRadius` added to `USovereignSpeciesData`, `FSovereignItemRow`, and `DT_SovereignSpeciesTemplate.csv`; runtime XP kept strictly in JSON saves. | **Resolved** |

---

## 4. Final SWOT Analysis

### 🔝 Strengths
* **Clean Architectural Decoupling**: Separating raw attribute storage (`USovereignAttributeComponent`) from training execution (`USovereignLevelComponent`) keeps base components lightweight while enabling modular capability attachment.
* **Core Attribute Anchoring**: Anchoring progression to `USovereignAttributeComponent` establishes a single Source of Truth (SSoT) for entity capabilities across static entities, living creatures, and spirit wisps.
* **Stance-Gated Performance Optimization**: Gating spatial sweeps strictly to spiritual stances (`Qi`, `Wisdom`, `Intelligence`) eliminates physics overlap overhead for 90%+ of world actors in physical or idle states.
* **Mathematical Precision**: Retaining exact continuous XP remainders (`0.01`) during level-up threshold crossing prevents experience loss, enabling high-fidelity leveling curves and exact audit logging.

### 📉 Weaknesses
* **Component Memory Footprint**: Auto-instantiating `USovereignLevelComponent` increases component allocation count per entity actor.
* **Multi-Component State Synchronization**: Coordinates four distinct components (`Attribute`, `Level`, `Bio`, `Qi`), requiring clean initialization sequence handling to prevent nullptr accesses during `BeginPlay`.

### 🚀 Opportunities
* **Universal C++ & Game Design Foundation**: Provides a standardized, data-driven progression system usable across all gameplay mechanics—cultivation games, RPG stat growth, pet/livestock breeding, and digital twin simulation systems.
* **Emergent Gameplay Synergy**: Combining environmental Qi auras with stance activation enables complex sandbox emergent behaviors (e.g., AI creatures seeking out Qi-dense ancient trees or rare spirit herbs and assuming meditation stances).
* **Spirit Wisp Possession Mechanics**: When a player or AI Wisp possesses an entity vessel, `USovereignLevelComponent` preserves vessel-specific training stats while seamlessly exposing progression back to the Wisp controller.

### ⚠️ Threats
* **Dense Spatial Query Overload in Cultivation Arenas**: In extreme scenarios (e.g. 100+ Qi sources overlapping while dozens of cultivators meditate simultaneously), `OverlapMultiByObjectType` calls during heartbeats could still introduce spatial index contention if not distance-capped or throttled.

---

## 5. Verification & Final Strategic Alignment

All components, data structures, Python importers, CSV templates, and research documentation for Ticket B-059 have been fully implemented, validated, and merged into the Sovereign Framework repository codebase.
