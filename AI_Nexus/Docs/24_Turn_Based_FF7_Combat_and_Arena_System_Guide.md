// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# Sovereign Framework: Turn-Based (Final Fantasy VII Style) Combat & Arena System Guide

**Author:** Sovereign Framework Research & Development
**Target Engine:** Unreal Engine 5.8
**Applicable Sprint Tickets:** CS-001 through CS-007, B-078, B-079, B-083, B-100

---

## 🏛️ System Architecture Overview

The Sovereign Turn-Based Combat and Dedicated 3D Arena System delivers a classic **Final Fantasy VII Active Time Battle (ATB)** tactical encounter experience integrated directly into procedural dungeon exploration:

1. **Dungeon Exploration:** Third-person field traversal controlling an inhabited party leader vessel.
2. **Dual Encounter Triggers:**
   - **Roaming Mobs:** Physical interactive corridor mobs (`ASovereignRoamingMob`) that trigger battles on collision or interaction.
   - **Step-Based Encounters:** Pokemon/FF7-style random encounters evaluated by distance walked (`ASovereignDungeonGenerator::ProcessPlayerMovement`).
3. **Dedicated 3D Battle Arena Platform:** Battles take place on a far-off 3D theater platform (`ASovereignBattleManager` spawned with 3-sided walls, open front, and stage lighting).
4. **Opposing Dual 3x3 Formations (18 Grid Anchors):**
   - **Player Party (9 slots):** Front Rank (0-2), Mid Rank (3-5), Rear Rank (6-8).
   - **Enemy Encounter (9 slots):** Front Rank (0-2), Mid Rank (3-5), Rear Rank (6-8) mirrored and facing the party.
5. **Cinematic FF7 Battle Camera:** Blends view target to a side-angle action camera (`ArenaCameraTransform`) framing all 18 formation positions, pausing exploration input.
6. **Active Time Battle (ATB) Engine:** Meters accumulate in real time from 0% to 100% based on Dexterity, equipped classes, and status effects (`Haste` / `Slow`).
7. **Tactical Action Execution:** Turn arbitration, Qi skill casting, status effects, and elemental affinities (1.5x weakness, 0.5x resistance).
8. **Enemy AI Roles:** Enemies make intelligent tactical decisions based on their assigned role (`AggressiveBrute`, `SkirmisherSniper`, `SupportHealer`, `TacticalLeader`).
9. **Victory Progression & Loot (Option A):** Defeated enemies award aggregated EXP that feeds directly into `USovereignLevelComponent` attribute training and `USovereignClassComponent` job levels, while dropping physical items into the leader's inventory.
10. **Clean Teardown:** Dynamically spawned enemy vessels are destroyed, and living party members teleport back to their dungeon floor location with restored cameras and controls.

---

## 📊 1. Authoring Enemies, Encounters & Skills via CSV

All combat entities, skills, jobs, and encounters are authored in spreadsheets under `AI_Nexus/Data/` and synthesized deterministically into Primary DataAssets.

### A. Authoring Skills (`AI_Nexus/Data/DT_SovereignSkillsTemplate.csv`)
| Column | Description | Example |
| :--- | :--- | :--- |
| `Name` | DataAsset row identifier | `Skill_MageFireball` |
| `SkillTag` | Authoritative GameplayTag | `Skill.Combat.Mage.Fireball` |
| `DisplayName` | In-game title | `Mage Fireball` |
| `Description` | Lore and tooltip description | `Concentrated sphere of draconic flame...` |
| `TargetPattern` | Formation target AoE pattern | `SingleTarget`, `FrontRowSweep`, `ColumnPierce`, `FullMatrixAOE`, `SelfOnly`, `AllAllies` |
| `ScalingAttribute` | Primary D&D attribute driving potency | `Strength`, `Dexterity`, `Intelligence`, `Wisdom`, `Constitution` |
| `ElementType` | Elemental affinity | `None`, `Fire`, `Water`, `Earth`, `Nature`, `Air`, `Light`, `Dark` |
| `bIsTelegraphed` | Whether this is a 2-stage charging ultimate | `true` or `false` |
| `TelegraphWarningText` | Warning text announced when charging begins | `Hobgoblin Chief is gathering earth energy!` |
| `BasePower` | Base potency before scaling | `60.0` |
| `StatMultiplier` | Potency multiplier per attribute point | `1.8` |
| `QiCost` | Spiritual cultivation energy consumed | `30.0` |
| `CooldownTurns` | Turn cooldown | `3` |
| `IconPath` | Texture package path | `/Game/ArtVault/Textures/T_Skill_Fireball` |

