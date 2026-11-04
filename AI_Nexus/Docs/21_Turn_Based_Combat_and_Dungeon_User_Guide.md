// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

# User Setup & Testing Guide: Turn-Based Combat, ATB Engine, 3x3 Formations & Procedural Dungeons

This guide provides step-by-step instructions for testing and integrating the newly implemented **Turn-Based Combat, Active Time Battle (ATB) Engine, 3x3 Tactical Formations, Meditation Garden Gateway, and Procedural Dungeon Generation Suite** (Tickets B-077 through B-086) in Unreal Engine 5.8+.

---

## 🏛️ System Architecture Summary

The suite is composed of 6 modular pillars:

1. **Party & 3x3 Formation Matrix (`USovereignPartyComponent`)**:
   - Attached to `ASovereignPlayerWisp` (with query accessor on `ASovereignBaseCharacter`).
   - Manages 9 deployment slots (0–2: Front, 3–5: Mid, 6–8: Rear).
   - Enforces milestone rank unlocks (Rank 0 Front unlocked by default; Ranks 1 and 2 unlocked via progression).
   - Persists state into `"Sovereign.Party"` JSON via `ISovereignBrokerInterface`.
2. **Active Time Battle Engine (`USovereignATBComponent`)**:
   - Attached to all character vessels (`ASovereignBaseCharacter`).
   - 0.0 to 100.0% real-time gauge filled based on Dexterity and class speed multipliers.
   - Defaults to **Wait Mode** (pauses meter while browsing submenus or during action animations).
   - Defend stance (`EnterDefendStance`) gives 50% damage reduction without boosting fill rate.
3. **Dual 3x3 Arena Director (`ASovereignBattleManager`)**:
   - Placed or spawned on dedicated battle platforms.
   - Arbitrates turn queues (`ReadyQueue`) prioritizing higher Dexterity with arrival tie-breaking.
   - Manages encounter phases: `EncounterStarting` $\to$ `ActiveCombat` $\to$ `ActionExecuting` $\to$ `Victory` / `Defeat` / `Fled`.
   - Generates default 200u spatial 3D slot anchors (`PlayerSlotTransforms` and `EnemySlotTransforms`).
4. **Qi Combat Pipeline & Item Bridge**:
   - Spells/skills consume Qi from `USovereignQiComponent` without mana clutter; insufficient Qi rejects casting without wasting the turn.
   - Consumable items (`USovereignItemFragment_CombatEffect`) can target friendly allies (healing/Qi pots) or enemies (throwable spirit grenades), consuming 1 turn.
5. **Dynamic Flee System**:
   - Escape roll based on: $50\% + (\text{PartyDEX} - \text{EnemyDEX}) \times 2\% + \text{PlayerLuck} \times 0.5\% + \text{EscapeProgress}$.
   - Failed flee attempts increment the escape bar by $+25\%$ (guaranteeing flight at 100%) and consume the acting dragon's turn.
   - Boss encounters strictly reject flight (`bIsEscapable = false`).
6. **Meditation Garden Gateway & Procedural Dungeon (`ASovereignDungeonGateway`, `ASovereignDungeonGenerator`, `USovereignDungeonSubsystem` & `ASovereignDungeonGameMode`)**:
   - Gateway door holds a sacrifice inventory receptacle and Qi funnel ("Risk it for the biscuit").
   - `EnterDungeon(PartyLeader)` saves the Garden to `"Garden_AutoSave"` JSON via `USaveManager`, caches expedition parameters (seed, buff/reward multipliers, return map name) in `USovereignDungeonSubsystem`, consumes sacrifices, and transitions to the procedural dungeon map via `OpenLevel`.
   - `ASovereignDungeonGameMode` manages the dungeon level: enforces uniform controller bindings, locks the player into their possessed party leader dragon (`bLockPossessionToLeader = true`), spawns/teleports the leader to `DungeonGenerator->GetEntranceTransform()`, and keeps non-leader party members hidden on the battle arena grid until combat begins.
   - Generator ingests expedition parameters on `BeginPlay` and builds modular 1000cm x 1000cm rooms/corridors, placing the terminal Boss Chamber at the furthest point.
   - `SpawnDefaultLightingRig()` ensures that even in completely blank/empty levels without lights, an ambient DirectionalLight and unbounded PostProcessVolume (with auto-exposure min/max bounds) are auto-spawned so the dungeon is never pitch black.
   - Calling `ReturnToGarden()` re-opens the Garden map and restores the exact `"Garden_AutoSave"` JSON world state.
   - Dedicated battle arena platforms are spawned far off-map at `(50000, 50000, 5000)`.

