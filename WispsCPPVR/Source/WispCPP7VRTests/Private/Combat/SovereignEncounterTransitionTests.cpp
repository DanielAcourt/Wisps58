// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignATBComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN ENCOUNTER TRANSITION TESTS - B-083 Verification
 * ============================================================================
 * Verifies default spatial 3x3 grid generation, TriggerEncounter field caching,
 * combatant positioning onto arena transforms, and return to dungeon floor.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignEncounterTransitionTest,
	"Sovereign.Combat.EncounterTransitionOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignEncounterTransitionTest::RunTest(const FString& Parameters)
{
	// 1. Battle Manager Spatial Grid Generation
	ASovereignBattleManager* BattleMgr = NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to instantiate ASovereignBattleManager"));
		return false;
	}

	BattleMgr->SetActorLocation(FVector(1000.0f, 2000.0f, 500.0f));
	BattleMgr->GenerateDefaultGridTransforms();

	TestEqual(TEXT("PlayerSlotTransforms size is 9"), BattleMgr->PlayerSlotTransforms.Num(), 9);
	TestEqual(TEXT("EnemySlotTransforms size is 9"), BattleMgr->EnemySlotTransforms.Num(), 9);

	// Check Front rank vs Mid rank offset (Slot 0 is Front, Slot 3 is Mid: X difference is 200 units)
	const FVector PlayerFrontPos = BattleMgr->PlayerSlotTransforms[0].GetLocation();
	const FVector PlayerMidPos = BattleMgr->PlayerSlotTransforms[3].GetLocation();
	TestTrue(TEXT("Mid rank is spaced behind Front rank by 200 units on X"), FMath::IsNearlyEqual(FMath::Abs(PlayerFrontPos.X - PlayerMidPos.X), 200.0f, 1.0f));

	// 2. Setup Subsystem and Combatants with a valid World context if available
	UWorld* World = nullptr;
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		World = GEngine->GetWorldContexts()[0].World();
	}

	USovereignEncounterSubsystem* Subsystem = World ? World->GetSubsystem<USovereignEncounterSubsystem>() : nullptr;
	if (!Subsystem)
	{
		Subsystem = NewObject<USovereignEncounterSubsystem>();
	}

	if (!Subsystem)
	{
		AddError(TEXT("Failed to instantiate USovereignEncounterSubsystem"));
		return false;
	}

	ASovereignBaseCharacter* DragonLeader = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();
	DragonLeader->SetActorTransform(FTransform(FRotator::ZeroRotator, FVector(100.0f, 100.0f, 0.0f)));

	ASovereignBaseCharacter* DungeonMob = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();
	DungeonMob->SetActorTransform(FTransform(FRotator::ZeroRotator, FVector(150.0f, 100.0f, 0.0f)));

	ASovereignBaseCharacter* ArenaEnemy = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();

	// 3. Test TriggerEncounter
	const bool bTriggerSuccess = Subsystem->TriggerEncounter(
		DragonLeader, DungeonMob, { ArenaEnemy }, true, BattleMgr
	);

	TestTrue(TEXT("TriggerEncounter should succeed"), bTriggerSuccess);
	TestTrue(TEXT("bIsEncounterActive is true"), Subsystem->bIsEncounterActive);
	TestEqual(TEXT("DungeonReturnTransform location matches leader original pos"), Subsystem->DungeonReturnTransform.GetLocation(), FVector(100.0f, 100.0f, 0.0f));
	TestTrue(TEXT("DungeonMob is hidden in game during encounter"), DungeonMob->IsHidden());

	// 4. Test ReturnPartyToDungeon on Fled
	Subsystem->ReturnPartyToDungeon(EBattleState::Fled);
	TestFalse(TEXT("bIsEncounterActive is false after return"), Subsystem->bIsEncounterActive);
	TestEqual(TEXT("DragonLeader returned to dungeon location (100, 100, 0)"), DragonLeader->GetActorTransform().GetLocation(), FVector(100.0f, 100.0f, 0.0f));
	TestFalse(TEXT("DungeonMob is unhidden on Fled"), DungeonMob->IsHidden());

	// Cleanup test-spawned actors if in world
	if (World)
	{
		DragonLeader->Destroy();
		DungeonMob->Destroy();
		ArenaEnemy->Destroy();
	}

	return true;
}
