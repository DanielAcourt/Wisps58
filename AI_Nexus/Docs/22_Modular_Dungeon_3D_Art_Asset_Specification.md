# Sovereign Framework: Modular Dungeon 3D Environment Art Specification Guide
**Document ID:** `AI_Nexus/Docs/22_Modular_Dungeon_3D_Art_Asset_Specification.md`
**Author:** Sovereign Research & Core Systems Engineering (07)
**Status:** Approved SSoT Standard for 3D Environment Artists
**Target Engine:** Unreal Engine 5.8 (LWC, Nanite, Lumen)
**Date:** October 2026

---

## 🏛️ 1. Core Coordinate, Unit & Metric Standards

Environment artists modeling for the Sovereign Procedural Dungeon suite must strictly adhere to the following metric guidelines to ensure seamless grid snapping, collision boundaries, and zero visual seam gaps:

| Standard | Engine Value | Real-World Equivalent | Notes |
|:---|:---|:---|:---|
| **System Units** | $1\text{ Unit} = 1\text{cm}$ | Centimeters | Metric standard across all DCC packages (Blender, Maya, 3ds Max). |
| **Grid Cell Size** | $1000\text{cm} \times 1000\text{cm}$ | $10\text{m} \times 10\text{m}$ | Standard procedural footprint unit ($1 \times 1$). |
| **Wall Height** | $600\text{cm}$ | $6\text{m}$ | Accommodates large possessed dragon vessels and third-person camera clearance. |
| **Wall Thickness** | $30\text{cm}$ | $0.3\text{m}$ | Symmetrical depth prevents edge raycast clipping and z-fighting. |
| **Floor Thickness** | $20\text{cm}$ | $0.2\text{m}$ | Top face rests at $Z = 0.0$, slab extends down to $Z = -20.0$. |
| **Doorway Opening** | $400\text{w} \times 500\text{h}\text{cm}$ | $4\text{m} \times 5\text{m}$ | Standard opening cut into doorway wall segments. |
| **Door Leaf** | $380\text{w} \times 480\text{h}\text{cm}$ | $3.8\text{m} \times 4.8\text{m}$ | $10\text{cm}$ aesthetic reveal/margin per side prevents frame clipping. |
| **Corner Seam Pillar**| $50 \times 50 \times 600\text{cm}$ | $0.5\text{m} \times 0.5\text{m} \times 6\text{m}$ | Covers 90-degree outer corner seams without hairline gaps. |

---

## 📍 2. Pivot Point Conventions & Invariants

Having uniform origin pivots is mandatory for procedural programmatic snapping:

```
            +X (Depth / Forward)
                    ▲
                    │
                    │
         ┌──────────┼──────────┐
         │          │          │
-Y (Left)│          ┼──────────┼────► +Y (Right / Width)
         │       PIVOT         │
         │     (0, 0, 0)       │
         └─────────────────────┘
```

1. **Standard Modular Pieces (Floors, Walls, Columns, Pedestals, Frames)**:
   - **Pivot Position**: **Bottom-Center** ($X = 0.0, Y = 0.0, Z = 0.0$).
   - The model must sit entirely on or above the ground plane ($Z \ge 0$), with the origin centered in $X$ and $Y$.
   - **Floors**: Top walkable surface is at $Z = 0.0$, thickness extends downward to $Z = -20.0$.
   - **Walls**: Front face faces $+X$, back face faces $-X$. Width spans along $Y$ from $-500\text{cm}$ to $+500\text{cm}$.

2. **Hinged Moving Pieces (Swinging Doors & Chest Lids)**:
   - **`SM_Dungeon_DoorLeaf_380x480`**:
     - **Pivot Position**: **Bottom-Left Hinge Edge** ($X = 0.0, Y = -190.0, Z = 0.0$).
     - Placing the pivot at the edge allows a simple Yaw rotation (`0` to `90` degrees) in C++/Blueprint to swing open like a real door without offset calculations!
   - **`SM_Dungeon_Chest_Lid`**:
     - **Pivot Position**: **Top-Rear Hinge Edge** ($X = -40.0, Y = 0.0, Z = 50.0$).
     - Placing the pivot at the back edge allows a simple Pitch rotation (`0` to `-90` degrees) to swing open the chest lid upward and backward!

3. **Floating Collectibles (`SM_Dungeon_Key_Pickup`)**:
   - **Pivot Position**: **Geometric Center** ($X = 0.0, Y = 0.0, Z = 0.0$).
   - Centered pivot allows smooth programmatic bobbing and continuous yaw rotation.

---

## 📦 3. Master 3D Asset Inventory & Metric Catalog

