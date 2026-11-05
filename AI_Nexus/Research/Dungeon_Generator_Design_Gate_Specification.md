# Sovereign Framework: Procedural Dungeon Generator Detailed Design Gate Specification
**Document ID:** `AI_Nexus/Research/Dungeon_Generator_Design_Gate_Specification.md`
**Author:** Sovereign Research & Core Systems Engineering (07)
**Status:** Design Gate Submitted for Review
**Date:** October 2026

---

## 🏛️ Executive Summary

This specification resolves the 7 foundational design decisions mandated by the Design Gate before any C++ modifications begin. It establishes:
1. **Grid Footprint Reservation & Anchor Alignment**: Exact coordinate occupancy rules mapping $1000\text{cm}$ grid units to multi-tile $2000\text{cm} \times 2000\text{cm}$ ($2 \times 2$) chambers and $4000\text{cm} \times 4000\text{cm}$ ($4 \times 4$) boss vaults without spatial overlap.
2. **Connection Semantics**: Decoupling abstract graph topology (`FSovereignDungeonEdge`) from physical passage geometry (`EPassageType`) and runtime interaction state (`EDoorState`).
3. **Progression Validation & Guaranteed Reachability**: Minimum main-spine length, mandatory elite-gate pacing, and pure-data Breadth-First Search (BFS) reachability validation.
4. **Strict Determinism**: Complete isolation of the generation pipeline to a single `FRandomStream(GenerationSeed)` verifying layout data reproducibility independently of world actor spawning.
5. **Validation Boundaries**: Clear separation between mathematical layout invariants (pure data) and world-level traversal/collision invariants.
6. **Strict Lifecycle & Ownership Architecture**: Explicit ownership lists preventing the destruction or interference of designer-authored level actors (lights, skyboxes, volumes).
7. **Implementation Scope & Staged Rollout**: Strictly isolating the initial milestone to the graph/data model, spine-and-branch generation, layout validator, and edge-aware instanced geometry.

---

## 1. Grid and Room Footprints

### 1.1 The Unit Metric
- The foundational grid unit is $\Delta G = 1000\text{cm}$ ($10\text{m}$).
- Grid coordinates are integer 2D coordinates: $(X, Y) \in \mathbb{Z}^2$.
- The world center of grid cell $(X, Y)$ relative to generator origin $O$ is:
  $$\text{CellCenter}(X, Y) = O + \begin{pmatrix} X \cdot 1000 \\ Y \cdot 1000 \\ 0 \end{pmatrix}$$

### 1.2 Multi-Tile Room Footprints
Rooms occupy a rectangular set of grid cells: $\text{Width} \times \text{Height}$ in cells ($W \times H$).
To ensure clean alignment, all rooms define an **Anchor Cell** $(X_{\text{anchor}}, Y_{\text{anchor}})$ corresponding to the **bottom-left (minimum $X, Y$) cell** of the footprint:

$$\text{OccupiedCells}(X_{\text{anchor}}, Y_{\text{anchor}}, W, H) = \{ (X_{\text{anchor}} + dx, Y_{\text{anchor}} + dy) \mid dx \in [0, W-1], dy \in [0, H-1] \}$$

| Room Type | Metric Dimensions | Grid Dimensions ($W \times H$) | Occupied Cells Count | Geometric Center Offset from Anchor Cell Center |
|:---|:---:|:---:|:---:|:---|
| **Corridor** | $1000\text{cm} \times 1000\text{cm}$ | $1 \times 1$ | 1 cell | $(0, 0, 0)$ |
| **Entrance Room** | $1000\text{cm} \times 1000\text{cm}$ | $1 \times 1$ | 1 cell | $(0, 0, 0)$ |
| **Chamber / Treasure** | $2000\text{cm} \times 2000\text{cm}$ | $2 \times 2$ | 4 cells | $(+500\text{cm}, +500\text{cm}, 0)$ |
| **Elite Chamber** | $2000\text{cm} \times 2000\text{cm}$ | $2 \times 2$ | 4 cells | $(+500\text{cm}, +500\text{cm}, 0)$ |
| **Boss Chamber** | $4000\text{cm} \times 4000\text{cm}$ | $4 \times 4$ | 16 cells | $(+1500\text{cm}, +1500\text{cm}, 0)$ |

