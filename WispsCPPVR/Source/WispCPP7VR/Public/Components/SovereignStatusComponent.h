// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignStatusComponent.generated.h"

class ASovereignBaseCharacter;

/**
 * ESovereignStatusType
 * Classic FF-style turn-based combat conditions (Ticket CS-005).
 */
UENUM(BlueprintType)
enum class ESovereignStatusType : uint8
{
	None    UMETA(DisplayName = "None"),
	Poison  UMETA(DisplayName = "Poison (Damage per Turn)"),
	Stun    UMETA(DisplayName = "Stun / Paralyze (Skip Turn)"),
	Silence UMETA(DisplayName = "Silence (Block Qi Spells)"),
	Blind   UMETA(DisplayName = "Blind (50% Physical Accuracy / Damage)"),
	Regen   UMETA(DisplayName = "Regen (Healing per Turn)"),
	Haste   UMETA(DisplayName = "Haste (1.5x ATB Speed)"),
	Slow    UMETA(DisplayName = "Slow (0.5x ATB Speed)")
};

/**
 * FSovereignStatusInstance
 * Active instance of a status condition on a combatant.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignStatusInstance
{
	GENERATED_BODY()

	/** Type of status effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Status")
	ESovereignStatusType StatusType = ESovereignStatusType::None;

	/** Number of action turns remaining before this effect expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Status")
	int32 RemainingTurns = 3;

	/** Potency value (e.g. Poison damage amount or percent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Status")
	float Potency = 10.0f;

	/** Actor that applied this effect */
	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Status")
	TWeakObjectPtr<AActor> SourceActor = nullptr;

	FSovereignStatusInstance()
		: StatusType(ESovereignStatusType::None)
		, RemainingTurns(3)
		, Potency(10.0f)
	{
	}

	FSovereignStatusInstance(ESovereignStatusType InType, int32 InTurns, float InPotency = 10.0f, AActor* InSource = nullptr)
		: StatusType(InType)
		, RemainingTurns(InTurns)
		, Potency(InPotency)
		, SourceActor(InSource)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectApplied, ESovereignStatusType, StatusType, int32, DurationTurns);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectExpired, ESovereignStatusType, StatusType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatusEffectTicked, ESovereignStatusType, StatusType, float, Value, bool, bIsHealing);

/**
 * USovereignStatusComponent
 * Manages active status effects and conditions on character vessels (Ticket CS-005).
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignStatusComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignStatusComponent();

	/** Currently active status effects on this combatant */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Status")
	TArray<FSovereignStatusInstance> ActiveStatuses;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Status|Events")
	FOnStatusEffectApplied OnStatusApplied;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Status|Events")
	FOnStatusEffectExpired OnStatusExpired;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Status|Events")
	FOnStatusEffectTicked OnStatusTicked;

	// --- Status Management API ---

	/** Applies a status effect. Refreshes duration if already active. */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Status")
	bool ApplyStatusEffect(ESovereignStatusType Type, int32 DurationTurns, float Potency = 10.0f, AActor* Source = nullptr);

	/** Removes an active status effect */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Status")
	bool RemoveStatusEffect(ESovereignStatusType Type);

	/** Clears all status effects (e.g. on battle end or death) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Status")
	void ClearAllStatuses();

	/** Checks whether a specific status effect is currently active */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	bool HasStatusEffect(ESovereignStatusType Type) const;

	/** Returns the instance of an active status effect */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	bool GetStatusInstance(ESovereignStatusType Type, FSovereignStatusInstance& OutInstance) const;

	/** Convenience status checks */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	bool IsStunned() const { return HasStatusEffect(ESovereignStatusType::Stun); }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	bool IsSilenced() const { return HasStatusEffect(ESovereignStatusType::Silence); }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	bool IsBlinded() const { return HasStatusEffect(ESovereignStatusType::Blind); }

	/** Returns speed multiplier based on Haste (1.5x) and Slow (0.5x) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Status")
	float GetATBSpeedMultiplier() const;

	/**
	 * Processes turn-end effects (Poison damage, Regen healing, decrements duration).
	 * Called by BattleManager when this combatant completes an action.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Status")
	void ProcessTurnEnd();

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;
};
