// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Entities/SovereignBlackBoxComponent.h"
#include "Interaction/SovereignSaveInterface.h"
#include "Subsystems/SovereignBlackBoxSubsystem.h"
#include "Subsystems/SovereignBlackBoxHeartbeat.h"
#include "SaveSystem/SovereignPSTAConfig.h"
#include "Engine/World.h"

USovereignBlackBoxComponent::USovereignBlackBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USovereignBlackBoxComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (USovereignBlackBoxHeartbeat* Heartbeat = World->GetSubsystem<USovereignBlackBoxHeartbeat>())
        {
            Heartbeat->RegisterComponent(this, UpdateFrequency);
        }
    }
}

void USovereignBlackBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (USovereignBlackBoxHeartbeat* Heartbeat = World->GetSubsystem<USovereignBlackBoxHeartbeat>())
        {
            Heartbeat->UnregisterComponent(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void USovereignBlackBoxComponent::RecordTruthSnapshot()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 1. DATA GATHERING: Pull truth data from the Vessel (Actor) and all compatible Components.
    TMap<FString, FString> RawData;

    // A. Actor-Level State: Capture primary physical properties (Location, Health, etc.)
    if (ISovereignSaveInterface* SaveInterface = Cast<ISovereignSaveInterface>(Owner))
    {
        RawData.Append(SaveInterface->GetSaveData());
    }

    // B. Component-Level State: Iterate over all components to capture specialized logic (Sensors, Inventory, etc.)
    TArray<UActorComponent*> Comps;
    Owner->GetComponents(Comps);
    for (UActorComponent* Comp : Comps)
    {
        if (Comp == this) continue; // Don't log the observer itself
        if (ISovereignSaveInterface* SaveInterface = Cast<ISovereignSaveInterface>(Comp))
        {
            TMap<FString, FString> CompData = SaveInterface->GetSaveData();
            FString Prefix = Comp->GetName() + TEXT(".");
            for (auto& Elem : CompData)
            {
                // Namespace scoping ensures no key collisions between components
                RawData.Add(Prefix + Elem.Key, Elem.Value);
            }
        }
    }

    // 2. DELTA & PSTA PROCESSING: Evaluate the data for changes and mission health impacts.
    bool bHasChanges = false;

    // "HOT PATH" OPTIMIZATION: Use static fixed-size array to avoid heap allocations during iteration.
    static const EPSTADimension Dimensions[] = { EPSTADimension::Psychological, EPSTADimension::Social, EPSTADimension::Technical, EPSTADimension::Administrative };

    // Reset dimension accumulators using persistent TMap members to minimize memory churn.
    for (EPSTADimension Dim : Dimensions)
    {
        DimWeightedSums.FindOrAdd(Dim) = 0.0f;
        DimTotalWeights.FindOrAdd(Dim) = 0.0f;
        DimAnchorZeroed.FindOrAdd(Dim) = false;
    }

    for (auto& Elem : RawData)
    {
        if (Elem.Value.IsNumeric())
        {
            float CurrentValue = FCString::Atof(*Elem.Value);

            // PSTA INTEGRITY CHECK: Map the telemetry tag to a dimension and normalize it.
            if (PSTAConfig)
            {
                if (const FPSTATagMapping* Mapping = PSTAConfig->GetMappingForTag(Elem.Key))
                {
                    float Normalized = PSTAConfig->NormalizeValue(*Mapping, CurrentValue);
                    DimWeightedSums[Mapping->Dimension] += Normalized * Mapping->Weight;
                    DimTotalWeights[Mapping->Dimension] += Mapping->Weight;

                    // ANCHOR TAG LOGIC: If a critical sensor (Anchor) hits 0, the entire dimension health collapses.
                    if (Mapping->bIsAnchorTag && FMath::IsNearlyZero(Normalized))
                    {
                        DimAnchorZeroed[Mapping->Dimension] = true;
                    }
                }
            }

            // DELTA FILTERING: Only log to disk if the value has changed significantly (LoggingThreshold).
            float* LastValuePtr = LastTruthValues.Find(Elem.Key);
            bool bShouldLog = !LastValuePtr || (FMath::Abs(CurrentValue - *LastValuePtr) >= LoggingThreshold);

            if (bShouldLog)
            {
                PendingEntries.Add(FBlackBoxEntry(Elem.Key, CurrentValue));
                LastTruthValues.Add(Elem.Key, CurrentValue);
                bHasChanges = true;
            }
        }
    }

    // 3. CALCULATE HOLISTIC SAFETY (VSS): Implement the Unified Safety Formula.
    if (PSTAConfig)
    {
        float VSS_WeightedSum = 0.0f;
        bool bAnyDimensionFailed = false;

        for (EPSTADimension Dim : Dimensions)
        {
            float Di = 0.0f;
            if (DimAnchorZeroed[Dim])
            {
                // Collapse dimension due to Anchor Tag failure
                Di = 0.0f;
            }
            else if (DimTotalWeights[Dim] > 0.0f)
            {
                // Nominal normalized weighted average
                Di = DimWeightedSums[Dim] / DimTotalWeights[Dim];
            }
            // else Di = 0.0f (VOID SAFETY: Untracked dimensions are considered untrusted)

            // UNIFIED SAFETY CHECK (Non-Compensatory): Check against dimension-specific failure thresholds
            float TauFail = PSTAConfig->DimensionFailureThresholds.Contains(Dim) ? PSTAConfig->DimensionFailureThresholds[Dim] : 0.0f;
            if (Di < TauFail)
            {
                bAnyDimensionFailed = true;
            }

            // Record Dimension Health (Di) only if a change is detected
            float* LastDi = LastDimensionHealth.Find(Dim);
            if (!LastDi || FMath::Abs(Di - *LastDi) >= 0.01f)
            {
                FString Key = FString::Printf(TEXT("PSTA.Di.%d"), (uint8)Dim);
                PendingEntries.Add(FBlackBoxEntry(Key, Di));
                LastDimensionHealth.Add(Dim, Di);
                bHasChanges = true;
            }

            // Accumulate weighted sum for the final Vessel Safety Status (VSS)
            float Alpha = PSTAConfig->DimensionWeights.Contains(Dim) ? PSTAConfig->DimensionWeights[Dim] : 0.0f;
            VSS_WeightedSum += Alpha * Di;
        }

        // UNIFIED SAFETY FORMULA: VSS = Product(Step(Di - TauFail)) * Sum(Alpha * Di)
        float VSS = bAnyDimensionFailed ? 0.0f : VSS_WeightedSum;

        if (FMath::Abs(VSS - LastVSS) >= 0.01f)
        {
            PendingEntries.Add(FBlackBoxEntry(TEXT("PSTA.VSS"), VSS));
            LastVSS = VSS;
            bHasChanges = true;
        }
    }

    // 4. Flush if we have data
    if (bHasChanges)
    {
        FlushToSubsystem();
    }
}

void USovereignBlackBoxComponent::RecordEvent(const FString& EventKey, const FString& EventDescription)
{
    PendingEntries.Add(FBlackBoxEntry(EventKey, 0.0f, EventDescription));
    FlushToSubsystem();
}

void USovereignBlackBoxComponent::IngestBlackBoxEntry(const FBlackBoxEntry& Entry)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 1. Update the Vessel (Actor) via Save Interface if it's numeric telemetry
    if (!Entry.Key.IsEmpty())
    {
        if (ISovereignSaveInterface* SaveInterface = Cast<ISovereignSaveInterface>(Owner))
        {
            TMap<FString, FString> Data;
            Data.Add(Entry.Key, FString::SanitizeFloat(Entry.Value));
            SaveInterface->RestoreSaveData(Data);
        }
    }

    // 2. Trigger a PSTA recalculation based on the new truth
    // We don't want to record a NEW snapshot during ingestion (to avoid infinite loops)
    // but we do want to update the internal PSTA health cache.
    RecordTruthSnapshot();
}

void USovereignBlackBoxComponent::FlushToSubsystem()
{
    if (PendingEntries.Num() == 0) return;

    if (UWorld* World = GetWorld())
    {
        if (USovereignBlackBoxSubsystem* BBSubsystem = World->GetSubsystem<USovereignBlackBoxSubsystem>())
        {
            BBSubsystem->RecordEntries(EntityID, PendingEntries);
            PendingEntries.Empty();
        }
    }
}
