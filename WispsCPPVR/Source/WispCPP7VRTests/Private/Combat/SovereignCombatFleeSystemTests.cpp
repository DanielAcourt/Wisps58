// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN COMBAT FLEE SYSTEM TESTS - B-082 Verification
 * ============================================================================
 * Verifies boss escape restrictions, escape chance calculation, turn consumption
 * on failed flee attempts, fallback escape meter progression, and guaranteed flight at 100%.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCombatFleeSystemTest,
	"Sovereign.Combat.FleeSystemOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCombatFleeSystemTest::RunTest(const FString& Parameters)
{
	ASovereignBattleManager* BattleMgr = NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to instantiate ASovereignBattleManager"));
		return false;
	}

	// 1. Setup Combatants
	ASovereignBaseCharacter* DragonFleer = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* EnemyMob = NewObject<ASovereignBaseCharacter>();

	if (USovereignAttributeComponent* Attr = DragonFleer->GetAttributeComponent())
	{
		Attr->Dexterity = 20; // +10 over enemy
		Attr->Luck = 10;
		Attr->MaxHealth = 100.0f;
		Attr->CurrentHealth = 100.0f;
	}

	if (USovereignAttributeComponent* Attr = EnemyMob->GetAttributeComponent())
	{
		Attr->Dexterity = 10;
		Attr->MaxHealth = 100.0f;
		Attr->CurrentHealth = 100.0f;
	}

	TArray<ASovereignBaseCharacter*> Players = { DragonFleer };
	TArray<ASovereignBaseCharacter*> Enemies = { EnemyMob };

	// 2. Test Boss / Unescapable Restraint
	BattleMgr->InitializeEncounter(Players, Enemies, false); // bCanFlee = false
	TestFalse(TEXT("bIsEscapable is false for boss encounter"), BattleMgr->bIsEscapable);

	const bool bBossFleeResult = BattleMgr->AttemptFlee(DragonFleer, 0.0f);
	TestFalse(TEXT("AttemptFlee must fail against unescapable boss"), bBossFleeResult);
	TestEqual(TEXT("BattleState remains ActiveCombat"), BattleMgr->BattleState, EBattleState::ActiveCombat);
	TestEqual(TEXT("Escape progress remains 0.0 on boss fights"), BattleMgr->CurrentEscapeProgress, 0.0f);

	// 3. Test Escape Chance Calculation
	BattleMgr->bIsEscapable = true;
	// Formula: 50% + (20 - 10) * 2% + 10 * 0.5% + 0% = 50 + 20 + 5 = 75.0%
	const float ExpectedChance = 75.0f;
	const float CalculatedChance = BattleMgr->CalculateEscapeChance(DragonFleer);
	TestEqual(TEXT("CalculateEscapeChance should equal 75% for 10 DEX advantage + 10 Luck"), CalculatedChance, ExpectedChance);

	// 4. Test Failed Escape: Roll 80.0 > 75.0%
	BattleMgr->ActiveCombatant = DragonFleer;
	if (USovereignATBComponent* ATB = DragonFleer->GetATBComponent())
	{
		ATB->CurrentATB = 100.0f;
		ATB->SetATBState(EATBState::ExecutingAction);
	}

	const bool bFailedFlee = BattleMgr->AttemptFlee(DragonFleer, 80.0f);
	TestFalse(TEXT("Roll 80.0 fails against 75.0% chance"), bFailedFlee);
	TestEqual(TEXT("Escape progress increments by 25.0% (now 25.0%)"), BattleMgr->CurrentEscapeProgress, 25.0f);
	TestEqual(TEXT("BattleState remains in combat"), BattleMgr->BattleState, EBattleState::ActiveCombat);

	// Turn consumption verification: ATB was reset to 0%
	if (USovereignATBComponent* ATB = DragonFleer->GetATBComponent())
	{
		TestEqual(TEXT("Dragon ATB reset to 0.0% upon failed flee attempt"), ATB->CurrentATB, 0.0f);
		TestEqual(TEXT("Dragon ATBState transitions back to Charging"), ATB->ATBState, EATBState::Charging);
	}

	// 5. Test Escape Progress Accumulation: New chance should now be 75 + 25 = 100.0%
	const float ChanceAfterFail = BattleMgr->CalculateEscapeChance(DragonFleer);
	TestEqual(TEXT("Escape chance is now 100.0% after +25% failure bonus"), ChanceAfterFail, 100.0f);

	// 6. Test Successful Escape
	BattleMgr->ActiveCombatant = DragonFleer;
	const bool bSuccessfulFlee = BattleMgr->AttemptFlee(DragonFleer, 99.0f); // Roll 99 <= 100.0%
	TestTrue(TEXT("AttemptFlee succeeds when roll is within escape chance"), bSuccessfulFlee);
	TestEqual(TEXT("BattleState transitions to Fled"), BattleMgr->BattleState, EBattleState::Fled);

	return true;
}