| Asset Name | Unreal Class | Dimensions ($X \times Y \times Z$ in cm) | Pivot ($X, Y, Z$) | Collision Preset | Description |
|:---|:---|:---:|:---:|:---:|:---|
| `SM_Dungeon_Floor_1000` | Static Mesh | $1000 \times 1000 \times 20$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Standard modular floor tile. |
| `SM_Dungeon_Wall_1000` | Static Mesh | $30 \times 1000 \times 600$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Solid perimeter boundary wall. |
| `SM_Dungeon_Wall_Doorway_1000`| Static Mesh | $30 \times 1000 \times 600$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Wall segment with $400\text{w} \times 500\text{h}$ center opening. |
| `SM_Dungeon_CornerPillar_50` | Static Mesh | $50 \times 50 \times 600$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Seam cover for 90-degree room corners. |
| `SM_Dungeon_DoorFrame_400x500`| Static Mesh | $30 \times 400 \times 500$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Outer frame snapped into doorway wall. |
| `SM_Dungeon_DoorLeaf_380x480` | Static Mesh | $10 \times 380 \times 480$ | $(0, -190, 0)$ Hinge Edge | `BlockAll` | Swinging door panel inside `BP_DungeonDoor`. |
| `SM_Dungeon_BossGateFrame` | Static Mesh | $40 \times 400 \times 500$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Ornate portal frame for Sealed Elite Gate. |
| `SM_Dungeon_BossGateLeaf` | Static Mesh | $20 \times 380 \times 480$ | $(0, -190, 0)$ Hinge Edge | `BlockAll` | Heavy runic barrier gate leaf inside `BP_BossGate`. |
| `SM_Dungeon_Chest_Base` | Static Mesh | $80 \times 120 \times 50$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Chest container box inside `BP_TreasureChest`. |
| `SM_Dungeon_Chest_Lid` | Static Mesh | $80 \times 120 \times 20$ | $(-40, 0, 50)$ Back Hinge | `BlockAll` | Rotating lid component inside `BP_TreasureChest`. |
| `SM_Dungeon_RestTerminal_Base`| Static Mesh | $120 \times 120 \times 60$ | $(0, 0, 0)$ Bottom-Center | `BlockAll` | Sanctuary campfire/font inside `BP_RestTerminal`. |
| `SM_Dungeon_Key_Pickup` | Static Mesh | $10 \times 20 \times 40$ | $(0, 0, 0)$ Center | `OverlapAllDynamic` | Floating golden vault guardian key pickup. |

---

## 🎨 4. Texturing, Materials & UV Standards

1. **Texel Density**:
   - Target: **$10.24\text{ px/cm}$** ($1024 \times 1024$ pixels per $100\text{cm}$, or $2048 \times 2048$ texture map per $200\text{cm}$ surface).
2. **Nanite & Lumen Compatibility**:
   - High-poly bevels and sculpted stone relief should be baked into normal maps, or authored directly as dense Nanite geometry for UE5.8.
   - Floor and wall slabs must have continuous UV coordinates to support tiling subterranean materials (slate, basalt, carved stone blocks, obsidian, lava veins).
3. **Collision Geometry**:
   - Use simple box/convex collision hulls (`UCX_...`) for performance, or enable **Use Complex Collision as Simple** on Nanite assets.

---

## 🛠️ 5. Blueprint Actor Assembly Hierarchy

For artists creating the Blueprint actors in `Content/Blueprints/Entities/`:

### `BP_DungeonDoor` (Inherits `ASovereignDungeonDoor`)
```
[Root] SceneComponent / PhysicalVessel
 ├── DoorFrameMeshComponent (`SM_Dungeon_DoorFrame_400x500`)
 └── DoorLeafMeshComponent  (`SM_Dungeon_DoorLeaf_380x480`, Relative Location: [0, 0, 0])
```

### `BP_TreasureChest` (Inherits `ASovereignTreasureChest`)
```
[Root] SceneComponent / PhysicalVessel
 ├── ChestBaseMeshComponent (`SM_Dungeon_Chest_Base`)
 └── ChestLidMeshComponent  (`SM_Dungeon_Chest_Lid`, Relative Location: [0, 0, 0])
```

### `BP_RestTerminal` (Inherits `ASovereignRestTerminal`)
```
[Root] PhysicalVessel (`SM_Dungeon_RestTerminal_Base`)
 ├── PointLight (Warm campfire glow, Intensity: 3000, Radius: 600cm)
 └── ParticleSystem / Niagara (Campfire sparks / spiritual mist)
```

### `BP_KeyPickup` (Inherits `ASovereignKeyPickup`)
```
[Root] PhysicalVessel / SceneComponent
 ├── KeyMeshComponent (`SM_Dungeon_Key_Pickup`)
 └── PointLight (Golden beacon glow, Intensity: 1500, Radius: 300cm)
```
