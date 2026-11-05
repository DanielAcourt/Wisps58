// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "GameplayTagContainer.h"
#include "SovereignClassComponent.generated.h"

class FJsonObject;

/**
 * FSovereignClassData
 * Represents an unlocked combat job/class, its level, CP cost, and combat multipliers.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignClassData
{
	GENERATED_BODY()

	/** Unique tag identifying the combat class (e.g. Class.Combat.Warrior, Class.Combat.Ninja) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class")
	FGameplayTag ClassTag;

	/** Display Name of the class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class")
	FText ClassDisplayName;

	/** Level in this specific class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class")
	int32 ClassLevel = 1;

	/** Class Points (CP) cost required to equip this job simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class")
	int32 CPCost = 1;

	// D&D Attribute Multipliers applied during turn-based combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|Multipliers")
	float StrengthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|Multipliers")
	float DexterityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|Multipliers")
	float ConstitutionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|Multipliers")
	float IntelligenceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|Multipliers")
	float WisdomMultiplier = 1.0f;

	FSovereignClassData()
		: ClassLevel(1)
		, CPCost(1)
	{
	}
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatClassUnlocked, const FGameplayTag&, ClassTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatClassEquipped, const FGameplayTag&, ClassTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatClassUnequipped, const FGameplayTag&, ClassTag);

/**
 * USovereignClassComponent
 * Manages combat specialization classes, multi-classing capacity via Class Points (CP),
 * active combat multipliers, and persistent class progression.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignClassComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignClassComponent();

	/** Maximum Class Points (CP) capacity for simultaneous multi-classing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Class|CP")
	int32 MaxCP = 5;

	/** All unlocked classes available to this character vessel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Class")
	TMap<FGameplayTag, FSovereignClassData> UnlockedClasses;

	/** Currently equipped classes actively contributing to CP and combat multipliers */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Class")
	FGameplayTagContainer EquippedClasses;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Class|Events")
	FOnCombatClassUnlocked OnClassUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Class|Events")
	FOnCombatClassEquipped OnClassEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Class|Events")
	FOnCombatClassUnequipped OnClassUnequipped;

	/** --- 1. CP CALCULATIONS --- */

	/** Returns total CP currently consumed by equipped classes */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|CP")
	int32 GetUsedCP() const;

	/** Returns remaining available CP */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|CP")
	int32 GetAvailableCP() const;

	/** --- 2. CLASS MANAGEMENT --- */

	/** Unlocks a class, registering its defaults if not already known */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Class")
	bool UnlockClass(const FSovereignClassData& NewClassData);

	/** Unlocks a class directly from a USovereignClassDefinition Primary DataAsset */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Class")
	bool UnlockClassFromDefinition(const class USovereignClassDefinition* ClassDef);

	/** Equips an unlocked class if sufficient CP is available */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Class")
	bool EquipClass(const FGameplayTag& ClassTag);

	/** Unequips an active class, freeing its CP cost */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Class")
	bool UnequipClass(const FGameplayTag& ClassTag);

	/** Checks whether a specific class is currently equipped */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class")
	bool IsClassEquipped(const FGameplayTag& ClassTag) const;

	/** Increases the level of an unlocked class */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Class")
	bool AddClassLevel(const FGameplayTag& ClassTag, int32 LevelsToAdd = 1);

	/** --- 3. COMBAT STAT AGGREGATION --- */

	/** Returns combined Strength multiplier from all equipped classes */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|Combat")
	float GetTotalStrengthMultiplier() const;

	/** Returns combined Dexterity multiplier (turn speed) from all equipped classes */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|Combat")
	float GetTotalDexterityMultiplier() const;

	/** Returns combined Intelligence multiplier from all equipped classes */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Class|Combat")
	float GetTotalIntelligenceMultiplier() const;

	/** --- 4. ISovereignBrokerInterface Implementation --- */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

protected:
	virtual void BeginPlay() override;

	/** Ensures starter Adventurer class is unlocked and equipped */
	void EnsureDefaultStarterClass();
};
