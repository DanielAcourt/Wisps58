# Sovereign Framework: Procedural Dungeon Milestones 2 & 3 Upgrade Specification
**Document ID:** `AI_Nexus/Research/Dungeon_Generator_Milestones_2_and_3_Spec.md`
**Author:** Sovereign Research & Core Systems Engineering (07)
**Status:** Architecture Planning & Modular Standards Review
**Date:** October 2026

---

## 🏛️ Executive Summary & Pushback Analysis

This document provides a thorough technical assessment and staged architectural plan for:
1. **Milestone 2**: Variable Room Footprints ($1 \times 1, 2 \times 2, 4 \times 4$ multi-cell reservations and geometric boundaries).
2. **Milestone 3**: Dungeon Variety (Varied room topologies, multiple entries/exits, treasure vaults, elite chambers, and controlled loops).
3. **Modular 3D Art Kit Standards Review**: Evaluating the proposed metric standards and providing targeted technical feedback.

---

## 1. Modular Mesh Kit Dimensions: Review & Pushback

### 1.1 Proposed Baseline Metrics
| Property | Value | Evaluation |
|:---|:---|:---|
| **Grid Unit** | $1000\text{cm}$ ($10\text{m}$) | **Approved.** Perfectly matches UE grid snapping, character speeds, and camera frustum. |
| **Floor Tile** | $1000 \times 1000 \times 20\text{cm}$ | **Approved.** Standard baseline with $20\text{cm}$ thickness preventing raycast bleed. |
| **Wall Section** | $1000\text{w} \times 600\text{h} \times 30\text{cm}$ thick | **Approved.** $30\text{cm}$ is cleaner and lighter than $40\text{cm}$, cleanly preventing edge overlap. |
| **Doorway Opening** | $400\text{w} \times 500\text{h}$ | **Approved.** Accommodates both humanoid visage ($200\text{cm}$) and mid-tier dragons ($350\text{cm}$ wingspan/height). |
| **Door Frame** | $400\text{w} \times 500\text{h}$ | **Approved.** Snaps into the doorway opening. |
| **Door Leaf** | $380\text{w} \times 480\text{h}$ | **Approved.** Leaves a $10\text{cm}$ aesthetic reveal/tolerance per side, preventing clipping with the frame. |
| **Ceiling Tile** | $1000 \times 1000 \times 20\text{cm}$ | **Approved.** Inverted normal / bottom-facing surface. |
| **Wall Pillar / Corner** | $50 \times 50 \times 600\text{cm}$ | **Crucial Addition.** Hides 90-degree corner wall seams without z-fighting. |
| **Wall Trim** | $1000 \times 20\text{h} \times 10\text{d}\text{cm}$ | **Approved.** Adds visual depth to baseboards. |
| **Pivot Convention** | Bottom Center | **Approved.** Standard SSoT across Unreal Engine modular kits. |

### 1.2 Constructive Technical Pushback & Observations

#### Pushback 1: Doorway Center Offset on 2x2 and 4x4 Multi-Cell Footprints
- For a $1 \times 1$ Corridor, the door is centered at $500\text{cm}$ along the $1000\text{cm}$ face.
- For a $2 \times 2$ Chamber ($2000\text{cm}$ total wall length), if artists model a single monolithic $2000\text{cm}$ wall piece with a center door at $1000\text{cm}$, **it will not align with a standard $1000\text{cm}$ incoming corridor**! An incoming corridor connects to either Cell 0 or Cell 1.
- **Architectural Solution:**
  - Multi-cell rooms should **not** require custom monolithic walls with off-grid center doorways.
  - Multi-cell rooms must use the **same $1000\text{cm}$ modular wall sections and doorway frames** snapped along their perimeter grid edges!
  - For a $2 \times 2$ room, each wall consists of two $1000\text{cm}$ modular wall pieces. Connecting a corridor to Cell 0 simply replaces Wall Piece 0 with Doorway Piece 0, maintaining 100% modular interchangeability and zero asset duplication!

#### Pushback 2: Boss Arena Scale ($4 \times 4 = 4000\text{cm} \times 4000\text{cm}$)
- $4000\text{cm} \times 4000\text{cm}$ ($40\text{m} \times 40\text{m}$) is a grand space suitable for walking exploration and approaching the Dragon Egg Altar.
- Remember: Under the WispsGame Phase 2 architecture, the actual turn-based Active Time Battle (ATB) takes place on the dedicated 3D battle arena platforms spawned far off-map (`(50000, 50000, 5000)`). The $4 \times 4$ Boss Chamber in the dungeon floor is the **exploration threshold** where the player approaches the roaming Boss Mob and unseals the Female Dragon Egg Altar upon victory. This separation must be maintained.

