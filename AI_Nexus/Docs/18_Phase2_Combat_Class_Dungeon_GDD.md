// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# Phase 2 Game Design Document: Combat, Class Systems, Formations & Dungeon Expeditions
## Specification Document for WispsGame Phase 2 Integration

---

## 1. Executive Overview & Design Scope

Phase 2 transitions **WispsGame** from the initial **Meditation Garden** incubation sandbox into active, strategic dungeon exploration and turn-based tactical combat.

Once the primary **Male Domain Dragon** reaches adulthood in the Meditation Garden, it channels its accumulated Qi to unseal the **Dungeon Gateway**. The player leads their party of cultivated dragons and entities through procedurally generated dungeons, engaging in turn-based battles to retrieve dormant **Female Dragon Eggs**, gather rare crafting resources, and expand their lineage.

---

## 2. Turn-Based Combat System & Dedicated Battle Arena

### 2.1 Active Time Battle (ATB) Mechanics & Turn Execution Flow
- **ATB Gauge Architecture (FF7-Style)**:
  - Combat operates on an **Active Time Battle (ATB)** system managed via `USovereignATBComponent`.
  - Each combatant maintains an ATB meter ($0.0 \to 100.0\%$) filling continuously in real time based on **Dexterity (DEX)** and class speed multipliers:
    $$\Delta \text{ATB} = \text{DeltaTime} \times \text{BaseATBRate} \times \left(1.0 + \frac{\text{Dexterity}}{50.0}\right) \times \text{ClassSpeedMultiplier}$$
  - When a combatant's ATB reaches $100.0\%$, they enter the ready queue, prompting action selection.
- **Wait Mode (Default Operational Setting)**:
  - Combat defaults to **Wait Mode**: while a player is actively navigating submenus (Skills, Qi Magic, Inventory Items, Target Selection), all combatant ATB meters pause completely, granting tactical breathing room.
  - While animations are actively executing (e.g. dragon breath sweeps, spell casts), ATB accumulation is globally frozen until resolution.
- **Qi-Powered Resource Model (No MP Clutter)**:
  - Spells, martial arts, and dragon abilities consume **Qi** directly from `USovereignQiComponent` rather than a disconnected Mana pool.
  - This harmonizes in-dungeon combat actions with Meditation Garden cultivation and elemental lineage techniques.
- **In-Combat Item Consumption**:
  - Pulls directly from the possessed vessel's `USovereignInventoryComponent`.
  - Using an item consumes that dragon's entire turn, resetting their ATB to $0\%$.
  - Items with `USovereignItemFragment_Consumable` and `USovereignItemFragment_Spiritual` can target either friendly allies (healing elixirs, Qi pills, attribute buff teas) or opposing enemy ranks (offensive throwable spirit grenades, alchemy flasks).
- **Flee Mechanic & Boss Restrictions**:
  - Escape attempts are calculated via Party DEX/Luck versus Enemy Encounter DEX with a charging escape bar fallback (+25% per failure).
  - Boss battles, Dungeon Gatekeepers, and Vault Guardians have escape disabled (`bIsEscapable = false`).
  - Successfully fleeing immediately teleports the party back to the dungeon floor with no grace period, preserving active roaming mob states.
- **Damage Calculation Engine (`USovereignCombatCalculationLibrary`)**:
  Combat uses a deterministic percentage mitigation curve rather than flat D&D subtraction:
  $$\text{RawPower} = \left( \text{BasePower} + \text{StatValue} \times \text{StatMultiplier} \right) \times \text{ClassMultiplier}$$
  $$\text{ScaledOffense} = \text{RawPower} \times \text{AttackerRankModifier}$$
  $$\text{EffectiveDefense} = (\text{Target.Constitution} + \text{Target.EquipmentDefense}) \times \text{TargetRankDefenseModifier}$$
  $$\text{MitigationFactor} = \frac{100}{100 + \text{EffectiveDefense}}$$
  $$\text{FinalDamage} = \max\left(1.0, \text{round}\left(\text{ScaledOffense} \times \text{MitigationFactor} \times \text{CritMultiplier}\right)\right)$$
  - **Attribute Scaling**:
    - Physical abilities scale with **Strength (STR)** (Melee) or **Dexterity (DEX)** (Ranged / Finesse).
    - Spells and Qi abilities scale with **Intelligence (INT)** (Arcana) or **Wisdom (WIS)** (Spiritual Qi / Restoration).
    - Physical defense scales from **Constitution (CON)** and equipment armor.
  - **Critical Hits**: Base $5\% + (\text{Attacker.Luck} - \text{Target.Luck}) \times 0.5\%$, clamped between $5\%$ and $50\%$. Critical hits apply a **1.5x multiplier**.
  - **Damage Floor**: Net damage is floored at minimum **1.0** on any successful hit.

