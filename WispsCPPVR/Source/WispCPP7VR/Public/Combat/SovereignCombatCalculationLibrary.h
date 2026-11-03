// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Combat/SovereignSkillAugment.h"
#include "SovereignCombatCalculationLibrary.generated.h"

class ASovereignBaseCharacter;

/**
 * ESovereignBattleRank
 * Formation ranks in the 3x3 tactical battle matrix.
 */
UENUM(BlueprintType)
enum class ESovereignBattleRank : uint8
{
	Front    UMETA(DisplayName = "Front Rank (Slots 1-3)"),
	Mid      UMETA(DisplayName = "Mid Rank (Slots 4-6)"),
	Rear     UMETA(DisplayName = "Rear Rank (Slots 7-9)")
};

/**
 * FSovereignCombatEntitySnapshot
 * Lightweight data snapshot of an entity's combat attributes and formation position.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignCombatEntitySnapshot
{
	GENERATED_BODY()

	/** Slot index in the 3x3 Rank Matrix (1 to 9, or 0 if unassigned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 FormationSlot = 1;

	/** Whether this entity is currently alive and active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	bool bIsAlive = true;

	/** Primary physical attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Strength = 10;

	/** Finesse and turn speed attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Dexterity = 10;

	/** Durability and physical armor attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Constitution = 10;

	/** Arcane spell scaling attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Intelligence = 10;

	/** Spiritual Qi and healing attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Wisdom = 10;

	/** Critical strike chance and proc attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	int32 Luck = 10;

	/** Flat armor / physical defense bonus from equipment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float EquipmentDefense = 0.0f;

	/** Total damage multiplier from equipped combat classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float ClassDamageMultiplier = 1.0f;

	FSovereignCombatEntitySnapshot()
		: FormationSlot(1)
		, bIsAlive(true)
		, Strength(10)
		, Dexterity(10)
		, Constitution(10)
		, Intelligence(10)
		, Wisdom(10)
		, Luck(10)
		, EquipmentDefense(0.0f)
		, ClassDamageMultiplier(1.0f)
	{
	}

	/** Returns which rank (Front, Mid, Rear) this slot belongs to */
	ESovereignBattleRank GetRank() const;
};

/**
 * FSovereignDamageCalculationResult
 * Output bundle containing detailed breakdown of calculated combat values.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDamageCalculationResult
{
	GENERATED_BODY()

	/** Raw attack potency output before defense mitigation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float RawPower = 0.0f;

	/** Attacker offensive rank modifier applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float AttackerRankModifier = 1.0f;

	/** Target defensive rank modifier applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float TargetDefenseModifier = 1.0f;

	/** Total calculated defense score used in percentage reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float EffectiveDefense = 0.0f;

	/** Mitigation factor (between 0.0 and 1.0) applied to incoming power: 100 / (100 + Def) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float MitigationFactor = 1.0f;

	/** Whether this calculation resulted in a critical strike */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	bool bIsCriticalHit = false;

	/** Final net damage floored at minimum 1.0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Combat")
	float FinalDamage = 1.0f;

	FSovereignDamageCalculationResult()
		: RawPower(0.0f)
		, AttackerRankModifier(1.0f)
		, TargetDefenseModifier(1.0f)
		, EffectiveDefense(0.0f)
		, MitigationFactor(1.0f)
		, bIsCriticalHit(false)
		, FinalDamage(1.0f)
	{
	}
};

/**
 * USovereignCombatCalculationLibrary
 * Deterministic static calculation engine for turn-based combat damage and 3x3 rank matrix target evaluation.
 */