### B. Authoring Enemies (`AI_Nexus/Data/DT_SovereignEnemiesTemplate.csv`)
| Column | Description | Example |
| :--- | :--- | :--- |
| `Name` | Asset row identifier | `Enemy_GoblinWarrior` |
| `EnemyTag` | Authoritative GameplayTag | `Enemy.Humanoid.Goblin.Warrior` |
| `DisplayName` | In-game title | `Goblin Warrior` |
| `EnemyRole` | AI behavioral personality (Ticket CS-003) | `AggressiveBrute`, `SkirmisherSniper`, `SupportHealer`, `TacticalLeader` |
| `PrimaryElement` | Innate elemental alignment | `Nature` |
| `WeaknessElement` | Element taking 1.5x damage | `Fire` |
| `ResistanceElement` | Element mitigating 0.5x damage | `Water` |
| `CharacterClass` | Blueprint or C++ class to instantiate | `/Script/WispCPP7VR.SovereignBaseCharacter` |
| `MaxHealth` & `MaxQi` | Vitals capacity | `110.0`, `30.0` |
| `Strength`..`Luck` | D&D core attribute block | `13`, `10`, `12`, `7`, `8`, `6`, `9` |
| `EquippedSkills` | Comma-separated list of skills | `Skill.Combat.Warrior.Cleave` |
| `ExpReward` | Base EXP bounty awarded on defeat | `65.0` |
| `ClassExpReward` | Class/Job EXP awarded on defeat | `15` |
| `QiReward` | Qi pool bounty restored to living party | `15.0` |
| `LootDropItem` | Soft DataAsset pointer for physical drop | `Item_StaminaPotion` |
| `LootDropChance` | Drop probability percentage (0-100%) | `50.0` |
| `LootQuantity` | Quantity dropped | `1` |
| `StealableItem` | Soft DataAsset pointer for in-combat steal | `Item_IronSword` |
| `BaseStealChance` | Base steal success percentage | `40.0` |

### C. Authoring Encounters (`AI_Nexus/Data/DT_SovereignEncountersTemplate.csv`)
| Column | Description | Example |
| :--- | :--- | :--- |
| `Name` | Asset row identifier | `Encounter_GoblinAmbush` |
| `EncounterTag` | Authoritative GameplayTag | `Encounter.Dungeon.GoblinAmbush` |
| `DisplayName` | Encounter title | `Goblin Ambush` |
| `EnemySlots` | Semicolon-delimited `Slot:Enemy` pairs | `0:Enemy_GoblinWarrior;2:Enemy_GoblinScout;6:Enemy_GoblinShaman` |
| `CanFlee` | Whether players can escape this battle | `true` (false for storyline bosses) |
| `MinDepth` | Minimum floor depth for generation | `0` |
| `SpawnWeight` | Relative probability weighting | `75.0` |

### D. 1-Click Python Synthesis
Run within the Unreal Editor Python console:
```python
import import_combat_data

# Synthesizes all Primary DataAssets into /Game/Data/
import_combat_data.import_skills()       # Creates /Game/Data/Skills/DA_<SkillName>
import_combat_data.import_classes()      # Creates /Game/Data/Classes/DA_<ClassName>
import_combat_data.import_enemies()      # Creates /Game/Data/Enemies/DA_<EnemyName>
import_combat_data.import_encounters()   # Creates /Game/Data/Encounters/DA_<EncounterName>
```

---

## 🎮 2. Setting Up Encounters in Procedural Dungeons

### Option A: Corridor Roaming Mobs
1. Place or spawn an `ASovereignRoamingMob` (or derived Blueprint `BP_RoamingMob`) inside dungeon corridors.
2. In the Details panel, assign the **Encounter Definition** to your desired DataAsset (e.g. `DA_Encounter_GoblinPatrol`).
3. When touched by the player leader vessel (`TriggerSphere` overlap) or upon interaction:
   - Exploration input is disabled.
   - The party and opposing enemies teleport to the Dedicated 3D Battle Arena.
   - The roaming mob is hidden and its collision disabled during combat.
   - On **Victory**, the roaming mob is destroyed. On **Fled**, the roaming mob is restored.

### Option B: Step-Based Random Encounters
1. Select your `ASovereignDungeonGenerator` in the level.
2. In the Details panel under **Sovereign|Dungeon|Encounters**:
   - `RandomEncounterChance`: Default 25.0%.
   - `StepThresholdDistance`: Default 1500cm (~15 steps).
   - `RandomEncounterPool`: Add your encounter DataAssets (`DA_Encounter_GoblinPatrol`, `DA_Encounter_GoblinAmbush`, etc.).
3. When exploring, `ProcessPlayerMovement` tracks distance and rolls random battles seamlessly.

---

## ⚔️ 3. Inside the Battle Arena: Combat Mechanics

### ATB System & Multipliers
Each combatant has a `USovereignATBComponent`.
- The gauge fills based on `CalculateEffectiveFillRate()`:
  $$\text{EffectiveRate} = \text{BaseRate} \times \left(1.0 + \frac{\text{Dexterity}}{50}\right) \times \text{ClassSpeedMultiplier} \times \text{StatusSpeedMultiplier}$$
- **Wait Mode (`SetWaitModeActive(true)`):** Navigating skill or item menus pauses all ATB progression across combatants.
- **Defend Stance (`EnterDefendStance()`):** Ends turn, resets ATB, and provides 50% damage mitigation until the next turn.

