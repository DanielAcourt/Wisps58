// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SovereignEntityInterface.generated.h"

// Forward Declaration: We only need to know it exists
class USovereignSaveableEntityComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USovereignEntityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 This class is purely for the Unreal Engine's reflection system.
 It allows Blueprints to "see" that the interface exists. 
 You do not add your code here.
 */
class WISPCPP7VR_API ISovereignEntityInterface
{
	GENERATED_BODY()

public:

	/** * Returns the "Sovereign Soul" (The Hub).
		 * This is how Isla reads unknown tags on a Rock or a Wolf.
		 */

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interface")
	USovereignSaveableEntityComponent* GetSovereignSoul() const;

	/** * The Handshake for the Wisp.
	 * Replaces native 'Possess' for non-pawns (like Rocks).
	 */

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interface")
	void OnWispEnter(AController* WispController);

	//When the Wisps Exits An Actor or character
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interface")
	void OnWispExit();

	/** Returns the mesh so the Wisp knows where the "Physical Center" is. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interface")
	UPrimitiveComponent* GetPrimaryMesh() const;

	/** Returns the System Confidence score (1 - ParadoxDensity) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Interface")
	float GetSystemConfidence() const;
public:
};