### 2.2 Dedicated 3D Battle Arena Transition
- **Field Navigation vs. Battle Arena**:
  - In procedural dungeon maps, entities move in real-time 3D space.
  - Upon touching an enemy or triggering an encounter line-of-sight, the field state pauses and transitions seamlessly to a **Dedicated 3D Battle Arena Platform**.
- **Camera & World Practicality**:
  - Dedicated arenas eliminate corridor camera clipping, tight wall collisions, and navigation mesh sticking points inherent in procedural dungeons.
  - Battle arenas feature biome-specific environmental aesthetics (e.g., Earth Cave Platform, Lava Qi Vault Stage, Overgrown Temple Arena).
  - Cinematic action cameras dynamically frame ability executions, critical hits, and team spell casting.
- **Post-Battle Transition**: Upon defeating the enemy group, the screen smoothly transitions back to the dungeon floor map at the exact field position.

---

## 3. Class System, Class Points (CP) & Multi-Classing

### 3.1 The Starter Class: "Adventurer"
- Every dragon or entity starts in the baseline **Adventurer** class.
- The Adventurer class provides basic physical strikes, minor Qi focus, and initial stat growth bonuses.

### 3.2 Action-Based Class Unlocks & Skill Tomes
- **Action Accumulation Protocol**:
  - Class unlocks are driven by action execution tracking integrated into `USovereignLevelComponent` and `USovereignSkillComponent`.
  - Repeatedly performing physical strikes builds progress toward unlocking physical specialization classes (e.g., *Warrior*, *Knight*, *Dragoon*).
  - Repeatedly casting elemental or Qi spells builds progress toward magical classes (e.g., *Elementalist*, *Wizard*, *Arcanist*).
  - Absorbing damage and protecting allies unlocks defensive classes (e.g., *Guardian*, *Paladin*).
- **Dungeon Skill Books / Class Tomes**:
  - Rare drop items in dungeon vaults and boss chests ("Tome of the Dragoon", "Scroll of the Qi Master").
  - Consuming a Class Tome immediately unlocks or accelerates access to specialized hybrid classes without requiring full action grinding.

### 3.3 Multi-Classing & Class Points (CP)
- **Non-Restrictive Multi-Classing**: Entities are not restricted to just two classes. An entity can equip any number of unlocked classes simultaneously, provided they do not exceed their **Class Point (CP) Capacity**.
- **Class Point (CP) Cost & Allocation**:
  - Each unlocked class costs a specific amount of CP to equip (e.g., Adventurer = 1 CP, Knight = 3 CP, Archmage = 5 CP).
  - Maximum CP capacity scales with entity level, maturity stage, and specific Meditation Garden milestones.
- **Equipped Abilities & Spells**: Equipping a class grants access to that class's active spellbook, passive traits, and stat scaling bonuses during turn-based combat.

---

## 4. Profession System: Gathering, Crafting & Garden Synergy

Distinct from combat classes, entities can develop **Professions** that function both inside dungeons and within the Meditation Garden sandbox.

### 4.1 Core Professions
1. **Mining**: Extracting raw mineral ores, spirit crystals, and geological fragments from dungeon nodes and garden rock structures.
2. **Lumbering**: Harvesting spirit wood, bark, and ancient sap from garden trees and overgrown dungeon roots.
3. **Alchemy**: Brewing health elixirs, Qi restorative potions, mutation catalysts, and combat buff tinctures using cultivated herbs and dungeon drops.
4. **Smithing**: Forging draconic armor pieces, claw sharpeners, and artifact charms to boost base attributes.

### 4.2 Profession Progression
- Driven by `USovereignProfessionComponent`.
- Gathering node quality and crafting success rates scale with profession levels and relevant base attributes (e.g., STR/CON for Mining & Lumbering, INT/WIS for Alchemy, STR/DEX for Smithing).

---

## 5. Party Formation & Opposing 3x3 Rank Matrices

Combat party deployment and enemy encounter composition utilize dual opposing tactical **3x3 Grid Formation Matrices** (up to 9 total slots each) divided into three distinct functional ranks managed by `USovereignPartyComponent` and `ASovereignBattleManager`.