### Tactical Enemy AI Roles (Ticket CS-003)
Enemies act automatically upon their ATB filling to 100%:
- **`AggressiveBrute`:** Prioritizes frontline player vessels (Slots 0-2).
- **`SkirmisherSniper`:** Executes the lowest absolute HP target or rear-rank casters.
- **`SupportHealer`:** Scans living allies for HP < 50% and prioritizes healing spells (`Cleric.Mend`); otherwise casts offensive spells.
- **`TacticalLeader`:** Prioritizes AoE spells if 2+ players are alive, or focuses down the highest-Strength threat.
- **Fallback:** Smoothly targets any living player combatant if preferred target/skill is unavailable.

### Elemental Vulnerabilities & Resistances (Ticket CS-004)
Calculated in `USovereignCombatCalculationLibrary::CalculateCombatDamage`:
- **Weakness Element (1.5x):** If attack element matches the target's `WeaknessElement`, damage is amplified by +50%.
- **Resistance Element (0.5x):** If attack element matches the target's `ResistanceElement`, damage is halved.

### Turn-Based Status Effects (Ticket CS-005)
Managed by `USovereignStatusComponent` (`GetStatusComponent()`):
- **`Poison`:** Ticks damage on turn end; decrements duration.
- **`Regen`:** Ticks healing on turn end; decrements duration.
- **`Stun`:** Skips turn when ATB reaches 100%.
- **`Silence`:** Blocks casting any skill with `QiCost > 0`.
- **`Blind`:** Applies 50% physical damage reduction in combat calculations.
- **`Haste`:** 1.5x ATB fill rate.
- **`Slow`:** 0.5x ATB fill rate.

### Telegraphed Boss Attacks & Stance Charging (Ticket CS-006)
For boss skills with `bIsTelegraphed == true` (e.g. `Skill_BossEarthBreaker`):
1. **Stage 1 (Telegraph & Charge):** The boss announces a warning message via `OnTelegraphedAttackStarted`, enters `EATBState::ChargingAttack`, resets ATB to 0%, and charges to 100%. Players can use Defend stance or heal.
2. **Interrupting:** If the charging boss is inflicted with `Stun`, the ultimate is cancelled and resets to standard charging, broadcasting `OnTelegraphedAttackInterrupted`.
3. **Stage 2 (Unleash):** If uninterrupted, when ATB fills to 100%, the ultimate attack executes across the party.

### In-Combat Steal & Victory Loot Drops (Ticket CS-007)
- **In-Combat Steal:** Call `BattleManager->ExecuteStealAction(ThiefChar, TargetEnemy)`:
  $$\text{StealChance} = \text{BaseStealChance} + (\text{ThiefDEX} - \text{TargetDEX}) \times 2\% + \text{ThiefLuck} \times 1\%$$
  - Success adds the stealable item into the thief's inventory.
  - Enforces single-steal-per-enemy (repeat steals on the same enemy are rejected).
- **Victory Loot Drops:** On victory, every defeated enemy's `LootDropChance` is rolled, and physical items are deposited directly into the party leader's `USovereignInventoryComponent`.

---

## 🏆 4. Post-Battle Rewards & Progression (Option A)

Upon defeating all enemies, `ASovereignBattleManager::DistributeVictoryRewards` executes:
1. **Attribute EXP:** Aggregated enemy EXP is divided among living party members and added to `USovereignLevelComponent::AddAttributeXP` for their active training focus (STR, DEX, CON, INT, WIS, CHA, LCK), triggering stat rollover increases.
2. **Class Job EXP / CP:** Grants class experience to all equipped jobs in `USovereignClassComponent`, leveling up jobs (Warrior, Rogue, Mage, etc.).
3. **Qi Replenishment:** Ambient spiritual Qi from defeated enemies restores living combatants' Qi pools up to `MaxQiCapacity`.
4. **Loot Deposit:** Dropped items are placed into the party leader's inventory.
5. **Dungeon Return:** The subsystem restores player camera view targets, re-enables exploration input, teleports the party back to the dungeon floor, and destroys spawned enemy arena actors.

---

## 🧪 5. Automated Regression Test Suite

The test suite includes headless verification covering all combat features:
- `SovereignEnemyEncounterSuiteTests.cpp`: Tests definition creation, formation spawning, role AI, elemental scaling, and reward distribution.
- `SovereignStatusEffectTests.cpp`: Tests Poison, Regen, Haste, Slow, Silence, and Blind.
- `SovereignBossTelegraphTests.cpp`: Tests 2-stage telegraphed charging, unleashing, and stun interruption.
- `SovereignCombatLootTests.cpp`: Tests in-combat steal mechanics and victory loot drops.

Run these tests via the Unreal Editor **Session Frontend** (`Window -> Developer Tools -> Session Frontend -> Automation -> Sovereign.Combat.*`).
