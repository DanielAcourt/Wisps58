// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Subsystems/SovereignBlackBoxHeartbeat.h"
#include "Entities/SovereignBlackBoxComponent.h"

void USovereignBlackBoxHeartbeat::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeIntervals();
}

void USovereignBlackBoxHeartbeat::Deinitialize()
{
    PulseGroups.Empty();
    Super::Deinitialize();
}

void USovereignBlackBoxHeartbeat::Tick(float DeltaTime)
{
    // 1. Periodic Cleanup (Every 10 seconds)
    LastCleanupTime += DeltaTime;
    if (LastCleanupTime >= 10.0f)
    {
        CleanupDeadComponents();
        LastCleanupTime = 0.0f;
    }

    // 2. Pulse the Groups
    for (auto& Pair : PulseGroups)
    {
        EUpdateFrequency Freq = Pair.Key;
        FBlackBoxHeartbeatGroup& Group = Pair.Value;

        float* IntervalPtr = Intervals.Find(Freq);
        float Interval = (IntervalPtr && *IntervalPtr > 0.0f) ? *IntervalPtr : 1.0f;

        // Skip dormant or manual frequencies
        if (Freq == EUpdateFrequency::Dormant || (IntervalPtr && *IntervalPtr < 0.0f)) continue;

        Group.TimeSinceLastPulse += DeltaTime;

        if (Group.TimeSinceLastPulse >= Interval)
        {
            // Synchronized Pulse: All components in this band fire now
            for (TWeakObjectPtr<USovereignBlackBoxComponent>& WeakComp : Group.Components)
            {
                if (USovereignBlackBoxComponent* Comp = WeakComp.Get())
                {
                    Comp->RecordTruthSnapshot();
                }
            }
            Group.TimeSinceLastPulse = 0.0f;
        }
    }
}

TStatId USovereignBlackBoxHeartbeat::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USovereignBlackBoxHeartbeat, STATGROUP_Tickables);
}

void USovereignBlackBoxHeartbeat::RegisterComponent(USovereignBlackBoxComponent* Component, EUpdateFrequency Frequency)
{
    if (!Component) return;

    // Ensure not already in any group
    UnregisterComponent(Component);

    PulseGroups.FindOrAdd(Frequency).Components.Add(Component);
}

void USovereignBlackBoxHeartbeat::UnregisterComponent(USovereignBlackBoxComponent* Component)
{
    if (!Component) return;

    for (auto& Pair : PulseGroups)
    {
        Pair.Value.Components.RemoveAll([Component](const TWeakObjectPtr<USovereignBlackBoxComponent>& Entry) {
            return Entry.Get() == Component;
        });
    }
}

void USovereignBlackBoxHeartbeat::ForceHeartbeat()
{
    for (auto& Pair : PulseGroups)
    {
        for (TWeakObjectPtr<USovereignBlackBoxComponent>& WeakComp : Pair.Value.Components)
        {
            if (USovereignBlackBoxComponent* Comp = WeakComp.Get())
            {
                Comp->RecordTruthSnapshot();
            }
        }
        Pair.Value.TimeSinceLastPulse = 0.0f;
    }
}

void USovereignBlackBoxHeartbeat::InitializeIntervals()
{
    Intervals.Add(EUpdateFrequency::Faster, 0.016f);   // ~60fps
    Intervals.Add(EUpdateFrequency::Realtime, 0.1f);   // 10 Hz
    Intervals.Add(EUpdateFrequency::Standard, 1.0f);   // 1 Hz
    Intervals.Add(EUpdateFrequency::Slow, 10.0f);      // 0.1 Hz
    Intervals.Add(EUpdateFrequency::Slower, 30.0f);
    Intervals.Add(EUpdateFrequency::Slowest, 60.0f);
    Intervals.Add(EUpdateFrequency::Glacier, 360.0f);
    Intervals.Add(EUpdateFrequency::Dormant, -1.0f);   // Manual Only
}

void USovereignBlackBoxHeartbeat::CleanupDeadComponents()
{
    for (auto& Pair : PulseGroups)
    {
        Pair.Value.Components.RemoveAll([](const TWeakObjectPtr<USovereignBlackBoxComponent>& Entry) {
            return !Entry.IsValid();
        });
    }
}
