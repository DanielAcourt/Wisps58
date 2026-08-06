// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignElementComponent.generated.h"

/**
 * USovereignElementComponent: Handles the entity's elemental nature and resistances.
 * Part of the Triple-Axis system (Body Socket).
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignElementComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:	
	USovereignElementComponent();

	/** --- 1. THE TRIPLE-AXIS (PHYSICAL LAYER) --- */

	/** The primary physical element (Nature, Fire, Water, Earth, Air) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics")
	ESovereignElement BodySocket = ESovereignElement::Nature;

	/** Influence level for the physical axis (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics", meta = (ClampMin = "0", ClampMax = "100"))
	float BodyInfluence = 100.0f;


	/** --- 2. AFFINITIES & RESISTANCES --- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float FireAffinity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float EarthAffinity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float WindAffinity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Resistances")
	float WaterAffinity;


	/** --- 3. LOGIC --- */

	/** Calculates multipliers based on elemental interaction */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Elements")
	float GetElementalMultiplier(ESovereignElement IncomingType);

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
