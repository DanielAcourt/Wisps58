// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "SovereignATBComponent.generated.h"

class ASovereignBaseCharacter;

/**
 * EATBState
 * State machine for individual combatant Active Time Battle (ATB) gauge progression.
 */
UENUM(BlueprintType)
enum class EATBState : uint8
{
	Charging        UMETA(DisplayName = "Charging"),
	Ready           UMETA(DisplayName = "Ready"),
	SelectingAction UMETA(DisplayName = "Selecting Action"),
	ExecutingAction UMETA(DisplayName = "Executing Action"),
	Paused          UMETA(DisplayName = "Paused")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnATBProgressChanged, float, CurrentATB, float, MaxATB);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnATBReady, ASovereignBaseCharacter*, Combatant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnATBStateChanged, EATBState, OldState, EATBState, NewState);

/**
 * USovereignATBComponent
 * Manages an individual entity's Active Time Battle gauge (0.0 to 100.0%).
 * ATB fill rate scales with entity Dexterity and Class multipliers.
 * Supports "Wait Mode" pauses while navigating tactical submenus, action execution freezes,
 * and Defend stance damage mitigation.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignATBComponent : public USovereignBaseComponent
{
	GENERATED_BODY()

public:
	USovereignATBComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- ATB Gauge Metrics ---
	static constexpr float MAX_ATB = 100.0f;

	/** Base ATB fill percentage added per second at 0 Dexterity (Default 20.0% / sec) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|ATB")
	float BaseATBRate = 20.0f;

	/** Current ATB progress (0.0 to 100.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|ATB")
	float CurrentATB = 0.0f;

	/** Current operational state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|ATB")
	EATBState ATBState = EATBState::Charging;

	/** Whether combat is currently in Wait Mode (pauses ATB accumulation while navigating submenus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|ATB")
	bool bIsWaitModeActive = false;

	/** Whether this combatant is currently defending (reduces incoming damage until next action) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|ATB")
	bool bIsDefending = false;

	/** Percentage of incoming damage taken while defending (0.5 = 50% damage reduction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|ATB")
	float DefendDamageMultiplier = 0.5f;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|ATB|Events")
	FOnATBProgressChanged OnATBProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|ATB|Events")
	FOnATBReady OnATBReady;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|ATB|Events")
	FOnATBStateChanged OnATBStateChanged;

	// --- ATB Operations ---

	/** Calculates current fill rate per second accounting for Dexterity and Class Speed Multipliers */
	UFUNCTION(BlueprintPure, Category = "Sovereign|ATB")
	float CalculateEffectiveFillRate() const;

	/** Advances ATB progress by DeltaTime */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void AdvanceATB(float DeltaTime);

	/** Resets the ATB meter to 0.0 and transitions state back to Charging */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void ResetATB();

	/** Transitions combatant into Action Selection state (e.g. player opened battle menu) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void BeginActionSelection();

	/** Transitions combatant into Action Execution state (e.g. ability animation running) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void BeginActionExecution();

	/** Concludes action execution, resets ATB to 0%, and resumes charging */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void CompleteActionExecution();

	/** Enters Defend stance: ends turn, resets ATB to 0, and mitigates incoming damage until next turn */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void EnterDefendStance();

	/** Sets operational state with delegate broadcast */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void SetATBState(EATBState NewState);

	/** Sets global/arena Wait Mode pause flag */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|ATB")
	void SetWaitModeActive(bool bActive);

	/** Returns normalized progress from 0.0 to 1.0 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|ATB")
	float GetNormalizedATB() const { return FMath::Clamp(CurrentATB / MAX_ATB, 0.0f, 1.0f); }

	/** Checks if ATB is full and ready to act */
	UFUNCTION(BlueprintPure, Category = "Sovereign|ATB")
	bool IsReady() const { return CurrentATB >= MAX_ATB; }
};