```text
    ================== PLAYER PARTY 3x3 GRID ==================
       [ FRONT RANK ]           [ MID RANK ]            [ REAR RANK ]
  +----------------------+ +----------------------+ +----------------------+
  | Slot 0 | Slot 1 | 2  | | Slot 3 | Slot 4 | 5  | | Slot 6 | Slot 7 | 8  |
  +----------------------+ +----------------------+ +----------------------+
  | +15% Defense         | | +15% Ranged Damage   | | +20% Healing Output  |
  | +10% Melee Attack    | | +15% Qi Spell Damage | | +20% Buff Duration   |
  | Target Priority 1    | | Target Priority 2    | | Low Target Priority  |
  +----------------------+ +----------------------+ +----------------------+

                               VS

    ================== ENEMY ENCOUNTER 3x3 GRID ==================
       [ FRONT RANK ]           [ MID RANK ]            [ REAR RANK ]
  +----------------------+ +----------------------+ +----------------------+
  | Slot 0 | Slot 1 | 2  | | Slot 3 | Slot 4 | 5  | | Slot 6 | Slot 7 | 8  |
  +----------------------+ +----------------------+ +----------------------+
  | Frontline Defenders  | | Ranged & Spellcasters| | Support & Boss Units  |
  +----------------------+ +----------------------+ +----------------------+
```

### 5.1 Rank Matrix Modifiers & Obstruction Rules
1. **Front Rank (Slots 1–3)**:
   - **Role**: Tanks and Melee Damage Dealers.
   - **Modifiers**: **+15% Physical Defense**, **+10% Melee Attack Power**. Draws highest enemy targeting priority.
2. **Mid Rank (Slots 4–6)**:
   - **Role**: Ranged Attackers and Offensive Spellcasters.
   - **Modifiers**: **+15% Ranged Physical Damage**, **+15% Spell & Qi Damage**.
   - **Protection Rule**: Shielded from direct single-target melee strikes while any living defenders occupy the Front Rank.
3. **Rear Rank (Slots 7–9)**:
   - **Role**: Support, Healers, and Buffer Units.
   - **Modifiers**: **+20% Healing Output**, **+20% Buff Duration & Potency**.
   - **Rank-Wide Protection Rule**: Shielded from direct single-target melee strikes until **both Front and Mid ranks are completely cleared of living units**.
   - **Obstruction Exceptions**: Spells (INT/WIS) and Ranged attacks (DEX) bypass front-line melee obstruction, directly targeting any active slot.
   - **AOE & Piercing**: `FrontRowSweep` targets all living entities in the foremost active rank; `ColumnPierce` strikes through the target column (Front -> Mid -> Rear); `FullMatrixAOE` strikes all active units.

### 5.2 Milestone Rank Unlocks & Party Scaling
- **Initial State**:
  - Expeditions begin with **1 active party member** (the wisp-possessed dragon).
  - Only **Rank 1 (Front Rank, Slots 0–2)** is unlocked initially.
- **Party Scaling to 9 Slots**:
  - As player lineage grows, additional dragons and cultivated beasts can be assigned to the battle party.
  - **Milestone 1**: Unlocking **Rank 2 (Mid Rank, Slots 3–5)** expands deployable party slots up to 6 units. Requires cultivating 3 distinct dragon species and conquering the *Trial of the Unseen Weave*.
  - **Milestone 2**: Unlocking **Rank 3 (Rear Rank, Slots 6–8)** unlocks all 9 grid slots for full legion deployment. Requires raising 6 adult dragons and defeating the *Vault Guardian of Realms*.

---

## 6. Dungeon Extraction, Rescued Eggs & The "Reverse Return" Loop

### 6.1 Standard Extraction Loop
1. The party navigates procedural dungeon floors, battling corrupted guardians and opening vaults.
2. Defeating the Floor Boss reveals a dormant **Female Dragon Egg** item.
3. A **Gateway Teleporter** spawns in the boss room, allowing the player to safely extract immediately back to the Meditation Garden with the rescued egg.

### 6.2 "Reverse Return" High-Risk Protocol ("Risk it for the Biscuit")
Instead of taking the immediate teleporter, the player can choose to execute a **Reverse Dungeon Return**:
- **Mechanism**: The player physically traverses back through all previously cleared dungeon floors to the entrance gateway.
- **Respawn Conditions**:
  - All standard enemy mobs respawn in aggressive roaming states.
  - Re-entering the initial entrance room triggers a respawned, empowered version of the floor boss.
- **The "Overburdened" Perk / Debuff**:
  - Carrying the rescued Female Dragon Egg manually through dangerous portal corridors inflicts the **Overburdened** condition on the entire party.
  - **Effect**: Reduces all party D&D attributes by **-20%** (Max HP, Defense, Attack, and Turn Speed reduced).
- **The Reward**:
  - Successfully surviving the reverse return and stepping through the entrance portal grants a massive **Bonus Qi Yield (+100% to +200% baseline energy)** directly into the rescued Female Dragon Egg.
  - Unlocks higher baseline stat inheritance and increased rare genetic mutation odds when hatched in the Meditation Garden!

---

## 7. C++ Framework & Component Architecture Integration

Phase 2 gameplay introduces three modular C++ Actor Components attaching directly to `ASovereignBaseCharacter` and `ASovereignBaseEntity`, working seamlessly alongside existing Sovereign components:

