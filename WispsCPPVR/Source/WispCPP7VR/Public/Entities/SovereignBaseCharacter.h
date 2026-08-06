// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h" 
#include "Interaction/SovereignInterfaceMain.h"
#include "Entities/SovereignBaseInteractable.h"


//This means i need a Input SetupPlayerInputComponent
#include "InputActionValue.h" // For FInputActionValue
#include "SovereignBaseCharacter.generated.h"


// 1. FORWARD DECLARATIONS (Tells the compiler: These classes exist, we'll define them later)
class USovereignElementComponent;
class USovereignControllerComponent;
class USovereignAttributeComponent;
class ASovereignPlayerWisp;
class USovereignQiComponent; // Added this
class AController ; // <--- MUST ADD THIS FOR POSSESSION

// This creates a custom event node for your Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorSensed, AActor*, SensedActor);

/**
 * ASovereignBaseCharacter
 * The 'Master Vessel' for everything from Erisis to Dragons.
 */
UCLASS()
class WISPCPP7VR_API ASovereignBaseCharacter : public ACharacter, public IInteractionInterface
{
	GENERATED_BODY()
	
public:
    ASovereignBaseCharacter();

	// This is the built-in Unreal function we are overriding
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	// This allows you to bind logic in BP (e.g., highlighting a wisp when you look at it)
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Sovereign|Senses")
	FOnActorSensed OnActorSensed;

	//Ideally we want a bool stored on the wisp to know if it is possessing anything
	bool IsPossessing();

	// --- IInteractionInterface Implementation ---
	// This is the "Universal Handshake" your Wisp uses.
	virtual class USovereignSaveableEntityComponent* GetSovereignSoul_Implementation() const override { return SaveDataComponent; }

	/** Since we are an IInteractionInterface, we can implement these standard functions here too */
	virtual bool CanBePossessed_Implementation() override { return bCanBePossessed; }
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Implementation of the Sovereign Interface to find the wisp currently inside us */
	virtual AActor* GetInhabitingSpirit_Implementation() override;

	/** Handles the 'F' key logic: Possession if a Wisp, Ejection if a Vessel */
	virtual void HandlePossessionLifecycle();


	/** Primary logic for growth/evolution.
	 * Making it virtual allows children like the Wisp to override it.
	 */
	virtual void Evolve();
	// This tells the engine we want to run code every frame
	virtual void Tick(float DeltaTime) override;

protected: 



	/** Every Sovereign Entity needs the Soul Hub */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignSaveableEntityComponent* SaveDataComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Possession")
	bool bCanBePossessed = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignAttributeComponent* AttributeComponent; // Renamed to match Wisp code

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignQiComponent* QiComponent; // Added this! The Wisp needs it.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignElementComponent* ElementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignControllerComponent* ControlComponent;

	/** Performs a sphere trace to find the actor the player is looking at this is the i think wrong way to do this Blueprint callable */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Components")
	AActor* GetSensedActor();


	/** The main interaction logic triggered by the Input Action */
	virtual void Interact(const FInputActionValue& Value);
	/** Get the component to attach to */
	virtual USceneComponent* GetPossessionAttachmentComponent_Implementation() override;


protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;



	/** The master movement logic for all Sovereign entities */
	// --- Movement Logic ---
	// These MUST exist here for the .cpp to recognize them as members
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);


/** Input Assets - Assign these in the Character Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Input")
	class UInputAction* LookAction;


	//Handles all Interaction Between 2 entities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Input")
	class UInputAction* InteractAction;

	//Checks if player can Possess or i possessing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Input")
	class UInputAction* PossessAction;
	
};

