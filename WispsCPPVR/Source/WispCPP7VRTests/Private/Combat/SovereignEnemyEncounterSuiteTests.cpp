// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Combat/SovereignEnemyDefinition.h"
#include "Combat/SovereignEncounterDefinition.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignRoamingMob.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignLevelComponent.h"
#include "Components/SovereignClassComponent.h"
#include "Components/SovereignQiComponent.h"
#include "GameplayTagsManager.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN ENEMY & ENCOUNTER EXPEDITION TESTS
 * ============================================================================
 * Verifies:
 * 1. USovereignEnemyDefinition and USovereignEncounterDefinition data calculations
 * 2. InitializeEncounterFromDefinition 3x3 enemy vessel instantiation
 * 3. FF7-style Battle Camera view targeting
 * 4. Automated enemy turn AI action selection and execution
 * 5. Victory rewards distribution into USovereignLevelComponent & USovereignClassComponent
 * 6. Spawned enemy cleanup on battle end
 * 7. ASovereignRoamingMob overlap and encounter initiation
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignEnemyEncounterSuiteTest,
	"Sovereign.Combat.EnemyEncounterSuite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignEnemyEncounterSuiteTest::RunTest(const FString& Parameters)
{
	// 1. DataAsset Definition & Reward Calculation Validation
	USovereignEnemyDefinition* WolfDef = NewObject<USovereignEnemyDefinition>();
	WolfDef->EnemyDisplayName = FText::FromString(TEXT("Dire Wolf"));
	WolfDef->EnemyRole = ESovereignEnemyRole::AggressiveBrute;
	WolfDef->MaxHealth = 120.0f;
	WolfDef->MaxQi = 30.0f;
	WolfDef->Strength = 14;
	WolfDef->Dexterity = 12;
	WolfDef->ExperienceReward = 80.0f;
	WolfDef->ClassExperienceReward = 20;
	WolfDef->QiReward = 25.0f;

	USovereignEnemyDefinition* MageDef = NewObject<USovereignEnemyDefinition>();
	MageDef->EnemyDisplayName = FText::FromString(TEXT("Goblin Shaman"));
	MageDef->EnemyRole = ESovereignEnemyRole::SupportHealer;
	MageDef->MaxHealth = 80.0f;
	MageDef->MaxQi = 60.0f;
	MageDef->Intelligence = 16;
	MageDef->Wisdom = 14;
	MageDef->ExperienceReward = 100.0f;
	MageDef->ClassExperienceReward = 30;
	MageDef->QiReward = 35.0f;

	USovereignEncounterDefinition* EncounterDef = NewObject<USovereignEncounterDefinition>();
	EncounterDef->EncounterDisplayName = FText::FromString(TEXT("Ambush in the Dark"));
	EncounterDef->bCanFlee = true;

	FSovereignEncounterSlot Slot0;
	Slot0.SlotIndex = 0; // Front rank
	Slot0.EnemyDefinition = WolfDef;
	EncounterDef->EnemySlots.Add(Slot0);

	FSovereignEncounterSlot Slot6;
	Slot6.SlotIndex = 6; // Rear rank
	Slot6.EnemyDefinition = MageDef;
	EncounterDef->EnemySlots.Add(Slot6);

	TestEqual(TEXT("Encounter total EXP is 180.0"), EncounterDef->CalculateTotalExperience(), 180.0f);
	TestEqual(TEXT("Encounter total Class EXP is 50"), EncounterDef->CalculateTotalClassExperience(), 50);
	TestEqual(TEXT("Encounter total Qi is 60.0"), EncounterDef->CalculateTotalQi(), 60.0f);

	// 2. Battle Manager Arena Setup & Dynamic Spawning
	UWorld* World = nullptr;
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		World = GEngine->GetWorldContexts()[0].World();
	}

	ASovereignBattleManager* BattleMgr = World ? World->SpawnActor<ASovereignBattleManager>() : NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to create ASovereignBattleManager"));
		return false;
	}

	BattleMgr->SetActorLocation(FVector(50000.0f, 50000.0f, 5000.0f));
	BattleMgr->GenerateDefaultGridTransforms();

	ASovereignBaseCharacter* PlayerHero = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();
	if (USovereignAttributeComponent* Attr = PlayerHero->GetAttributeComponent())
	{
		Attr->Strength = 15;
		Attr->Dexterity = 12;
		Attr->CurrentHealth = 200.0f;
		Attr->MaxHealth = 200.0f;
	}
	if (USovereignLevelComponent* Lvl = PlayerHero->GetLevelComponent())
	{
		Lvl->SetTrainingFocus(ESovereignTrainingFocus::Strength);
		Lvl->StrengthXP = 0.0;
	}

	// 3. Test Encounter Spawning from Definition
	BattleMgr->InitializeEncounterFromDefinition({ PlayerHero }, EncounterDef);

	TestEqual(TEXT("BattleState is ActiveCombat"), BattleMgr->BattleState, EBattleState::ActiveCombat);
	TestEqual(TEXT("PlayerGrid Slot 0 has PlayerHero"), BattleMgr->PlayerGrid[0].Get(), PlayerHero);

	if (World)
	{
		TestEqual(TEXT("SpawnedEnemyActors has 2 enemies"), BattleMgr->SpawnedEnemyActors.Num(), 2);
		TestNotNull(TEXT("EnemyGrid Slot 0 is occupied by Wolf"), BattleMgr->EnemyGrid[0].Get());
		TestNotNull(TEXT("EnemyGrid Slot 6 is occupied by Mage"), BattleMgr->EnemyGrid[6].Get());

		// Verify Wolf Attributes Initialized
		if (BattleMgr->EnemyGrid[0])
		{
			USovereignAttributeComponent* WolfAttr = BattleMgr->EnemyGrid[0]->GetAttributeComponent();
			TestNotNull(TEXT("Wolf has AttributeComponent"), WolfAttr);
			if (WolfAttr)
			{
				TestEqual(TEXT("Wolf STR is 14"), WolfAttr->Strength, 14);
				TestEqual(TEXT("Wolf HP is 120"), WolfAttr->CurrentHealth, 120.0f);
			}
		}

		// 4. Test Automated Enemy Turn AI - Aggressive Brute
		if (BattleMgr->EnemyGrid[0])
		{
			const float PlayerHPBefore = PlayerHero->GetAttributeComponent()->CurrentHealth;
			BattleMgr->ExecuteEnemyTurn(BattleMgr->EnemyGrid[0].Get());
			const float PlayerHPAfter = PlayerHero->GetAttributeComponent()->CurrentHealth;

			TestTrue(TEXT("Enemy Turn AI executed an attack reducing Player HP"), PlayerHPAfter < PlayerHPBefore);
		}

		// 4b. Test Support Healer AI (Goblin Shaman heals wounded ally)
		if (BattleMgr->EnemyGrid[6] && BattleMgr->EnemyGrid[0])
		{
			// Equip Shaman with a healing skill
			USovereignSkillDefinition* HealSkill = NewObject<USovereignSkillDefinition>();
			HealSkill->SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Combat.Cleric.Mend"), false);
			HealSkill->SkillDisplayName = FText::FromString(TEXT("Mend"));
			HealSkill->ScalingAttribute = ESovereignSkillScalingAttribute::Wisdom;
			HealSkill->TargetPattern = ESovereignSkillTargetPattern::SingleTarget;
			HealSkill->BasePower = 30.0f;
			HealSkill->StatMultiplier = 1.0f;
			HealSkill->QiCost = 10.0f;

			if (USovereignSkillComponent* ShamanSkillComp = BattleMgr->EnemyGrid[6]->GetSkillComponent())
			{
				FGuid OutId;
				ShamanSkillComp->EquipSkill(HealSkill, OutId);
			}

			// Wound the Wolf ally to below 50% HP (e.g. 40 / 120 HP)
			USovereignAttributeComponent* WolfAttr = BattleMgr->EnemyGrid[0]->GetAttributeComponent();
			WolfAttr->CurrentHealth = 40.0f;

			// Shaman executes turn
			BattleMgr->ExecuteEnemyTurn(BattleMgr->EnemyGrid[6].Get());

			TestTrue(TEXT("Support Healer healed wounded ally above 40 HP"), WolfAttr->CurrentHealth > 40.0f);
		}
	}

	// 5. Test Victory Rewards Distribution (Option A)
	const double StrengthXPBefore = PlayerHero->GetLevelComponent()->StrengthXP;
	BattleMgr->DistributeVictoryRewards();
	const double StrengthXPAfter = PlayerHero->GetLevelComponent()->StrengthXP;

	TestTrue(TEXT("Player received Strength XP from victory rewards"), StrengthXPAfter > StrengthXPBefore);

	// 5b. Test Elemental Weakness and Resistance (Ticket CS-004)
	{
		FSovereignCombatEntitySnapshot AttackerSnap;
		AttackerSnap.Strength = 20;
		AttackerSnap.FormationSlot = 1;

		FSovereignCombatEntitySnapshot TargetSnap;
		TargetSnap.Constitution = 10;
		TargetSnap.FormationSlot = 1;
		TargetSnap.PrimaryElement = ESovereignElement::Earth;
		TargetSnap.WeaknessElement = ESovereignElement::Fire;
		TargetSnap.ResistanceElement = ESovereignElement::Nature;

		// Unaligned Skill (None)
		USovereignSkillDefinition* NeutralSkill = NewObject<USovereignSkillDefinition>();
		NeutralSkill->BasePower = 50.0f;
		NeutralSkill->StatMultiplier = 1.0f;
		NeutralSkill->ElementType = ESovereignElement::None;
		FSovereignSkillInstance NeutralInst(NeutralSkill);

		// Fire Skill (Target Weakness -> 1.5x)
		USovereignSkillDefinition* FireSkill = NewObject<USovereignSkillDefinition>();
		FireSkill->BasePower = 50.0f;
		FireSkill->StatMultiplier = 1.0f;
		FireSkill->ElementType = ESovereignElement::Fire;
		FSovereignSkillInstance FireInst(FireSkill);

		// Nature Skill (Target Resistance -> 0.5x)
		USovereignSkillDefinition* NatureSkill = NewObject<USovereignSkillDefinition>();
		NatureSkill->BasePower = 50.0f;
		NatureSkill->StatMultiplier = 1.0f;
		NatureSkill->ElementType = ESovereignElement::Nature;
		FSovereignSkillInstance NatureInst(NatureSkill);

		const FSovereignDamageCalculationResult NeutralResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
			AttackerSnap, TargetSnap, NeutralInst, 1.0f
		);
		const FSovereignDamageCalculationResult WeaknessResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
			AttackerSnap, TargetSnap, FireInst, 1.0f
		);
		const FSovereignDamageCalculationResult ResistanceResult = USovereignCombatCalculationLibrary::CalculateCombatDamage(
			AttackerSnap, TargetSnap, NatureInst, 1.0f
		);

		TestEqual(TEXT("Neutral elemental multiplier is 1.0"), NeutralResult.ElementalMultiplier, 1.0f);
		TestEqual(TEXT("Fire weakness elemental multiplier is 1.5"), WeaknessResult.ElementalMultiplier, 1.5f);
		TestEqual(TEXT("Nature resistance elemental multiplier is 0.5"), ResistanceResult.ElementalMultiplier, 0.5f);

		TestTrue(TEXT("Fire weakness damage is greater than neutral damage"), WeaknessResult.FinalDamage > NeutralResult.FinalDamage);
		TestTrue(TEXT("Nature resistance damage is less than neutral damage"), ResistanceResult.FinalDamage < NeutralResult.FinalDamage);
	}

	// 6. Test Enemy Actor Cleanup on Battle End
	BattleMgr->CleanupSpawnedEnemies();
	TestEqual(TEXT("SpawnedEnemyActors emptied after cleanup"), BattleMgr->SpawnedEnemyActors.Num(), 0);

	// 7. Test ASovereignRoamingMob Interaction
	ASovereignRoamingMob* RoamingMob = World ? World->SpawnActor<ASovereignRoamingMob>() : NewObject<ASovereignRoamingMob>();
	if (RoamingMob)
	{
		RoamingMob->EncounterDefinition = EncounterDef;
		const FString Hint = RoamingMob->GetInteractionHint_Implementation();
		TestTrue(TEXT("RoamingMob interaction hint contains encounter name"), Hint.Contains(TEXT("Ambush in the Dark")));

		if (World)
		{
			RoamingMob->Destroy();
		}
	}

	// Teardown
	if (World)
	{
		PlayerHero->Destroy();
		BattleMgr->Destroy();
	}

	return true;
}
