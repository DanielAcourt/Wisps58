# Sovereign Framework: Procedural Dungeon Generator Milestone 1 Implementation Plan & Formal Acceptance Specification
**Document ID:** `AI_Nexus/Timeline/Dungeon_Generator_Milestone1_Plan.md`
**Author:** Sovereign Research & Core Systems Engineering (07)
**Status:** Approved Architecture Plan / Final Design Gate
**Date:** October 2026

---

## 🏛️ Executive Summary

This document formalizes the exact implementation sequence, header/API contracts, data models, algorithm invariants, and acceptance test criteria for **Milestone 1** of `ASovereignDungeonGenerator`.

In accordance with strict stakeholder directives, this milestone is laser-focused on:
1. **Uniform $1 \times 1$ grid cells ($1000\text{cm} \times 1000\text{cm}$)** — zero fractional seams, zero spatial overlap.
2. **Authoritative edge list (`DungeonEdges`)** as the Single Source of Truth (SSoT), with derived node adjacency caches.
3. **Pure-data generation and BFS validation** executing before any 3D world geometry or actors are instantiated.
4. **Deterministic Spine-and-Branch topology** with bounded retries, explicit spine tracking, mandatory elite chamber preceding the single-entrance Boss Chamber, and default spanning tree invariant ($|E| = |V| - 1$ when `MaxLoopCount == 0`).
5. **Edge-aware geometry** distinguishing exterior boundary walls, shared walls between disconnected neighbors, and open passage arches.
6. **Strict ownership and cleanup** destroying only generator-spawned components/actors, completely isolating designer-placed level actors and lights.

Variable footprints ($2 \times 2, 4 \times 4$), keys, locks, possession gates, and polymorph puzzles are strictly deferred to subsequent milestones.

---

## 1. Exact Header & API Specification (`SovereignDungeonGenerator.h`)

Below is the exact public and protected interface to be integrated into `WispsCPPVR/Source/WispCPP7VR/Public/Combat/SovereignDungeonGenerator.h`:

```cpp
// --- Direction, Passage, and Door Enums ---

UENUM(BlueprintType)
enum class EDungeonDirection : uint8
{
	North = 0, // +X direction
	East  = 1, // +Y direction
	South = 2, // -X direction
	West  = 3, // -Y direction
	None  = 4
};

UENUM(BlueprintType)
enum class EPassageType : uint8
{
	OpenArchway     UMETA(DisplayName = "Open Archway"),
	SolidWall       UMETA(DisplayName = "Solid Wall"),
	StandardDoorway UMETA(DisplayName = "Standard Doorway")
};

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Open            UMETA(DisplayName = "Open"),
	ClosedUnlocked  UMETA(DisplayName = "Closed Unlocked"),
	SealedEliteGate UMETA(DisplayName = "Sealed Elite Gate")
};

// --- Value Structs ---

/**
 * FSovereignDungeonEdge
 * Authoritative Single Source of Truth (SSoT) for connectivity between two rooms.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonEdge
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 EdgeId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndexA = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndexB = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EDungeonDirection DirectionAtoB = EDungeonDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EPassageType PassageType = EPassageType::OpenArchway;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EDoorState DoorState = EDoorState::Open;

	bool Connects(int32 NodeIdx) const { return NodeIndexA == NodeIdx || NodeIndexB == NodeIdx; }
	int32 GetOtherNode(int32 NodeIdx) const { return (NodeIndexA == NodeIdx) ? NodeIndexB : (NodeIndexB == NodeIdx ? NodeIndexA : INDEX_NONE); }
};

/**
 * FSovereignDungeonNode
 * Represents an individual uniform 1000cm x 1000cm tile node in the layout.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Corridor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FTransform WorldTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	bool bHasFixedEliteMob = false;

	/** Transient bitmask cache derived strictly from DungeonEdges (Bit 0=N, 1=E, 2=S, 3=W) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	uint8 ConnectedPassageMask = 0;
};

// --- ASovereignDungeonGenerator Class Additions ---

// Config
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
int32 MinSpineLength = 4;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
int32 MaxLoopCount = 0; // Default 0: Strict spanning tree invariant |E| = |V| - 1

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
int32 MaxGenerationRetries = 20; // Bounded deterministic retry limit

// Authoritative Graph State
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
TArray<FSovereignDungeonEdge> DungeonEdges;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
TArray<int32> MainSpineNodeIndices;

// Explicit Ownership Tracking
UPROPERTY(Transient)
TArray<TObjectPtr<AActor>> GeneratorSpawnedActors;

UPROPERTY(Transient)
TObjectPtr<ADirectionalLight> GeneratorSpawnedLight;

UPROPERTY(Transient)
TObjectPtr<APostProcessVolume> GeneratorSpawnedPostProcess;

UPROPERTY(Transient)
TArray<TObjectPtr<ASovereignBattleManager>> GeneratorSpawnedArenas;

// Core Engine API
UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
bool GenerateDungeonLayoutData(); // Pure-data generation & validation step

UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
bool ValidateTopologicalReachability(int32 EntranceIdx, int32 BossIdx) const;

UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
void RebuildNodeAdjacencyCaches();

UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
void CleanupDungeon(); // Explicit isolated cleanup of owned actors/components only
```

