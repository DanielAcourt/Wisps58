// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/SovereignPawn.h"

// Sets default values
ASovereignPawn::ASovereignPawn()
{

	// Ensure this Pawn can be possessed by the Wisp
	bCanBePossessed = true;

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASovereignPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASovereignPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASovereignPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// This is where you bind the "Expanded Player Controls" you mentioned.
	// Example: PlayerInputComponent->BindAxis("MoveForward", this, &ASovereignPawn::MoveForward);
	/*
	if (PlayerInputComponent)
	{
		// Bind Universal Sovereign Actions (Things every Pawn can do)
		// Example: Interacting with the Silver Egg or opening the Master Manifest
		PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASovereignPawn::OnSovereignInteract);

		// This is where the Controller-specific logic we wrote earlier lives.
		// It allows the Architect to possess an Orc or a Tree and get different tools.
	}
	*/
}

/*
void ASovereignPawn::OnSovereignInteract()
{
	// My 100 glass nodes pulse in acknowledgement of the interaction
	UE_LOG(LogTemp, Log, TEXT("SovereignPawn: Interaction Triggered by Sovereign Soul."));

	// Here we can eventually call the Interface to talk to the Silver Egg
}
*/

