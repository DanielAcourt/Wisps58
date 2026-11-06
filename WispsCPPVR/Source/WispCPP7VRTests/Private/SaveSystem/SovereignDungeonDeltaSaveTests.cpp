// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#include "Misc/AutomationTest.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "SaveSystem/SovereignSaveGame.h"
#include "SaveSystem/SovereignDungeonDelta.h"
#include "Utilities/SovereignWidgetSanitizer.h"
#include "Entities/SovereignDungeonDoor.h"
#include "Entities/SovereignTreasureChest.h"
#include "Entities/SovereignRestTerminal.h"
#include "Entities/SovereignKeyPickup.h"
#include "Entities/SovereignDungeonEntrance.h"
#include "Entities/SovereignBossTeleporter.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Unit Test: B-114 UMG Widget Property Duplicate Validation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignWidgetSanitizerValidationTest, "WispsGame.SaveSystem.WidgetSanitizerValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignWidgetSanitizerValidationTest::RunTest(const FString& Parameters)
{
	TArray<FString> Duplicates;

	// Validating nullptr returns false gracefully
	TestFalse(TEXT("Null class should return false"), USovereignWidgetSanitizer::ValidateWidgetClassProperties(nullptr, Duplicates));

	// Standard UObject class has no duplicate properties
	bool bClean = USovereignWidgetSanitizer::ValidateWidgetClassProperties(UObject::StaticClass(), Duplicates);
	TestTrue(TEXT("UObject base class properties must be clean"), bClean);
	TestEqual(TEXT("Duplicates array must be empty"), Duplicates.Num(), 0);

	return true;
}

/**
 * Unit Test: B-115 Mega-Dungeon Procedural Actor Serialization Bypass
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignProceduralActorBypassTest, "WispsGame.SaveSystem.ProceduralActorBypass", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignProceduralActorBypassTest::RunTest(const FString& Parameters)
{
	// Null check
	TestFalse(TEXT("Null actor is not bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(nullptr));

	// Unmutated Closed Door should be bypassed
	ASovereignDungeonDoor* Door = NewObject<ASovereignDungeonDoor>();
	Door->bIsOpen = false;
	TestTrue(TEXT("Unopened door should be bypassed during serialization"), USaveManager::ShouldBypassProceduralActorSerialization(Door));

	// Mutated / Opened Door should NOT be bypassed (must be saved!)
	Door->bIsOpen = true;
	TestFalse(TEXT("Opened door should NOT be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Door));

	// Unopened Chest should be bypassed
	ASovereignTreasureChest* Chest = NewObject<ASovereignTreasureChest>();
	Chest->bHasBeenOpened = false;
	TestTrue(TEXT("Unopened chest should be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Chest));

	// Opened Chest should NOT be bypassed
	Chest->bHasBeenOpened = true;
	TestFalse(TEXT("Opened chest should NOT be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Chest));

	// Rest terminal with full charges should be bypassed
	ASovereignRestTerminal* Rest = NewObject<ASovereignRestTerminal>();
	Rest->MaxRestCharges = 3;
	Rest->RemainingRestCharges = 3;
	TestTrue(TEXT("Unused rest terminal should be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Rest));

	// Rest terminal with charges used should NOT be bypassed
	Rest->RemainingRestCharges = 2;
	TestFalse(TEXT("Used rest terminal should NOT be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Rest));

	// Entrance is static and deterministically reconstructed at Node 0, so it is always bypassed
	ASovereignDungeonEntrance* Entrance = NewObject<ASovereignDungeonEntrance>();
	TestTrue(TEXT("Dungeon entrance should always be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Entrance));

	// Boss teleporter: dormant should be bypassed; activated should be saved
	ASovereignBossTeleporter* Teleporter = NewObject<ASovereignBossTeleporter>();
	Teleporter->bIsActive = false;
	TestTrue(TEXT("Dormant teleporter should be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Teleporter));

	Teleporter->bIsActive = true;
	TestFalse(TEXT("Active teleporter should NOT be bypassed"), USaveManager::ShouldBypassProceduralActorSerialization(Teleporter));

	return true;
}

/**
 * Unit Test: B-115 Mega-Dungeon Delta Serialization & Seed Persistence
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignDungeonDeltaSerializationTest, "WispsGame.SaveSystem.DungeonDeltaSerialization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignDungeonDeltaSerializationTest::RunTest(const FString& Parameters)
{
	USovereignSaveGame* Suitcase = NewObject<USovereignSaveGame>();
	TestNotNull(TEXT("Suitcase created"), Suitcase);

	Suitcase->DungeonDelta.ExpeditionSeed = 9999;
	Suitcase->DungeonDelta.EnemyBuffMultiplier = 2.0f;
	Suitcase->DungeonDelta.RewardMultiplier = 1.75f;
	Suitcase->DungeonDelta.bBossDefeated = true;

	FGuid DoorGuid = FGuid::NewGuid();
	Suitcase->DungeonDelta.DoorStates.Add(DoorGuid, true);

	FGuid ChestGuid = FGuid::NewGuid();
	Suitcase->DungeonDelta.ChestStates.Add(ChestGuid, true);

	FGuid RestGuid = FGuid::NewGuid();
	Suitcase->DungeonDelta.RestTerminalCharges.Add(RestGuid, 1);

	FGuid KeyGuid = FGuid::NewGuid();
	Suitcase->DungeonDelta.CollectedKeyGuids.Add(KeyGuid);

	// Verify delta state holds values accurately
	TestEqual(TEXT("Seed matches"), Suitcase->DungeonDelta.ExpeditionSeed, 9999);
	TestEqual(TEXT("EnemyBuff matches"), (float)Suitcase->DungeonDelta.EnemyBuffMultiplier, 2.0f);
	TestEqual(TEXT("RewardMult matches"), (float)Suitcase->DungeonDelta.RewardMultiplier, 1.75f);
	TestTrue(TEXT("BossDefeated matches"), Suitcase->DungeonDelta.bBossDefeated);
	TestEqual(TEXT("DoorStates count"), Suitcase->DungeonDelta.DoorStates.Num(), 1);
	TestEqual(TEXT("ChestStates count"), Suitcase->DungeonDelta.ChestStates.Num(), 1);
	TestEqual(TEXT("RestTerminalCharges count"), Suitcase->DungeonDelta.RestTerminalCharges.Num(), 1);
	TestEqual(TEXT("CollectedKeyGuids count"), Suitcase->DungeonDelta.CollectedKeyGuids.Num(), 1);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
