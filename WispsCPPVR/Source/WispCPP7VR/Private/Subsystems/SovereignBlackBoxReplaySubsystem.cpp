// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Subsystems/SovereignBlackBoxReplaySubsystem.h"
#include "Entities/SovereignBlackBoxComponent.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "EngineUtils.h"

void USovereignBlackBoxReplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USovereignBlackBoxReplaySubsystem::Deinitialize()
{
    ActiveSessions.Empty();
    Super::Deinitialize();
}

void USovereignBlackBoxReplaySubsystem::Tick(float DeltaTime)
{
    if (ActiveSessions.Num() == 0) return;

    FDateTime Now = FDateTime::Now();

    for (auto& Pair : ActiveSessions)
    {
        FReplaySession& Session = Pair.Value;
        if (!Session.bIsActive) continue;

        USovereignBlackBoxComponent* TargetComp = FindComponentForEntity(Session.EntityID);
        if (!TargetComp) continue;

        // Calculate how much "log time" has passed since the start of replay
        FTimespan ElapsedRealTime = Now - Session.ReplayStartTime;
        FTimespan ElapsedLogTime = ElapsedRealTime * Session.PlaybackSpeed;
        FDateTime TargetLogTime = Session.LogStartTime + ElapsedLogTime;

        // Push all entries that have occurred up to this moment
        while (Session.CurrentIndex < Session.LogEntries.Num())
        {
            const FBlackBoxEntry& Entry = Session.LogEntries[Session.CurrentIndex];

            if (Entry.Timestamp <= TargetLogTime)
            {
                TargetComp->IngestBlackBoxEntry(Entry);
                Session.CurrentIndex++;
            }
            else
            {
                // Next entry is in the future relative to our playback
                break;
            }
        }

        // Check if finished
        if (Session.CurrentIndex >= Session.LogEntries.Num())
        {
            Session.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Replay finished for Entity %s"), *Session.EntityID.ToString());
        }
    }
}

TStatId USovereignBlackBoxReplaySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USovereignBlackBoxReplaySubsystem, STATGROUP_Tickables);
}

bool USovereignBlackBoxReplaySubsystem::LoadReplayLog(const FString& FilePath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath)) return false;

    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, RootObj) && RootObj.IsValid())
    {
        // For the Pi log, we expect an "EntityID" and a "Logs" array
        FString EntityIDStr;
        if (!RootObj->TryGetStringField(TEXT("EntityID"), EntityIDStr))
        {
            // Fallback: try to extract from filename BB_[GUID].json
            FString Filename = FPaths::GetBaseFilename(FilePath);
            if (Filename.StartsWith(TEXT("BB_")))
            {
                EntityIDStr = Filename.Mid(3);
            }
        }

        FGuid EntityID;
        if (!FGuid::Parse(EntityIDStr, EntityID)) return false;

        const TArray<TSharedPtr<FJsonValue>>* LogArray;
        if (RootObj->TryGetArrayField(TEXT("Logs"), LogArray))
        {
            FReplaySession NewSession;
            NewSession.EntityID = EntityID;

            for (auto& Value : *LogArray)
            {
                TSharedPtr<FJsonObject> EntryObj = Value->AsObject();
                if (EntryObj.IsValid())
                {
                    FBlackBoxEntry Entry;
                    FDateTime::Parse(EntryObj->GetStringField(TEXT("Timestamp")), Entry.Timestamp);
                    Entry.Key = EntryObj->GetStringField(TEXT("Key"));
                    Entry.Value = EntryObj->GetNumberField(TEXT("Value"));
                    Entry.EventData = EntryObj->GetStringField(TEXT("EventData"));

                    NewSession.LogEntries.Add(Entry);
                }
            }

            if (NewSession.LogEntries.Num() > 0)
            {
                // Sort by timestamp to ensure chronological playback
                NewSession.LogEntries.Sort([](const FBlackBoxEntry& A, const FBlackBoxEntry& B) {
                    return A.Timestamp < B.Timestamp;
                });

                NewSession.LogStartTime = NewSession.LogEntries[0].Timestamp;
                ActiveSessions.Add(EntityID, NewSession);
                return true;
            }
        }
    }

    return false;
}

void USovereignBlackBoxReplaySubsystem::StartReplay(const FGuid& EntityID, float PlaybackSpeed)
{
    if (FReplaySession* Session = ActiveSessions.Find(EntityID))
    {
        Session->ReplayStartTime = FDateTime::Now();
        Session->PlaybackSpeed = PlaybackSpeed;
        Session->CurrentIndex = 0;
        Session->bIsActive = true;
        UE_LOG(LogTemp, Warning, TEXT("Started replay for Entity %s at %fx speed"), *EntityID.ToString(), PlaybackSpeed);
    }
}

void USovereignBlackBoxReplaySubsystem::SetReplayPaused(const FGuid& EntityID, bool bPaused)
{
    if (FReplaySession* Session = ActiveSessions.Find(EntityID))
    {
        Session->bIsActive = !bPaused;
    }
}

void USovereignBlackBoxReplaySubsystem::StopReplay(const FGuid& EntityID)
{
    ActiveSessions.Remove(EntityID);
}

USovereignBlackBoxComponent* USovereignBlackBoxReplaySubsystem::FindComponentForEntity(const FGuid& EntityID) const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (USovereignBlackBoxComponent* Comp = It->FindComponentByClass<USovereignBlackBoxComponent>())
        {
            if (Comp->EntityID == EntityID) return Comp;
        }
    }

    return nullptr;
}