---

## 2. Refined Algorithm & Pipeline Pseudocode

```text
Algorithm GenerateDungeon():
    // Step 0: Ensure generator state is clean before starting
    CleanupDungeon()

    // Step 1: Pure-Data Generation with Bounded Retries
    bLayoutSuccess = false
    For Attempt = 0 to MaxGenerationRetries:
        // Derive unique seed per attempt deterministically from primary GenerationSeed
        AttemptSeed = (GenerationSeed != 0) ? (GenerationSeed + Attempt * 7919) : FMath::Rand()
        Stream.Initialize(AttemptSeed)

        If BuildSpineAndBranchGraph(Stream):
            If ValidateTopologicalReachability(EntranceIdx=0, BossIdx=BossNodeIndex):
                bLayoutSuccess = true
                Break

    If not bLayoutSuccess:
        UE_LOG("Failed to generate valid layout within retry budget")
        Return false

    // Step 2: Derive Transient Adjacency Caches from Authoritative Edge List
    RebuildNodeAdjacencyCaches()

    // Step 3: Spawn Far-Off Isolated Battle Arenas
    SpawnFarOffBattleArenas()

    // Step 4: Edge-Aware 3D Geometry Instantiation
    If bSpawnPhysicalGeometry:
        SpawnDungeonGeometry()

    // Step 5: Optional Fallback Lighting Rig
    If bAutoSpawnDefaultLighting:
        SpawnDefaultLightingRig()

    OnDungeonGenerated.Broadcast(DungeonNodes.Num())
    Return true
```

### Detailed Sub-Routines:

#### A. `BuildSpineAndBranchGraph(Stream)`
1. **Initialize Grid:** Clear `DungeonNodes`, `DungeonEdges`, `MainSpineNodeIndices`, and `MasterGridOccupancy` (`TMap<FIntPoint, int32>`).
2. **Node 0 (Entrance Room):**
   - Place at `(0, 0)` with `TileType = EntranceRoom`.
   - Add to `DungeonNodes`, `MainSpineNodeIndices`, and register in `MasterGridOccupancy`.
3. **Build Critical Main Spine:**
   - Target spine count $S = \text{Clamp}(\text{Floor}(\text{TargetRoomCount} \times 0.65), \text{MinSpineLength}, \text{TargetRoomCount} - 2)$.
   - Current coordinate $C = (0, 0)$, Current index $P = 0$.
   - For step $i = 1$ to $S - 1$:
     - Find available cardinal neighbor coordinates not in `MasterGridOccupancy`.
     - If none available: return false (triggers bounded attempt retry).
     - Select direction $D$ via `Stream.RandRange`.
     - Next coordinate $N = C + \text{DirectionVector}(D)$, Next index $Idx = \text{DungeonNodes.Num()}$.
     - Room type: if $(i == S - 2)$ set `EliteChamber` with `bHasFixedEliteMob = true`, else `Chamber` or `Corridor`.
     - Add node to `DungeonNodes`, `MainSpineNodeIndices`, and `MasterGridOccupancy`.
     - Create authoritative edge: `(EdgeId, P, Idx, D, PassageType = OpenArchway, DoorState = Open)`.
     - Add to `DungeonEdges`.
     - Advance $C = N$, $P = Idx$.
4. **Place Terminal Boss Chamber:**
   - Find available cardinal neighbor off current spine tip $C$.
   - If none available: return false.
   - Place node $B$ at coordinate $N_{\text{Boss}}$ with `TileType = BossChamber`, `bHasFixedEliteMob = true`.
   - Register $B$ as `BossNodeIndex`, append to `MainSpineNodeIndices` and `MasterGridOccupancy`.
   - Create edge between spine tip and boss: `PassageType = StandardDoorway`, `DoorState = SealedEliteGate`.
   - Add to `DungeonEdges`.
