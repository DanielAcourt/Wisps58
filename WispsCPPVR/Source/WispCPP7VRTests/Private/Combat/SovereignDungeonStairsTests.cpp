// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-07

#include "Misc/AutomationTest.h"
#include "Entities/SovereignDungeonStairs.h"
#include "Entities/SovereignUpwardStairs.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Entities/SovereignPlayerWisp.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Unit Test: B-116 ASovereignDungeonStairs Inheritance, Sacrifice & Boss Defeat Unlocking
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignDungeonStairsInheritanceTest, "WispsGame.Combat.DungeonStairsInheritance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignDungeonStairsInheritanceTest::RunTest(const FString& Parameters)
{
	ASovereignDungeonStairs* Stairs = NewObject<ASovereignDungeonStairs>();
	TestNotNull(TEXT("Stairs instance created"), Stairs);

	// Test default state: sealed until boss defeat
	TestTrue(TEXT("Requires boss defeat by default"), Stairs->bRequireBossDefeat);
	TestFalse(TEXT("Initially locked"), Stairs->bIsUnlocked);

	// Verify hint text when locked
	TestTrue(TEXT("Locked hint displays requirement"), Stairs->GetInteractionHint_Implementation().Contains(TEXT("Defeat Vault Boss")));

	// Unseal upon boss defeat
	Stairs->NotifyBossDefeated();
	TestTrue(TEXT("Unlocked after boss defeated notification"), Stairs->bIsUnlocked);
	TestTrue(TEXT("Unlocked hint displays sacrifice/descent options"), Stairs->GetInteractionHint_Implementation().Contains(TEXT("Descend Deeper")));

	// Verify sacrifice calculation mechanics inherited from ASovereignDungeonGateway
	TestNearlyEqual(TEXT("Default enemy buff is 1.0"), (float)Stairs->CalculateEnemyBuffMultiplier(), 1.0f);
	TestNearlyEqual(TEXT("Default reward mult is 1.0"), (float)Stairs->CalculateRewardMultiplier(), 1.0f);

	return true;
}

/**
 * Unit Test: B-117 Floor Depth Advancement & Scaling Formula Verification
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignDungeonDepthScalingTest, "WispsGame.Combat.DungeonDepthScaling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignDungeonDepthScalingTest::RunTest(const FString& Parameters)
{
	USovereignDungeonSubsystem* Subsystem = NewObject<USovereignDungeonSubsystem>();
	TestNotNull(TEXT("Subsystem instance created"), Subsystem);

	// Test initialization
	Subsystem->RecordExpedition(1000, 1.0f, 1.0f, FName(TEXT("TestGarden")), TEXT("TestSaveSlot"));
	TestEqual(TEXT("Initial depth is 0"), Subsystem->GetCurrentDepth(), 0);
	TestEqual(TEXT("Initial seed is 1000"), Subsystem->ActiveExpeditionSeed, 1000);

	// Advance to Depth 1
	const int32 Depth1 = Subsystem->AdvanceFloorDepth(1.0f, 1.0f);
	TestEqual(TEXT("Depth advances to 1"), Depth1, 1);
	TestEqual(TEXT("Seed shifted deterministically"), Subsystem->ActiveExpeditionSeed, 1000 + 10007);

	// Depth 1 enemy scaling: 1.0 * (1.0 + 1 * 0.25) = 1.25
	TestNearlyEqual(TEXT("Depth 1 enemy buff is 1.25"), (float)Subsystem->ActiveEnemyBuffMultiplier, 1.25f);
	// Depth 1 reward scaling: 1.0 * (1.0 + 1 * 0.20) = 1.20
	TestNearlyEqual(TEXT("Depth 1 reward mult is 1.20"), (float)Subsystem->ActiveRewardMultiplier, 1.20f);

	// Advance to Depth 2
	const int32 Depth2 = Subsystem->AdvanceFloorDepth(1.0f, 1.0f);
	TestEqual(TEXT("Depth advances to 2"), Depth2, 2);

	// Clear expedition resets depth
	Subsystem->ClearExpedition();
	TestEqual(TEXT("Depth reset to 0"), Subsystem->GetCurrentDepth(), 0);

	return true;
}

/**
 * Unit Test: B-117 Upward Stairs Interface & Player Wisp Deepest Dungeon Metric
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignUpwardStairsInterfaceTest, "WispsGame.Combat.UpwardStairsInterface", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSovereignUpwardStairsInterfaceTest::RunTest(const FString& Parameters)
{
	ASovereignUpwardStairs* UpStairs = NewObject<ASovereignUpwardStairs>();
	TestNotNull(TEXT("Upward stairs created"), UpStairs);

	TestEqual(TEXT("Upward stairs interaction hint"), UpStairs->GetInteractionHint_Implementation(), TEXT("Ascend to Previous Floor"));

	ASovereignPlayerWisp* Wisp = NewObject<ASovereignPlayerWisp>();
	TestNotNull(TEXT("Player Wisp created"), Wisp);
	TestEqual(TEXT("Default deepest dungeon completed is 0"), Wisp->DeepestDungeonCompleted, 0);

	Wisp->DeepestDungeonCompleted = 5;
	TestEqual(TEXT("Deepest dungeon completed tracks correctly"), Wisp->DeepestDungeonCompleted, 5);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
