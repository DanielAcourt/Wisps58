// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.


#pragma once

/*
* @brief The "Sovereign Live Registry."
 * * Acting as a World Subsystem, this is the central hub for tracking every "Saveable" entity
 * currently active in the garden. This avoids expensive 'GetAllActorsOfClass' calls during
 * save cycles, ensuring we maintain our 90 FPS target.
 */


#include "CoreMinimal.h"
#include "SaveSystem/SovereignWorldSubsystem.h"
#include "SovereignActorRegistry.generated.h"

UCLASS()
class WISPCPP7VR_API UActorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /** * Adds an actor to the live tracking system.
     * Called by SovereignBaseEntity in BeginPlay.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    void RegisterActor(FGuid ID, AActor* Actor);

    /** * Removes an actor from tracking.
     * Called by SovereignBaseEntity in EndPlay.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    void UnregisterActor(FGuid ID);

    /** Finds a specific actor using its unique Passport ID (GUID) */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    AActor* FindActor(FGuid ID) const;

     UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    TArray<AActor*> GetTrackedActorsAsList() const;

    /** Returns all valid tracked actors; useful for the SaveManager loop */
    const TMap<FGuid, TWeakObjectPtr<AActor>>& GetActiveRegistry() const { return TrackedActors; }

private:


    /** * Using TWeakObjectPtr prevents crashes if an Actor is destroyed
     * without UnregisterActor being called.
     */
    UPROPERTY()
    TMap<FGuid, TWeakObjectPtr<AActor>> TrackedActors;

};