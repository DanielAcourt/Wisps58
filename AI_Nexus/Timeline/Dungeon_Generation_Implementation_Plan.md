# Sovereign Framework: Procedural Dungeon Generation Implementation & Ticket Breakdown
**Document ID:** `AI_Nexus/Timeline/Dungeon_Generation_Implementation_Plan.md`
**Author:** Sovereign Research & Core Systems Engineering (07)
**Status:** Approved Architecture Plan / Ready for Sprint Staging
**Date:** October 2026

---

## 🏛️ Executive Summary

Following the accepted **Design Gate Specification** (`AI_Nexus/Research/Dungeon_Generator_Design_Gate_Specification.md`), this document formalizes the multi-ticket engineering roadmap for implementing the modular, edge-aware, spine-and-branch procedural dungeon suite.

The plan breaks down the implementation into **4 discrete, testable tickets (B-090 through B-093)** that incrementally deliver the data model, topological generation, edge-aware geometry construction, and validation test suite without feature bloat or subsystem regression.

---

## 🎫 Ticket Breakdown

### Ticket B-090: Procedural Dungeon Graph Data Model, Edge Representation & Validation Engine
- **Complexity:** 5 Points | **Node:** DevOps/C++
- **Context:** The current generator relies on an unstructured random walk of coordinate points without explicit graph edges, passage types, or topological validation.
- **Scope & Deliverables:**
  - Define enums in `SovereignDungeonGenerator.h`: `EDungeonDirection`, `EPassageType` (SolidWall, OpenArchway, StandardDoorway, NarrowCrevice, HiddenSecretWall), `EDoorState` (Open, ClosedUnlocked, LockedKeyRequired, SealedEliteGate, BossVaultSealed).
  - Define value structs:
    - `FSovereignDungeonEdge` (`EdgeId`, `FromRoomIndex`, `ToRoomIndex`, `CellCoordA`, `CellCoordB`, `Direction`, `PassageType`, `DoorState`, `LockTag`).
    - Update `FSovereignDungeonNode` with `RoomIndex`, `AnchorCoord`, `FootprintSize` ($1 \times 1, 2 \times 2, 4 \times 4$), and `ConnectedEdgeIndices`.
  - Implement pure-data BFS validator: `ValidateTopologicalReachability()` asserting:
    1. Critical path from Entrance to Boss exists.
    2. Zero orphan/unreachable rooms.
    3. Path length $\ge \text{MinSpineLength}$.
  - Author unit tests in `WispCPP7VRTests` verifying data structures, edge indexing, and BFS reachability validation independently of world actor spawning.

---

### Ticket B-091: Deterministic Spine-and-Branch Topology Generation Engine
- **Complexity:** 5 Points | **Node:** DevOps/C++
- **Context:** Dungeon rooms must follow a deliberate critical path (Main Spine) with paced encounters and optional dead-end reward branches rather than a wandering random walk.
- **Scope & Deliverables:**
  - Refactor `ASovereignDungeonGenerator::GenerateDungeon()`:
    1. Isolate all random rolls to `FRandomStream(GenerationSeed)`.
    2. Build the **Main Spine**: $R_0 \to \dots \to R_L$, guaranteeing length $\ge \text{MinSpineLength}$.
    3. Mandatory **Elite Gate**: Room $R_{L-1}$ assigned `EliteChamber`, connecting to $R_L$ (`BossChamber`) with edge `PassageType = StandardDoorway` and `DoorState = SealedEliteGate`.
    4. Multi-tile footprint reservations: $1 \times 1$ Corridors, $2 \times 2$ Chambers, $4 \times 4$ Boss Chamber via `MasterGridOccupancy` map with overlap rejection.
    5. Build **Side Branches**: Sprout $1 \le \text{Depth} \le 3$ branches off intermediate spine nodes terminating in `TreasureRoom` dead ends.
  - Author unit tests verifying exact reproducible graph outputs given the same seed, no overlapping cell reservations, and proper room role pacing.

---

### Ticket B-092: Edge-Aware Instanced Geometry Spawner & Strict Lifecycle Cleanup
- **Complexity:** 5 Points | **Node:** DevOps/C++
- **Context:** The generator currently places boundary walls blindly on missing coordinate neighbors, causing accidental openings or wall bleeding, and lacks isolated cleanup of spawned actors.
- **Scope & Deliverables:**
  - Implement `SpawnDungeonGeometry()` using `FSovereignDungeonEdge` data:
    - Instantiates $1000\text{cm} \times 1000\text{cm}$ floor tiles across all cells in `MasterGridOccupancy` into `FloorInstancedComponent`.
    - Boundary walls are placed **only** on cell faces where no connecting edge exists in `DungeonEdges`.
    - Doorway archways / door frames are placed **only** where an edge explicitly crosses between cells.
  - Implement strict `CleanupDungeon()`:
    - Tracks spawned actors in `GeneratedSpawnedActors`, `GeneratorSpawnedArenas`, `GeneratorSpawnedLight`, `GeneratorSpawnedPostProcess`.
    - Destroys strictly owned objects without touching designer-authored lights, skyboxes, or level geometry.
  - Author tests verifying lifecycle idempotency (running generation 5 times without actor leaks).

---

### Ticket B-093: Dungeon Navigation Mesh Integration, World Verification & Editor Ergonomics
- **Complexity:** 5 Points | **Node:** DevOps/C++
- **Context:** Generated 3D dungeons must support real-time navigation mesh generation for dragon vessels, player interaction raycasts, and clear visual debugging in Unreal Editor.
- **Scope & Deliverables:**
  - Ensure dynamic NavMesh generation across procedural instanced floors and doorways (`UNavigationSystemV1`).
  - Add Editor Details panel buttons and debug visualizers (drawing color-coded graph edges in viewport: Green for Spine, Blue for Branches, Red for Sealed Boss Gate).
  - Update `AI_Nexus/Docs/21_Turn_Based_Combat_and_Dungeon_User_Guide.md` and `AI_Nexus/Docs/20_Procedural_Dungeon_Modular_Kit_Specification.md` with complete artist workflows and gameplay tuning guidelines.

---

## 🏛️ Strategic Alignment & Sprint Velocity

| Ticket | Task | Complexity | Status |
|:---|:---|:---:|:---:|
| **B-090** | Procedural Dungeon Graph Data Model, Edge Representation & Validation Engine | 5 | Ready to Implement |
| **B-091** | Deterministic Spine-and-Branch Topology Generation Engine | 5 | Ready to Implement |
| **B-092** | Edge-Aware Instanced Geometry Spawner & Strict Lifecycle Cleanup | 5 | Ready to Implement |
| **B-093** | Dungeon Navigation Mesh Integration, World Verification & Editor Ergonomics | 5 | Backlog |

- **Total Planned Work:** 20 Velocity Points across 4 focused tickets.
- **Subsystem Compatibility:** 100% backward compatible with `USovereignDungeonSubsystem`, `ASovereignDungeonGateway`, `ASovereignDungeonGameMode`, and `USovereignEncounterSubsystem`.
