// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Combat/SovereignSkillDefinition.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignSkillAugment.generated.h"

/**
 * ESovereignAugmentType
 * Category of customization applied to a combat skill via Qi infusion.
 */
UENUM(BlueprintType)
enum class ESovereignAugmentType : uint8
{
	Potency            UMETA(DisplayName = "Potency (Damage & Multiplier Boost)"),
	ShapeSpread        UMETA(DisplayName = "Shape Spread (AOE / Rank Cleave)"),
	ElementalInfusion  UMETA(DisplayName = "Elemental Infusion (Draconic Essence)"),
	Efficiency         UMETA(DisplayName = "Efficiency (Cost / Cooldown Relief)")
};

/**
 * USovereignSkillAugment
 * Composable modification subobject attached to a runtime skill instance by spending Qi.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class WISPCPP7VR_API USovereignSkillAugment : public UObject
{
	GENERATED_BODY()

public:
	USovereignSkillAugment();

	/** Tag identifying the augment (e.g. Augment.Shape.RankCleave, Augment.Potency.Empower) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	FGameplayTag AugmentTag;

	/** User-facing display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	FText AugmentName;

	/** Type of modification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	ESovereignAugmentType AugmentType = ESovereignAugmentType::Potency;

	/** Permanent Qi cost deducted from entity's Qi pool to infuse this augment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	float QiCostToInfuse = 25.0f;

	/** Additional stat multiplier added to skill (e.g. +0.80 boosts 1.20 -> 2.00) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	float AdditionalStatMultiplier = 0.0f;

	/** Additional flat base power added to skill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	float AdditionalBasePower = 0.0f;

	/** Whether this augment overrides the base target pattern (e.g. SingleTarget -> FrontRowSweep) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	bool bOverrideTargetPattern = false;

	/** New target formation pattern if override is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment", meta = (EditCondition = "bOverrideTargetPattern"))
	ESovereignSkillTargetPattern NewTargetPattern = ESovereignSkillTargetPattern::FrontRowSweep;

	/** Draconic elemental type infused into the skill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill|Augment")
	ESovereignElement InfusedElement = ESovereignElement::None;
};

/**
 * FSovereignSkillInstance
 * Runtime representation of a character vessel's active skill, holding level and Qi augments.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignSkillInstance
{
	GENERATED_BODY()

	/** Unique runtime instance identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill", meta = (IgnoreForMemberInitializationTest))
	FGuid SkillInstanceId;

	/** Soft reference to the base immutable archetype definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill")
	TSoftObjectPtr<USovereignSkillDefinition> SkillDefinition;

	/** Skill upgrade rank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill")
	int32 SkillLevel = 1;

	/** Active modular augments infused into this skill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill")
	TArray<TObjectPtr<USovereignSkillAugment>> ActiveAugments;

	FSovereignSkillInstance()
		: SkillInstanceId(FGuid::NewGuid())
		, SkillLevel(1)
	{
	}

	FSovereignSkillInstance(USovereignSkillDefinition* InDef)
		: SkillInstanceId(FGuid::NewGuid())
		, SkillDefinition(InDef)
		, SkillLevel(1)
	{
	}

	/** Resolves effective target pattern considering any attached shape spread augments */
	ESovereignSkillTargetPattern GetEffectiveTargetPattern() const;

	/** Resolves effective stat multiplier combining base definition and augment bonuses */
	float GetEffectiveStatMultiplier() const;

	/** Resolves effective base power combining base definition and augment additions */
	float GetEffectiveBasePower() const;

	/** Calculates final total potency considering stat value, augments, and class multipliers */
	float CalculateFinalPotency(int32 StatValue, float ClassMultiplier = 1.0f) const;
};
