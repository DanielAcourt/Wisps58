// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Inventory/SovereignItemFragment.h"
#include "GameplayTagContainer.h"
#include "DataTables/SovereignDataTypes.h"
#include "SovereignItemDomainFragments.generated.h"

class UTexture2D;
class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;

/**
 * USovereignItemFragment_UIData
 * Visual and textual representation of an item for UMG grids, tooltips, and inspection cards.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "UI & Display Fragment"))
class WISPCPP7VR_API USovereignItemFragment_UIData : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|UI")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|UI")
	FText LoreText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|UI")
	FGameplayTag RarityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|UI")
	bool bShowInQuickBar = true;
};

/**
 * USovereignItemFragment_Weight
 * Manages item mass, stack limits, and encumbrance metrics.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Weight & Stacking Fragment"))
class WISPCPP7VR_API USovereignItemFragment_Weight : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Physics")
	float UnitWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Physics")
	int32 MaxStackSize = 99;

	UFUNCTION(BlueprintPure, Category = "Sovereign|Physics")
	float GetStackWeight(int32 Quantity) const
	{
		return UnitWeight * FMath::Max(1, Quantity);
	}
};

/**
 * USovereignItemFragment_Consumable
 * Replenishes biological vitals on USovereignBioComponent and awards stat XP to USovereignLevelComponent.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Consumable & Metabolic Fragment"))
class WISPCPP7VR_API USovereignItemFragment_Consumable : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	// Vitals Replenishment
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Bio")
	float HungerRestore = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Bio")
	float HydrationRestore = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Bio")
	float FatigueRelief = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Bio")
	float ToxicityIncrease = 0.0f;

	// D&D Attribute Experience Gains
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double StrengthXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double DexterityXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double ConstitutionXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double IntelligenceXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double WisdomXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double CharismaXP = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Experience")
	double LuckXP = 0.0;

	virtual bool OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const override;
};

/**
 * USovereignItemFragment_Spiritual
 * Modifies Qi flow, grants instant spiritual energy, and interfaces with USovereignQiComponent.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Spiritual & Qi Fragment"))
class WISPCPP7VR_API USovereignItemFragment_Spiritual : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Qi")
	float QiGive = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Qi")
	float QiTake = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Qi")
	float QiAuraRadiusBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Qi")
	ESovereignTrainingFocus SynergyFocus = ESovereignTrainingFocus::None;

	virtual bool OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const override;
};

/**
 * USovereignItemFragment_Equippable
 * Handles 3D visual equipment attachment to skeletal sockets on character vessels.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Equipment & Visual Fragment"))
class WISPCPP7VR_API USovereignItemFragment_Equippable : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Equipment")
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Equipment")
	TSoftObjectPtr<UStaticMesh> StaticMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Equipment")
	TSoftObjectPtr<UMaterialInterface> MaterialOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Equipment")
	FName SocketName;

	virtual bool OnEquipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const override;
	virtual bool OnUnequipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const override;
};

/**
 * USovereignItemFragment_CombatEffect
 * Handles turn-based battle item actions: offensive throwable grenades/alchemy flasks
 * and tactical combat restoration targeting friendly allies or opposing enemy ranks.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Combat Effect & Grenade Fragment"))
class WISPCPP7VR_API USovereignItemFragment_CombatEffect : public USovereignItemFragment
{
	GENERATED_BODY()

public:
	/** Direct flat damage dealt to target health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Combat")
	float DirectDamage = 0.0f;

	/** Direct flat health restored to target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Combat")
	float DirectHealing = 0.0f;

	/** Whether this item can legally target opposing enemy combatants (e.g. Spirit Grenade) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Combat")
	bool bCanTargetEnemies = false;

	/** Whether this item can legally target friendly party combatants (e.g. Healing Draught) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Combat")
	bool bCanTargetAllies = true;

	virtual bool OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const override;
};
