// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-15

# Sovereign Framework Game Design Document
## 15: Autonomous Entity Leveling, Stat Training, Metabolism & Environmental Qi Aura Systems

---

## 1. Executive Summary & Vision

The **Sovereign Framework Entity Progression Suite** provides a simulation-driven, autonomous training and cultivation ecosystem for all living and spirit entities in the Sovereign universe (ranging from simple plants and animals to complex humanoid vessels and dragons). Inspired by deep cultivation mechanics (such as *Amazing Cultivation Simulator*) and classic d20/D&D attribute progression, this system decouples core stat storage from progression execution, allowing actors to train attributes, process biological metabolism, and harvest ambient Qi from surrounding environmental objects.

---

## 2. Architectural Component Hierarchy

To maintain clean separation of concerns and prevent bloated base entity classes, progression logic is modularized across specialized components:

```
                      +-----------------------------+
                      |    ASovereignBaseEntity     |
                      +--------------+--------------+
                                     |
                                     v
                      +-----------------------------+
                      |   ASovereignLivingEntity    |
                      |  / ASovereignBaseCharacter  |
                      +--------------+--------------+
                                     |
      +------------------+-----------+------------+--------------------+
      |                  |                        |                    |
      v                  v                        v                    v
+-----------+   +-----------------+     +-------------------+   +--------------------+
| Bio Comp  |   | Attribute Comp  |     |     Qi Comp       |   |     Level Comp     |
| (Vitals,  |   | (Base Stats &   |     | (Spiritual Pool & |   | (Training Focus,   |
| Metabolism|   | Realtime Bars)  |     | Ambient Aura)     |   | Progression, XP)   |
+-----------+   +-----------------+     +-------------------+   +--------------------+
```

### Component Roles & Responsibilities

1. **`USovereignBioComponent`**:
   - Manages metabolic vitals (`Hunger`, `Hydration`, `Fatigue`, `Tiredness`, `Toxicity`, `WasteLevel`).
   - Tracks nutrient reserves (`Carbs`, `Proteins`, `Fats`, `Minerals`, `QiEssence`).
   - Drains reserves during active physical or spiritual training focus.

2. **`USovereignAttributeComponent`**:
   - Stores base D&D physical and mental attributes (`Strength`, `Dexterity`, `Constitution`, `Intelligence`, `Wisdom`, `Charisma`, `Luck`).
   - Stores usable resource clamps (`MaxHealth`, `MaxStamina`).
   - Performs stat sync from `USovereignSpeciesData` templates.

3. **`USovereignQiComponent`**:
   - Stores current Qi pool (`CurrentQi`), capacity (`MaxQiCapacity`), and purity (`QiPurity`).
   - Defines environmental Qi emitter properties (`QiGive`, `QiTake`, `bIsQiSource`).
   - Stores ambient `QiAuraRadius` for spatial energy radiation.

4. **`USovereignLevelComponent`** *(New Subsystem Component)*:
   - Houses the active training focus (`ESovereignTrainingFocus`).
   - Stores stat experience accumulators and level-up rollover logic.
   - Evaluates tick/heartbeat progress depending on entity training state.
   - Can be selectively attached only to entities capable of growth/leveling.

---

## 3. Training Focus & Auto-Update Cap Loop

### Training Focus Enum (`ESovereignTrainingFocus`)
Location: `WispsCPPVR/Source/WispCPP7VR/Public/DataTables/SovereignDataTypes.h`

```cpp
UENUM(BlueprintType)
enum class ESovereignTrainingFocus : uint8
{
    None         UMETA(DisplayName = "Idle / None"),
    Qi           UMETA(DisplayName = "Qi Cultivation"),
    Strength     UMETA(DisplayName = "Strength Training"),
    Dexterity    UMETA(DisplayName = "Agility & Coordination"),
    Constitution UMETA(DisplayName = "Endurance & Hardening"),
    Intelligence UMETA(DisplayName = "Study & Arcana"),
    Wisdom       UMETA(DisplayName = "Meditation & Insight"),
    Charisma     UMETA(DisplayName = "Presence & Aura"),
    Luck         UMETA(DisplayName = "Fortune Seeking"),
    MAX          UMETA(Hidden)
};
```

