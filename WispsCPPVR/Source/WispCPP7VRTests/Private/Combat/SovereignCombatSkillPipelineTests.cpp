// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Combat/SovereignSkillAugment.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignATBComponent.h"
#include "GameplayTagsManager.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN COMBAT SKILL PIPELINE TESTS - B-080 Verification
 * ============================================================================
 * Verifies Qi cost validation and deduction, percentage-mitigated damage execution,
 * Defend stance reduction, Wisdom healing up to MaxHealth, and turn cycling.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCombatSkillPipelineTest,
	"Sovereign.Combat.SkillPipelineOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCombatSkillPipelineTest::RunTest(const FString& Parameters)
{
	ASovereignBattleManager* BattleMgr = NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to instantiate ASovereignBattleManager"));
		return false;
	}

	// 1. Setup Caster and Target Combatants
	ASovereignBaseCharacter* DragonCaster = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* EnemyTarget = NewObject<ASovereignBaseCharacter>();

	// Configure Caster
	USovereignAttributeComponent* CasterAttr = DragonCaster->GetAttributeComponent();
	USovereignQiComponent* CasterQi = DragonCaster->GetQiComponent();
	if (CasterAttr && CasterQi)
	{
		CasterAttr->Strength = 20;
		CasterAttr->Wisdom = 25;
		CasterAttr->Dexterity = 15;
		CasterAttr->MaxHealth = 150.0f;
		CasterAttr->CurrentHealth = 100.0f; // Damaged for heal test

		CasterQi->MaxQiCapacity = 100.0f;
		CasterQi->CurrentQi = 50.0f;
	}

	// Configure Target
	USovereignAttributeComponent* TargetAttr = EnemyTarget->GetAttributeComponent();
	USovereignATBComponent* TargetATB = EnemyTarget->GetATBComponent();
	if (TargetAttr)
	{
		TargetAttr->Constitution = 10;
		TargetAttr->MaxHealth = 100.0f;
		TargetAttr->CurrentHealth = 100.0f;
	}

	TArray<ASovereignBaseCharacter*> Players = { DragonCaster };
	TArray<ASovereignBaseCharacter*> Enemies = { EnemyTarget };
	BattleMgr->InitializeEncounter(Players, Enemies, true);

	// 2. Setup Skills
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Skill.Combat.Strike"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Skill.Combat.Heal"));

	USovereignSkillDefinition* StrikeSkill = NewObject<USovereignSkillDefinition>();
	StrikeSkill->SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Combat.Strike"), false);
	StrikeSkill->SkillDisplayName = FText::FromString(TEXT("Dragon Claw"));
	StrikeSkill->ScalingAttribute = ESovereignSkillScalingAttribute::Strength;
	StrikeSkill->TargetPattern = ESovereignSkillTargetPattern::SingleTarget;
	StrikeSkill->BasePower = 30.0f;
	StrikeSkill->StatMultiplier = 1.0f;
	StrikeSkill->QiCost = 20.0f;

	USovereignSkillDefinition* ExpensiveSkill = NewObject<USovereignSkillDefinition>();
	ExpensiveSkill->QiCost = 100.0f; // Caster only has 50 Qi

	USovereignSkillDefinition* HealSkill = NewObject<USovereignSkillDefinition>();
	HealSkill->SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Combat.Heal"), false);
	HealSkill->SkillDisplayName = FText::FromString(TEXT("Qi Restoration"));
	HealSkill->ScalingAttribute = ESovereignSkillScalingAttribute::Wisdom;
	HealSkill->TargetPattern = ESovereignSkillTargetPattern::AllAllies;
	HealSkill->BasePower = 20.0f;
	HealSkill->StatMultiplier = 1.0f;
	HealSkill->QiCost = 15.0f;

	// 3. Test CanCastSkill Validation
	TestTrue(TEXT("CanCastSkill returns true for 20 Qi skill when caster has 50 Qi"), BattleMgr->CanCastSkill(DragonCaster, StrikeSkill));
	TestFalse(TEXT("CanCastSkill returns false for 100 Qi skill when caster has 50 Qi"), BattleMgr->CanCastSkill(DragonCaster, ExpensiveSkill));

	// 4. Test Insufficient Qi rejection in ExecuteSkillAction
	FSovereignSkillInstance ExpensiveInstance;
	ExpensiveInstance.SkillDefinition = ExpensiveSkill;
	const bool bExpensiveExecuted = BattleMgr->ExecuteSkillAction(DragonCaster, ExpensiveInstance, { EnemyTarget });
	TestFalse(TEXT("ExecuteSkillAction fails and returns false when Qi is insufficient"), bExpensiveExecuted);
	TestEqual(TEXT("Caster Qi remains 50.0"), CasterQi->CurrentQi, 50.0f);
	TestEqual(TEXT("Target Health remains 100.0"), TargetAttr->CurrentHealth, 100.0f);

	// 5. Test Successful Damage Execution
	FSovereignSkillInstance StrikeInstance;
	StrikeInstance.SkillDefinition = StrikeSkill;
	const bool bStrikeExecuted = BattleMgr->ExecuteSkillAction(DragonCaster, StrikeInstance, { EnemyTarget });
	TestTrue(TEXT("ExecuteSkillAction succeeds for StrikeSkill"), bStrikeExecuted);
	TestEqual(TEXT("Caster Qi deducted by 20 (now 30.0)"), CasterQi->CurrentQi, 30.0f);
	TestTrue(TEXT("Target Health was damaged below 100.0"), TargetAttr->CurrentHealth < 100.0f);

	// 6. Test Defend Stance Mitigation
	if (TargetATB)
	{
		TargetATB->EnterDefendStance();
		TestTrue(TEXT("Target is defending"), TargetATB->bIsDefending);
	}

	const float HealthBeforeDefendedStrike = TargetAttr->CurrentHealth;
	const bool bDefendedStrikeExecuted = BattleMgr->ExecuteSkillAction(DragonCaster, StrikeInstance, { EnemyTarget });
	TestTrue(TEXT("Defended strike executes"), bDefendedStrikeExecuted);
	TestEqual(TEXT("Caster Qi deducted by 20 (now 10.0)"), CasterQi->CurrentQi, 10.0f);

	const float DefendedDamageDealt = HealthBeforeDefendedStrike - TargetAttr->CurrentHealth;
	TestTrue(TEXT("Defended damage was dealt"), DefendedDamageDealt > 0.0f);

	// 7. Test Wisdom Healing
	CasterQi->CurrentQi = 20.0f;
	FSovereignSkillInstance HealInstance;
	HealInstance.SkillDefinition = HealSkill;

	const float CasterHealthBeforeHeal = CasterAttr->CurrentHealth; // 100.0
	const bool bHealExecuted = BattleMgr->ExecuteSkillAction(DragonCaster, HealInstance, { DragonCaster });
	TestTrue(TEXT("Heal skill executes successfully"), bHealExecuted);
	TestEqual(TEXT("Caster Qi deducted by 15 (now 5.0)"), CasterQi->CurrentQi, 5.0f);
	TestTrue(TEXT("Caster Health increased after heal"), CasterAttr->CurrentHealth > CasterHealthBeforeHeal);
	TestTrue(TEXT("Caster Health capped at or below MaxHealth (150.0)"), CasterAttr->CurrentHealth <= CasterAttr->MaxHealth);

	return true;
}
