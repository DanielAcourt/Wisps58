// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignBlackBoxReplaySubsystem.generated.h"

/**
 * @struct FReplaySession
 * @brief Container for a loaded Black Box log being replayed.
 */
struct FReplaySession
{
    FGuid EntityID;
    TArray<FBlackBoxEntry> LogEntries;
    int32 CurrentIndex = 0;
    FDateTime ReplayStartTime;
    FDateTime LogStartTime;
    float PlaybackSpeed = 1.0f;
    bool bIsActive = false;

    FReplaySession() : EntityID(FGuid()), CurrentIndex(0), PlaybackSpeed(1.0f), bIsActive(false) {}
};

/**
 * @class USovereignBlackBoxReplaySubsystem
 * @brief Orchestrates the playback of external Black Box logs.
 * Acts as the "Driver" for the Digital Museum's truth reconstruction.
 */
UCLASS()
class WISPCPP7VR_API USovereignBlackBoxReplaySubsystem : public UWorldSubsystem, public FTickableGameObject
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

    /**
     * Loads a Black Box JSON file and prepares it for replay.
     * @param FilePath The path to the BB_[GUID].json file from the Pi.
     * @return True if loaded successfully.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Replay")
    bool LoadReplayLog(const FString& FilePath);

    /**
     * Starts replaying a loaded log for a specific entity.
     * @param EntityID The GUID of the entity to drive.
     * @param PlaybackSpeed Speed multiplier (e.g., 2.0 = double speed).
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Replay")
    void StartReplay(const FGuid& EntityID, float PlaybackSpeed = 1.0f);

    /** Pauses or Resumes a replay. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Replay")
    void SetReplayPaused(const FGuid& EntityID, bool bPaused);

    /** Stops and clears a replay session. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Replay")
    void StopReplay(const FGuid& EntityID);

private:
    /** Active replay sessions keyed by EntityID. */
    TMap<FGuid, FReplaySession> ActiveSessions;

    /** Helper to find the target component in the world. */
    class USovereignBlackBoxComponent* FindComponentForEntity(const FGuid& EntityID) const;
};
