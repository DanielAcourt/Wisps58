// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# WispsGame: The Meditation Garden & Unseen Weave
## Master Game Design Document (GDD)

---

## 1. Executive Summary & Core Concept

**WispsGame** is a Desktop 3D Meditation Garden Cultivation & Rogue-lite Sandbox RPG built on Unreal Engine 5 and the **Sovereign Framework**.

The player embodies a disembodied **Wisp Spirit** (`ASovereignPlayerWisp`) operating in first-person view within a sacred **Meditation Garden**. By flying through the garden and possessing various entities (flora, fauna, rocks, ancient beasts), the player dynamically switches entity **Training Focus States** (`ESovereignTrainingFocus`), optimizes spatial Qi resonance, and cultivates the surrounding ecosystem.

The primary objective of the Meditation Garden phase is to nurture the surrounding environmental Qi fields to incubate and hatch a dormant **Male Domain Dragon Egg**. Once hatched and raised to maturity, the Dragon unlocks the sealed **Dungeon Gateway**. The player leads their Dragon into procedural dungeon expeditions to rescue dormant **Female Dragon Eggs**, unlock rare mutations, and ascend through realms.

If the Dragon perishes during a dungeon run, the world resets back to the Meditation Garden—but a percentage of accrued **Soul Qi**, unlocked species seeds, Wisp attribute caps, and inherited genetic mutations carry over permanently into subsequent runs.

---

## 2. Core Gameplay Loop

```text
                           +-------------------------------------+
                           |         1. THE WISPS SPIRIT          |
                           |  Flies in First-Person Flight Mode  |
                           |  Possesses Garden Entities / Vessel  |
                           +------------------+------------------+
                                              |
                                              v
                           +-------------------------------------+
                           |      2. MEDITATION GARDEN PHASE     |
                           |  - Switch Training States           |
                           |  - Spatial Qi Aura Placement        |
                           |  - Cultivate Plants & Soil          |
                           |  - Incubate Male Dragon Egg         |
                           +------------------+------------------+
                                              |
                                              v
                           +-------------------------------------+
                           |      3. DRAGON HATCH & MATURITY     |
                           |  - Raise Male Dragon to Adult Stage |
                           |  - Unseal Dungeon Gateway           |
                           +------------------+------------------+
                                              |
                                              v
                           +-------------------------------------+
                           |      4. DUNGEON EXPEDITION (PHASE 2)|
                           |  - 3x3 Rank Matrix Party Formation  |
                           |  - Turn-Based Dedicated Arena Combat|
                           |  - Action Class Unlocks & CP Multi  |
                           |  - Seek Dormant Female Dragon Eggs  |
                           +------------------+------------------+
                                  /                        \
                    [IF DRAGON DIES]                 [RESCUE FEMALE EGG]
                            /                                \
                           v                                  v
         +----------------------------------+   +----------------------------------+
         |  5A. ROGUE-LITE WORLD RESET      |   |  5B. EXTRACTION & BREEDING       |
         |  - Convert Progress -> Soul Qi   |   |  - Instant Teleport vs Reverse   |
         |  - Retain Wisp % Stat Caps       |   |    Return ("Overburdened" +Qi)   |
         |  - Retain Unlocked Species Seeds |   |  - Hatch & Mating Mutation Loop  |
         +----------------------------------+   +----------------------------------+
                           |                                  |
                           +----------------->  <-------------+
                                              |
                                              v
                           +-------------------------------------+
                           |    6. NEXT RUN / DEEPER GARDEN      |
                           |  Stronger Baseline Garden & Dragons |
                           +-------------------------------------+
```

---

## 3. Meditation Garden Mechanics ("Tamagotchi Cultivation")

### 3.1 Spirit Possession & Entity State Switching
- **First-Person Spirit Movement**: The player controls `ASovereignPlayerWisp` with first-person flying controls.
- **Possession Mechanics**: Approaching any interactable object or living entity and pressing `[F]` initiates the Possession Lifecycle via `IInteractionInterface`.
- **Training Focus Enum (`ESovereignTrainingFocus`)**:
  While possessing an entity (or selecting it via hotkey/UI), the Wisp can toggle its active training focus:
  - `None` / `Idle`: Standard passive state.
  - `Qi`: **Qi Cultivation**. Entity stops physical growth and draws ambient Qi from surrounding objects, increasing its maximum Qi capacity (`MaxQiCapacity`) and spiritual resonance.
  - `Strength`: **Physical Growth & Yield**. Accelerates physical growth speed and biological scale; draws physical nutrients from surrounding soil/rocks.
  - `Dexterity`: **Agility & Coordination**. Increases movement speed and action frequency.
  - `Constitution`: **Endurance & Health**. Increases maximum health (`MaxHealth`) and durability; grants seed propagation resistance.
  - `Intelligence`: **Arcana & Qi Max Capacity**. Expands statutory energy thresholds and unlocks higher magic efficiency.
  - `Wisdom`: **Meditation & Qi Regeneration**. Dramatically boosts local environmental Qi aura output to surrounding entities.
  - `Charisma`: **Presence & Breeding Attractiveness**. Boosts aura radius and mating compatibility.
  - `Luck`: **Mutation & Rare Harvest Chance**. Increases seed drop rate, mutation odds, and critical yield probabilities.

