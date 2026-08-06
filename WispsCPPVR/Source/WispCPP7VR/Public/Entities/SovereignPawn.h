// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseEntity.h" // Inherit from your base!

#include "GameFramework/Pawn.h"
#include "interaction/SovereignEntityInterface.h"

//#include "SovereignEntityInterface.h""

#include "SovereignPawn.generated.h"

/**
 * ASovereignPawn: A 'Living' version of the Base Entity.
 * It has all the Soul/DNA logic of the BaseEntity, but adds Pawn capabilities
 * for Wisp Possession and Movement.
 */
UCLASS()
class WISPCPP7VR_API ASovereignPawn : public ASovereignBaseEntity // <--- Critical Change
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASovereignPawn();

	// The logic hub for the Wisp's "Small" body
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Interface implementation (Shared with Character)
	//virtual class USovereignSaveableEntityComponent* GetSovereignSoul_Implementation() const override { return SaveDataComponent; }

	// Since we are now a Pawn, we can override these for Wisp controls
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** * Overriding the Interface from BaseEntity to allow possession
	 * (Trees might return false, but Pawns return true)
	 */
	virtual bool CanBePossessed() const { return bCanBePossessed; }

	/** Function to handle the Interaction action */
	//UFUNCTION() // Must be a UFUNCTION to be bound to input
	//	void OnSovereignInteract();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Components for movement - can be added here or in children (like Orc/Bird) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Movement")
	class UPawnMovementComponent* MovementComponent;

	// Simple movement for a floating/growing entity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Movement")
	class UFloatingPawnMovement* FloatingMovement;


	// The simplified "Interact" for the Pawn tier
	//void Interact();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** * MANDATORY DECLARATION: This fixes Error C2509.
	 * It tells the compiler 'Yes, I am defining this function in the cpp'.
	 */

	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