---

## 🧪 1. Running Automated Unit Tests in Unreal Editor

All systems include native automation test suites with zero external dependencies.

### Step 1: Open Unreal Session Frontend
1. In the Unreal Editor toolbar, navigate to:
   `Tools -> Session Frontend` (or `Window -> Developer Tools -> Session Frontend`).
2. Click on the **Automation** tab.

### Step 2: Filter and Run Sovereign Combat Tests
1. In the search filter box, type:
   `Sovereign.Combat` or `Sovereign.Dungeon`
2. You will see the complete test suite:
   - `Sovereign.Combat.ATBComponentOperations`
   - `Sovereign.Combat.BattleManagerOperations`
   - `Sovereign.Combat.DamageCalculationFormula`
   - `Sovereign.Combat.EncounterTransitionOperations`
   - `Sovereign.Combat.FleeSystemOperations`
   - `Sovereign.Combat.ItemBridgeOperations`
   - `Sovereign.Combat.PartyComponentOperations`
   - `Sovereign.Combat.RankMatrixTargetEvaluation`
   - `Sovereign.Combat.SkillPipelineOperations`
   - `Sovereign.Dungeon.GatewayAndGeneratorOperations` (Verifies B-084 gateway, B-087 lighting, B-088 GameMode, B-089 geometry, B-090 graph model & BFS validation, B-092 edge-aware boundary enclosure and cleanup, and B-093 multi-seed determinism fuzzing)
3. Check the root checkbox and click **Start Tests** (green play icon).
4. All tests will execute in milliseconds and display green checkmarks!

---

## 🎮 2. Setting Up the Meditation Garden Gateway Door

To test the physical dungeon threshold in your level:

1. **Place the Actor**:
   - In the Content Browser or Place Actors panel, search for `SovereignDungeonGateway` (or create a Blueprint derived from `ASovereignDungeonGateway`, e.g., `BP_DungeonGateway`).
   - Drag and drop it into your Meditation Garden level near the perimeter edge.
2. **Assign Visual Mesh**:
   - In the Details Panel, assign your gateway door static mesh to `GatewayDoorMesh` (e.g. `SM_Dungeon_Gateway_Door`).
3. **Configure Unlocking**:
   - By default, `bIsUnlocked = false` and `bAutoUnlockOnHatch = true`.
   - To unlock manually in editor for testing, check `bIsUnlocked = true` or call `SetGatewayUnlocked(true)` via Blueprint/Event Graph.
4. **Test Item Sacrifice & Qi Funneling**:
   - Approach the gateway as a player dragon.
   - In Blueprint, call:
     ```blueprint
     Gateway->ChannelQiFromCharacter(PlayerDragon, 50.0);
     ```
   - In the Details Panel, observe `InfusedQi` incrementing, and call `CalculateEnemyBuffMultiplier()` and `CalculateRewardMultiplier()` to see the risk-vs-reward buffs update live!
