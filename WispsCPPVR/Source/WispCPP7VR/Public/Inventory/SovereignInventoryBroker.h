// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignInventoryBroker.generated.h"

class USovereignInventoryComponent;
class FJsonObject;

/**
 * USovereignInventoryBroker
 * Connects USovereignInventoryComponent to USovereignSaveableEntityComponent.
 * Implements ISovereignBrokerInterface to serialize inventory slots into atomic "Sovereign.Inventory" JSON.
 */
UCLASS()
class WISPCPP7VR_API USovereignInventoryBroker : public UObject, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignInventoryBroker();

	/** Binds this broker to the target inventory component */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory|Persistence")
	void Initialize(USovereignInventoryComponent* InComponent);

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

protected:
	UPROPERTY()
	TWeakObjectPtr<USovereignInventoryComponent> TargetInventory;
};