### 1.3 Footprint Reservation & Overlap Prevention
- The generator maintains a master spatial reservation map:
  $$\text{TMap<FIntPoint, int32> MasterGridOccupancy;}$$
  mapping every occupied grid cell coordinate to its owning `RoomIndex`.
- **Pre-Placement Test**: Before placing any room of size $W \times H$ at candidate anchor $(X_A, Y_A)$:
  $$\forall dx \in [0, W-1], dy \in [0, H-1]: \quad \text{MasterGridOccupancy.Contains}(X_A + dx, Y_A + dy) == \text{false}$$
  If any cell is already occupied, placement is rejected.
- **Safety Margin / Buffer Zone**: Multi-cell chambers enforce a 1-cell empty perimeter buffer for external connections, ensuring distinct rooms never share walls unless explicitly linked by a designated passage.

### 1.4 Doorway Sockets & Wall Alignment
Every boundary face between cell $(X, Y)$ and neighbor cell $(X + dx, Y + dy)$ corresponds to a standardized edge segment of length $1000\text{cm}$ centered at:
$$\text{BoundaryEdgeCenter} = \text{CellCenter}(X, Y) + \begin{pmatrix} dx \cdot 500 \\ dy \cdot 500 \\ 300 \end{pmatrix}$$
- On multi-cell chambers ($2 \times 2$ or $4 \times 4$), doors are located at the midpoint of specific constituent boundary cells (never on a multi-tile corner seam).
- For a $2 \times 2$ chamber entering from the South, connection occurs at cell $(X_{\text{anchor}}, Y_{\text{anchor}})$ or $(X_{\text{anchor}}+1, Y_{\text{anchor}})$, perfectly matching the incoming $1000\text{cm}$ corridor without fractional misalignment.

---

## 2. Connection Semantics

Logical graph connectivity is strictly decoupled from visual passage geometry and runtime interaction state:

```
                      [ Logical Graph Edge ]
                    (FSovereignDungeonEdge)
                              │
               ┌──────────────┴──────────────┐
               ▼                             ▼
    [ Physical Passage Type ]       [ Runtime Door State ]
        (EPassageType)                   (EDoorState)
      - SolidWall                     - Open
      - OpenArchway                   - ClosedUnlocked
      - StandardDoorway               - LockedKeyRequired
      - NarrowCrevice (Polymorph)     - SealedEliteGate
      - HiddenSecretWall              - BossVaultSealed
```

### 2.1 The Data Models
```cpp
UENUM(BlueprintType)
enum class EDungeonDirection : uint8
{
    North = 0,
    East  = 1,
    South = 2,
    West  = 3,
    None  = 4
};

UENUM(BlueprintType)
enum class EPassageType : uint8
{
    SolidWall,          // No passage exists; boundary edge sealed with standard wall mesh
    OpenArchway,        // Unrestricted opening between connected rooms/corridors
    StandardDoorway,    // Structural frame fitted with an interactive door actor
    NarrowCrevice,      // Impassable to large vessels; requires Humanoid Visage polymorph
    HiddenSecretWall    // Appears as solid wall; revealed upon proximity or Qi channeling
};

UENUM(BlueprintType)
enum class EDoorState : uint8
{
    Open,               // Fully passable, no collision barrier
    ClosedUnlocked,     // Standard door; opens on player interaction/approach
    LockedKeyRequired,  // Requires dungeon key or item sacrifice to unseal
    SealedEliteGate,    // Locked until associated elite mob in previous chamber is defeated
    BossVaultSealed     // Terminal threshold locked until floor trial requirements met
};

USTRUCT(BlueprintType)
struct FSovereignDungeonEdge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EdgeId = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FromRoomIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ToRoomIndex = INDEX_NONE;

    /** Specific cell coordinate on Room A where passage attaches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint CellCoordA = FIntPoint::ZeroValue;

    /** Specific cell coordinate on Room B where passage attaches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint CellCoordB = FIntPoint::ZeroValue;

    /** Cardinal direction from Cell A to Cell B */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDungeonDirection Direction = EDungeonDirection::None;

    /** Structural rendering style of the passage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPassageType PassageType = EPassageType::OpenArchway;

    /** Runtime interaction and locking barrier state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDoorState DoorState = EDoorState::Open;

    /** Associated lock/key tag (e.g. "Key.Dungeon.Bronze", "Quest.EliteDefeated") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag LockTag;
};
```

