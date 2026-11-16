// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-05
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

	// --- 1. CORE ATTRIBUTES ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Dexterity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Constitution;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Charisma;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 Luck;

	/** Derived from Dexterity and Equipment */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Core Attributes")
	int32 ArmourClass;

	// --- CORE ATTRIBUTE GETTERS & SETTERS ---

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetStrength() const { return Strength; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetStrength(int32 Value) { Strength = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetDexterity() const { return Dexterity; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetDexterity(int32 Value) { Dexterity = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetConstitution() const { return Constitution; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetConstitution(int32 Value) { Constitution = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetIntelligence() const { return Intelligence; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetIntelligence(int32 Value) { Intelligence = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetWisdom() const { return Wisdom; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetWisdom(int32 Value) { Wisdom = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetCharisma() const { return Charisma; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetCharisma(int32 Value) { Charisma = Value; }

	UFUNCTION(BlueprintPure, Category = "Sovereign|Core Attributes")
	int32 GetLuck() const { return Luck; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Core Attributes")
	void SetLuck(int32 Value) { Luck = Value; }

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
