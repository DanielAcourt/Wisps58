// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SovereignInterfaceMain.generated.h"

// 1. The U-Class (The Engine's "Social Security Number" for the interface)
UINTERFACE(MinimalAPI, Blueprintable)
class  UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 2. The I-Class (The actual Contract/Interface)
 * This is where you define the functions.
 */

class WISPCPP7VR_API IInteractionInterface
{
	GENERATED_BODY()

public:

	/** * THE MASTER KEY:
 * Returns the Sovereign Soul (The 'One Sense of Simulated Truth').
 * This allows the Wisp to read IDs, Tags, and Qi from any interactable object.
 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Core")
	class USovereignSaveableEntityComponent* GetSovereignSoul() const;

	/** * The Master Interaction Command.
	 * BlueprintNativeEvent allows C++ to handle the logic (Metabolical)
	 * while Blueprints handle the visuals (UI/Emitters).
	 */

	 // Triggered when the Wisp or Bee looks at this object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	void OnBeginHover();

	// Triggered when the focus is lost
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	void OnEndHover();

	/** Returns true if the object is currently in a state where it can be touched. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	bool CanInteract(AActor* Interactor);

	// The actual "Action" (Harvesting Qi, Opening Save Menu, etc.)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	void OnInteract(AActor* Interactor);

	// NEW: Allows the object to tell the UI what its name is
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	FText GetInteractableName();

	/** Returns a 'hint' string for the UI (e.g., "Harvest", "Plant", "Save"). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	FString GetInteractionHint();

	/** A second action (like Long Press or Trigger + Grip). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interaction")
	void OnSecondaryInteract(AActor* Interactor);

	/** NEW: Check if this entity can be inhabited by a Sovereign Spirit */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Possession")
	bool CanBePossessed();

	/** NEW: The command to initiate the Sovereign Trust Handover */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Possession")
	void RequestPossession(AController* RequestingController);

	/** NEW: Returns the component to attach the spirit to when possessed */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Possession")
	USceneComponent* GetPossessionAttachmentComponent();

	// Inside IInteractionInterface.h
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign")
	AActor* GetInhabitingSpirit();

	// Inside IInteractionInterface.h
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign")
	bool IsSpiritEntity();

	//Adding this so the interface can pass along wether it has been Possessed and therefore can release the wisp

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign")
	void RequestSoulEject();


};


