// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN SKILL CORE TESTS - B-074 Verification
 * ============================================================================
 * Verifies USovereignSkillDefinition creation, PrimaryAssetId generation,
 * targeting pattern defaults, and raw power calculations with stat scaling.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignSkillCoreTest,
	"Sovereign.Skill.CoreOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignSkillCoreTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate USovereignSkillDefinition
	USovereignSkillDefinition* SkillDef = NewObject<USovereignSkillDefinition>();
	if (!SkillDef)
	{
		AddError(TEXT("Failed to instantiate USovereignSkillDefinition"));
		return false;
	}

	// 2. Verify PrimaryAssetId registration
	FPrimaryAssetId AssetId = SkillDef->GetPrimaryAssetId();
	TestEqual(TEXT("PrimaryAssetType should be SovereignSkill"), AssetId.PrimaryAssetType.GetName(), FName(TEXT("SovereignSkill")));

	// 3. Configure Skill Parameters (e.g. Warrior Cleave)
	const_cast<float&>(SkillDef->BasePower) = 30.0f;
	const_cast<float&>(SkillDef->StatMultiplier) = 1.50f;
	const_cast<ESovereignSkillTargetPattern&>(SkillDef->TargetPattern) = ESovereignSkillTargetPattern::FrontRowSweep;
	const_cast<ESovereignSkillScalingAttribute&>(SkillDef->ScalingAttribute) = ESovereignSkillScalingAttribute::Strength;

	// 4. Test Damage Calculation Formula: (BasePower + StatValue * StatMultiplier) * ClassMultiplier
	// Given STR = 20: 30 + (20 * 1.5) = 30 + 30 = 60
	float RawPowerBase = SkillDef->CalculateRawPower(20, 1.0f);
	TestEqual(TEXT("Raw power with 1.0x class multiplier should be 60.0f"), RawPowerBase, 60.0f);

	// Given Warrior Job +25% STR multiplier (ClassMultiplier = 1.25x): 60 * 1.25 = 75.0f
	float RawPowerWithJob = SkillDef->CalculateRawPower(20, 1.25f);
	TestEqual(TEXT("Raw power with 1.25x class multiplier should be 75.0f"), RawPowerWithJob, 75.0f);

	// 5. Verify Target Pattern
	TestTrue(TEXT("TargetPattern is FrontRowSweep"), SkillDef->TargetPattern == ESovereignSkillTargetPattern::FrontRowSweep);

	return true;
}
