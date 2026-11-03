// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Combat/SovereignSkillAugment.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Components/SovereignSkillComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN SKILL AUGMENT TESTS - B-075 Verification
 * ============================================================================
 * Verifies custom skill augment attachments, Qi expenditure deduction from
 * USovereignQiComponent, target formation override (Single -> FrontRowSweep),
 * damage multiplier boosting (120% -> 200%), and JSON persistence roundtrip.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignSkillAugmentTest,
	"Sovereign.Skill.AugmentCustomization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignSkillAugmentTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate Character Vessel with QiComponent & SkillComponent
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	USovereignQiComponent* QiComp = Character->FindComponentByClass<USovereignQiComponent>();
	USovereignSkillComponent* SkillComp = Character->GetSkillComponent();

	TestNotNull(TEXT("QiComponent exists"), QiComp);
	TestNotNull(TEXT("SkillComponent exists"), SkillComp);

	if (!QiComp || !SkillComp)
	{
		return false;
	}

	QiComp->CurrentQi = 100.0f;
	QiComp->MaxQiCapacity = 200.0f;

	// 2. Create Base Skill Definition: Warrior Strike (SingleTarget, 1.20x STR, BasePower: 20)
	USovereignSkillDefinition* StrikeDef = NewObject<USovereignSkillDefinition>();
	const_cast<float&>(StrikeDef->BasePower) = 20.0f;
	const_cast<float&>(StrikeDef->StatMultiplier) = 1.20f;
	const_cast<ESovereignSkillTargetPattern&>(StrikeDef->TargetPattern) = ESovereignSkillTargetPattern::SingleTarget;

	FGuid SkillId;
	TestTrue(TEXT("Equipping base strike skill should succeed"), SkillComp->EquipSkill(StrikeDef, SkillId));

	FSovereignSkillInstance InstanceBefore;
	TestTrue(TEXT("Skill instance found"), SkillComp->FindSkillInstance(SkillId, InstanceBefore));
	TestEqual(TEXT("Base target pattern is SingleTarget"), InstanceBefore.GetEffectiveTargetPattern(), ESovereignSkillTargetPattern::SingleTarget);
	TestEqual(TEXT("Base stat multiplier is 1.20x"), InstanceBefore.GetEffectiveStatMultiplier(), 1.20f);

	// Base power for STR=20: 20 + (20 * 1.20) = 44.0
	TestEqual(TEXT("Initial potency should be 44.0f"), InstanceBefore.CalculateFinalPotency(20, 1.0f), 44.0f);

	// 3. Create Augment 1: Shape Spread -> FrontRowSweep (Cost: 25 Qi)
	USovereignSkillAugment* CleaveAugment = NewObject<USovereignSkillAugment>();
	CleaveAugment->AugmentTag = FGameplayTag::RequestGameplayTag(TEXT("Augment.Shape.RankCleave"), false);
	CleaveAugment->AugmentName = FText::FromString(TEXT("Front Rank Cleave"));
	CleaveAugment->AugmentType = ESovereignAugmentType::ShapeSpread;
	CleaveAugment->QiCostToInfuse = 25.0f;
	CleaveAugment->bOverrideTargetPattern = true;
	CleaveAugment->NewTargetPattern = ESovereignSkillTargetPattern::FrontRowSweep;

	// 4. Create Augment 2: Potency Boost -> +80% Damage (1.20 -> 2.00) (Cost: 35 Qi)
	USovereignSkillAugment* EmpowerAugment = NewObject<USovereignSkillAugment>();
	EmpowerAugment->AugmentTag = FGameplayTag::RequestGameplayTag(TEXT("Augment.Potency.Empower"), false);
	EmpowerAugment->AugmentName = FText::FromString(TEXT("Empowered Strike"));
	EmpowerAugment->AugmentType = ESovereignAugmentType::Potency;
	EmpowerAugment->QiCostToInfuse = 35.0f;
	EmpowerAugment->AdditionalStatMultiplier = 0.80f;
	EmpowerAugment->AdditionalBasePower = 10.0f;

	// 5. Infuse Skill with Cleave Augment (Costs 25 Qi)
	TestTrue(TEXT("Infusing Cleave Augment should succeed"), SkillComp->InfuseSkillWithAugment(SkillId, CleaveAugment));
	TestEqual(TEXT("CurrentQi after Cleave should be 75.0 (100 - 25)"), QiComp->CurrentQi, 75.0f);

	// 6. Infuse Skill with Empower Augment (Costs 35 Qi)
	TestTrue(TEXT("Infusing Empower Augment should succeed"), SkillComp->InfuseSkillWithAugment(SkillId, EmpowerAugment));
	TestEqual(TEXT("CurrentQi after Empower should be 40.0 (75 - 35)"), QiComp->CurrentQi, 40.0f);

	// 7. Verify Custom Skill Transformations
	FSovereignSkillInstance InstanceAfter;
	TestTrue(TEXT("Customized skill instance found"), SkillComp->FindSkillInstance(SkillId, InstanceAfter));

	// Pattern transformed: SingleTarget -> FrontRowSweep
	TestEqual(TEXT("Effective target pattern overridden to FrontRowSweep"),
		InstanceAfter.GetEffectiveTargetPattern(), ESovereignSkillTargetPattern::FrontRowSweep);

	// Multiplier boosted: 1.20 + 0.80 = 2.00x (200% damage!)
	TestEqual(TEXT("Effective stat multiplier boosted to 2.00x"), InstanceAfter.GetEffectiveStatMultiplier(), 2.00f);

	// Base power increased: 20 + 10 = 30
	TestEqual(TEXT("Effective base power increased to 30.0f"), InstanceAfter.GetEffectiveBasePower(), 30.0f);

	// Final Potency for STR=20: 30 + (20 * 2.00) = 30 + 40 = 70.0f (vs 44.0f base!)
	TestEqual(TEXT("Final potency upgraded to 70.0f"), InstanceAfter.CalculateFinalPotency(20, 1.0f), 70.0f);

	// 8. Test JSON Persistence Roundtrip of Customized Skills & Augments
	TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
	SkillComp->OnSave(RootJson);

	TestTrue(TEXT("RootJson should have Sovereign.Skills key"), RootJson->HasField(TEXT("Sovereign.Skills")));

	// Mutate before loading
	SkillComp->EquippedSkills.Empty();

	SkillComp->OnLoad(RootJson);
	TestEqual(TEXT("Restored equipped skills count should be 1"), SkillComp->EquippedSkills.Num(), 1);

	const FSovereignSkillInstance& RestoredSkill = SkillComp->EquippedSkills[0];
	TestEqual(TEXT("Restored skill has 2 active augments"), RestoredSkill.ActiveAugments.Num(), 2);
	TestEqual(TEXT("Restored target pattern remains FrontRowSweep"),
		RestoredSkill.GetEffectiveTargetPattern(), ESovereignSkillTargetPattern::FrontRowSweep);
	TestEqual(TEXT("Restored stat multiplier remains 2.00x"), RestoredSkill.GetEffectiveStatMultiplier(), 2.00f);

	return true;
}
