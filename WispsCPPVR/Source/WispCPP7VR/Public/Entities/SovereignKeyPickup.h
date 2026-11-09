// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "GameplayTagContainer.h"
#include "SovereignKeyPickup.generated.h"

class UStaticMeshComponent;
class USovereignItemDefinition;
class ASovereignBaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeyPickedUp, ASovereignBaseCharacter*, Looter);

/**
 * ASovereignKeyPickup
 * Interactive 3D physical key pickup placed in procedural dungeons (Ticket B-108).
 * Features a visual floating key mesh with idle hover/bob animation, transfers its key
 * item into the interacting character's USovereignInventoryComponent, and auto-destroys.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignKeyPickup : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignKeyPickup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Visual Components ---

	/** Floating 3D key mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Key")
	TObjectPtr<UStaticMeshComponent> KeyMeshComponent;

	// --- Key Item Data ---

	/** Primary DataAsset item definition representing the key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key")
	TObjectPtr<USovereignItemDefinition> KeyItemDefinition;

	/** Identifying tag for the key item (Default: Item.Key.BossGate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key")
	FGameplayTag KeyTag;

	/** Display name for UI / interaction prompt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key")
	FText KeyDisplayName;

	// --- Idle Floating Animation Settings ---

	/** Idle rotation speed around Z (Yaw) in degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key|Animation")
	float RotationSpeed = 60.0f;

	/** Idle vertical bobbing amplitude in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key|Animation")
	float BobAmplitude = 12.0f;

	/** Idle vertical bobbing frequency in Hz */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Key|Animation")
	float BobFrequency = 2.0f;

	/** Whether this key pickup has been collected */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Key")
	bool bIsPickedUp = false;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Key")
	FOnKeyPickedUp OnKeyPickedUp;

	// --- Interaction API ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;

	/** Executes key pickup and inventory transfer into the looter's pack */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Key")
	bool PickupKey(ASovereignBaseCharacter* Looter);

protected:
	FVector BaseLocation = FVector::ZeroVector;
	float RunningTime = 0.0f;
};
