// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignEnemyDefinition.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignSaveableEntityComponent.h"


#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignElementComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignSkillComponent.h"

USovereignEnemyDefinition::USovereignEnemyDefinition()
{
	EnemyDisplayName = FText::FromString(TEXT("Enemy"));
	EnemyRole = ESovereignEnemyRole::Default;
	PrimaryElement = ESovereignElement::None;
	WeaknessElement = ESovereignElement::None;
	ResistanceElement = ESovereignElement::None;
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
	InitializeEnemyVesselScaled(Vessel, 1.0f);
}

void USovereignEnemyDefinition::InitializeEnemyVesselScaled(ASovereignBaseCharacter* Vessel, float StatMultiplier) const
{
	if (!Vessel)
	{
		return;
	}

	const float ClampedMult = FMath::Max(0.1f, StatMultiplier);
	const float ScaledMaxHealth = (MaxHealth > 0.0f ? MaxHealth : (Constitution * 10.0f)) * ClampedMult;
	const float ScaledMaxQi = MaxQi * ClampedMult;

	// 1. Initialize Attributes Component (MaxHealth, CurrentHealth, and D&D attributes)
	if (USovereignAttributeComponent* AttrComp = Vessel->GetAttributeComponent())
	{
		AttrComp->InitializeAttributes(
			Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma, Luck,
			ScaledMaxHealth, 1.0f);
	}

	// 2. Initialize Qi Component
	if (USovereignQiComponent* QiComp = Vessel->GetQiComponent())
	{
		QiComp->MaxQiCapacity = FMath::Max(10.0f, ScaledMaxQi);
		QiComp->CurrentQi = QiComp->MaxQiCapacity;
		QiComp->bIsLoadedFromSave = true;
	}

	// 3. Initialize Element Affinities
	if (USovereignElementComponent* ElemComp = Vessel->GetElementComponent())
	{
		if (PrimaryElement != ESovereignElement::None)
		{
			ElemComp->BodySocket = PrimaryElement;
		}
	}

	// 4. Resolve soft skill references in EquippedSkills and apply on SkillComponent
	if (USovereignSkillComponent* SkillComp = Vessel->GetSkillComponent())
	{
		for (const TSoftObjectPtr<USovereignSkillDefinition>& SkillRef : EquippedSkills)
		{
			if (!SkillRef.IsNull())
			{
				if (USovereignSkillDefinition* SkillDef = SkillRef.LoadSynchronous())
				{
					FGuid OutInstanceId;
					SkillComp->EquipSkill(SkillDef, OutInstanceId);
				}
			}
		}
	}
}