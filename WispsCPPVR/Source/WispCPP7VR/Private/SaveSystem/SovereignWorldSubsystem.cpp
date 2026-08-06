// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.


#include "SaveSystem/SovereignWorldSubsystem.h"

void USovereignWorldSubsystem::RegisterEntity(FGuid ID, AActor* Entity)
{
    if (ID.IsValid() && Entity)
    {
        // Add or Update the entity in the map
        EntityRegistry.Add(ID, Entity);

        UE_LOG(LogTemp, Warning, TEXT("Sovereign System: Registered %s with ID %s"),
            *Entity->GetName(), *ID.ToString());
    }
}

void USovereignWorldSubsystem::DeregisterEntity(FGuid ID)
{
    if (ID.IsValid())
    {
        EntityRegistry.Remove(ID);
        UE_LOG(LogTemp, Log, TEXT("Sovereign System: Unregistered ID %s"), *ID.ToString());
    }
}

AActor* USovereignWorldSubsystem::GetEntityByID(FGuid ID) const
{
    // We look for the pointer. The 'Find' result is a pointer-to-a-pointer.
    AActor* const* FoundActor = EntityRegistry.Find(ID);

    if (FoundActor)
    {
        return *FoundActor; // Return the actual Actor pointer
    }

    return nullptr;
}