---

## 3. Progression Validation & Pacing Rules

The generator must enforce gameplay progression mathematically before any actors or mesh instances are spawned.

### 3.1 Pacing Invariants
1. **Critical Path (Main Spine):**
   $$\text{SpineNodes} = [R_0, R_1, \dots, R_L]$$
   - $R_0 \equiv \text{EntranceRoom}$ at $(0, 0)$.
   - $R_L \equiv \text{BossChamber}$ (terminal node of spine).
   - $\text{SpineLength } L \ge \text{MinSpineLength}$ (e.g., $\ge 5$ rooms for a standard 8-to-12 room dungeon).
2. **Mandatory Elite-Gate Placement:**
   - The node immediately preceding the Boss Chamber ($R_{L-1}$) is guaranteed to be an `EliteChamber`.
   - The edge connecting $R_{L-1} \to R_L$ has:
     $$\text{PassageType} = \text{StandardDoorway}, \quad \text{DoorState} = \text{SealedEliteGate}$$
   - Defeating the guardian in $R_{L-1}$ broadcasts an event that unlocks this gate.
3. **Branch Pacing & Dead-End Guarantees:**
   - Optional branches sprout only from intermediate spine nodes $R_k$ ($0 < k < L-1$).
   - Branch depth is constrained: $1 \le \text{Depth} \le 3$.
   - The terminal leaf of every side branch must be assigned a reward semantic (`TreasureRoom`, `QiShrine`, or `DragonRestChamber`).
   - Side branches never connect into the Boss Chamber.

### 3.2 Topological Reachability Algorithm (Pure-Data BFS)
Prior to geometry construction, the generator runs a Breadth-First Search over the logical graph $(V, E)$:
```text
Algorithm ValidateTopologicalReachability(Nodes, Edges, EntranceIdx, BossIdx):
    Visited = Set()
    Queue = FIFOQueue()
    ParentMap = Map()

    Queue.Enqueue(EntranceIdx)
    Visited.Add(EntranceIdx)

    while Queue is not empty:
        Current = Queue.Dequeue()
        for Edge in GetOutgoingEdges(Current, Edges):
            Neighbor = Edge.GetOtherNode(Current)
            if Neighbor not in Visited:
                Visited.Add(Neighbor)
                ParentMap[Neighbor] = Current
                Queue.Enqueue(Neighbor)

    // Invariant 1: Boss Reachability
    if BossIdx not in Visited:
        return Failure("Boss Chamber unreachable from Entrance")

    // Invariant 2: Path Length
    PathLength = ComputePathLength(ParentMap, EntranceIdx, BossIdx)
    if PathLength < MinSpineLength:
        return Failure("Main spine path length below minimum threshold")

    // Invariant 3: Zero Orphan Nodes
    if Visited.Count != Nodes.Count:
        return Failure("Dungeon contains disconnected orphan rooms")

    return Success
```

---

## 4. Determinism Specification

