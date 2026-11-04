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
	TestTrue(TEXT("bAutoSpawnDefaultLighting defaults to true"), Generator->bAutoSpawnDefaultLighting);
	TestTrue(TEXT("bSpawnDirectionalLight defaults to true"), Generator->bSpawnDirectionalLight);
	TestTrue(TEXT("bSpawnPostProcessVolume defaults to true"), Generator->bSpawnPostProcessVolume);
	TestTrue(TEXT("bSpawnPhysicalGeometry defaults to true"), Generator->bSpawnPhysicalGeometry);

	// Toggle lighting off to test optional override when level defines custom SkyLight
	Generator->bAutoSpawnDefaultLighting = false;
	const bool bGenerated = Generator->GenerateDungeon();

	TestTrue(TEXT("GenerateDungeon should succeed"), bGenerated);
	TestEqual(TEXT("Total generated nodes should equal TargetRoomCount"), Generator->DungeonNodes.Num(), 6);

	// Verify Ticket B-089 3D geometry spawning
	TestNotNull(TEXT("FloorInstancedComponent exists"), Generator->FloorInstancedComponent.Get());
	TestNotNull(TEXT("WallInstancedComponent exists"), Generator->WallInstancedComponent.Get());
	TestEqual(TEXT("Floor instance count matches node count"), Generator->FloorInstancedComponent ? Generator->FloorInstancedComponent->GetInstanceCount() : 0, 6);
	TestTrue(TEXT("Boundary wall instances generated for perimeter edges"), Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() > 0 : false);

	// 5. Test Ticket B-090 Graph Data Model, Edge Representation & BFS Validation
	TestTrue(TEXT("DungeonEdges populated"), Generator->DungeonEdges.Num() > 0);
	TestEqual(TEXT("Spanning tree edge count invariant |E| == |V| - 1"), Generator->DungeonEdges.Num(), Generator->DungeonNodes.Num() - 1);

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

	// 6. Test Ticket B-092 Edge-Aware Geometry & Cleanup Lifecycle
	// Edge-aware invariant: Total walls + Total open passages across all nodes == Nodes * 4
	int32 OpenPassageTotal = 0;
	for (const FSovereignDungeonNode& Node : Generator->DungeonNodes)
	{
		for (int32 b = 0; b < 4; ++b)
		{
			if ((Node.ConnectedPassageMask & (1 << b)) != 0)
			{
				OpenPassageTotal++;
			}
		}
	}
	const int32 ExpectedWallCount = (Generator->DungeonNodes.Num() * 4) - OpenPassageTotal;
	TestEqual(TEXT("Wall instance count matches expected edge-aware boundary formula"),
		Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() : 0, ExpectedWallCount);

	// Test CleanupDungeon idempotency (running multiple times clears cleanly)
	Generator->CleanupDungeon();
	TestEqual(TEXT("Nodes cleared after cleanup"), Generator->DungeonNodes.Num(), 0);
	TestEqual(TEXT("Edges cleared after cleanup"), Generator->DungeonEdges.Num(), 0);
	TestEqual(TEXT("Floor instances cleared after cleanup"), Generator->FloorInstancedComponent ? Generator->FloorInstancedComponent->GetInstanceCount() : 0, 0);
	TestEqual(TEXT("Wall instances cleared after cleanup"), Generator->WallInstancedComponent ? Generator->WallInstancedComponent->GetInstanceCount() : 0, 0);

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
	USovereignDungeonSubsystem* DungeonSubsystem = NewObject<USovereignDungeonSubsystem>();
	if (TestNotNull(TEXT("DungeonSubsystem created"), DungeonSubsystem))
	{
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

	// 10. Test Subsystem Regression Compatibility
	TestEqual(TEXT("GetEntranceTransform returns Node 0 transform"), DetGenA->GetEntranceTransform().GetLocation(), DetGenA->DungeonNodes[0].WorldTransform.GetLocation());

	return true;
}
