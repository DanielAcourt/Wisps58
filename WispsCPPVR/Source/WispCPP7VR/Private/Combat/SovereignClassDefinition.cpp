// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21

#include "Combat/SovereignClassDefinition.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignClassComponent.h"
#include "Components/SovereignAttributeComponent.h"

USovereignClassDefinition::USovereignClassDefinition()
	: CPCost(1)
	, StrengthMultiplier(1.0f)
	, DexterityMultiplier(1.0f)
	, ConstitutionMultiplier(1.0f)
	, IntelligenceMultiplier(1.0f)
	, WisdomMultiplier(1.0f)
	, RequiredStrength(0)
	, RequiredDexterity(0)
	, RequiredConstitution(0)
	, RequiredIntelligence(0)
	, RequiredWisdom(0)
{
}

FPrimaryAssetId USovereignClassDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("SovereignClass"), GetFName());
}

bool USovereignClassDefinition::CanUnlockClass(const ASovereignBaseCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	// 1. Check Attribute Thresholds via USovereignAttributeComponent
	if (const USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent())
	{
		if (RequiredStrength > 0 && AttrComp->GetStrength() < RequiredStrength)
		{
			return false;
		}
		if (RequiredDexterity > 0 && AttrComp->GetDexterity() < RequiredDexterity)
		{
			return false;
		}
		if (RequiredConstitution > 0 && AttrComp->GetConstitution() < RequiredConstitution)
		{
			return false;
		}
		if (RequiredIntelligence > 0 && AttrComp->GetIntelligence() < RequiredIntelligence)
		{
			return false;
		}
		if (RequiredWisdom > 0 && AttrComp->GetWisdom() < RequiredWisdom)
		{
			return false;
		}
	}
	else if (RequiredStrength > 0 || RequiredDexterity > 0 || RequiredConstitution > 0 || RequiredIntelligence > 0 || RequiredWisdom > 0)
	{
		// Has attribute requirements but character lacks attribute component
		return false;
	}

	// 2. Check Prerequisite Classes & Levels via USovereignClassComponent
	if (PrerequisiteClasses.Num() > 0)
	{
		const USovereignClassComponent* ClassComp = Character->GetClassComponent();
		if (!ClassComp)
		{
			return false;
		}

		for (const FSovereignClassPrerequisite& Prereq : PrerequisiteClasses)
		{
			if (!Prereq.RequiredClassTag.IsValid())
			{
				continue;
			}

			const FSovereignClassData* FoundData = ClassComp->UnlockedClasses.Find(Prereq.RequiredClassTag);
			if (!FoundData)
			{
				return false;
			}

			if (FoundData->ClassLevel < Prereq.RequiredLevel)
			{
				return false;
			}
		}
	}

	return true;
}
