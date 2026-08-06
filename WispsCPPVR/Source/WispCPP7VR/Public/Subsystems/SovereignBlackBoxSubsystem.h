// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignBlackBoxSubsystem.generated.h"

/**
 * @class USovereignBlackBoxSubsystem
 * @brief Manages decoupled persistence for "Truth" logs.
 * Handles the /Saved/BlackBox/ directory and standalone JSON serialization.
 */
UCLASS()
class WISPCPP7VR_API USovereignBlackBoxSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Records a batch of truth entries for a specific entity. */
    void RecordEntries(const FGuid& EntityID, const TArray<FBlackBoxEntry>& Entries);

private:
    FString GetBlackBoxDirectory() const;
    FString GetFilePathForEntity(const FGuid& EntityID) const;

    /** Persists a batch of entries to its entity's log file. */
    void AppendEntriesToFile(const FGuid& EntityID, const TArray<FBlackBoxEntry>& NewEntries);
};
