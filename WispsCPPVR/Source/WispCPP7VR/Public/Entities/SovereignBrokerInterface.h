// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SovereignBrokerInterface.generated.h"

class FJsonObject;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USovereignBrokerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISovereignBrokerInterface: The contract for all specialized data brokers (Truth, Magic, Telemetry).
 * Part of the Sovereign Soul Mediator Pattern.
 */
class WISPCPP7VR_API ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	/** Called when the Soul is capturing state for saving */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) = 0;

	/** Called when the Soul is restoring state from a save */
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) = 0;

	/** Called during the Soul's logic tick or when specific external data is injected (e.g. Lidar Manifest) */
	virtual void OnProcessData(const TMap<FString, FString>& Data) = 0;
};
