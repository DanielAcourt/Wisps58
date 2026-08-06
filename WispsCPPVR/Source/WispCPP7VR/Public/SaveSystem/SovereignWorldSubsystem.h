// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SovereignWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class WISPCPP7VR_API USovereignWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	// This system will manage the Registry for Players

public:
    /** Registers an actor. Called by the Component on BeginPlay. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    void RegisterEntity(FGuid ID, AActor* Entity);

    /** Removes an actor. Called by the Component on EndPlay. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    void DeregisterEntity(FGuid ID);

    /** Finds an actor by its ID. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    AActor* GetEntityByID(FGuid ID) const;

    /** Finds an actor by its Map. */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Registry")
    const TMap<FGuid, AActor*>& GetFullRegistry() const { return EntityRegistry; }


private:
    /** The Master Map: This is the "Phone Book" for the garden. */
    UPROPERTY()
    TMap<FGuid, AActor*> EntityRegistry;
	
};