### 4.1 Definition of Determinism
Determinism is defined strictly at the **Data Model Layer**:
> For any given `(GenerationSeed, TargetRoomCount, MinSpineLength)`, executing the topology algorithm must produce the **exact same** sequence of room definitions (types, grid anchor coordinates, cell reservations) and edge definitions (source, target, direction, passage type, door state), regardless of platform, framerate, or editor execution context.

### 4.2 Stream Isolation Rules
- **No Global Randomness:** Calls to `FMath::Rand()`, `FMath::FRand()`, or `rand()` are strictly forbidden.
- All random branching, directional choices, room semantic rolls, and loot table seeding must query a dedicated local `FRandomStream GenerationStream;` initialized with `GenerationSeed`.
- **Spawning Independence:** World actor spawning and component registration must not consume random numbers from `GenerationStream`. The layout data must be completely resolved first; spawning is a pure 1:1 translation from layout data to 3D instances.

---

## 5. Validation Boundaries (Layout Data vs. World Verification)

| Validation Domain | Properties Proven at Data Model Layer (Fast Unit Tests) | Properties Verified at World Level (Integration Tests) |
|:---|:---|:---|
| **Graph Topology** | - Main spine connectivity<br>- Zero orphan nodes<br>- Boss reachability via BFS<br>- Path distance $\ge \text{MinSpineLength}$ | - AI Navigation Mesh generation (`UNavigationSystemV1`)<br>- Player pawn pathfinding traversal across thresholds |
| **Grid Placement** | - Zero overlapping cell reservations<br>- Anchor coordinates within bounds<br>- Correct footprint cell math ($1 \times 1, 2 \times 2, 4 \times 4$) | - Physical collision volumes correctly blocking pawns<br>- Floor collision planes continuous without gaps |
| **Connectivity** | - Edges strictly link adjacent cardinal cells<br>- Edge count and neighbor symmetry | - Doorway frame meshes render without z-fighting<br>- Door actor opening/closing animation and collision toggles |
| **Pacing & Roles** | - Node $0$ is `EntranceRoom`<br>- Node $L$ is `BossChamber`<br>- Node $L-1$ is `EliteChamber`<br>- Branch leaves are `TreasureRoom` | - Fixed elite mob spawns inside $R_{L-1}$<br>- Defeating mob triggers `SealedEliteGate` door opening<br>- Boss egg altar spawns at boss room transform |

---

## 6. Lifecycle & Ownership Architecture

To prevent deleting designer-placed lights, level skyboxes, or persistent gameplay volumes:

### 6.1 Explicit Tracked Arrays
`ASovereignDungeonGenerator` maintains explicit tracking pointers for objects it creates:
```cpp
UPROPERTY(Transient)
TArray<TObjectPtr<AActor>> GeneratedSpawnedActors;

UPROPERTY(Transient)
TObjectPtr<ADirectionalLight> GeneratorSpawnedLight;

UPROPERTY(Transient)
TObjectPtr<APostProcessVolume> GeneratorSpawnedPostProcess;

UPROPERTY(Transient)
TArray<TObjectPtr<ASovereignBattleManager>> GeneratorSpawnedArenas;
```

### 6.2 Strict Cleanup Contract (`CleanupDungeon()`)
Before any re-generation:
1. **Instances Cleared:**
   - `FloorInstancedComponent->ClearInstances()`
   - `WallInstancedComponent->ClearInstances()`
2. **Only Tracked Actors Destroyed:**
   - Iterate through `GeneratedSpawnedActors`, call `Destroy()`, and empty the array.
   - If `GeneratorSpawnedLight` was created by this generator, destroy it and set to `nullptr`. (Designer-authored lights in the level are never touched!)
   - If `GeneratorSpawnedPostProcess` was created by this generator, destroy it and set to `nullptr`.
   - Destroy `GeneratorSpawnedArenas` and empty the array.
3. **Data Emptied:**
   - `DungeonNodes.Empty()`
   - `DungeonEdges.Empty()`
   - `MasterGridOccupancy.Empty()`

---