UCLASS()
class WISPCPP7VR_API USovereignCombatCalculationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Converts a 1-based slot index (1 to 9) to its corresponding ESovereignBattleRank */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Formation")
	static ESovereignBattleRank GetRankForSlot(int32 SlotIndex);

	/** Converts a 1-based slot index (1 to 9) to its column index (0, 1, or 2) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Formation")
	static int32 GetColumnForSlot(int32 SlotIndex);

	/** Returns the offensive damage multiplier granted by the attacker's formation rank */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Formation")
	static float GetAttackerRankModifier(ESovereignBattleRank AttackerRank, ESovereignSkillScalingAttribute ScalingAttr);

	/** Returns the defensive multiplier granted by the target's formation rank */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Formation")
	static float GetTargetRankDefenseModifier(ESovereignBattleRank TargetRank);

	/**
	 * Extracts the relevant offensive D&D attribute value from an entity snapshot based on skill scaling.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Calculation")
	static int32 GetRelevantAttributeValue(const FSovereignCombatEntitySnapshot& Attacker, ESovereignSkillScalingAttribute ScalingAttr);

	/**
	 * Calculates total physical defense from Constitution, equipment, and rank modifiers.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Calculation")
	static float CalculateEffectiveDefense(const FSovereignCombatEntitySnapshot& Target);

	/**
	 * Evaluates critical hit chance based on Attacker and Target Luck attributes.
	 * Formula: Base 5% + (Attacker.Luck - Target.Luck) * 0.5%, clamped between 5% and 50%.
	 * @param Attacker Attacker snapshot.
	 * @param Target Target snapshot.
	 * @param ForcedRoll Optional deterministic roll between 0.0 and 1.0 (-1.0 uses FMath::FRand).
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Calculation")
	static bool EvaluateCriticalHit(const FSovereignCombatEntitySnapshot& Attacker, const FSovereignCombatEntitySnapshot& Target, float ForcedRoll = -1.0f);

	/**
	 * Calculates complete turn-based damage using diminishing returns mitigation curve:
	 * FinalDamage = (RawPower * AttackerRankModifier) * (100 / (100 + EffectiveDefense)) * CritMultiplier
	 * @param Attacker Attacker snapshot.
	 * @param Target Target snapshot.
	 * @param SkillInstance Active skill instance being executed.
	 * @param ForcedCritRoll Optional deterministic roll for unit testing (-1.0 uses random).
	 * @return Detailed calculation result bundle.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Calculation")
	static FSovereignDamageCalculationResult CalculateCombatDamage(
		const FSovereignCombatEntitySnapshot& Attacker,
		const FSovereignCombatEntitySnapshot& Target,
		const FSovereignSkillInstance& SkillInstance,
		float ForcedCritRoll = -1.0f);

	/**
	 * Evaluates whether a target slot in a 9-slot defending formation is valid and legally targetable.
	 * Enforces rank-based protection: Rear rank cannot be targeted by single-target melee strikes
	 * if any living combatants remain in the Front or Mid ranks.
	 *
	 * @param TargetSlot The slot being targeted (1 to 9).
	 * @param TargetFormation Array of snapshots representing defending formation (indexed 1..9, or 0..8).
	 * @param SkillPattern Target pattern of the skill being executed.
	 * @param ScalingAttr Attribute type driving the skill (Spells and Ranged bypass front-line melee obstruction).
	 * @return True if the target slot is legally targetable under formation rules.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Combat|Targeting")
	static bool IsTargetSlotLegal(
		int32 TargetSlot,
		const TArray<FSovereignCombatEntitySnapshot>& TargetFormation,
		ESovereignSkillTargetPattern SkillPattern,
		ESovereignSkillScalingAttribute ScalingAttr);

	/**
	 * Resolves all valid target snapshots from a defending formation based on the skill pattern and primary target slot.
	 * For SingleTarget: returns just the primary target if legal.
	 * For FrontRowSweep: returns all living units in the foremost active rank.
	 * For ColumnPierce: returns all living units sharing the column of the primary target.
	 * For FullMatrixAOE: returns all living units in the defending formation.
	 *
	 * @param PrimaryTargetSlot Desired primary target slot (1 to 9).
	 * @param DefendingFormation Defending formation snapshots.
	 * @param SkillPattern Area-of-effect pattern.
	 * @param ScalingAttr Attribute scaling.
	 * @param OutTargetSlots Resulting array of valid target slot indices (1 to 9).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Combat|Targeting")
	static void ResolveTargetSlots(
		int32 PrimaryTargetSlot,
		const TArray<FSovereignCombatEntitySnapshot>& DefendingFormation,
		ESovereignSkillTargetPattern SkillPattern,
		ESovereignSkillScalingAttribute ScalingAttr,
		TArray<int32>& OutTargetSlots);

	/**
	 * Helper function to create an entity snapshot directly from a live ASovereignBaseCharacter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Combat|Utility")
	static FSovereignCombatEntitySnapshot CreateSnapshotFromCharacter(ASovereignBaseCharacter* Character, int32 FormationSlot = 1);
};
