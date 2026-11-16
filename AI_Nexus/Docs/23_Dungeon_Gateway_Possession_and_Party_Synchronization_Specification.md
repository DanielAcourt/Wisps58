// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# Sovereign Architectural Specification: Gateway Possession Verification & Party Progression Synchronization

**Document ID:** SPEC-B121-B123
**Sprint Tracking:** B-121, B-122, B-123
**Status:** Approved & Implemented
**Author:** 07 Agent (Software Engineer)

---

## 1. Overview & Problem Statement

In the Sovereign Framework, player exploration flows between two distinct world modes:
1. **The Meditation Garden:** The non-combat simulation world where creatures (Dragons, Erisis, Beasts) live, hatch, mature, train cultivation, and interact. The player exists as a spiritual entity (`ASovereignPlayerWisp`) capable of possessing physical vessels (`ASovereignBaseCharacter`) and organizing a 3x3 tactical battle formation (`USovereignPartyComponent`).
2. **Procedural Trial Dungeons:** Real-time corridor traversal with turn-based / ATB encounters executed on dedicated 3D battle arenas (`ASovereignBattleManager`).

Prior to Tickets B-121–B-123, transition across the Meditation Garden Gateway (`ASovereignDungeonGateway`) exhibited three critical architectural gaps:
- **Unpossessed Wisp Leak:** If the player interacted with the Gateway as an unpossessed spirit wisp, the system attempted to enter the dungeon without a physical vessel, creating a situation where an entity with no physical form, attributes, or combat capabilities wandered into combat.
- **Roster Truncation:** While the possessed vessel acting as Party Leader crossed over into the dungeon, the remaining 8 party members configured in the 3x3 battle grid were omitted from cross-level persistence, leaving combat slots empty on the Battle Arena stage.
- **One-Way Progression Amnesia:** Progress earned within the dungeon (leveled combat jobs, gained XP, acquired loot/items, consumed vitals) was discarded upon returning to the Garden, because `ReturnToGarden` simply reloaded the pre-expedition auto-save without synchronizing the modified runtime states of the returning vessels.

---

## 2. Engineering Architecture & Core Tenets

```
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                            MEDITATION GARDEN                                │
 │                                                                             │
 │    ASovereignPlayerWisp ──[Possesses]──► ASovereignBaseCharacter (Leader)   │
 │                                                  │                          │
 │                                          USovereignPartyComponent           │
 │                                          (Slots 0..8: Roster)               │
 └──────────────────────────────────────┬──────────────────────────────────────┘
                                        │
                         [ASovereignDungeonGateway]
                         - If Unpossessed Wisp:
                             * Block EnterDungeon
                             * Broadcast OnUnpossessedWispInteraction
                             * Trigger Inspection UI
                         - If Possessed Vessel:
                             * Save Garden to Garden_AutoSave
                             * Record Expedition Seed & Multipliers
                             * Serialize Leader State JSON
                             * Serialize All Party Member States (B-122)
                             * Consume Sacrifices -> OpenLevel
                                        │
                                        ▼
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         USovereignDungeonSubsystem                          │
 │                     (Survives OpenLevel in GameInstance)                    │
 │                                                                             │
 │  - PartyLeaderClassPath / PartyLeaderGUID / PartyLeaderStateJson            │
 │  - PartyMembersData: Array<FSovereignPartyMemberData>                       │
 └──────────────────────────────────────┬──────────────────────────────────────┘
                                        │
                                        ▼
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         PROCEDURAL DUNGEON MAP                              │
 │                                                                             │
 │  [ASovereignDungeonGameMode]                                                │
 │  - Dynamic Pawn Spawn: Instantiates Leader Vessel Class                     │
 │  - SetupPartyLeaderAtEntrance:                                              │
 │      * Restores Leader JSON (Attrs, Qi, Bio, Class, Skill, Inv, Visuals)    │
 │      * Spawns Non-Leader Party Members (B-122)                              │
 │      * Restores Party Members' Full State JSONs                             │
 │      * Pre-anchors Non-Leaders to ASovereignBattleManager Player Slots      │
 │      * Hides Non-Leaders (Single Roamer: FF7 Cloud Strife paradigm)         │
 └──────────────────────────────────────┬──────────────────────────────────────┘
                                        │
                         [ReturnToGarden] (B-123)
                         - ASovereignBossTeleporter / ASovereignDungeonEntrance
                         - Captures final runtime JSON of Leader & Party
                         - Merges deltas directly into Garden_AutoSave JSON
                         - Transitions level back to Garden -> Symmetry Restored!
```

---

## 3. Detailed Component Specifications

### 3.1 Ticket B-121: Gateway Possession Gate & UI Dispatcher
- **Target Actor:** `ASovereignDungeonGateway`
- **Rule:** A player interacting with the Gateway MUST be possessing a physical vessel (`ASovereignBaseCharacter`) to enter the dungeon.
- **Unpossessed Wisp Behavior:**
  - If interactor is `ASovereignPlayerWisp` with `!Wisp->IsPossessing()`:
    - Blocks `EnterDungeon()` immediately (returns `false`).
    - Broadcasts delegate `OnUnpossessedWispInteraction(ASovereignPlayerWisp* Wisp)`.
    - Returns dynamic hint string: `"Vessel Required: Inspect Dungeon Gateway"`.
    - Does NOT consume infused Qi or item sacrifices.

### 3.2 Ticket B-122: Full 3x3 Party Roster Reconstitution
- **Target Struct:** `FSovereignPartyMemberData`
  - `int32 SlotIndex`
  - `FGuid MemberGuid`
  - `FString ClassPath`
  - `FString StateJson`
  - `bool bIsLeader`
- **Subsystem Extension:** `USovereignDungeonSubsystem`
  - `TArray<FSovereignPartyMemberData> ActivePartyMembersData`
  - `RecordPartyMemberState(int32 SlotIndex, const FString& ClassPath, const FGuid& Guid, const FString& StateJson, bool bIsLeader)`
  - `ClearPartyMembersState()`
- **GameMode Reconstitution:** `ASovereignDungeonGameMode::SetupPartyLeaderAtEntrance`
  - For each non-leader member in `ActivePartyMembersData`:
    - Spawns the vessel actor via `SpawnActor<ASovereignBaseCharacter>`.
    - Restores full entity state JSON via `USovereignSaveableEntityComponent::ApplyStateFromJsonObject`.
    - Anchors the member transform to `ASovereignBattleManager::PlayerSlotTransforms[SlotIndex]`.
    - Calls `Member->SetActorHiddenInGame(true)` and `Member->SetActorEnableCollision(false)`.
    - Assigns the restored member pointer back into `USovereignPartyComponent::GridSlots[SlotIndex]`.

### 3.3 Ticket B-123: Bidirectional Progression Synchronization
- **Target Function:** `USovereignDungeonSubsystem::ReturnToGarden` and `USaveManager`
- **Data Flow:**
  - Before invoking `UGameplayStatics::OpenLevel(World, TargetMap)`:
    - Queries the active dungeon party leader and all active party members in the dungeon world.
    - Captures updated runtime entity state JSONs (`CaptureFullEntityState()`).
    - Ingests the updated state JSON into the existing Garden save file (`GardenSaveSlot` JSON) using `USaveManager::UpdateEntityInSaveJson(Guid, StateJson)` or merging into the saved suitcase.
    - When the Garden level opens and `LoadWorldState(GardenSaveSlot)` executes, the restored vessels in the Garden hold the exact XP, levels, HP/Qi/Stamina, inventory items, and growth states achieved in the dungeon.