5. **Triggering "Enter Dungeon" & Level Transition**:
   - In the Details Panel of `ASovereignDungeonGateway`, configure:
     - `DungeonLevelName`: Name of your target dungeon map (default: `Lvl_Dungeon`).
     - `GardenSaveSlot`: Name of save slot for the garden (default: `Garden_AutoSave`).
     - `ExpeditionSeed`: Generation seed (set to 0 for random seed).
   - In Blueprint or from interaction, call:
     ```blueprint
     Gateway->EnterDungeon(PlayerDragon);
     ```
   - **What happens under the hood**:
     1. Verifies `bIsUnlocked == true`.
     2. Calls `USaveManager->SaveWorldState("Garden_AutoSave", true)` to persist all garden entities, dragons, eggs, and items into JSON.
     3. Records expedition parameters in `USovereignDungeonSubsystem` (`ActiveExpeditionSeed`, `ActiveEnemyBuffMultiplier`, `ActiveRewardMultiplier`, `GardenReturnMapName`).
     4. Consumes sacrificed items and infused Qi.
     5. Calls `UGameplayStatics::OpenLevel(World, DungeonLevelName)`.

---

## 🗺️ 3. Testing the Procedural Dungeon Generator

1. **Spawn or Place Generator**:
   - Place an instance of `ASovereignDungeonGenerator` into your dungeon level (or spawn it via Blueprint).
2. **Configure GameMode & Parameters in Dungeon Map**:
   - In your dungeon map (`Lvl_Dungeon`), open **World Settings** and set **GameMode Override** to `ASovereignDungeonGameMode` (or a Blueprint child `BP_DungeonGameMode`).
   - `TargetRoomCount = 8` (or 12 for deeper vaults).
   - `bLockPossessionToLeader = true`: Prohibits unpossessing into wisp form while inside the dungeon.
   - `StepThresholdDistance = 1500.0` (triggers random encounter evaluation every ~15 steps).
   - `RandomEncounterChance = 25.0` (25% chance per check).
   - `FarOffArenaOrigin = (50000, 50000, 5000)` (keeps the battle platform isolated).
3. **Generate Layout, Physical 3D Geometry & Optional Lighting Rig**:
   - On `BeginPlay()`, the generator automatically executes `GenerateDungeon()`.
   - **Graph Connectivity & BFS Validation (Ticket B-090)**:
     - Authoritative connectivity is managed through `DungeonEdges` (`FSovereignDungeonEdge`).
     - Pure-data BFS validation (`ValidateTopologicalReachability`) asserts 100% room reachability, boss reachability, minimum spine length (`MinSpineLength`), and sealed elite gate thresholds before world actor construction.
     - Fast rendering lookup bitmask `ConnectedPassageMask` on `FSovereignDungeonNode` is automatically derived from `DungeonEdges`.
   - **Physical 3D Room & Wall Spawning (Ticket B-089 & B-092)**:
     - `bSpawnPhysicalGeometry = true`: Automatically instantiates 3D floor tiles ($1000\text{cm} \times 1000\text{cm}$) and perimeter boundary walls using `UInstancedStaticMeshComponent` (rendering hundreds of room tiles in a single draw call).
     - In the Details panel of `ASovereignDungeonGenerator`, you can assign custom modular meshes to `FloorMesh`, `WallMesh`, and `BossAltarMesh` (defaults to `/Engine/BasicShapes/Cube` scaled to metric dimensions).
     - **Edge-Aware Enclosure (Ticket B-092)**: Boundary walls spawn strictly on cell faces that have no connecting edge in `DungeonEdges`, ensuring that physically adjacent rooms that are disconnected in the graph have solid separating walls between them.
     - **Strict Lifecycle Cleanup (Ticket B-092)**: Calling `CleanupDungeon()` clears instanced meshes and destroys only generator-owned actors without touching designer-placed lighting, skyboxes, or level geometry.
   - **Custom SkyLight / Level Lighting**: If your dungeon level template already defines custom SkyLight, atmospheric lighting, or torches, you can simply uncheck `bAutoSpawnDefaultLighting = false` in the Details panel of `ASovereignDungeonGenerator`.
   - `bAutoSpawnDefaultLighting = true` (Default): If loading into a blank or unlit level, the generator auto-spawns fallback ambient lighting (`ADirectionalLight`) and an unbounded `APostProcessVolume` with clamped exposure (0.5 to 2.0 EV100) so the procedural corridors are never pitch black.
   - Individual sub-toggles `bSpawnDirectionalLight` and `bSpawnPostProcessVolume` are also provided for granular control.
   - You can inspect `DungeonNodes` array in the Details panel:
     - `DungeonNodes[0]` is always `EntranceRoom` at `(0, 0)`.
     - `DungeonNodes[BossNodeIndex]` is the terminal `BossChamber` at the furthest coordinates with `bHasFixedEliteMob = true`.
   - You can query `GetEntranceTransform()` to spawn or teleport the player party to the dungeon entrance.
