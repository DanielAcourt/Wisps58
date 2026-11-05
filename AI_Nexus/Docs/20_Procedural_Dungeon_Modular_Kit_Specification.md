// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# Procedural Dungeon Modular 3D Art Kit & Arena Placement Specification
## Technical Specification for 3D Environment Artists & Dungeon Generation Architecture

---

## 1. Overview & Architectural Standards

The **WispsGame Procedural Dungeon Generator (`ASovereignDungeonGenerator`)** constructs subterranean trial vaults using modular 3D environment meshes. The system supports:
1. **Dungeon Navigation**: Explorable real-time 3D corridors and chambers connected via modular sockets.
2. **Hybrid Encounters**: Random step-based hazards (like Pokemon/FF7) and fixed visible guardians.
3. **Sacrifice Empowerment ("Risk it for the Biscuit")**: Items and Qi fed into the **Dungeon Gateway Door (`ASovereignDungeonGateway`)** buff enemy stats and multiply egg mutation rewards.
4. **Dedicated Battle Arena Clusters**: Battle platforms placed in far-off coordinates (`X = +50,000, Y = +50,000, Z = +5,000`) across multiple elemental biomes.

---

## 2. Modular Mesh Dimensions & Snapping Standards

To ensure zero geometry overlap, clean lightmaps, and rapid procedural assembly, all modular dungeon tiles follow strict metric dimensions in Unreal Engine units ($1\text{ unit} = 1\text{ cm}$).

### 2.1 Core Tile Metric: 1000cm x 1000cm Grid Unit
- **Floor Tile**: $1000\text{cm} \times 1000\text{cm} \times 20\text{cm}$ thick.
- **Ceiling Tile**: $1000\text{cm} \times 1000\text{cm} \times 20\text{cm}$ thick.
- **Wall Height**: $600\text{cm}$ clear vertical interior height (allowing dragon vessels to navigate comfortably).
- **Doorway Openings**: $400\text{cm}$ wide $\times 500\text{cm}$ high centered on the tile edge.
- **Pivot Point Standard**:
  - All floor tiles must have their pivot point at **$(0, 0, 0)$ bottom center** of the floor.
  - All wall tiles must have their pivot point at the **bottom center of the wall baseline**, aligned with the grid boundary.

---

## 3. Required 3D Modular Mesh Inventory

Environment artists need to create the following foundational meshes:

| Asset Name | Dimensions ($X \times Y \times Z$) | Description / Socket Rules |
|:---|:---|:---|
| `SM_Dungeon_Floor_10x10` | $1000 \times 1000 \times 20$ | Flat stone/brick floor with subtle Qi rune grooves. |
| `SM_Dungeon_Wall_Straight_10x6` | $1000 \times 30 \times 600$ | Solid perimeter boundary wall ($30\text{cm}$ thick, $600\text{cm}$ high). |
| `SM_Dungeon_Doorway_10x6` | $1000 \times 30 \times 600$ | Archway opening ($400\text{w} \times 500\text{h}$) connecting rooms to corridors. |
| `SM_Dungeon_Door_Frame` | $400 \times 30 \times 500$ | Structural frame fitted into doorway opening. |
| `SM_Dungeon_Door_Leaf` | $380 \times 10 \times 480$ | Interactive door leaf with $10\text{cm}$ reveal margin per side. |
| `SM_Dungeon_Wall_Pillar` | $50 \times 50 \times 600$ | Vertical corner seam pillar hiding 90-degree wall junctions. |
| `SM_Dungeon_Wall_Trim` | $1000 \times 10 \times 20$ | Perimeter baseboard trim running along floor/wall baseline. |
| `SM_Dungeon_Corridor_Straight_10x10` | $1000 \times 1000 \times 600$ | Pre-assembled corridor segment with left and right walls and ceiling. |
| `SM_Dungeon_Corridor_Corner_10x10` | $1000 \times 1000 \times 600$ | 90-degree corner corridor piece. |
| `SM_Dungeon_Corridor_TJunction_10x10` | $1000 \times 1000 \times 600$ | 3-way branching intersection. |
| `SM_Dungeon_Corridor_4Way_10x10` | $1000 \times 1000 \times 600$ | 4-way crossroad intersection. |
| `SM_Dungeon_Room_Center_20x20` | $2000 \times 2000 \times 800$ | Large open chamber tile (e.g. for treasure vaults, shrines, and elite encounters). |
| `SM_Dungeon_Boss_Chamber_40x40` | $4000 \times 4000 \times 1200$ | Grand terminal arena for the Floor Boss, featuring the dormant **Female Dragon Egg Altar** in the center. |
| `SM_Dungeon_Gateway_Door` | $600 \times 120 \times 700$ | The mystical sealed gateway door placed in the Meditation Garden. Contains glowing Qi inlay runes and an item sacrifice receptacle. |