#### Pushback 3: Treasure Room Footprint ($1 \times 1$)
- A $1 \times 1$ footprint ($1000\text{cm} \times 1000\text{cm}$) is $10\text{m} \times 10\text{m}$. That is more than enough space for a treasure chest, ore veins, and lighting props. Having it be $1 \times 1$ is ideal because dead-end leaves can fit snugly into tight spatial branches without needing multi-cell clearing!

---

## 2. Milestone 2: Variable Room Footprints

### 2.1 Multi-Cell Spatial Reservation Engine
In Milestone 1, `MasterGridOccupancy` maps `FIntPoint -> int32 RoomIndex`.
In Milestone 2:
```cpp
USTRUCT(BlueprintType)
struct FSovereignRoomFootprint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint AnchorCoord = FIntPoint::ZeroValue; // Bottom-Left (Min X, Min Y)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint Dimensions = FIntPoint(1, 1); // (W, H) in 1000cm cells

    TArray<FIntPoint> GetOccupiedCells() const
    {
        TArray<FIntPoint> Cells;
        for (int32 X = 0; X < Dimensions.X; ++X)
        {
            for (int32 Y = 0; Y < Dimensions.Y; ++Y)
            {
                Cells.Add(AnchorCoord + FIntPoint(X, Y));
            }
        }
        return Cells;
    }
};
```

### 2.2 Room Type Footprints
- **Entrance:** $1 \times 1$ (Anchor = $(0, 0)$).
- **Corridor:** $1 \times 1$.
- **Treasure Room:** $1 \times 1$.
- **Elite Chamber:** $2 \times 2$ ($2000\text{cm} \times 2000\text{cm}$, 4 grid cells).
- **Boss Arena:** $4 \times 4$ ($4000\text{cm} \times 4000\text{cm}$, 16 grid cells).

### 2.3 Boundary Wall and Door Placement Rules
For any multi-cell room, its outer perimeter has $2(W + H)$ cell faces:
- For a $2 \times 2$ room: $2(2 + 2) = 8$ wall segments of $1000\text{cm}$ each.
- For a $4 \times 4$ room: $2(4 + 4) = 16$ wall segments of $1000\text{cm}$ each.
- **Placement Logic:** The generator checks each constituent edge face:
  - If a graph edge in `DungeonEdges` crosses that specific cell face: spawn a doorway opening (`EPassageType::OpenArchway` or `StandardDoorway`).
  - If no graph edge crosses that face: spawn a standard $1000\text{cm} \times 30\text{cm} \times 600\text{cm}$ wall segment.
  - Internal seams (between cells inside the same room) receive **no walls**, creating a seamless open interior hall!

---

## 3. Milestone 3: Dungeon Variety

### 3.1 Non-Rectangular Room Topologies
Beyond standard boxes, composite shapes can be assembled by reserving L-shaped or T-shaped cell sets:
- **L-Shaped Gallery:** 3 cells ($1 \times 2$ plus an attached $1 \times 1$ alcove).
- **Cruciform Hall:** 5 cells (central hub with 4 cardinal corridor naves).

### 3.2 Multiple Entrances / Exits & Controlled Loops
- Parameter: `MaxLoopCount` (default: 1 or 2).
- When generating side branches, candidate leaf nodes that are adjacent to an earlier corridor can optionally form a **Loop Edge** if:
  1. $\text{TreeDistance}(u, v) \ge 3$ (meaningful navigational shortcut).
  2. Neither node is `BossChamber` (Boss chamber strictly maintains 1 single entrance).
- Provides optional flanking routes during dungeon navigation and step-counter exploration.

### 3.3 Dynamic Room Content Decorators
- **Treasure Rooms:** Spawn a locked chest (`BP_TreasureChest`) containing fragment-based loot (`USovereignItemFragment_Consumable`, rare ore).
- **Elite Chambers:** Spawn a fixed guardian mob blocking the forward exit doorway.
- **Qi Wells:** Small spiritual springs replenishing `USovereignQiComponent` before the boss fight.

---

## 4. Staged Tickets for Milestones 2 & 3

| Ticket ID | Title | Points | Scope |
|:---|:---|:---:|:---|
| **B-094** | Procedural Multi-Cell Footprint Reservation Engine ($1 \times 1, 2 \times 2, 4 \times 4$) | 5 | Implement `FSovereignRoomFootprint` and multi-cell occupancy testing in `ASovereignDungeonGenerator`. |
| **B-095** | Multi-Cell Perimeter Boundary & Edge-Aware Doorway Snapping | 5 | Refactor `SpawnDungeonGeometry()` to iterate perimeter faces of multi-cell rooms without internal walls. |
| **B-096** | Procedural Dungeon Controlled Loops & Shortcut Routing Engine | 5 | Implement cycle candidates with distance filters ($\ge 3$) and boss isolation. |
| **B-097** | Room Semantic Decorators: Treasure Chests, Elite Guardians & Boss Altar | 5 | Spawn contextual interactive actors based on room semantic type. |
