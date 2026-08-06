// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SovereignControllerComponent.generated.h"

// Forward declarations
class AController;
class APawn;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USovereignControllerComponent();

    /** Track if a human is currently controlling this vessel */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Possession")
    bool bIsPlayerPossessed;

    /** Called when the parent Pawn is possessed by ANY controller */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Possession")
    void OnPossessed(AController* NewController);

    /** Called when the parent Pawn is unpossessed */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Possession")
    void OnUnpossessed();

    /** Reference to the current controller (Player or AI) */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Possession")
    AController* CurrentController;

protected:
    virtual void BeginPlay() override;

private:
    /** Reference to the Pawn this component lives on */
    APawn* OwnerPawn;
};
