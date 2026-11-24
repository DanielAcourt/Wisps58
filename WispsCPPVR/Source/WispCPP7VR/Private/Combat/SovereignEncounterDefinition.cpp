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

static void LogRandomEncounterSummary(
	int32 Seed,
	int32 InitialBudget,
	int32 RemainingBudget,
	int32 PoolCount,
	int32 TransformCount,
	float StatMultiplier,
	const TArray<ASovereignBaseCharacter*>& SpawnedEnemies,
	const TArray<FString>& SpawnedLines)
{
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("⚔️  [SOVEREIGN ENCOUNTER SPAWNER] PARTY GENERATION COMPLETE (Seed %d, Buff x%.2f)"),
		Seed, StatMultiplier);
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("  Spawned Count: %d | Transforms Provided: %d | Candidate Pool Entries: %d"),
		SpawnedEnemies.Num(), TransformCount, PoolCount);
	UE_LOG(LogTemp, Display, TEXT("  Budget: %d -> Remaining: %d (Spent: %d)"),
		InitialBudget, RemainingBudget, FMath::Max(0, InitialBudget - RemainingBudget));
	UE_LOG(LogTemp, Display, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(LogTemp, Display, TEXT("  👾 SPAWNED ENEMY ROSTER (%d Total):"), SpawnedEnemies.Num());

	if (SpawnedLines.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("    • [NONE SPAWNED] Check CandidatePool entries, EncounterBudget, or SpawnTransforms.Num()!"));
	}
	else
	{
		for (const FString& Line : SpawnedLines)
		{
			UE_LOG(LogTemp, Display, TEXT("    • %s"), *Line);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
}