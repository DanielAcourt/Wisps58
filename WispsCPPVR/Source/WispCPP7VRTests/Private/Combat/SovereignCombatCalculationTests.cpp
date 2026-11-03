// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Combat/SovereignCombatCalculationLibrary.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Combat/SovereignSkillAugment.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignCombatCalculationDamageTest, "WispsGame.Combat.DamageCalculationFormula", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSovereignCombatCalculationDamageTest::RunTest(const FString& Parameters)
{
	// 1. Setup Skill Definition (Warrior Slash: BasePower 20, STR scaling 1.2x)
	USovereignSkillDefinition* SkillDef = NewObject<USovereignSkillDefinition>();
	SkillDef->BasePower = 20.0f;
	SkillDef->StatMultiplier = 1.2f;
	SkillDef->ScalingAttribute = ESovereignSkillScalingAttribute::Strength;

	FSovereignSkillInstance Instance(SkillDef);

	// 2. Setup Attacker (STR 50, Front Rank, Class Mult 1.0)
	FSovereignCombatEntitySnapshot Attacker;
	Attacker.FormationSlot = 1; // Front Rank
	Attacker.Strength = 50;
	Attacker.Luck = 10;
	Attacker.ClassDamageMultiplier = 1.0f;

	// 3. Setup Target (CON 20, Front Rank, Luck 10)
	FSovereignCombatEntitySnapshot Target;
	Target.FormationSlot = 2; // Front Rank
	Target.Constitution = 20;
	Target.Luck = 10;
	Target.EquipmentDefense = 0.0f;

	// Raw Power: 20 + 50 * 1.2 = 80.0f
	// Attacker Rank (Front with STR): +10% -> 80 * 1.10 = 88.0f
	// Target Defense (Front Rank): 20 * 1.15 = 23.0f
	// Mitigation: 100 / (100 + 23) = 100 / 123 ~= 0.813008f
	// Mitigated Damage: 88.0f * 0.813008f ~= 71.54f -> Round to 72.0f

	FSovereignDamageCalculationResult Result = USovereignCombatCalculationLibrary::CalculateCombatDamage(
		Attacker, Target, Instance, 0.99f // Forced non-crit roll
	);

	TestFalse(TEXT("Should not be critical hit with roll 0.99"), Result.bIsCriticalHit);
	TestEqual(TEXT("RawPower should equal 80.0"), Result.RawPower, 80.0f);
	TestEqual(TEXT("AttackerRankModifier should be 1.10 (Front Rank Melee)"), Result.AttackerRankModifier, 1.10f);
	TestEqual(TEXT("TargetDefenseModifier should be 1.15 (Front Rank Def)"), Result.TargetDefenseModifier, 1.15f);
	TestEqual(TEXT("EffectiveDefense should be 23.0"), Result.EffectiveDefense, 23.0f);
	TestEqual(TEXT("FinalDamage rounded should equal 72.0"), Result.FinalDamage, 72.0f);

	// 4. Test Critical Strike (Forced roll 0.01)
	FSovereignDamageCalculationResult CritResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
		Attacker, Target, Instance, 0.01f // Forced crit roll
	);

	TestTrue(TEXT("Should be critical hit with roll 0.01"), CritResult.bIsCriticalHit);
	// 71.54f * 1.5 = 107.31f -> Round to 107.0f
	TestEqual(TEXT("Critical FinalDamage should equal 107.0"), CritResult.FinalDamage, 107.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSovereignCombatTargetEvaluationTest, "WispsGame.Combat.RankMatrixTargetEvaluation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSovereignCombatTargetEvaluationTest::RunTest(const FString& Parameters)
{
	// Setup 9-slot defending formation:
	// Slot 1 (Front): Alive
	// Slot 2 (Front): Dead
	// Slot 3 (Front): Dead
	// Slot 4 (Mid): Alive
	// Slot 7 (Rear): Alive
	TArray<FSovereignCombatEntitySnapshot> Formation;
	Formation.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
	{
		Formation[i].FormationSlot = i + 1;
		Formation[i].bIsAlive = false;
	}

	Formation[0].bIsAlive = true; // Slot 1 (Front)
	Formation[3].bIsAlive = true; // Slot 4 (Mid)
	Formation[6].bIsAlive = true; // Slot 7 (Rear)

	// Melee single target (STR scaling) targeting Slot 7 (Rear)
	// Front rank still has living defender (Slot 1), so Slot 7 must be illegal
	bool bRearLegalMelee = USovereignCombatCalculationLibrary::IsTargetSlotLegal(
		7, Formation, ESovereignSkillTargetPattern::SingleTarget, ESovereignSkillScalingAttribute::Strength
	);
	TestFalse(TEXT("Rear Rank should be protected from melee while Front Rank has living units"), bRearLegalMelee);

	// Spell single target (INT scaling) targeting Slot 7 (Rear)
	// Spells bypass front-line melee obstruction!
	bool bRearLegalSpell = USovereignCombatCalculationLibrary::IsTargetSlotLegal(
		7, Formation, ESovereignSkillTargetPattern::SingleTarget, ESovereignSkillScalingAttribute::Intelligence
	);
	TestTrue(TEXT("Rear Rank should be legally targetable by spells regardless of front units"), bRearLegalSpell);

	// Eliminate Front Rank unit (Slot 1 dies)
	Formation[0].bIsAlive = false;

	// Mid Rank unit (Slot 4) still alive. Can Melee hit Rear Rank (Slot 7)?
	// Rear rank protected while Mid rank has living units!
	bool bRearLegalMidAlive = USovereignCombatCalculationLibrary::IsTargetSlotLegal(
		7, Formation, ESovereignSkillTargetPattern::SingleTarget, ESovereignSkillScalingAttribute::Strength
	);
	TestFalse(TEXT("Rear Rank should be protected from melee while Mid Rank has living units"), bRearLegalMidAlive);

	// Can Melee hit Mid Rank (Slot 4) now that Front Rank is dead?
	bool bMidLegal = USovereignCombatCalculationLibrary::IsTargetSlotLegal(
		4, Formation, ESovereignSkillTargetPattern::SingleTarget, ESovereignSkillScalingAttribute::Strength
	);
	TestTrue(TEXT("Mid Rank becomes legally targetable by melee once Front Rank is cleared"), bMidLegal);

	// Eliminate Mid Rank unit (Slot 4 dies). Now Rear Rank (Slot 7) is exposed to melee!
	Formation[3].bIsAlive = false;
	bool bRearLegalExposed = USovereignCombatCalculationLibrary::IsTargetSlotLegal(
		7, Formation, ESovereignSkillTargetPattern::SingleTarget, ESovereignSkillScalingAttribute::Strength
	);
	TestTrue(TEXT("Rear Rank becomes legally targetable by melee once all front and mid units are dead"), bRearLegalExposed);

	// FrontRowSweep test:
	// Revive Slot 4 (Mid) and Slot 7 (Rear), with Front dead. Foremost active rank should be Mid (Slot 4).
	Formation[3].bIsAlive = true;
	TArray<int32> SweepTargets;
	USovereignCombatCalculationLibrary::ResolveTargetSlots(
		0, Formation, ESovereignSkillTargetPattern::FrontRowSweep, ESovereignSkillScalingAttribute::Strength, SweepTargets
	);

	TestEqual(TEXT("FrontRowSweep should resolve 1 target (Slot 4) when front is empty"), SweepTargets.Num(), 1);
	if (SweepTargets.Num() > 0)
	{
		TestEqual(TEXT("Resolved sweep target should be Slot 4"), SweepTargets[0], 4);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