## 7. Algorithm Pseudocode (Spine-and-Branch Spanning Tree)

```text
Algorithm GenerateDungeonLayout(Seed, TargetRoomCount, MinSpineLength):
    RandomStream.Initialize(Seed)
    CleanupDungeon()

    // 1. Initialize Master Grid & Node List
    Nodes = []
    Edges = []
    MasterGridOccupancy = Map<FIntPoint, int32>()

    // 2. Place Entrance Room (Node 0) at (0, 0)
    EntranceNode = CreateNode(Index=0, Type=EntranceRoom, Anchor=(0,0), Size=(1,1))
    Nodes.Add(EntranceNode)
    ReserveCells(MasterGridOccupancy, Anchor=(0,0), Size=(1,1), RoomIndex=0)

    // 3. Build Critical Main Spine
    CurrentAnchor = FIntPoint(0, 0)
    CurrentNodeIndex = 0
    SpineRoomCount = Clamp(Floor(TargetRoomCount * 0.6), MinSpineLength, TargetRoomCount - 2)

    for Step = 1 to SpineRoomCount - 1:
        CardinalDir = SelectOpenDirection(RandomStream, CurrentAnchor, Size=(1,1), MasterGridOccupancy)
        if CardinalDir == None:
            return FailAndRetry("Spine blocked by self-intersection")

        NextAnchor = CurrentAnchor + DirectionToVector(CardinalDir)
        NodeType = (Step == SpineRoomCount - 2) ? EliteChamber : Chamber
        RoomSize = (NodeType == EliteChamber) ? (2, 2) : (1, 1)

        // If placing 2x2 Chamber, verify multi-cell footprint
        if not CanFitRoom(NextAnchor, RoomSize, MasterGridOccupancy):
            // fallback to 1x1 if space constrained
            RoomSize = (1, 1)

        NextNodeIndex = Nodes.Num()
        NextNode = CreateNode(Index=NextNodeIndex, Type=NodeType, Anchor=NextAnchor, Size=RoomSize)
        Nodes.Add(NextNode)
        ReserveCells(MasterGridOccupancy, NextAnchor, RoomSize, NextNodeIndex)

        // Record bidirectional edge with connection metadata
        Edge = CreateEdge(CurrentNodeIndex, NextNodeIndex, CurrentAnchor, NextAnchor, CardinalDir)
        Edges.Add(Edge)

        CurrentAnchor = NextAnchor
        CurrentNodeIndex = NextNodeIndex

    // 4. Place Terminal Boss Chamber at Terminus of Spine
    BossDir = SelectOpenDirection(RandomStream, CurrentAnchor, Size=(4,4), MasterGridOccupancy)
    BossAnchor = CurrentAnchor + DirectionToVector(BossDir)
    BossNodeIndex = Nodes.Num()
    BossNode = CreateNode(Index=BossNodeIndex, Type=BossChamber, Anchor=BossAnchor, Size=(4,4))
    Nodes.Add(BossNode)
    ReserveCells(MasterGridOccupancy, BossAnchor, Size=(4,4), RoomIndex=BossNodeIndex)

    BossEdge = CreateEdge(CurrentNodeIndex, BossNodeIndex, CurrentAnchor, BossAnchor, BossDir)
    BossEdge.PassageType = StandardDoorway
    BossEdge.DoorState = SealedEliteGate
    Edges.Add(BossEdge)

    // 5. Build Side Branches for Remaining Budget
    RoomsRemaining = TargetRoomCount - Nodes.Num()
    while RoomsRemaining > 0:
        ParentSpineIdx = RandomStream.RandRange(1, SpineRoomCount - 2)
        BranchAnchor = Nodes[ParentSpineIdx].Anchor
        BranchDir = SelectOpenDirection(RandomStream, BranchAnchor, Size=(1,1), MasterGridOccupancy)
        if BranchDir != None:
            LeafAnchor = BranchAnchor + DirectionToVector(BranchDir)
            LeafIdx = Nodes.Num()
            LeafNode = CreateNode(Index=LeafIdx, Type=TreasureRoom, Anchor=LeafAnchor, Size=(1,1))
            Nodes.Add(LeafNode)
            ReserveCells(MasterGridOccupancy, LeafAnchor, Size=(1,1), LeafIdx)

            LeafEdge = CreateEdge(ParentSpineIdx, LeafIdx, BranchAnchor, LeafAnchor, BranchDir)
            Edges.Add(LeafEdge)
            RoomsRemaining--
        else:
            // No open space on this spine node; try another or terminate branch phase
            break

    // 6. Execute Topological Validation
    ValidationResult = ValidateTopologicalReachability(Nodes, Edges, EntranceIdx=0, BossIdx=BossNodeIndex)
    if not ValidationResult.bSuccess:
        return FailAndRetry("Generated graph failed validation invariants")

    // 7. Render Edge-Aware Geometry (ISMC Floors, Boundary Walls, Sockets)
    ConstructGeometry(Nodes, Edges, MasterGridOccupancy)
    return Success
```

