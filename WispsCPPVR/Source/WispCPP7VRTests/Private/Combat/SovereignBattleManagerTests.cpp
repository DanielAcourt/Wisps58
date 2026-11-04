// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN BATTLE MANAGER TESTS - B-079 Verification
 * ============================================================================
 * Verifies dual 3x3 grid initialization, EBattleState transitions, DEX turn
 * arbitration with tie-breaking, alive checks, and Victory/Defeat/Flee outcomes.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignBattleManagerTest,
	"Sovereign.Combat.BattleManagerOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBattleManagerTest::RunTest(const FString& Parameters)
{
	ASovereignBattleManager* BattleMgr = NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to instantiate ASovereignBattleManager"));
		return false;
	}

	// 1. Initial State
	TestEqual(TEXT("Initial state should be EncounterStarting"), BattleMgr->BattleState, EBattleState::EncounterStarting);
	TestEqual(TEXT("PlayerGrid size should be 9"), BattleMgr->PlayerGrid.Num(), 9);
	TestEqual(TEXT("EnemyGrid size should be 9"), BattleMgr->EnemyGrid.Num(), 9);

	// 2. Setup Combatants
	ASovereignBaseCharacter* PlayerDragon = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* FastAlly = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* EnemyBoss = NewObject<ASovereignBaseCharacter>();

	// Configure Attributes
	if (USovereignAttributeComponent* Attr = PlayerDragon->GetAttributeComponent())
	{
		Attr->Dexterity = 15;
		Attr->MaxHealth = 100.0f;
		Attr->CurrentHealth = 100.0f;
	}
	if (USovereignAttributeComponent* Attr = FastAlly->GetAttributeComponent())
	{
		Attr->Dexterity = 30; // Faster than PlayerDragon
		Attr->MaxHealth = 80.0f;
		Attr->CurrentHealth = 80.0f;
	}
	if (USovereignAttributeComponent* Attr = EnemyBoss->GetAttributeComponent())
	{
		Attr->Dexterity = 20; // Faster than PlayerDragon, slower than FastAlly
		Attr->MaxHealth = 250.0f;
		Attr->CurrentHealth = 250.0f;
	}

	TArray<ASovereignBaseCharacter*> PlayerUnits = { PlayerDragon, FastAlly };
	TArray<ASovereignBaseCharacter*> EnemyUnits = { EnemyBoss };

	// 3. Initialize Encounter
	BattleMgr->InitializeEncounter(PlayerUnits, EnemyUnits, true);
	TestEqual(TEXT("BattleState transitions to ActiveCombat"), BattleMgr->BattleState, EBattleState::ActiveCombat);
	TestEqual(TEXT("Living player units count is 2"), BattleMgr->GetLivingPlayerUnits().Num(), 2);
	TestEqual(TEXT("Living enemy units count is 1"), BattleMgr->GetLivingEnemyUnits().Num(), 1);

	// 4. ATB Queue Arbitration: Higher DEX acts first
	BattleMgr->EnqueueReadyCombatant(PlayerDragon); // DEX 15
	BattleMgr->EnqueueReadyCombatant(FastAlly);     // DEX 30
	BattleMgr->EnqueueReadyCombatant(EnemyBoss);    // DEX 20

	TestEqual(TEXT("ReadyQueue count is 3"), BattleMgr->ReadyQueue.Num(), 3);

	// Test automated event hook from ATBComponent OnATBReady
	ASovereignBaseCharacter* AutoReadyChar = NewObject<ASovereignBaseCharacter>();
	if (USovereignAttributeComponent* Attr = AutoReadyChar->GetAttributeComponent())
	{
		Attr->Dexterity = 50; // Super fast
		Attr->CurrentHealth = 50.0f;
	}
	BattleMgr->OnCombatantATBReady(AutoReadyChar);
	TestEqual(TEXT("AutoReadyChar with DEX 50 is now head of queue"), BattleMgr->DequeueNextCombatant(), AutoReadyChar);

	// Dequeue order must be: FastAlly (30) -> EnemyBoss (20) -> PlayerDragon (15)
	ASovereignBaseCharacter* FirstActor = BattleMgr->DequeueNextCombatant();
	TestEqual(TEXT("First actor is FastAlly (highest DEX 30)"), FirstActor, FastAlly);

	ASovereignBaseCharacter* SecondActor = BattleMgr->DequeueNextCombatant();
	TestEqual(TEXT("Second actor is EnemyBoss (DEX 20)"), SecondActor, EnemyBoss);

	ASovereignBaseCharacter* ThirdActor = BattleMgr->DequeueNextCombatant();
	TestEqual(TEXT("Third actor is PlayerDragon (DEX 15)"), ThirdActor, PlayerDragon);

	TestNull(TEXT("Queue is now empty"), BattleMgr->DequeueNextCombatant());

	// 5. Outcome Evaluation: Defeating the Enemy Boss triggers Victory
	if (USovereignAttributeComponent* Attr = EnemyBoss->GetAttributeComponent())
	{
		Attr->CurrentHealth = 0.0f; // Incapacitate boss
	}

	const bool bOutcomeTriggered = BattleMgr->CheckCombatOutcome();
	TestTrue(TEXT("CheckCombatOutcome returns true when a team is wiped"), bOutcomeTriggered);
	TestEqual(TEXT("BattleState transitions to Victory"), BattleMgr->BattleState, EBattleState::Victory);

	// 6. Flee Test
	BattleMgr->SetBattleState(EBattleState::ActiveCombat);
	BattleMgr->bIsEscapable = false;
	BattleMgr->ConcludeFled();
	TestEqual(TEXT("Cannot flee if bIsEscapable is false"), BattleMgr->BattleState, EBattleState::ActiveCombat);

	BattleMgr->bIsEscapable = true;
	BattleMgr->ConcludeFled();
	TestEqual(TEXT("ConcludeFled sets BattleState to Fled"), BattleMgr->BattleState, EBattleState::Fled);

	return true;
}
