// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignBlackBoxHeartbeat.generated.h"

class USovereignBlackBoxComponent;

/**
 * @struct FBlackBoxHeartbeatGroup
 * @brief Tracks the pulse of a specific frequency band.
 */
struct FBlackBoxHeartbeatGroup
{
    float TimeSinceLastPulse = 0.0f;
    TArray<TWeakObjectPtr<USovereignBlackBoxComponent>> Components;
};

/**
 * @class USovereignBlackBoxHeartbeat
 * @brief Centralized "Sovereign Pulse" for Black Box telemetry.
 * Provides synchronized logging intervals across all entities.
 */
UCLASS()
class WISPCPP7VR_API USovereignBlackBoxHeartbeat : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject Interface
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !IsTemplate(); }
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }

    /** Registers a component for the heartbeat. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox")
    void RegisterComponent(USovereignBlackBoxComponent* Component, EUpdateFrequency Frequency);

    /** Unregisters a component. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox")
    void UnregisterComponent(USovereignBlackBoxComponent* Component);

    /** Forces all registered components to record a snapshot immediately. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox")
    void ForceHeartbeat();

private:
    /** Components grouped by their pulse frequency. */
    TMap<EUpdateFrequency, FBlackBoxHeartbeatGroup> PulseGroups;

    /** Pre-defined intervals for each frequency. */
    TMap<EUpdateFrequency, float> Intervals;

    void InitializeIntervals();
    void CleanupDeadComponents();

    float LastCleanupTime = 0.0f;
};
