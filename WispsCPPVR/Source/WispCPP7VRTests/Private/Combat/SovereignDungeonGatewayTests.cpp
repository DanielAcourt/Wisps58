// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Entities/SovereignDungeonGateway.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignQiComponent.h"
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
	const bool bGenerated = Generator->GenerateDungeon();

	TestTrue(TEXT("GenerateDungeon should succeed"), bGenerated);
	TestEqual(TEXT("Total generated nodes should equal TargetRoomCount"), Generator->DungeonNodes.Num(), 6);

	// Node 0 must be EntranceRoom
	TestEqual(TEXT("Node 0 is EntranceRoom"), Generator->DungeonNodes[0].TileType, EDungeonTileType::EntranceRoom);
	TestEqual(TEXT("Node 0 grid coord is (0, 0)"), Generator->DungeonNodes[0].GridCoord, FIntPoint(0, 0));

	// Furthest node must be BossChamber
	TestTrue(TEXT("BossNodeIndex is valid"), Generator->BossNodeIndex >= 0);
	TestEqual(TEXT("Terminal node is BossChamber"), Generator->DungeonNodes[Generator->BossNodeIndex].TileType, EDungeonTileType::BossChamber);
	TestTrue(TEXT("Boss chamber has fixed elite mob"), Generator->DungeonNodes[Generator->BossNodeIndex].bHasFixedEliteMob);

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

	return true;
}