---

## 4. Far-Off Battle Arena Cluster Architecture

Because combat uses our dedicated turn-based system (`ASovereignBattleManager`), battle platforms must **never be reachable by walking or physical exploration**.

### 4.1 Coordinate Offsetting
- **Arena Coordinate Anchor**:
  $$\text{ArenaClusterOrigin} = (50000.0, 50000.0, 5000.0)$$
- **Separation**: The arena is located 500 meters to the north-east and 50 meters above the highest possible dungeon ceiling.

### 4.2 Array of Biome Arena Platforms
The battle manager maintains an array of themed arena stages:

```text
    [ Biome 0: Earth Cave ]       [ Biome 1: Lava Vault ]
    (X: 50000, Y: 50000, Z: 5000)  (X: 55000, Y: 50000, Z: 5000)
             |                              |
             +------------------------------+
                            |
    [ Biome 2: Crystal Sanctum ]  [ Biome 3: Spirit Forest ]
    (X: 50000, Y: 55000, Z: 5000)  (X: 55000, Y: 55000, Z: 5000)
```

- **Arena Platform Dimensions**:
  - $3000\text{cm} \times 3000\text{cm}$ circular or octagonal floating stage (`SM_Battle_Arena_Platform`).
  - Contains 9 Player Slot Anchors (Front, Mid, Rear) facing 9 Enemy Slot Anchors across the center line.
  - Dedicated cinematic camera socket `Socket_ArenaCam` at $(0, -800, 350)$ looking at the battle center.

---

## 5. Gateway Door & Sacrifice Empowerment Mechanics ("Risk it for the Biscuit")

The **Dungeon Gateway Door (`ASovereignDungeonGateway`)** acts as the physical threshold in the Meditation Garden:

### 5.1 Configurable Unlocking
- Can be unlocked via:
  1. Hatching milestone of the primary dragon (`bAutoUnlockOnHatch`).
  2. Manual designer flag / quest directive (`UnlockGateway()`).
  3. Qi channeling threshold.

### 5.2 Sacrifice & Empowerment Receptacle
Players can insert extra items into the Gateway's `USovereignInventoryComponent` and channel raw Qi:
$$\text{SacrificeMass} = \sum (\text{Item.UnitWeight} \times \text{Item.Quantity})$$
$$\text{EnemyBuffMultiplier} = 1.0 + (\text{SacrificeMass} \times 0.05) + (\text{InfusedQi} \times 0.002)$$
$$\text{RewardLootMultiplier} = 1.0 + (\text{SacrificeMass} \times 0.10) + (\text{InfusedQi} \times 0.005)$$

### 5.3 The Roguelike Stakes
- **Victory**: Defeating the Floor Boss unlocks the dormant **Female Dragon Egg** drop. All bonus Qi and reward multipliers are permanently infused into the egg, boosting mutation chance and hatch stats!
- **Defeat / Party Wipe**: If all party members are defeated and the primary vessel dies, **all sacrificed items and channeled Qi are destroyed**, and the Meditation Garden initiates a roguelike lineage reset.