### Qi Cap & Level-Up Auto-Update Loop
When an entity trains Qi (`TrainingFocus == ESovereignTrainingFocus::Qi`):
1. **Accumulation**: `CurrentQi` increases based on base regen + wisdom bonus + environmental aura bonus.
2. **Cap Breach**: When `CurrentQi >= MaxQiCapacity`:
   - `MaxQiCapacity` increases by `+1` (or scaling tier formula).
   - `CurrentQi` is reset back to `1.0f` + `LeftoverXP` (preserving overflow energy).
   - `TotalQiAccumulated` tracks lifetime progress.
3. **Metabolic Drain**: Training Qi accelerates `Tiredness` and consumes `QiEssence` nutrient reserves.

---

## 4. Action & Food-Based Attribute Progression

### Item Schema XP Integration (`FSovereignItemRow` & CSV Template)
Food and consumable items specify direct attribute experience gains in `DT_SovereignSpeciesTemplate.csv` / `DT_SovereignItemTemplate.csv`:

| Property | Type | Description |
|:---|:---|:---|
| `StrengthXP` | `double` | Experience granted towards Strength upon consumption/action. |
| `DexterityXP` | `double` | Experience granted towards Dexterity. |
| `ConstitutionXP` | `double` | Experience granted towards Constitution. |
| `IntelligenceXP` | `double` | Experience granted towards Intelligence. |
| `WisdomXP` | `double` | Experience granted towards Wisdom. |
| `CharismaXP` | `double` | Experience granted towards Charisma. |
| `LuckXP` | `double` | Experience granted towards Luck. |

### Integer Rollover Math (`TrainAttribute`)
When attribute experience is granted (e.g. eating high-protein dragon meat grants `+0.05` `StrengthXP`):
$$\text{TargetXP} \leftarrow \text{TargetXP} + \text{Amount}$$

If $\text{TargetXP} \ge 1.0$:
$$\Delta \text{Levels} = \lfloor \text{TargetXP} \rfloor$$
$$\text{TargetLevel} \leftarrow \text{TargetLevel} + \Delta \text{Levels}$$
$$\text{TargetXP} \leftarrow \text{TargetXP} - \Delta \text{Levels}$$

*Example*: If `Strength` is 12 and `StrengthXP` reaches `1.04`, `Strength` increments to 13, and `StrengthXP` carries over as `0.04`.

---

## 5. Environmental Qi Aura & Low-Frequency Gathering System

### Qi Gathering Sphere Mechanics
Entities and items with `USovereignQiComponent` can act as ambient Qi sources (e.g., Spirit Herbs, Qi Crystals, Ancient Trees, Dragon Veins).

- **Variable Sphere Radius**: `QiAuraRadius` (default 500.0 units, configurable per item/actor).
- **Low-Frequency Performance Strategy**: To avoid per-frame collision physics overhead, ambient Qi gathering avoids `TickComponent` sphere triggers and instead uses **Heartbeat Queries** (`OnSovereignHeartbeat`).

### Qi Flow Formula
During `OnSovereignHeartbeat`:
1. The entity performs a low-frequency spatial query (`GetOverlappingActors` or `SphereSweep`) using `QiAuraRadius`.
2. For every nearby actor containing a `USovereignQiComponent` where `bIsQiSource == true`:
   - Calculate distance falloff $D = \text{FMath::Clamp}(1.0 - \frac{\text{Distance}}{\text{QiAuraRadius}}, 0.0, 1.0)$.
   - Calculate flow rate:
   $$\text{QiGain} = \min(\text{Source.QiGive}, \text{Self.QiTake}) \times D \times \text{HeartbeatInterval}$$
   - Apply gain to entity's `CurrentQi` and boost effective `QiRegenRate`.

---

## 6. Roadmap & Next Steps

1. Implement `USovereignLevelComponent` and expose `ESovereignTrainingFocus` in C++.
2. Expand `FSovereignItemRow` in `SovereignDataTypes.h` to include item XP fields.
3. Integrate `OnSovereignHeartbeat` Qi aura sweep into `USovereignQiComponent`.
4. Update Python importer scripts (`import_species_data_assets.py`) to map XP reward columns from CSV data.
