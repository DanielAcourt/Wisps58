// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DataTables/SovereignSpeciesData.h"
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

	// --- SPECIES TEMPLATE FIELDS ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity")
	FGameplayTag SpeciesTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Spawning")
	TSoftClassPtr<AActor> ActorClass;

	// Physical & Mental Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseStrength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseDexterity = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseConstitution = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseIntelligence = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseWisdom = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseCharisma = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Attributes")
	int32 BaseLuck = 10;

	// Biological Rates & Vitals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bio")
	float MetabolicRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bio")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bio")
	float MaxHydration = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bio")
	float GestationRate = 0.5f;

	// Capability & Trait Flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Capabilities")
	bool bCanFly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Capabilities")
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Capabilities")
	bool bIsBiological = true;

	// Growth Curve Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Growth")
	ESovereignGrowthCurve GrowthCurveType = ESovereignGrowthCurve::Exponential;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Growth")
	float GrowthScaleFactor = 1.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity")
	FGameplayTagContainer SpeciesTraits;


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
