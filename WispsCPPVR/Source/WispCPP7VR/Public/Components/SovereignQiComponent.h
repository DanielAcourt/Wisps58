// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignQiComponent.generated.h"

/**
 * USovereignQiComponent: Handles the "Spiritual" energy and Alignment of the entity.
 * Part of the Triple-Axis system (Alignment Socket).
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignQiComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignQiComponent();

	/** --- 1. THE TRIPLE-AXIS (SPIRITUAL LAYER) --- */

	/** The spiritual alignment (Grey, Light, Dark) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics")
	ESovereignElement AlignmentSocket;

	/** Influence level for the spiritual axis (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics", meta = (ClampMin = "0", ClampMax = "100"))
	float AlignmentInfluence = 100.0f;

	/** The magic essence (Fairy, Dragon, Electric) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics")
	ESovereignElement MagicSocket;

	/** Influence level for the magic axis (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Genetics", meta = (ClampMin = "0", ClampMax = "100"))
	float MagicInfluence = 0.0f;


	/** --- 2. QI POOL --- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Qi")
	float CurrentQi;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Qi")
	float MaxQiCapacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Qi")
	float QiPurity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Qi")
	float TotalQiAccumulated;


	/** --- 3. RESONANCE --- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Qi")
	TMap<FName, float> ElementalResonance;

	/** Universal Territory & Domain Rune Systems [B-036] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Qi Domain")
	bool bIsDomainOwner = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Qi Domain")
	FString DomainRuneTier = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Qi Domain")
	float DomainResonanceRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Qi Domain")
	float DomainNetworkSaturation = 0.0f;


	/** --- 4. LOGIC --- */

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Qi")
	void AbsorbQi(float Amount, float SourcePurity);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Qi")
	bool SpendQi(float Amount);

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void ProcessQiFlow(float DeltaTime, int32 WisdomStat);
};
