// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Combat/SovereignSkillDefinition.h"
#include "SovereignClassDefinition.generated.h"

class ASovereignBaseCharacter;

/**
 * FSovereignClassPrerequisite
 * Specifies a class level milestone required to unlock a combat class.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignClassPrerequisite
{
	GENERATED_BODY()

	/** Required class tag (e.g. Class.Combat.Rogue, Class.Combat.Warrior) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisite")
	FGameplayTag RequiredClassTag;

	/** Minimum level in that class required to unlock (default 1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisite")
	int32 RequiredLevel = 1;

	FSovereignClassPrerequisite()
		: RequiredLevel(1)
	{
	}

	FSovereignClassPrerequisite(const FGameplayTag& InTag, int32 InLevel)
		: RequiredClassTag(InTag)
		, RequiredLevel(InLevel)
	{
	}
};

/**
 * USovereignClassDefinition
 * Primary DataAsset defining a combat class specialization job, its CP cost,
 * stat multipliers, prerequisites to unlock, and granted skills.
 * Registered with Unreal AssetManager as PrimaryAssetType "SovereignClass".
 */
UCLASS(BlueprintType, Const)
class WISPCPP7VR_API USovereignClassDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USovereignClassDefinition();

	/** Unique tag identifying the combat class (e.g. Class.Combat.Warrior, Class.Combat.Ninja) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Identity")
	FGameplayTag ClassTag;

	/** User-facing display title */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Display")
	FText ClassDisplayName;

	/** Lore and role description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Display")
	FText ClassDescription;

	/** Class Points (CP) cost required to equip this job simultaneously */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|CP")
	int32 CPCost = 1;

	// D&D Attribute Multipliers applied during turn-based combat
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Multipliers")
	float StrengthMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Multipliers")
	float DexterityMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Multipliers")
	float ConstitutionMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Multipliers")
	float IntelligenceMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Multipliers")
	float WisdomMultiplier = 1.0f;

	/** Prerequisites: Required prior class levels (e.g. Rogue Level 5, Warrior Level 5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	TArray<FSovereignClassPrerequisite> PrerequisiteClasses;

	/** Minimum attribute requirements to unlock this class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	int32 RequiredStrength = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	int32 RequiredDexterity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	int32 RequiredConstitution = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	int32 RequiredIntelligence = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Prerequisites")
	int32 RequiredWisdom = 0;

	/** Default or archetype skills granted when unlocking/equipping this class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Class|Skills")
	TArray<TSoftObjectPtr<USovereignSkillDefinition>> GrantedSkills;

	/** UPrimaryDataAsset Interface: Provides asset manager ID */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Evaluates whether a given character satisfies all prerequisites to unlock this class.
	 * Checks both prior class levels on USovereignClassComponent and attributes on USovereignAttributeComponent.
	 * @param Character The character vessel attempting to unlock the class.
	 * @return True if all prerequisites and attribute minimums are satisfied.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|Prerequisites")
	bool CanUnlockClass(const ASovereignBaseCharacter* Character) const;
};
