// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignEncounterDefinition.h"

USovereignEncounterDefinition::USovereignEncounterDefinition()
{
	EncounterDisplayName = FText::FromString(TEXT("Enemy Encounter"));
	bCanFlee = true;
	MinDungeonDepth = 0;
	SpawnWeight = 100.0f;
}

FPrimaryAssetId USovereignEncounterDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("SovereignEncounter")), GetFName());
}

float USovereignEncounterDefinition::CalculateTotalExperience() const
{
	float TotalExp = 0.0f;
	for (const FSovereignEncounterSlot& Slot : EnemySlots)
	{
		if (Slot.EnemyDefinition)
		{
			TotalExp += Slot.EnemyDefinition->ExperienceReward;
		}
	}
	return TotalExp;
}

int32 USovereignEncounterDefinition::CalculateTotalClassExperience() const
{
	int32 TotalClassExp = 0;
	for (const FSovereignEncounterSlot& Slot : EnemySlots)
	{
		if (Slot.EnemyDefinition)
		{
			TotalClassExp += Slot.EnemyDefinition->ClassExperienceReward;
		}
	}
	return TotalClassExp;
}

float USovereignEncounterDefinition::CalculateTotalQi() const
{
	float TotalQi = 0.0f;
	for (const FSovereignEncounterSlot& Slot : EnemySlots)
	{
		if (Slot.EnemyDefinition)
		{
			TotalQi += Slot.EnemyDefinition->QiReward;
		}
	}
	return TotalQi;
}