5. **Build Side Branches:**
   - Remaining rooms budget $R = \text{TargetRoomCount} - \text{DungeonNodes.Num()}$.
   - Eligible parent nodes: spine indices $[1, S - 2]$ (excluding Entrance and Boss).
   - While $R > 0$ and eligible parents remain:
     - Pick random parent from eligible list.
     - Find open adjacent cardinal directions. If none, remove parent from list and continue.
     - Pick open direction, allocate branch node with `TileType = Chamber` (or leaf as `Chamber` with potential rewards).
     - Add edge between parent and branch node (`OpenArchway`, `Open`).
     - Decrement $R$.
6. **Controlled Loops (Only when `MaxLoopCount > 0`):**
   - If `MaxLoopCount > 0`, scan node pairs that are physically adjacent on the grid but share no edge in `DungeonEdges`.
   - Filter out any connections to `BossNodeIndex`.
   - Check graph distance: only add edge if tree distance $\ge 3$.
   - Add up to `MaxLoopCount` loop edges.
   - *Invariant:* When `MaxLoopCount == 0`, $|E| == |V| - 1$ strictly holds.

#### B. `ValidateTopologicalReachability(EntranceIdx, BossIdx)`
1. **Physical Adjacency & Direction Invariant:**
   - For every edge in `DungeonEdges`:
     - Assert $A \neq B$.
     - Let $C_A = \text{Nodes}[A].\text{GridCoord}$, $C_B = \text{Nodes}[B].\text{GridCoord}$.
     - Assert $|C_A.X - C_B.X| + |C_A.Y - C_B.Y| == 1$.
     - Assert $C_B == C_A + \text{DirectionOffset}(D)$.
2. **Structural BFS Traversal:**
   - Queue initialized with `EntranceIdx`.
   - Traverse across all edges in `DungeonEdges`.
   - Record `Visited` set and track distance from Entrance.
   - Assert `BossIdx` is in `Visited`.
   - Assert `Visited.Num() == DungeonNodes.Num()` (zero orphan rooms).
3. **Pacing Invariants:**
   - Assert distance from `EntranceIdx` to `BossIdx` $\ge \text{MinSpineLength}$.
   - Assert `DungeonNodes[EntranceIdx].TileType == EntranceRoom`.
   - Assert `DungeonNodes[BossIdx].TileType == BossChamber`.
   - Assert exactly 1 edge connects to `BossIdx`.
   - Let BossEdge be the edge connected to `BossIdx`.
     - Let PredIdx be `BossEdge.GetOtherNode(BossIdx)`.
     - Assert `DungeonNodes[PredIdx].TileType == EliteChamber`.
     - Assert `DungeonNodes[PredIdx].bHasFixedEliteMob == true`.
     - Assert `BossEdge.DoorState == EDoorState::SealedEliteGate`.

#### C. `RebuildNodeAdjacencyCaches()`
- For every node in `DungeonNodes`:
  - Reset `ConnectedPassageMask = 0`.
- For every edge in `DungeonEdges`:
  - Bit for direction $D$ is set on Node A (`ConnectedPassageMask |= (1 << D)`).
  - Bit for opposite direction $\text{Opposite}(D)$ is set on Node B (`ConnectedPassageMask |= (1 << Opposite(D))`).

#### D. Edge-Aware `SpawnDungeonGeometry()`
- Clear `FloorInstancedComponent` and `WallInstancedComponent`.
- For every node $i$:
  - Add floor instance at `Node.WorldTransform` ($1000\text{cm} \times 1000\text{cm} \times 20\text{cm}$).
  - For each of the 4 cardinal directions (North, East, South, West):
    - Check if bit is set in `Node.ConnectedPassageMask`:
      - **If bit IS set (Edge Exists):**
        - Do **NOT** spawn a solid wall.
        - If `PassageType == StandardDoorway`, spawn doorway frame mesh if assigned.
      - **If bit is NOT set (No Edge):**
        - Spawn a solid perimeter boundary wall instance at that cell face ($40\text{cm}$ thick, $1000\text{cm}$ wide, $600\text{cm}$ high).
        - *Crucial:* This handles both exterior boundary faces AND shared faces between physically adjacent nodes that are disconnected in the graph!

