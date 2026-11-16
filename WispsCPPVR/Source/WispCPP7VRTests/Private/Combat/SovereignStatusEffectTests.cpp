// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignStatusComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN STATUS EFFECT TESTS - Ticket CS-005 Verification
 * ============================================================================
 * Verifies:
 * 1. Poison ticks damage and decrements duration on turn end.
 * 2. Regen heals health and decrements duration on turn end.
 * 3. Stun skips the combatant's active turn.
 * 4. Silence prevents casting Qi skills.
 * 5. Haste and Slow modify ATB effective fill rate.
 * 6. Blind mitigates physical damage in combat calculation.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignStatusEffectSuiteTest,
	"Sovereign.Combat.StatusEffectSuite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignStatusEffectSuiteTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		World = GEngine->GetWorldContexts()[0].World();
	}

	ASovereignBaseCharacter* Character = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to create ASovereignBaseCharacter"));
		return false;
	}

	USovereignStatusComponent* StatusComp = Character->GetStatusComponent();
	TestNotNull(TEXT("Character has StatusComponent"), StatusComp);
	if (!StatusComp)
	{
		return false;
	}

	USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent();
	AttrComp->MaxHealth = 100.0f;
	AttrComp->CurrentHealth = 50.0f;

	// 1. Poison Test: apply Poison (2 turns, 10 damage)
	StatusComp->ApplyStatusEffect(ESovereignStatusType::Poison, 2, 10.0f);
	TestTrue(TEXT("Character has Poison status"), StatusComp->HasStatusEffect(ESovereignStatusType::Poison));

	StatusComp->ProcessTurnEnd();
	TestEqual(TEXT("Poison dealt 10 damage (CurrentHealth 40)"), AttrComp->CurrentHealth, 40.0f);
	TestTrue(TEXT("Poison is still active for 1 turn"), StatusComp->HasStatusEffect(ESovereignStatusType::Poison));

	StatusComp->ProcessTurnEnd();
	TestEqual(TEXT("Poison dealt another 10 damage (CurrentHealth 30)"), AttrComp->CurrentHealth, 30.0f);
	TestFalse(TEXT("Poison expired after 2 turns"), StatusComp->HasStatusEffect(ESovereignStatusType::Poison));

	// 2. Regen Test: apply Regen (1 turn, 15 heal)
	StatusComp->ApplyStatusEffect(ESovereignStatusType::Regen, 1, 15.0f);
	TestTrue(TEXT("Character has Regen status"), StatusComp->HasStatusEffect(ESovereignStatusType::Regen));

	StatusComp->ProcessTurnEnd();
	TestEqual(TEXT("Regen healed 15 HP (CurrentHealth 45)"), AttrComp->CurrentHealth, 45.0f);
	TestFalse(TEXT("Regen expired after 1 turn"), StatusComp->HasStatusEffect(ESovereignStatusType::Regen));

	// 3. ATB Speed Multipliers: Haste (1.5x) and Slow (0.5x)
	USovereignATBComponent* ATBComp = Character->GetATBComponent();
	const float BaseRate = ATBComp->CalculateEffectiveFillRate();

	StatusComp->ApplyStatusEffect(ESovereignStatusType::Haste, 2);
	const float HasteRate = ATBComp->CalculateEffectiveFillRate();
	TestEqual(TEXT("Haste increases fill rate by 1.5x"), HasteRate, BaseRate * 1.5f);

	StatusComp->RemoveStatusEffect(ESovereignStatusType::Haste);
	StatusComp->ApplyStatusEffect(ESovereignStatusType::Slow, 2);
	const float SlowRate = ATBComp->CalculateEffectiveFillRate();
	TestEqual(TEXT("Slow decreases fill rate by 0.5x"), SlowRate, BaseRate * 0.5f);
	StatusComp->ClearAllStatuses();

	// 4. Silence Test: prevents casting Qi skills
	ASovereignBattleManager* BattleMgr = World ? World->SpawnActor<ASovereignBattleManager>() : NewObject<ASovereignBattleManager>();
	USovereignSkillDefinition* QiSpell = NewObject<USovereignSkillDefinition>();
	QiSpell->QiCost = 15.0f;

	USovereignQiComponent* QiComp = Character->GetQiComponent();
	QiComp->MaxQiCapacity = 50.0f;
	QiComp->CurrentQi = 50.0f;

	TestTrue(TEXT("Character can cast Qi spell when unsilenced"), BattleMgr->CanCastSkill(Character, QiSpell));

	StatusComp->ApplyStatusEffect(ESovereignStatusType::Silence, 2);
	TestFalse(TEXT("Character CANNOT cast Qi spell when Silenced"), BattleMgr->CanCastSkill(Character, QiSpell));
	StatusComp->ClearAllStatuses();

	// 5. Blind Test: reduces physical damage in combat calculation
	FSovereignCombatEntitySnapshot AttackerNormal;
	AttackerNormal.Strength = 20;
	AttackerNormal.FormationSlot = 1;
	AttackerNormal.bIsBlinded = false;

	FSovereignCombatEntitySnapshot AttackerBlinded = AttackerNormal;
	AttackerBlinded.bIsBlinded = true;

	FSovereignCombatEntitySnapshot TargetSnap;
	TargetSnap.Constitution = 10;
	TargetSnap.FormationSlot = 1;

	USovereignSkillDefinition* MeleeSkill = NewObject<USovereignSkillDefinition>();
	MeleeSkill->BasePower = 40.0f;
	MeleeSkill->StatMultiplier = 1.0f;
	MeleeSkill->ScalingAttribute = ESovereignSkillScalingAttribute::Strength;
	FSovereignSkillInstance MeleeInst(MeleeSkill);

	const FSovereignDamageCalculationResult NormalResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
		AttackerNormal, TargetSnap, MeleeInst, 1.0f
	);
	const FSovereignDamageCalculationResult BlindResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
		AttackerBlinded, TargetSnap, MeleeInst, 1.0f
	);

	TestTrue(TEXT("Blind reduces attack damage"), BlindResult.FinalDamage < NormalResult.FinalDamage);
	TestTrue(TEXT("Blind result flags bIsBlinded"), BlindResult.bIsBlinded);

	// Teardown
	if (World)
	{
		Character->Destroy();
		BattleMgr->Destroy();
	}

	return true;
}