---

## 8. Invariants and Acceptance Tests

### 8.1 Layout Invariants (Verified by Unit Tests)
1. **Uniqueness:** For all $i \neq j$, $\text{OccupiedCells}(R_i) \cap \text{OccupiedCells}(R_j) = \emptyset$.
2. **Spine Length:** $\text{ShortestPath}(R_0, R_{\text{Boss}}) \ge \text{MinSpineLength}$.
3. **Pacing Guard:** The direct parent of $R_{\text{Boss}}$ in the spanning tree has `TileType == EliteChamber` and the connecting edge has `DoorState == SealedEliteGate`.
4. **Edge Consistency:** For every edge $(A, B)$ with direction $D$, $\text{CellB} == \text{CellA} + \text{DirectionOffset}(D)$.
5. **Enclosure Integrity:** For every room cell face, a wall instance exists **if and only if** no edge crosses that face in the graph.

### 8.2 Automation Test Suite (`WispCPP7VRTests`)
- `FSovereignDungeonDeterminismTest`: Generates layout data twice with Seed 1337; asserts exact matching node count, cell sets, edge types, and door states.
- `FSovereignDungeonReachabilityTest`: Verifies that BFS traverses from Entrance to Boss across 100 randomly sampled seeds with 0 reachability failures.
- `FSovereignDungeonOverlapTest`: Asserts that `MasterGridOccupancy` contains zero duplicate coordinate assignments for $1 \times 1, 2 \times 2,$ and $4 \times 4$ rooms.
- `FSovereignDungeonLifecycleTest`: Runs generation 5 times on the same actor; asserts no leaked actors, component instance count resets cleanly, and level lights remain intact.

---

## 9. Header Changes & Interface Compatibility

### 9.1 Compatibility Confirmation
- `USovereignDungeonSubsystem`: **Zero breaking changes.** `ActiveExpeditionSeed`, `EnemyBuffMultiplier`, `RewardMultiplier`, and `RecordExpedition()` remain the source of truth for expedition parameters.
- `ASovereignDungeonGameMode`: **Zero breaking changes.** Queries `ActiveDungeonGenerator->GetEntranceTransform()` exactly as before.
- `USovereignEncounterSubsystem`: **Zero breaking changes.** Teleports to `ASovereignBattleManager` platforms.

### 9.2 Modified Header: `SovereignDungeonGenerator.h`
- Adds `EDungeonDirection`, `EPassageType`, `EDoorState` enums.
- Adds `FSovereignDungeonEdge` struct.
- Expands `FSovereignDungeonNode` with `RoomIndex`, `AnchorCoord`, `FootprintSize`, `ConnectedEdges`.
- Replaces coordinate-only walk with Spine-and-Branch generator and `ValidateTopologicalReachability()` helper.
- Adds explicit tracking array `GeneratedSpawnedActors` and cleanup methods.
