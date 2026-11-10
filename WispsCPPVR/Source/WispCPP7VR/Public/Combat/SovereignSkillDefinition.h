// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SaveSystem/SovereignGameData.h"
#include "Components/SovereignStatusComponent.h"
#include "SovereignSkillDefinition.generated.h"

class UTexture2D;

/**
 * ESovereignSkillTargetPattern
 * Defines the formation area of effect for combat skills in the 3x3 Rank Matrix.
 */
UENUM(BlueprintType)
enum class ESovereignSkillTargetPattern : uint8
{
	SingleTarget    UMETA(DisplayName = "Single Target"),
	FrontRowSweep   UMETA(DisplayName = "Front Rank Sweep (Cleave 3)"),
	ColumnPierce    UMETA(DisplayName = "Column Pierce (Front to Rear)"),
	FullMatrixAOE   UMETA(DisplayName = "Full Matrix AOE (All 9)"),
	SelfOnly        UMETA(DisplayName = "Self Only"),
	AllAllies       UMETA(DisplayName = "All Allies")
};

/**
 * ESovereignSkillScalingAttribute
 * Identifies the primary D&D stat driving skill damage and potency.
 */
UENUM(BlueprintType)
enum class ESovereignSkillScalingAttribute : uint8
{
	Strength        UMETA(DisplayName = "Strength (Physical Melee)"),
	Dexterity       UMETA(DisplayName = "Dexterity (Ranged / Finesse)"),
	Intelligence    UMETA(DisplayName = "Intelligence (Arcane / Spells)"),
	Wisdom          UMETA(DisplayName = "Wisdom (Qi / Healing)"),
	Constitution    UMETA(DisplayName = "Constitution (Shield / Defend)")
};

/**
 * USovereignSkillDefinition
 * Primary DataAsset defining a data-driven combat ability or spell.
 * Registered with Unreal's AssetManager as PrimaryAssetType "SovereignSkill".
 */
UCLASS(BlueprintType, Const)
class WISPCPP7VR_API USovereignSkillDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USovereignSkillDefinition();

	/** Primary GameplayTag identifying this skill (e.g. Skill.Combat.Warrior.Strike) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Identity")
	FGameplayTag SkillTag;

	/** User-facing display title */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Display")
	FText SkillDisplayName;

	/** Description and tooltip text */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Display")
	FText SkillDescription;

	/** UI Icon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Display")
	TSoftObjectPtr<UTexture2D> SkillIcon;

	/** Target formation pattern in the 3x3 Rank Matrix */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Targeting")
	ESovereignSkillTargetPattern TargetPattern = ESovereignSkillTargetPattern::SingleTarget;

	/** Which D&D attribute scales this ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Scaling")
	ESovereignSkillScalingAttribute ScalingAttribute = ESovereignSkillScalingAttribute::Strength;

	/** Elemental affinity of this skill (Fire, Water, Earth, etc.) - Ticket CS-004 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Elemental")
	ESovereignElement ElementType = ESovereignElement::None;

	/** Optional status effect inflicted on targets upon execution (Ticket CS-005) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Status")
	ESovereignStatusType InflictedStatus = ESovereignStatusType::None;

	/** Duration in combat turns for the inflicted status effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Status")
	int32 StatusDurationTurns = 3;

	/** Potency value for the inflicted status (e.g. poison damage amount) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Status")
	float StatusPotency = 10.0f;

	/** Base ability power before stat scaling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Scaling")
	float BasePower = 20.0f;

	/** Multiplier applied to the scaling attribute (e.g. 1.20 = 120% of stat) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Scaling")
	float StatMultiplier = 1.20f;

	/** Spiritual Qi consumed on execution */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Cost")
	float QiCost = 0.0f;

	/** Cooldown in combat turns (0 = usable every turn) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Skill|Cost")
	int32 CooldownTurns = 0;

	/** UPrimaryDataAsset Interface: Provides asset manager ID */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Calculates raw ability power output: (BasePower + StatValue * StatMultiplier) * ClassMultiplier
	 * @param StatValue The caster's relevant D&D attribute value (e.g. Strength or Intelligence).
	 * @param ClassMultiplier Total multiplier from equipped jobs on USovereignClassComponent.
	 * @return Calculated raw ability potency before target defense mitigation.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Skill|Calculation")
	float CalculateRawPower(int32 StatValue, float ClassMultiplier = 1.0f) const;
};
