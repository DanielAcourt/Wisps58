// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21

# Sovereign Combat Skills & Classes CSV Importer Specification
## Technical Architecture & Operational Guide (Tickets CS-001 & CS-002)

---

## 1. Overview & Objectives

In WispsGame Phase 2, combat abilities and specialization jobs are authored deterministically via dedicated CSV templates:
1. `DT_SovereignSkillsTemplate.csv` -> Synthesized into `USovereignSkillDefinition` Primary DataAssets in `/Game/Data/Skills/`.
2. `DT_SovereignClassesTemplate.csv` -> Synthesized into `USovereignClassDefinition` Primary DataAssets in `/Game/Data/Classes/`.

This avoids polluting the 29-column biological species template (`DT_SovereignSpeciesTemplate.csv`) with unrelated combat math, while maintaining a pure-data, reproducible workflow.

---

## 2. Skills CSV Specification (`DT_SovereignSkillsTemplate.csv`)

### 2.1 File Location
- Source: `AI_Nexus/Data/DT_SovereignSkillsTemplate.csv`
- Target DataAsset: `USovereignSkillDefinition` (`PrimaryAssetType = "SovereignSkill"`)
- Destination: `/Game/Data/Skills/DA_<SkillName>`

### 2.2 Column Schema

| Column | Type | Example | Description |
|:---|:---|:---|:---|
| `Name` | String | `Skill_WarriorCleave` | Base row name sanitized into asset name (`DA_WarriorCleave`). |
| `SkillTag` | GameplayTag | `Skill.Combat.Warrior.Cleave` | Primary identifier tag. |
| `DisplayName` | Text | `Warrior Cleave` | User-facing display title in battle UI. |
| `Description` | Text | `Sweeping wide arc slashing across frontline.` | Tooltip and combat log description. |
| `TargetPattern` | Enum String | `FrontRowSweep` | Formation pattern: `SingleTarget`, `FrontRowSweep`, `ColumnPierce`, `FullMatrixAOE`, `SelfOnly`, `AllAllies`. |
| `ScalingAttribute`| Enum String | `Strength` | Primary D&D attribute: `Strength`, `Dexterity`, `Intelligence`, `Wisdom`, `Constitution`. |
| `BasePower` | Float | `35.0` | Base damage/potency before attribute scaling. |
| `StatMultiplier` | Float | `1.40` | Multiplier applied to caster's scaling attribute. |
| `QiCost` | Float | `10.0` | Cultivation Qi consumed from `USovereignQiComponent`. |
| `CooldownTurns` | Integer | `1` | Turn cooldown counter before skill can be recast. |
| `IconPath` | SoftPath | `/Game/ArtVault/Textures/T_Skill_Cleave` | Soft pointer to UI texture icon. |

---

## 3. Classes CSV Specification (`DT_SovereignClassesTemplate.csv`)

### 3.1 File Location
- Source: `AI_Nexus/Data/DT_SovereignClassesTemplate.csv`
- Target DataAsset: `USovereignClassDefinition` (`PrimaryAssetType = "SovereignClass"`)
- Destination: `/Game/Data/Classes/DA_<ClassName>`

### 3.2 Column Schema

| Column | Type | Example | Description |
|:---|:---|:---|:---|
| `Name` | String | `Class_Ninja` | Row identifier sanitized into `DA_Ninja`. |
| `ClassTag` | GameplayTag | `Class.Combat.Ninja` | Unique job tag. |
| `DisplayName` | Text | `Ninja` | Class name displayed on character cards and battle UI. |
| `Description` | Text | `Master of stealth and assassinations.` | Role description and lore background. |
| `CPCost` | Integer | `3` | Class Points cost required to equip simultaneously. |
| `StrengthMultiplier` | Float | `1.15` | STR modifier applied in turn-based damage calculations. |
| `DexterityMultiplier`| Float | `1.45` | DEX modifier scaling ATB fill speed and finesse skills. |
| `ConstitutionMultiplier` | Float | `0.90` | CON modifier scaling physical defense. |
| `IntelligenceMultiplier` | Float | `1.10` | INT modifier scaling arcane magic. |
| `WisdomMultiplier` | Float | `0.95` | WIS modifier scaling Qi restoration potency. |
| `PrerequisiteClasses` | String List | `Class.Combat.Rogue:5` | Semicolon-separated prior classes and required levels. |
| `ReqSTR` | Integer | `10` | Minimum Strength attribute to unlock. |
| `ReqDEX` | Integer | `15` | Minimum Dexterity attribute to unlock. |
| `ReqCON` | Integer | `0` | Minimum Constitution attribute to unlock. |
| `ReqINT` | Integer | `0` | Minimum Intelligence attribute to unlock. |
| `ReqWIS` | Integer | `0` | Minimum Wisdom attribute to unlock. |
| `GrantedSkills` | String List | `Skill.Combat.Ninja.ShadowStrike` | Comma-separated list of skills granted by the class. |

---

## 4. In-Engine Execution & Python API

The importer tools are automatically loaded during editor boot via `init_unreal.py` and exposed in Python:

```python
# Import skills from default or custom CSV
import_skills(csv_path="AI_Nexus/Data/DT_SovereignSkillsTemplate.csv", output_dir="/Game/Data/Skills")

# Import classes with prerequisite resolution and granted skills soft pointers
import_classes(csv_path="AI_Nexus/Data/DT_SovereignClassesTemplate.csv", output_dir="/Game/Data/Classes")
```

Or run offline from the command line:
```bash
python WispsCPPVR/Content/Python/import_combat_data.py
```
