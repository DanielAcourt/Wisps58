// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Combat/SovereignEnemyDefinition.h"
#include "SovereignEncounterDefinition.generated.h"

/**
 * FSovereignEncounterSlot
 * Maps an enemy archetype definition to a specific slot (0-8) in the 3x3 enemy rank formation.
 * Slots 0-2: Front Rank
 * Slots 3-5: Mid Rank
 * Slots 6-8: Rear Rank
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignEncounterSlot
{
	GENERATED_BODY()

	/** Slot index in the 3x3 enemy grid (0 to 8) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Formation", meta = (ClampMin = "0", ClampMax = "8"))
	int32 SlotIndex = 0;

	/** Enemy definition assigned to this formation slot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Formation")
	TObjectPtr<USovereignEnemyDefinition> EnemyDefinition = nullptr;
};

/**
 * USovereignEncounterDefinition
 * Primary DataAsset defining a tactical enemy combat group in the procedural dungeon.
 * Specifies enemy formation slot assignments across the 3x3 grid, escapability,
 * encounter tags, and aggregate victory bonuses.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API USovereignEncounterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USovereignEncounterDefinition();

	/** Unique tag identifying this encounter group (e.g. Encounter.Dungeon.WolfPack) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Identity")
	FGameplayTag EncounterTag;

	/** User-facing display title for this battle (e.g. "Ambush: Dire Wolves") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Display")
	FText EncounterDisplayName;

	/** Array of enemy slot configurations across the 3x3 grid */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Formation")
	TArray<FSovereignEncounterSlot> EnemySlots;

	/** Whether the player party can flee from this encounter (false for bosses/elites) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Rules")
	bool bCanFlee = true;

	/** Minimum dungeon depth required for this encounter to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Rules")
	int32 MinDungeonDepth = 0;

	/** Relative weight when picking random encounters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Encounter|Rules")
	float SpawnWeight = 100.0f;

	/** Asset Manager identifier */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Helper: calculates total base EXP reward from all assigned enemies in the encounter */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Encounter")
	float CalculateTotalExperience() const;

	/** Helper: calculates total base Class EXP reward from all assigned enemies in the encounter */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Encounter")
	int32 CalculateTotalClassExperience() const;

	/** Helper: calculates total base Qi reward from all assigned enemies in the encounter */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Encounter")
	float CalculateTotalQi() const;
};
