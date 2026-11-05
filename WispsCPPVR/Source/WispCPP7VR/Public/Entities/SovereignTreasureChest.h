// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "SovereignTreasureChest.generated.h"

class UStaticMeshComponent;
class USovereignInventoryComponent;
class ASovereignBaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTreasureChestOpened);

/**
 * ASovereignTreasureChest
 * Interactive loot chest placed in procedural dungeon Treasure Rooms (Ticket B-107).
 * Smoothly rotates open its lid, transfers contained loot items to the interacting character's
 * inventory, and locks into an opened state to prevent infinite looting.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignTreasureChest : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignTreasureChest();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Visual Components ---

	/** Base chest container mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Chest")
	TObjectPtr<UStaticMeshComponent> ChestBaseMeshComponent;

	/** Rotating lid mesh attached to top-rear hinge */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Chest")
	TObjectPtr<UStaticMeshComponent> ChestLidMeshComponent;

	// --- Loot Inventory Component ---

	/** Chest inventory holding rewards (gold, potions, dragon eggs, gems) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Chest")
	TObjectPtr<USovereignInventoryComponent> LootInventory;

	// --- State & Settings ---

	/** Whether this chest has already been opened and looted */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Chest")
	bool bHasBeenOpened = false;

	/** Target opening pitch angle in degrees (default -90 deg Pitch opens backwards) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Chest")
	float LidOpenAngle = -90.0f;

	/** Lid opening interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Chest")
	float LidOpenSpeed = 5.0f;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Chest")
	FOnTreasureChestOpened OnChestOpened;

	// --- Interaction API ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;

	/** Opens the chest lid and transfers all contained loot to the character */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Chest")
	bool OpenChest(ASovereignBaseCharacter* Looter);

protected:
	FRotator ClosedLidRotation = FRotator::ZeroRotator;
	FRotator TargetLidRotation = FRotator::ZeroRotator;
};
