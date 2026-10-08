// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "SovereignTelemetryComponent.generated.h"

/**
 * USovereignTelemetryComponent: Standalone IoT sensor telemetry component for Digital Twin hardware integration.
 * Holds real-time sensor streams (Temperature, pH Value, Water Depth) and handles saving, restoring,
 * and UI inspection independently of base interactable actors.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignTelemetryComponent : public USovereignBaseComponent
{
	GENERATED_BODY()

public:
	USovereignTelemetryComponent();

	/** Temperature reading in Celsius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Telemetry")
	float TemperatureCelsius = 0.0f;

	/** pH Value reading (0.0 - 14.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Telemetry")
	float PhValue = 7.0f;

	/** Water level depth reading in millimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Telemetry")
	float WaterDepthMM = 0.0f;

	/* =========================
	   Hardware Ingestion API
	   ========================= */

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Telemetry")
	void SetTemperatureCelsius(float InTemperature);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Telemetry")
	void SetPhValue(float InPhValue);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Telemetry")
	void SetWaterDepthMM(float InWaterDepth);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Telemetry")
	void UpdateTelemetry(float InTemperature, float InPhValue, float InWaterDepth);

	/* =========================
	   ISovereignSaveInterface
	   ========================= */

	virtual TMap<FString, FString> GetSaveData() override;
	virtual void RestoreSaveData(const TMap<FString, FString>& Data) override;

	/* =========================
	   ISovereignUIInspectable
	   ========================= */

	virtual FText GetInspectorDisplayName_Implementation() const override;
	virtual FString GetInspectorCategory_Implementation() const override;
};