4. **Step-Based Encounter Movement**:
   - In your character movement loop or pawn Tick, call:
     ```blueprint
     DungeonGenerator->ProcessPlayerMovement(PlayerCharacter, MovementDeltaDistance);
     ```
   - Walking through corridors accumulates distance; when it passes `StepThresholdDistance`, it rolls for a sudden Pokemon/FF7-style battle transition!
5. **Returning to the Garden & Restoring Saved State**:
   - When the expedition ends (boss defeated or party flees), call:
     ```blueprint
     DungeonGenerator->ReturnToGarden();
     // or directly:
     GetGameInstance()->GetSubsystem<USovereignDungeonSubsystem>()->ReturnToGarden(WorldContextObject);
     ```
   - This transitions the level back to the saved garden map and restores the exact `"Garden_AutoSave"` state via `USaveManager->LoadWorldState("Garden_AutoSave", true)`.

---

## ⚔️ 4. Testing Turn-Based Combat & The Dedicated Battle Arena

### Starting an Encounter Manually
To test a battle without walking through a dungeon, you can trigger one directly in Blueprint:

1. Spawn 1 to 3 player dragons and place them in the party:
   ```blueprint
   PartyComp->AssignPartyMember(0, PlayerDragon, true); // Slot 0 = Front Rank Leader
   ```
2. Spawn 1 to 3 enemy creatures.
3. Call `USovereignEncounterSubsystem`:
   ```blueprint
   GetWorld()->GetSubsystem<USovereignEncounterSubsystem>()->TriggerEncounter(
       PlayerDragon,   // Leader
       nullptr,        // Overlapped Mob (null for direct testing)
       EnemyUnits,     // Array of enemy characters
       true            // bCanFlee
   );
   ```
4. **What Happens Automatically**:
   - The party and enemies are teleported to the dedicated 3D arena platform (`FarOffArenaOrigin`).
   - The battle manager positions each combatant at their respective 3x3 slot transform (Front rank in front, Mid rank behind, Rear rank support).
   - All combatants' `USovereignATBComponent` meters begin ticking and filling in real-time based on their Dexterity!

### Testing In-Battle Commands
When a dragon's ATB reaches 100%:
- **Execute Skill / Magic**:
  ```blueprint
  BattleManager->ExecuteSkillAction(ActingDragon, SkillInstance, TargetUnits);
  ```
  - Deducts Qi from `USovereignQiComponent`.
  - Calculates percentage-mitigated damage using `USovereignCombatCalculationLibrary`.
  - Clears Defend stance and resets the dragon's ATB to 0%.
- **Use Inventory Item**:
  ```blueprint
  BattleManager->ExecuteItemAction(ActingDragon, ItemInstanceId, TargetCharacter);
  ```
  - Consumes 1 potion or grenade from the acting dragon's `USovereignInventoryComponent`.
  - Heals ally or damages enemy; resets ATB to 0%.
- **Defend**:
  ```blueprint
  ActingDragon->GetATBComponent()->EnterDefendStance();
  ```
  - Grants 50% damage reduction against incoming attacks until next action.
- **Flee / Run**:
  ```blueprint
  BattleManager->AttemptFlee(ActingDragon);
  ```
  - Rolls DEX/Luck escape chance.
  - If failed: increments escape meter by $+25\%$ and consumes the dragon's turn.
  - If successful: transitions state to `Fled` and `USovereignEncounterSubsystem` teleports the party back to the dungeon floor!
