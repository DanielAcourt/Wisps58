// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignDiagnosticBroker.generated.h"

/**
 * UDiagnosticBroker: Handles the "Truth" - raw scientific, historical, and sensor data.
 */
UCLASS()
class WISPCPP7VR_API UDiagnosticBroker : public UObject, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	// ISovereignBrokerInterface
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

	/**
	 * Raw Lidar / Museum Metadata storage (The Physical "Truth" namespace).
	 * We isolate raw physical telemetry, historical properties, and sensor data here
	 * to ensure physical measurements remain strictly decoupled from progress mechanics.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Truth")
	TMap<FString, FString> TruthMetadata;

	/**
	 * Vetted Curator Sign-off (Curation Baseline).
	 * Indicates which curator or historical database has validated this entity's state.
	 * When set, this string resolves physical uncertainty and triggers an 80% paradox density
	 * mitigation discount, directly boosting the Soul Hub's dynamic VSS System Confidence.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Truth")
	FString VettedBy;

	/**
	 * BlueprintCallable accessor to query a specific physical measurement or Lidar property.
	 * Ensures the Sovereign Nested UI and gameplay systems can fetch raw truth values
	 * dynamically without performing complex text/JSON parsing on the Blueprint side.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Truth")
	FString GetTruthValue(const FString& Key) const;

	/**
	 * BlueprintCallable modifier to set or overwrite a raw scientific measurement.
	 * Allows Save Terminals, sensors, or Lidar APIs to ingest raw values on the fly.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Truth")
	void SetTruthValue(const FString& Key, const FString& Value);
};
