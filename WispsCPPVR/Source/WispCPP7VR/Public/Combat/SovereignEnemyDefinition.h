// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SaveSystem/SovereignGameData.h"
#include "Combat/SovereignSkillDefinition.h"
#include "SovereignEnemyDefinition.generated.h"

class ASovereignBaseCharacter;

/**
 * ESovereignEnemyRole
 * Tactical behavioral personality governing turn-based AI decision-making (Ticket CS-003).
 */
UENUM(BlueprintType)
enum class ESovereignEnemyRole : uint8
{
	Default           UMETA(DisplayName = "Default (Balanced)"),
	AggressiveBrute   UMETA(DisplayName = "Aggressive Brute (Focus Frontline)"),
	SkirmisherSniper  UMETA(DisplayName = "Skirmisher / Sniper (Execute Weakest / Rear)"),
	SupportHealer     UMETA(DisplayName = "Support / Healer (Prioritize Wounded Allies)"),
	TacticalLeader    UMETA(DisplayName = "Tactical Leader (AoE / Highest Threat)")
};

/**
 * USovereignEnemyDefinition
 * Primary DataAsset defining an enemy combatant archetype.
 * Specifies the character vessel actor class, visual identity, base combat attributes,
 * equipped skills for battle AI, and EXP/Qi rewards granted to the player party upon defeat.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API USovereignEnemyDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USovereignEnemyDefinition();

	/** Unique tag identifying this enemy archetype (e.g. Enemy.Beast.Wolf, Enemy.Undead.Skeleton) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Identity")
	FGameplayTag EnemyTag;

	/** User-facing display name of the enemy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Display")
	FText EnemyDisplayName;

	/** Tactical AI role and behavioral personality in battle (Ticket CS-003) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Behavior")
	ESovereignEnemyRole EnemyRole = ESovereignEnemyRole::Default;

	/** Actor class to instantiate for this enemy in the battle arena (inherits ASovereignBaseCharacter) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Spawning")
	TSubclassOf<ASovereignBaseCharacter> CharacterClass;

	// --- Elemental Affinities (Ticket CS-004) ---

	/** Primary element this enemy is aligned with */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Elemental")
	ESovereignElement PrimaryElement = ESovereignElement::None;

	/** Element this enemy takes 1.5x damage from */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Elemental")
	ESovereignElement WeaknessElement = ESovereignElement::None;

	/** Element this enemy mitigates 0.5x damage from */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Elemental")
	ESovereignElement ResistanceElement = ESovereignElement::None;

	// --- Combat Attributes ---

	/** Base Health Points */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	float MaxHealth = 100.0f;

	/** Base Qi Pool */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	float MaxQi = 50.0f;

	/** D&D Core Attributes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Strength = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Dexterity = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Constitution = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Intelligence = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Wisdom = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Charisma = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Attributes")
	int32 Luck = 10;

	// --- Combat Skills ---

	/** Skills available to the enemy AI during turn-based combat */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Skills")
	TArray<TSoftObjectPtr<USovereignSkillDefinition>> EquippedSkills;

	// --- Victory Rewards ---

	/** Total Experience Points (EXP) awarded to the party upon defeating this enemy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Rewards")
	float ExperienceReward = 50.0f;

	/** Class / Job Points (CP/XP) awarded to equipped classes upon defeating this enemy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Rewards")
	int32 ClassExperienceReward = 10;

	/** Spiritual Qi awarded to the party upon defeat */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Enemy|Rewards")
	float QiReward = 15.0f;

	/** Asset Manager identifier */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Initializes an instantiated character vessel with this definition's attributes,
	 * health, Qi, and equipped skills.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Enemy")
	void InitializeEnemyVessel(ASovereignBaseCharacter* Vessel) const;
};
