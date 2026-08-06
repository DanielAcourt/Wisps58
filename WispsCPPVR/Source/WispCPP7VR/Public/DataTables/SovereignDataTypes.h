// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SovereignDataTypes.generated.h"

// Forward declaration of your Advanced Data Asset
class USovereignSpeciesData;
/**
 * FItemDefaults
 * The "Baseline" Row Structure for your Master Item Table.
 */

USTRUCT(BlueprintType)
struct FSovereignItemRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// The primary identifier for the item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Core")
	FGameplayTag ItemTag;

	// Flexibility for the new system: handles attributes the system doesn't know yet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Core")
	FGameplayTagContainer AdditionalMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Core")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Physics")
	float Weight = 1.0f;

	/** * Connection to Advanced Species Data.
	 * Used for Erisis (the elf) or specific biological entities like Isla.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Evolution")
	TSoftObjectPtr<USovereignSpeciesData> SpeciesVesselData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Evolution")
	bool bIsBiologicalOrigin = false;


	//vERSION 3,2 iDEA
	/*
	UENUM(BlueprintType)
		enum class ESovereignAttribute : uint8
	{
		Strength,
		Dexterity,
		Constitution,
		Intelligence,
		Wisdom,
		Charisma,
		Luck,
		MAX // Used for array sizing
	};
	*/
};
