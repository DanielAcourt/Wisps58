// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Entities/SovereignDungeonGateway.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "CoreModules/SovereignDungeonGameMode.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignQiComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN DUNGEON GATEWAY & PROCEDURAL GENERATOR TESTS
 * ============================================================================
 * Verifies gateway unlocking, item sacrifice receptacle, Qi channeling,
 * risk-vs-reward formulas, procedural dungeon graph generation, and boss chambers.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignDungeonGatewayTest,
	"Sovereign.Dungeon.GatewayAndGeneratorOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignDungeonGatewayTest::RunTest(const FString& Parameters)
{
	// 1. Test Gateway Door State and Unlocking
	ASovereignDungeonGateway* Gateway = NewObject<ASovereignDungeonGateway>();
	if (!Gateway)
	{
		AddError(TEXT("Failed to instantiate ASovereignDungeonGateway"));
		return false;
	}

	TestFalse(TEXT("Gateway starts locked"), Gateway->bIsUnlocked);
	TestTrue(TEXT("bAutoUnlockOnHatch defaults to true"), Gateway->bAutoUnlockOnHatch);

	Gateway->SetGatewayUnlocked(true);
	TestTrue(TEXT("Gateway is now unlocked"), Gateway->bIsUnlocked);

	// 2. Test Qi Channeling Funnel
	ASovereignBaseCharacter* Cultivator = NewObject<ASovereignBaseCharacter>();
	if (USovereignQiComponent* QiComp = Cultivator->GetQiComponent())
	{
		QiComp->MaxQiCapacity = 200.0f;
		QiComp->CurrentQi = 100.0f;
	}

	const bool bChannelSuccess = Gateway->ChannelQiFromCharacter(Cultivator, 50.0f);
	TestTrue(TEXT("ChannelQiFromCharacter should succeed"), bChannelSuccess);
	TestEqual(TEXT("Gateway InfusedQi is now 50.0"), Gateway->InfusedQi, 50.0f);
	TestEqual(TEXT("Cultivator Qi is now 50.0"), Cultivator->GetQiComponent()->CurrentQi, 50.0f);

	// 3. Test Item Sacrifice Receptacle & Multipliers
	USovereignItemDefinition* HeavyOre = NewObject<USovereignItemDefinition>();
	USovereignItemFragment_Weight* WeightFrag = NewObject<USovereignItemFragment_Weight>(HeavyOre);
	WeightFrag->UnitWeight = 5.0f; // 5kg per ore
	HeavyOre->Fragments.Add(WeightFrag);

	// Add 4 ores = 20kg mass
	Gateway->SacrificeInventory->AddItem(HeavyOre, 4);
	TestEqual(TEXT("Total sacrificed mass should be 20.0kg"), Gateway->GetTotalSacrificedMass(), 20.0f);

	// Formula Check:
	// EnemyBuff = 1.0 + (20 * 0.05) + (50 * 0.002) = 1.0 + 1.0 + 0.10 = 2.10x
	// RewardMult = 1.0 + (20 * 0.10) + (50 * 0.005) = 1.0 + 2.0 + 0.25 = 3.25x
	TestEqual(TEXT("EnemyBuffMultiplier should be 2.10x"), Gateway->CalculateEnemyBuffMultiplier(), 2.10f);
	TestEqual(TEXT("RewardMultiplier should be 3.25x"), Gateway->CalculateRewardMultiplier(), 3.25f);

	// 4. Test Procedural Dungeon Generator Graph Assembly
	ASovereignDungeonGenerator* Generator = NewObject<ASovereignDungeonGenerator>();
	if (!Generator)
	{
		AddError(TEXT("Failed to instantiate ASovereignDungeonGenerator"));
		return false;
	}

	Generator->TargetRoomCount = 6;
	Generator->GenerationSeed = 42;
	Generator->MaxLoopCount = 0; // Spanning tree mode for baseline tests
	TestTrue(TEXT("bWallMeshCenterPivot defaults to true"), Generator->bWallMeshCenterPivot);
	TestFalse(TEXT("bProceduralArchwayFallback defaults to false"), Generator->bProceduralArchwayFallback);
	TestTrue(TEXT("bAutoSpawnDefaultLighting defaults to true"), Generator->bAutoSpawnDefaultLighting);
	TestTrue(TEXT("bSpawnDirectionalLight defaults to true"), Generator->bSpawnDirectionalLight);
	TestTrue(TEXT("bSpawnPostProcessVolume defaults to true"), Generator->bSpawnPostProcessVolume);
	TestTrue(TEXT("bSpawnPhysicalGeometry defaults to true"), Generator->bSpawnPhysicalGeometry);

	// Toggle lighting off to test optional override when level defines custom SkyLight
	Generator->bAutoSpawnDefaultLighting = false;
	const bool bGenerated = Generator->GenerateDungeon();

	TestTrue(TEXT("GenerateDungeon should succeed"), bGenerated);
	TestEqual(TEXT("Total generated nodes should equal TargetRoomCount"), Generator->DungeonNodes.Num(), 6);

	// Verify Ticket B-089, B-094 & B-095 3D geometry & multi-cell footprint spawning
	TestNotNull(TEXT("FloorInstancedComponent exists"), Generator->FloorInstancedComponent.Get());
	TestNotNull(TEXT("WallInstancedComponent exists"), Generator->WallInstancedComponent.Get());
	TestNotNull(TEXT("DoorwayInstancedComponent exists"), Generator->DoorwayInstancedComponent.Get());
	TestTrue(TEXT("Floor instances match or exceed node count due to multi-cell footprints"),
		Generator->FloorInstancedComponent ? Generator->FloorInstancedComponent->GetInstanceCount() >= 6 : false);
	TestTrue(TEXT("Boundary wall instances generated for perimeter edges"), Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() > 0 : false);

	// 5. Test Ticket B-094 Multi-Cell Footprint Reservation Engine
	TestEqual(TEXT("Default footprint for Entrance is 1x1"), ASovereignDungeonGenerator::GetDefaultFootprintForTileType(EDungeonTileType::EntranceRoom), FIntPoint(1, 1));
	TestEqual(TEXT("Default footprint for Corridor is 1x1"), ASovereignDungeonGenerator::GetDefaultFootprintForTileType(EDungeonTileType::Corridor), FIntPoint(1, 1));
	TestEqual(TEXT("Default footprint for Treasure is 1x1"), ASovereignDungeonGenerator::GetDefaultFootprintForTileType(EDungeonTileType::TreasureRoom), FIntPoint(1, 1));
	TestEqual(TEXT("Default footprint for EliteChamber is 2x2"), ASovereignDungeonGenerator::GetDefaultFootprintForTileType(EDungeonTileType::EliteChamber), FIntPoint(2, 2));
	TestEqual(TEXT("Default footprint for BossChamber is 4x4"), ASovereignDungeonGenerator::GetDefaultFootprintForTileType(EDungeonTileType::BossChamber), FIntPoint(4, 4));

	// Test GetOccupiedCells cell counts
	FSovereignDungeonNode CellTestNode;
	CellTestNode.GridCoord = FIntPoint(5, 5);
	CellTestNode.FootprintDimensions = FIntPoint(1, 1);
	CellTestNode.GeneratePerimeterSockets();
	TestEqual(TEXT("1x1 footprint has 4 perimeter sockets"), CellTestNode.PerimeterSockets.Num(), 4);

	CellTestNode.FootprintDimensions = FIntPoint(2, 2);
	CellTestNode.GeneratePerimeterSockets();
	TestEqual(TEXT("2x2 footprint has 4 occupied cells"), CellTestNode.GetOccupiedCells().Num(), 4);
	TestEqual(TEXT("2x2 footprint has 8 perimeter sockets"), CellTestNode.PerimeterSockets.Num(), 8);

	CellTestNode.FootprintDimensions = FIntPoint(4, 4);
	CellTestNode.GeneratePerimeterSockets();
	TestEqual(TEXT("4x4 footprint has 16 occupied cells"), CellTestNode.GetOccupiedCells().Num(), 16);
	TestEqual(TEXT("4x4 footprint has 16 perimeter sockets"), CellTestNode.PerimeterSockets.Num(), 16);

	// Test Ticket B-094.2 MaxConnections capacity checks
	CellTestNode.MaxConnections = 2;
	TestTrue(TEXT("Initially can accept connections"), CellTestNode.CanAcceptConnection());
	CellTestNode.PerimeterSockets[0].bIsConnected = true;
	CellTestNode.PerimeterSockets[1].bIsConnected = true;
	TestFalse(TEXT("Cannot accept connection once MaxConnections reached"), CellTestNode.CanAcceptConnection());

	// Verify connected sockets in generated dungeon
	for (const FSovereignDungeonNode& Node : Generator->DungeonNodes)
	{
		TestTrue(TEXT("Room node perimeter sockets populated"), Node.PerimeterSockets.Num() >= 4);
		TestTrue(TEXT("Node connected sockets within MaxConnections"), Node.GetConnectedSocketCount() <= Node.MaxConnections);
	}

	// Verify zero spatial overlap in MasterGridOccupancy
	TSet<FIntPoint> VerifyUniqueCells;
	for (const auto& Pair : Generator->MasterGridOccupancy)
	{
		TestFalse(TEXT("MasterGridOccupancy cell is unique without overlap"), VerifyUniqueCells.Contains(Pair.Key));
		VerifyUniqueCells.Add(Pair.Key);
	}
	TestEqual(TEXT("Floor instances match total occupied cell count"),
		Generator->FloorInstancedComponent ? Generator->FloorInstancedComponent->GetInstanceCount() : 0, Generator->MasterGridOccupancy.Num());

	// 5. Test Ticket B-090 & B-096 Graph Data Model, Edge Representation & BFS Validation
	TestTrue(TEXT("DungeonEdges populated"), Generator->DungeonEdges.Num() > 0);
	TestEqual(TEXT("Spanning tree edge count invariant |E| == |V| - 1 when MaxLoopCount == 0"), Generator->DungeonEdges.Num(), Generator->DungeonNodes.Num() - 1);

	// Validate topological reachability via pure BFS
	const bool bGraphValid = Generator->ValidateTopologicalReachability(0, Generator->BossNodeIndex);
	TestTrue(TEXT("ValidateTopologicalReachability should succeed on generated graph"), bGraphValid);

	// Verify Node 0 (Entrance Room)
	TestEqual(TEXT("Node 0 is EntranceRoom"), Generator->DungeonNodes[0].TileType, EDungeonTileType::EntranceRoom);
	TestEqual(TEXT("Node 0 grid coord is (0, 0)"), Generator->DungeonNodes[0].GridCoord, FIntPoint(0, 0));

	// Verify Terminal Boss Chamber
	TestTrue(TEXT("BossNodeIndex is valid"), Generator->BossNodeIndex >= 0);
	TestEqual(TEXT("Terminal node is BossChamber"), Generator->DungeonNodes[Generator->BossNodeIndex].TileType, EDungeonTileType::BossChamber);
	TestTrue(TEXT("Boss chamber has fixed elite mob"), Generator->DungeonNodes[Generator->BossNodeIndex].bHasFixedEliteMob);

	// Verify Edge Adjacency Helper methods
	FSovereignDungeonEdge SampleEdge = Generator->DungeonEdges[0];
	TestTrue(TEXT("Edge connects Node A"), SampleEdge.Connects(SampleEdge.NodeIndexA));
	TestTrue(TEXT("Edge connects Node B"), SampleEdge.Connects(SampleEdge.NodeIndexB));
	TestEqual(TEXT("GetOtherNode returns Node B from Node A"), SampleEdge.GetOtherNode(SampleEdge.NodeIndexA), SampleEdge.NodeIndexB);
	TestEqual(TEXT("GetOtherNode returns Node A from Node B"), SampleEdge.GetOtherNode(SampleEdge.NodeIndexB), SampleEdge.NodeIndexA);

	// Verify ConnectedPassageMask cache derived on nodes
	for (const FSovereignDungeonNode& Node : Generator->DungeonNodes)
	{
		TestTrue(TEXT("Node has non-zero connected passage mask"), Node.ConnectedPassageMask > 0);
	}

	// 6. Test Ticket B-095 Multi-Cell Perimeter Boundary & Edge-Aware Snapping Invariants
	// Invariant: Total wall instances strictly equals the sum of unconnected perimeter sockets across all nodes
	// Invariant: Zero internal walls inside multi-cell rooms (only perimeter sockets spawn walls)
	int32 TotalUnconnectedSockets = 0;
	int32 TotalConnectedSockets = 0;
	for (const FSovereignDungeonNode& Node : Generator->DungeonNodes)
	{
		for (const FSovereignDoorwaySocket& Socket : Node.PerimeterSockets)
		{
			if (!Socket.bIsConnected)
			{
				TotalUnconnectedSockets++;
			}
			else
			{
				TotalConnectedSockets++;
			}
		}
	}
	TestEqual(TEXT("Wall instances strictly equal unconnected perimeter socket count (zero interior walls)"),
		Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() : 0, TotalUnconnectedSockets);
	// Option A: DoorwayMesh is default null in testing, so connected faces remain open cutouts (0 instances)
	TestEqual(TEXT("Doorway instances is 0 when DoorwayMesh is null (Option A open passage)"),
		Generator->DoorwayInstancedComponent ? Generator->DoorwayInstancedComponent->GetInstanceCount() : 0, 0);

	// Verify uniform wall instance scale (Thickness = 0.3, Width = 10.0, Height = 6.0) across all instances
	if (Generator->WallInstancedComponent && Generator->WallInstancedComponent->GetInstanceCount() > 0)
	{
		FTransform FirstWallTransform;
		Generator->WallInstancedComponent->GetInstanceTransform(0, FirstWallTransform, true);
		const FVector WallScale = FirstWallTransform.GetScale3D();
		TestEqual(TEXT("Wall scale X is 0.3 (thickness)"), WallScale.X, 0.3f);
		TestEqual(TEXT("Wall scale Y is 10.0 (1000cm width)"), WallScale.Y, 10.0f);
		TestEqual(TEXT("Wall scale Z is 6.0 (600cm height)"), WallScale.Z, 6.0f);
	}

	// Test Procedural Archway Fallback
	Generator->bProceduralArchwayFallback = true;
	Generator->SpawnDungeonGeometry();
	const int32 ExpectedArchwayWallCount = TotalUnconnectedSockets + (TotalConnectedSockets * 3);
	TestEqual(TEXT("Wall instances include 3-part archway posts and lintel when bProceduralArchwayFallback is true"),
		Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() : 0, ExpectedArchwayWallCount);

	// Restore Option A
	Generator->bProceduralArchwayFallback = false;
	Generator->SpawnDungeonGeometry();

	// Test CleanupDungeon idempotency (running multiple times clears cleanly)
	Generator->CleanupDungeon();
	TestEqual(TEXT("Nodes cleared after cleanup"), Generator->DungeonNodes.Num(), 0);
	TestEqual(TEXT("Edges cleared after cleanup"), Generator->DungeonEdges.Num(), 0);
	TestEqual(TEXT("Floor instances cleared after cleanup"), Generator->FloorInstancedComponent ? Generator->FloorInstancedComponent->GetInstanceCount() : 0, 0);
	TestEqual(TEXT("Wall instances cleared after cleanup"), Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() : 0, 0);
	TestEqual(TEXT("Doorway instances cleared after cleanup"), Generator->DoorwayInstancedComponent ? Generator->DoorwayInstancedComponent->GetInstanceCount() : 0, 0);

	// Re-run generation to ensure clean rebuild
	const bool bRebuilt = Generator->GenerateDungeon();
	TestTrue(TEXT("GenerateDungeon succeeds after cleanup"), bRebuilt);

	// 5. Test Random Step Counter
	Generator->AccumulatedStepDistance = 0.0f;
	Generator->StepThresholdDistance = 1000.0f;

	// Move 500 units: below threshold
	Generator->ProcessPlayerMovement(Cultivator, 500.0f);
	TestEqual(TEXT("Accumulated distance is 500.0"), Generator->AccumulatedStepDistance, 500.0f);

	// Move another 600 units: reaches 1100 -> triggers check and resets accumulator to 0
	Generator->ProcessPlayerMovement(Cultivator, 600.0f);
	TestEqual(TEXT("Accumulated distance reset to 0.0 after threshold check"), Generator->AccumulatedStepDistance, 0.0f);

	// 6. Test EnterDungeon Flow: Rejection when locked vs execution when unlocked
	ASovereignDungeonGateway* EnterGateway = NewObject<ASovereignDungeonGateway>();
	EnterGateway->bIsUnlocked = false;
	EnterGateway->DungeonLevelName = FName(TEXT("Lvl_Dungeon"));
	EnterGateway->GardenSaveSlot = TEXT("Garden_AutoSave");
	EnterGateway->ExpeditionSeed = 8888;

	// Must fail if locked
	const bool bLockedEnter = EnterGateway->EnterDungeon(Cultivator);
	TestFalse(TEXT("EnterDungeon should fail when gateway is locked"), bLockedEnter);

	// Sacrifice some mass & Qi
	EnterGateway->SacrificeInventory->AddItem(HeavyOre, 2); // 10kg
	EnterGateway->InfusedQi = 20.0f;
	EnterGateway->SetGatewayUnlocked(true);

	const bool bUnlockedEnter = EnterGateway->EnterDungeon(Cultivator);
	TestTrue(TEXT("EnterDungeon should succeed when unlocked"), bUnlockedEnter);
	// Sacrifices should have been consumed
	TestEqual(TEXT("Sacrifices consumed after entering dungeon"), EnterGateway->SacrificeInventory->InventorySlots.Num(), 0);
	TestEqual(TEXT("InfusedQi reset to 0.0 after entering dungeon"), EnterGateway->InfusedQi, 0.0f);

	// 7. Test SovereignDungeonSubsystem Persistent Expedition State
	// UGameInstanceSubsystem requires a UGameInstance outer to satisfy ClassWithin = GameInstance
	UWorld* TestWorld = nullptr;
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		TestWorld = GEngine->GetWorldContexts()[0].World();
	}
	UGameInstance* GI = TestWorld ? TestWorld->GetGameInstance() : nullptr;
	if (!GI)
	{
		GI = NewObject<UGameInstance>();
	}

	USovereignDungeonSubsystem* DungeonSubsystem = GI->GetSubsystem<USovereignDungeonSubsystem>();
	if (!DungeonSubsystem)
	{
		DungeonSubsystem = NewObject<USovereignDungeonSubsystem>(GI);
	}

	if (TestNotNull(TEXT("DungeonSubsystem created with GameInstance outer"), DungeonSubsystem))
	{
		DungeonSubsystem->ClearExpedition();
		TestFalse(TEXT("Expedition starts inactive"), DungeonSubsystem->bHasActiveExpedition);

		DungeonSubsystem->RecordExpedition(777, 2.5f, 3.0f, FName(TEXT("Garden_Level")), TEXT("Garden_AutoSave"));
		TestTrue(TEXT("Expedition is now active"), DungeonSubsystem->bHasActiveExpedition);
		TestEqual(TEXT("Expedition seed is 777"), DungeonSubsystem->ActiveExpeditionSeed, 777);
		TestEqual(TEXT("Enemy buff multiplier is 2.5"), DungeonSubsystem->ActiveEnemyBuffMultiplier, 2.5f);
		TestEqual(TEXT("Reward multiplier is 3.0"), DungeonSubsystem->ActiveRewardMultiplier, 3.0f);
		TestEqual(TEXT("Return map name recorded"), DungeonSubsystem->GardenReturnMapName, FName(TEXT("Garden_Level")));

		DungeonSubsystem->ClearExpedition();
		TestFalse(TEXT("Expedition cleared"), DungeonSubsystem->bHasActiveExpedition);
		TestEqual(TEXT("Enemy buff reset to 1.0"), DungeonSubsystem->ActiveEnemyBuffMultiplier, 1.0f);
	}

	// 8. Test SovereignDungeonGameMode Configuration & Entrance Alignment
	ASovereignDungeonGameMode* DungeonGM = NewObject<ASovereignDungeonGameMode>();
	if (TestNotNull(TEXT("DungeonGM created"), DungeonGM))
	{
		TestTrue(TEXT("bLockPossessionToLeader defaults to true"), DungeonGM->bLockPossessionToLeader);
		TestTrue(TEXT("DefaultPawnClass is ASovereignBaseCharacter"), DungeonGM->DefaultPawnClass == ASovereignBaseCharacter::StaticClass());

		DungeonGM->ActiveDungeonGenerator = Generator;
		DungeonGM->DungeonPartyLeader = Cultivator;
		DungeonGM->ConfigurePartyForExploration();
		TestNotNull(TEXT("DungeonPartyLeader registered"), DungeonGM->DungeonPartyLeader.Get());
	}

	// 9. Test Ticket B-093 Multi-Seed Determinism & Invariant Fuzzing
	ASovereignDungeonGenerator* DetGenA = NewObject<ASovereignDungeonGenerator>();
	DetGenA->TargetRoomCount = 8;
	DetGenA->GenerationSeed = 1337;
	DetGenA->bSpawnPhysicalGeometry = false;
	DetGenA->bAutoSpawnDefaultLighting = false;
	DetGenA->GenerateDungeon();

	ASovereignDungeonGenerator* DetGenB = NewObject<ASovereignDungeonGenerator>();
	DetGenB->TargetRoomCount = 8;
	DetGenB->GenerationSeed = 1337;
	DetGenB->bSpawnPhysicalGeometry = false;
	DetGenB->bAutoSpawnDefaultLighting = false;
	DetGenB->GenerateDungeon();

	// Identical Seed Determinism check
	TestEqual(TEXT("Deterministic node counts match"), DetGenA->DungeonNodes.Num(), DetGenB->DungeonNodes.Num());
	TestEqual(TEXT("Deterministic edge counts match"), DetGenA->DungeonEdges.Num(), DetGenB->DungeonEdges.Num());
	TestEqual(TEXT("Deterministic boss node indices match"), DetGenA->BossNodeIndex, DetGenB->BossNodeIndex);

	for (int32 i = 0; i < DetGenA->DungeonNodes.Num(); ++i)
	{
		TestEqual(TEXT("Deterministic node grid coords match"), DetGenA->DungeonNodes[i].GridCoord, DetGenB->DungeonNodes[i].GridCoord);
		TestEqual(TEXT("Deterministic node tile types match"), DetGenA->DungeonNodes[i].TileType, DetGenB->DungeonNodes[i].TileType);
	}

	for (int32 e = 0; e < DetGenA->DungeonEdges.Num(); ++e)
	{
		TestEqual(TEXT("Deterministic edge NodeA matches"), DetGenA->DungeonEdges[e].NodeIndexA, DetGenB->DungeonEdges[e].NodeIndexA);
		TestEqual(TEXT("Deterministic edge NodeB matches"), DetGenA->DungeonEdges[e].NodeIndexB, DetGenB->DungeonEdges[e].NodeIndexB);
		TestEqual(TEXT("Deterministic edge directions match"), DetGenA->DungeonEdges[e].DirectionAtoB, DetGenB->DungeonEdges[e].DirectionAtoB);
	}

	// Multi-Seed Invariant Fuzzing (10 distinct seeds)
	for (int32 SeedIdx = 101; SeedIdx <= 110; ++SeedIdx)
	{
		ASovereignDungeonGenerator* FuzzGen = NewObject<ASovereignDungeonGenerator>();
		FuzzGen->TargetRoomCount = 8;
		FuzzGen->GenerationSeed = SeedIdx;
		FuzzGen->bSpawnPhysicalGeometry = false;
		FuzzGen->bAutoSpawnDefaultLighting = false;
		const bool bFuzzSuccess = FuzzGen->GenerateDungeon();

		TestTrue(TEXT("Fuzz generation succeeds"), bFuzzSuccess);
		TestTrue(TEXT("Fuzz reachability validation succeeds"), FuzzGen->ValidateTopologicalReachability(0, FuzzGen->BossNodeIndex));
		TestEqual(TEXT("Fuzz spanning tree invariant holds (|E| == |V| - 1)"), FuzzGen->DungeonEdges.Num(), FuzzGen->DungeonNodes.Num() - 1);
		TestTrue(TEXT("Fuzz entrance is EntranceRoom"), FuzzGen->DungeonNodes[0].TileType == EDungeonTileType::EntranceRoom);
		TestTrue(TEXT("Fuzz boss is BossChamber"), FuzzGen->DungeonNodes[FuzzGen->BossNodeIndex].TileType == EDungeonTileType::BossChamber);

		// Zero coordinate duplicates check
		TSet<FIntPoint> UniqueCoords;
		for (const FSovereignDungeonNode& Node : FuzzGen->DungeonNodes)
		{
			TestFalse(TEXT("Fuzz unique grid coordinates"), UniqueCoords.Contains(Node.GridCoord));
			UniqueCoords.Add(Node.GridCoord);
		}
	}

	// 10. Test Ticket B-096 Controlled Shortcut Loops
	ASovereignDungeonGenerator* LoopGen = NewObject<ASovereignDungeonGenerator>();
	LoopGen->TargetRoomCount = 10;
	LoopGen->GenerationSeed = 1337;
	LoopGen->MaxLoopCount = 2;
	LoopGen->bSpawnPhysicalGeometry = false;
	LoopGen->bAutoSpawnDefaultLighting = false;
	const bool bLoopGenSuccess = LoopGen->GenerateDungeon();

	TestTrue(TEXT("Loop generation succeeds"), bLoopGenSuccess);
	TestTrue(TEXT("Loop reachability validation succeeds"), LoopGen->ValidateTopologicalReachability(0, LoopGen->BossNodeIndex));

	// Verify loop edge bounds: |E| >= |V| - 1 and |E| <= (|V| - 1) + MaxLoopCount
	TestTrue(TEXT("Loop edge count >= |V| - 1"), LoopGen->DungeonEdges.Num() >= LoopGen->DungeonNodes.Num() - 1);
	TestTrue(TEXT("Loop edge count <= (|V| - 1) + MaxLoopCount"), LoopGen->DungeonEdges.Num() <= (LoopGen->DungeonNodes.Num() - 1) + LoopGen->MaxLoopCount);

	// Verify Boss Chamber invariant: strictly 1 edge connected to Boss
	int32 BossEdgeCount = 0;
	for (const FSovereignDungeonEdge& Edge : LoopGen->DungeonEdges)
	{
		if (Edge.Connects(LoopGen->BossNodeIndex))
		{
			BossEdgeCount++;
		}
	}
	TestEqual(TEXT("Boss Chamber strictly retains 1 entrance edge even with shortcut loops"), BossEdgeCount, 1);

	// Verify Entrance Room invariant: no loop edges connected to Entrance (Entrance only has forward spine edge)
	int32 EntranceEdgeCount = 0;
	for (const FSovereignDungeonEdge& Edge : LoopGen->DungeonEdges)
	{
		if (Edge.Connects(0))
		{
			EntranceEdgeCount++;
		}
	}
	TestTrue(TEXT("Entrance Room has at most 2 connections and never an invalid loop shortcut"), EntranceEdgeCount <= 2);

	// Test CalculateGraphDistance helper
	TestEqual(TEXT("Distance from node to itself is 0"), LoopGen->CalculateGraphDistance(0, 0), 0);
	TestTrue(TEXT("Distance from Entrance to Boss is positive"), LoopGen->CalculateGraphDistance(0, LoopGen->BossNodeIndex) >= LoopGen->MinSpineLength);

	// 11. Test Subsystem Regression Compatibility
	TestEqual(TEXT("GetEntranceTransform returns Node 0 transform"), DetGenA->GetEntranceTransform().GetLocation(), DetGenA->DungeonNodes[0].WorldTransform.GetLocation());

	return true;
}