### 3.2 Spatial Qi Resonance & Proximity Interactions
Entities in the garden do not exist in isolation—their spatial placement creates synergistic Qi fields:
- **Low-Frequency Heartbeat Queries**: Driven by `OnSovereignHeartbeat` (preventing per-frame physics overhead).
- **Mutual Interactions**:
  - **Tree in `Wisdom` State**: Acts as a Qi emitter (`bIsQiSource = true`, radiating energy via `QiAuraRadius`). Nearby plants and the Dragon Egg in range gather this energy to accelerate incubation.
  - **Tree in `Strength` State**: Absorbs minerals from surrounding Rocks/Soil, depleting nearby physical nutrients to rapidly mature and produce seeds.
  - **Rocks & Minerals**: Non-biological entities (`USovereignBioComponent` is null). They do not have health or hunger, but store high mineral density (`QiGive` / `QiTake`) that surrounding biological entities draw upon.
  - **Biological Entities (Trees, Herbs, Animals)**: Possess `USovereignBioComponent`. When health/maturity reaches peak threshold, trees flower and scatter seeds into nearby soil for natural propagation. If health drops to zero, the plant dies and leaves behind enriched soil.

---

## 4. The Dragon Cycle, Hatching & Unsealed Gateway

### 4.1 Male Domain Dragon Incubator
- **Garden Focal Point**: At the center of the Meditation Garden lies the ancient **Male Domain Dragon Egg**.
- **Incubation Condition**: The egg requires a minimum total ambient Qi threshold to begin hatching.
- **Raising the Dragon**: Once hatched, the Male Dragon starts at Stage 0 (Hatchling) and requires biological feeding and `Wisdom`/`Qi` stance cultivation across its heartbeat maturation stages to reach Stage 3 (Adult Domain Dragon).

### 4.2 Unsealing the Dungeon Gateway
- **The Sealed Gateway**: A grand rune-carved archway stands at the edge of the Meditation Garden, sealed by ancient seals.
- **Gateway Activation**: When the Male Dragon reaches adulthood, it can channel its accumulated domain Qi into the archway, breaking the seal and opening the passage to the **Dungeon Realm**.

---

## 5. Phase 2: Dungeon Exploration, Turn-Based Combat & Female Dragon Rescue

*(For detailed specifications, see `AI_Nexus/Docs/18_Phase2_Combat_Class_Dungeon_GDD.md`)*

### 5.1 Turn-Based Combat & Dedicated Battle Arena
- **Sequential Turn Queue**: Combat uses a strict sequential turn-based queue ordered by **Dexterity (DEX)** and speed modifiers. Time pauses during action selection.
- **Dedicated 3D Battle Arena Platform**: Touching an enemy in 3D procedural corridors transitions to a dedicated biome-tailored battle stage, eliminating camera clipping and corridor navigation bugs.

### 5.2 Action-Based Class Unlocks, Class Points (CP) & Professions
- **Starter Class**: All entities begin as **Adventurer**.
- **Action Accumulation**: Performing physical strikes, casting Qi spells, or taking damage accumulates progress towards specialized classes (*Knight*, *Wizard*, *Guardian*, *Dragoon*). Dungeon Skill Tomes also unlock advanced classes.
- **Multi-Classing via CP**: Entities equip multiple unlocked classes limited only by their **Class Point (CP)** budget.
- **Professions**: Features dedicated gathering and crafting professions (*Mining*, *Lumbering*, *Alchemy*, *Smithing*) managed via `USovereignProfessionComponent`.

### 5.3 Tactical 3x3 Rank Matrix Party Formation
Party deployment supports up to 9 slots arranged in 3 distinct functional ranks:
1. **Front Rank (Slots 1–3)**: Tanks & Melee (+15% Defense, +10% Melee Power, High Enemy Aggro).
2. **Mid Rank (Slots 4–6)**: Ranged & Offensive Spells (+15% Ranged & Qi Damage).
3. **Rear Rank (Slots 7–9)**: Support & Healers (+20% Healing & Buff Potency).
*Progression Unlocks*: Starts with Front Rank only. Mid and Rear ranks are unlocked by completing storyline taming/dungeon milestones.

### 5.4 Female Dragon Rescue & The "Reverse Return" High-Risk Loop
- **Vault Objective**: Defeating floor bosses reveals dormant **Female Dragon Eggs**.
- **Extraction Choice**:
  - **Safe Exit**: Step into the boss room Gateway Teleporter for instant return with standard egg Qi.
  - **Reverse Return ("Risk it for the Biscuit")**: Manually walk back through repopulated floors with an empowered respawned boss at the entrance. The party suffers the **Overburdened** condition (-20% all stats), but successfully escaping yields a **+100% to +200% Bonus Qi Yield** on the rescued egg!

---

