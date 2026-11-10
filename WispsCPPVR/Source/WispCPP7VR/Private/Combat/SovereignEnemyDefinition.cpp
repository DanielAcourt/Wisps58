// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignEnemyDefinition.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignSkillComponent.h"

USovereignEnemyDefinition::USovereignEnemyDefinition()
{
	EnemyDisplayName = FText::FromString(TEXT("Enemy"));
	CharacterClass = ASovereignBaseCharacter::StaticClass();
	MaxHealth = 100.0f;
	MaxQi = 50.0f;
	Strength = 10;
	Dexterity = 10;
	Constitution = 10;
	Intelligence = 10;
	Wisdom = 10;
	Charisma = 10;
	Luck = 10;
	ExperienceReward = 50.0f;
	ClassExperienceReward = 10;
	QiReward = 15.0f;
}

FPrimaryAssetId USovereignEnemyDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("SovereignEnemy")), GetFName());
}

void USovereignEnemyDefinition::InitializeEnemyVessel(ASovereignBaseCharacter* Vessel) const
{
	if (!Vessel)
	{
		return;
	}

	// 1. Initialize Attributes
	if (USovereignAttributeComponent* AttrComp = Vessel->GetAttributeComponent())
	{
		AttrComp->Strength = Strength;
		AttrComp->Dexterity = Dexterity;
		AttrComp->Constitution = Constitution;
		AttrComp->Intelligence = Intelligence;
		AttrComp->Wisdom = Wisdom;
		AttrComp->Charisma = Charisma;
		AttrComp->Luck = Luck;

		AttrComp->MaxHealth = MaxHealth;
		AttrComp->CurrentHealth = MaxHealth;
	}

	// 2. Initialize Qi
	if (USovereignQiComponent* QiComp = Vessel->GetQiComponent())
	{
		QiComp->MaxQi = MaxQi;
		QiComp->CurrentQi = MaxQi;
	}

	// 3. Initialize Skills
	if (USovereignSkillComponent* SkillComp = Vessel->GetSkillComponent())
	{
		SkillComp->EquippedSkills.Empty();
		for (const TSoftObjectPtr<USovereignSkillDefinition>& SoftSkill : EquippedSkills)
		{
			USovereignSkillDefinition* SkillDef = SoftSkill.Get();
			if (!SkillDef)
			{
				SkillDef = SoftSkill.LoadSynchronous();
			}
			if (SkillDef)
			{
				SkillComp->EquipSkill(SkillDef);
			}
		}
	}
}