#### E. Strict `CleanupDungeon()`
- Clear `FloorInstancedComponent->ClearInstances()`.
- Clear `WallInstancedComponent->ClearInstances()`.
- Destroy and empty `GeneratorSpawnedActors` (boss altar, props).
- Destroy and empty `GeneratorSpawnedArenas` (`ASovereignBattleManager`).
- If `GeneratorSpawnedLight` is valid, call `Destroy()` and reset to nullptr.
- If `GeneratorSpawnedPostProcess` is valid, call `Destroy()` and reset to nullptr.
- Empty data structures: `DungeonNodes`, `DungeonEdges`, `MainSpineNodeIndices`.
- **Level actors rule:** Never calls `GetAllActorsOfClass` to destroy actors. Only destroys actors tracked in its own `GeneratorSpawned*` references.

---

## 3. Strict Acceptance Criteria & Test Matrix

The test suite in `WispsCPPVR/Source/WispCPP7VRTests/Private/Combat/SovereignDungeonGatewayTests.cpp` will be expanded with the following automated assertions:

| Test Name | Verification Target | Assertion Criteria |
|:---|:---|:---|
| **1. Edge Adjacency & Direction Integrity** | Edge validity | For 100% of edges in `DungeonEdges`: $A \neq B$, grid Manhattan distance is exactly 1, coordinate matches declared cardinal direction. |
| **2. Spanning Tree Invariant** | Graph topology when `MaxLoopCount == 0` | $|E| == |V| - 1$ strictly holds. Exactly zero cycles exist. |
| **3. Loop Control Invariant** | Graph topology when `MaxLoopCount > 0` | $|E| \le (|V| - 1) + \text{MaxLoopCount}$. Boss node has strictly degree 1 (0 extra loops to boss). |
| **4. Critical Spine & Pacing** | Main spine length & role order | Path length from Entrance to Boss $\ge \text{MinSpineLength}$. Node 0 is `EntranceRoom`. Last spine node is `BossChamber`. Node preceding Boss is `EliteChamber` with `SealedEliteGate`. |
| **5. Pure-Data BFS Reachability** | Structural traversability | 100% of nodes visited from Entrance. 0 orphan nodes. Tested across 20 distinct seeds with zero failures. |
| **6. Seed Determinism** | Reproducibility | Two separate runs with `Seed = 1337`: exact matching `DungeonNodes.Num()`, `DungeonEdges.Num()`, matching coordinate sequence, matching edge IDs and directions. Two runs with different seeds produce distinct layouts. |
| **7. Edge-Aware Wall Enclosure** | Geometry fidelity | A node with 1 open passage has exactly 3 walls. A node with 2 open passages has 2 walls. Two adjacent nodes without an edge have solid walls between them. |
| **8. Strict Lifecycle Cleanup** | No memory leaks / no collateral damage | Running generation 5 consecutive times: instance count resets cleanly each time, `GeneratorSpawnedActors` count does not accumulate, external level actors are untouched. |
| **9. Subsystem Regression Compatibility** | Seamless gameplay integration | `USovereignDungeonSubsystem` passes expedition seed cleanly; `ASovereignDungeonGateway::EnterDungeon` succeeds; `ASovereignDungeonGameMode` aligns leader to Entrance transform; `USovereignEncounterSubsystem` transitions to generated arenas. |

---

## 4. Staged Implementation Sequence

1. **Step 1: Set Plan**
   - Register plan steps in the environment following acceptance approval.
2. **Step 2: Header Updates**
   - Add `EDungeonDirection`, `EPassageType`, `EDoorState`, `FSovereignDungeonEdge`, expanded `FSovereignDungeonNode`, and new method declarations to `SovereignDungeonGenerator.h`.
3. **Step 3: Core Implementation (`SovereignDungeonGenerator.cpp`)**
   - Implement `CleanupDungeon()`.
   - Implement `BuildSpineAndBranchGraph()` and bounded retry loop in `GenerateDungeon()`.
   - Implement `ValidateTopologicalReachability()`.
   - Implement `RebuildNodeAdjacencyCaches()`.
   - Implement edge-aware `SpawnDungeonGeometry()`.
4. **Step 4: Unit Test Suite Expansion**
   - Add the 9 automated acceptance tests to `SovereignDungeonGatewayTests.cpp`.
5. **Step 5: Pre-Commit Checks & Build Verification**
   - Compile code, run unit test suites, request code review, implement feedback, and record memories.
6. **Step 6: Submission**
   - Submit changes with clear commit message and branch push.
