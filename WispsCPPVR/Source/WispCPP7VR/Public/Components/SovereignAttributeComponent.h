// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignAttributeComponent.generated.h"

/**
 * USovereignAttributeComponent
 * Handles the D&D-style physical and mental vessel stats.
 * Values are synced from the SaveDataComponent to allow dynamic species scaling.
 */


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WISPCPP7VR_API USovereignAttributeComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USovereignAttributeComponent();

	// --- 1. CORE ATTRIBUTES (Infinite Accumulator Logic) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Strength; // The Prestige Level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double StrengthExperience; // The Scientific Accumulator

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Dexterity;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double DexterityExperience;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Constitution;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double ConstitutionExperience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Intelligence;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double IntelligenceExperience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Wisdom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double WisdomExperience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Charisma;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double CharismaExperience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Luck;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	double LuckExperience;

	/** Derived from Dexterity and Equipment */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 ArmourClass;

	// --- 2. USABLE RESOURCES (Real-time Bars) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Realtime Attributes")
	float CurrentHealth;
	// These act as the 'Clamps' for the bars
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Realtime Attributes")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Realtime Attributes")
	float CurrentStamina;//add this here? or in biology?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Realtime Attributes")
	float MaxStamina;


	// --- 3. RESISTANCES (Innate Vessel Defense) ---
	// Kept here as base values before gear modifications.


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float PhysicalResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float MagicalResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float MentalResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float PoisonResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float SlowResistance;

	// --- 4. LOGIC & DATA SYNC ---

	/** * Pulls base stats from the Owner's SaveDataComponent.
	 * Essential for ensuring Humans, Plants, and Dragons have different starting potential.
	 */

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Attributes")
	void SyncStatsFromEntity();

	/** Incremental training function for scientific growth */
	void TrainAttribute(double& TargetXP, int32& TargetLevel, double Amount, float Difficulty);

	/** Helper to get Max Health based on Constitution and Maturity */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Calculations")
	float GetMaxHealth() const;

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