| Component | Responsibility | Integrates With |
|:---|:---|:---|
| `USovereignClassComponent` | Manages active/unlocked classes, CP allocation, class levels, equipped active spells & passive ability slots. | `USovereignLevelComponent`, `USovereignAttributeComponent` |
| `USovereignProfessionComponent` | Tracks profession levels (Mining, Lumbering, Alchemy, Smithing), craft recipes, and yield multipliers. | `USovereignBioComponent`, `USovereignSaveableEntityComponent` |
| `USovereignSkillComponent` | Manages equipped skill instances, custom Qi augment slots, action accumulation for class unlocks, and cooldowns. | `USovereignClassComponent`, `USovereignQiComponent`, `USovereignLevelComponent` |
| `USovereignSkillDefinition` | Primary DataAsset defining base ability power, scaling attribute, cooldowns, and base target pattern. | AssetManager (`"SovereignSkill"`), `USovereignSkillComponent` |
| `USovereignSkillAugment` | Instanced DataAsset modifier attached to skills by spending Qi: alters target shapes, stat multipliers, and elemental affinity. | `USovereignSkillComponent`, `USovereignQiComponent` |
| `USovereignCombatCalculationLibrary` | Deterministic BlueprintFunctionLibrary executing percentage-mitigated damage formulas, crit rolls, and 3x3 rank matrix target evaluation. | Battle Arena Turn Manager, UI previews, AI Behavior Trees |
| `USovereignPolymorphComponent` | Modular shapeshifting component for adult dragons: toggles TrueForm vs. Humanoid Visage, swapping mesh/materials and capsule dimensions. | `ASovereignBaseCharacter`, Party Formations, Dungeon Navigation |
| `USovereignInventoryComponent` | Physical vessel container managing 24+ item slots, stack logic, dynamic item tags/properties, and fragment execution. | `ASovereignBaseCharacter`, `USovereignInventoryBroker`, World Pickups |
| `USovereignLevelComponent` | Single Source of Truth for experience accumulators (`StrengthXP` ... `LuckXP`) and `ESovereignTrainingFocus`. | `USovereignAttributeComponent`, `USovereignQiComponent` |
| `USovereignAttributeComponent` | Core D&D attributes (STR, DEX, CON, INT, WIS, CHA, LCK); feeds raw stat inputs into turn-based combat damage formulas. | All combat & skill components |

---

## 8. Summary of Phase 2 Game Loop

```text
+-----------------------------------------------------------------------+
|                       MEDITATION GARDEN PHASE                         |
|  - Incubate & Hatch Male Domain Dragon Egg                             |
|  - Train Attributes via Stances (ESovereignTrainingFocus)             |
|  - Adult Male Dragon Channels Qi to Unseal Dungeon Gateway            |
+-----------------------------------+-----------------------------------+
                                    |
                                    v
+-----------------------------------------------------------------------+
|                      PROCEDURAL DUNGEON EXPEDITION                    |
|  - Field Movement in 3D Procedural Vaults                             |
|  - Touch Enemy -> Transition to Dedicated 3D Battle Arena             |
|  - Strict Sequential Turn-Based Combat (Order based on DEX)          |
|  - Earn CP, Level Classes (Adventurer -> Specializations), Acquire Skill Books|
|  - Deploy Party in 3x3 Rank Matrix (Front / Mid / Rear Ranks)         |
+-----------------------------------+-----------------------------------+
                                    |
                                    v
+-----------------------------------------------------------------------+
|                    BOSS DEFEATED & FEMALE EGG FOUND                    |
+-----------------------------------+-----------------------------------+
                 /                                         \
  [TAKE BOSS ROOM TELEPORTER]                     [EXECUTE REVERSE RETURN]
              /                                               \
             v                                                 v
+---------------------------+                   +-------------------------------+
| SAFE EXTRACTION           |                   | REVERSE DUNGEON RETURN        |
| - Instant return home     |                   | - Apply "Overburdened" (-20%) |
| - Standard Rescued Egg    |                   | - Battle respawned mobs & boss|
|   Qi baseline             |                   | - Reward: +150% Bonus Qi Yield|
|                           |                   |   & Increased Mutation Odds!  |
+-------------+-------------+                   +---------------+---------------+
              |                                                 |
              +---------------------+---------------------------+
                                    |
                                    v
+-----------------------------------------------------------------------+
|                   RETURN TO MEDITATION GARDEN                         |
|  - Hatch Rescued Female Dragon Egg in Garden                          |
|  - Pair Adult Male & Female Dragons for Breeding & Mutation           |
|  - Inherit High Attributes & Rare Traits for Next Expedition Cycle     |
+-----------------------------------------------------------------------+
```
