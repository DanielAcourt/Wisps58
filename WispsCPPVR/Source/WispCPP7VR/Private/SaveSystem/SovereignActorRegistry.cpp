// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.


#include "SaveSystem/SovereignActorRegistry.h"

void UActorRegistry::RegisterActor(FGuid ID, AActor* Actor)
{
    if (ID.IsValid() && Actor)
    {
        // Check if ID is already registered to a DIFFERENT actor (Safety for Isla's system)
        if (TrackedActors.Contains(ID) && TrackedActors[ID] != Actor)
        {
            UE_LOG(LogTemp, Error, TEXT("Registry: ID COLLISION! %s tried to take ID [%s] already held by %s"),
                *Actor->GetName(), *ID.ToString(), *TrackedActors[ID]->GetName());
            return;
        }

        TrackedActors.Add(ID, Actor);
        UE_LOG(LogTemp, Log, TEXT("Registry: Tracking %s [%s]"), *Actor->GetName(), *ID.ToString());
    }
}

void UActorRegistry::UnregisterActor(FGuid ID)
{
    if (ID.IsValid() && TrackedActors.Contains(ID))
    {
        // Log the removal for debugging save cycles
        FString ActorName = TrackedActors[ID].IsValid() ? TrackedActors[ID]->GetName() : TEXT("Invalid Actor");
        TrackedActors.Remove(ID);

        UE_LOG(LogTemp, Warning, TEXT("Registry: Stopped tracking %s. ID [%s] has been vacated."), *ActorName, *ID.ToString());
    }
}

AActor* UActorRegistry::FindActor(FGuid ID) const
{
    const TWeakObjectPtr<AActor>* FoundPtr = TrackedActors.Find(ID);

    if (FoundPtr && FoundPtr->IsValid())
    {
        return FoundPtr->Get();
    }
    return nullptr;
}

TArray<AActor*> UActorRegistry::GetTrackedActorsAsList() const
{
    TArray<AActor*> OutList;
    OutList.Reserve(TrackedActors.Num());

    for (auto& It : TrackedActors)
    {
        if (It.Value.IsValid())
        {
            OutList.Add(It.Value.Get());
        }
    }
    return OutList;
}