## 6. Death, World Reset & Rogue-lite "Soul Qi" Meta-Progression

### 6.1 The Death Reset Trigger
If the player's primary Dragon perishes during a dungeon run:
1. The active dungeon instance and garden world state reset.
2. The player's Spirit returns to the Meditation Garden.

### 6.2 "Soul Qi" Cookie-Clicker Carryover
Upon world reset, all accrued energy, harvested crops, and total lifetime stat gains are converted into **Soul Qi** (the permanent meta-currency).

**Soul Qi Permanent Unlocks**:
- **Starter Species Seeds**: Spend Soul Qi to permanently unlock new species templates (rare trees, spirit herbs, biological fauna) in the starting garden shop/manifest.
- **Garden Baseline Qi Floor**: Permanent increase to baseline soil Qi richness and default aura radius.
- **Egg Incubation Acceleration**: Reduces total Qi energy required to hatch starting dragon eggs.
- **Wisp Stat Cap Carryover**: A percentage ($X\%$) of accumulated Wisp attributes and stat caps carry over permanently across runs, making the Wisp spirit stronger for the next cycle.
- **Retained Genetic Lineages**: Unlocked dragon mutations and female dragon breeding lineages remain in the player's permanent Soul Archive.

---

## 7. C++ & Sovereign Framework Technical Mapping

The design directly leverages existing Sovereign Framework C++ subsystems:

| Game Mechanics | C++ Class / Subsystem | Key Implementation Protocol |
|:---|:---|:---|
| **Wisp Spirit Control** | `ASovereignPlayerWisp` | First-person flight controls, targeting line-of-sight sweep, possession trigger. |
| **Possession SOP** | `IInteractionInterface` | `AttemptPossession()`, `EjectFromHost()`, `SnapToTargetNotIncludingScale`. |
| **Entity Core & Soul** | `USovereignSaveableEntityComponent` | Soul Hub mediator serializing `Bio`, `Qi`, `Attribute`, `Level`, `Class`, `Profession` brokers under named JSON tags. |
| **Training States & Level** | `USovereignLevelComponent` | `ESovereignTrainingFocus`, `TrainAttribute()` integer stat rollover, XP accumulators. |
| **Class System & CP** | `USovereignClassComponent` | Unlocked class records, CP budget validation, equipped spellbooks and passive ability slots. |
| **Professions System** | `USovereignProfessionComponent` | Tracks Mining, Lumbering, Alchemy, Smithing skill levels, recipes, and yield multipliers. |
| **Skill & Action Tracker**| `USovereignSkillComponent` | Action accumulation counters for class unlocks, skill cooldowns, and damage formulas. |
| **Attributes & Stats** | `USovereignAttributeComponent` | D&D core stats (STR, DEX, CON, INT, WIS, CHA, LCK), `SyncStatsFromEntity()`, health scaling. |
| **Spatial Qi Aura** | `USovereignQiComponent` | Low-frequency `OnSovereignHeartbeat` spatial queries, `QiAuraRadius`, `QiGive`/`QiTake` flow. |
| **Biological Metabolism** | `USovereignBioComponent` | Vitals (Hunger, Hydration), growth maturity accumulation, gestation rate for breeding. |
| **Species Templates** | `USovereignSpeciesData` | `GrowthStages`, dynamic `GrowthStageCount`, mesh soft pointers, scaling curve models. |
| **Data Importing** | `import_species_data_assets.py` | Imports `DT_SovereignSpeciesTemplate.csv` to generate C++ PDAs and Blueprint CDOs. |

---

## 8. Development Roadmap & Phase Priorities

### Phase 1 (Current Focus): Meditation Garden Core
1. **Desktop Wisp & Possession**: Refine First-Person Wisp flight and 1-Click possession interface.
2. **Garden Entity State Switching**: UI/Hotkey interface for toggling `ESovereignTrainingFocus` on possessed/selected entities.
3. **Spatial Qi Proximity**: Heartbeat Qi aura radiation and mutual plant/rock interactions in the Meditation Garden level.
4. **Male Dragon Egg Incubation**: Qi threshold gathering, hatching, and stage growth pipeline.

### Phase 2: Dungeon Expansion, Combat & Breeding Loop
1. **Unsealed Dungeon Gateway**: Gateway unsealing trigger upon Male Dragon adult maturity.
2. **Dedicated Battle Arena & Turn Queue**: Sequential turn-based combat system on dedicated biome stages.
3. **Class, CP & Skill Components**: C++ `USovereignClassComponent`, `USovereignSkillComponent`, and `USovereignProfessionComponent` integration with action-based unlocks.
4. **3x3 Rank Matrix Party Deployment**: Formation grid and rank progression milestones.
5. **Female Dragon Rescue & Reverse Return Loop**: Teleport vs. Reverse Return risk-reward loop (+Qi vs. Overburdened -20% debuff), gestation, and Ark-style attribute/mutation inheritance.
6. **Rogue-lite Soul Qi Persistence**: Death reset logic, Soul Qi currency calculation, and permanent Wisp upgrade unlocks.
