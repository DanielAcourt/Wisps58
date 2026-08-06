// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SovereignControllerComponent.h"

// Sets default values for this component's properties
USovereignControllerComponent::USovereignControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsPlayerPossessed = false; // This will now work because it's in the .h
}

void USovereignControllerComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerPawn = Cast<APawn>(GetOwner());
}


void USovereignControllerComponent::OnPossessed(AController* NewController)
{
    if (!NewController) return;

    CurrentController = NewController;

    // Check if the pilot is a human (PlayerController) or code (AIController)
    bIsPlayerPossessed = NewController->IsPlayerController();

    if (bIsPlayerPossessed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sovereign: Player has possessed %s"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Sovereign: AI Controller has taken command of %s"), *GetOwner()->GetName());
    }
}

void USovereignControllerComponent::OnUnpossessed()
{
    UE_LOG(LogTemp, Log, TEXT("Sovereign: %s is no longer possessed."), *GetOwner()->GetName());

    CurrentController = nullptr;
    bIsPlayerPossessed = false;
}


// Called when the game starts
/*
void UUSovereignControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
*/

// Called every frame
/*
void UUSovereignControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/
