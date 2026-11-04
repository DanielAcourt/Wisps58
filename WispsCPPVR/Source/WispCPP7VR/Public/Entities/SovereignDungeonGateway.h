// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "SovereignDungeonGateway.generated.h"

class ASovereignBaseCharacter;
class ASovereignDungeonGenerator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGatewayUnlocked, bool, bUnlocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGatewayEmpowered, float, EnemyBuffMultiplier, float, RewardMultiplier);

/**
 * ASovereignDungeonGateway
 * The mystical unsealable gateway door located in the Meditation Garden.
 * Features configurable unlock triggers (e.g. dragon hatching or manual command),
 * an item sacrifice inventory receptacle, and a raw Qi channel ("Risk it for the Biscuit").
 * Generates and initiates procedural dungeon expeditions.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonGateway : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignDungeonGateway();

	/** Whether the dungeon door is currently unsealed and accessible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Gateway|State")
	bool bIsUnlocked = false;

	/** If true, automatically unlocks when an egg in the garden hatches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Gateway|Config")
	bool bAutoUnlockOnHatch = true;

	/** Accumulated raw spiritual Qi channeled into the gateway */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Gateway|Empowerment")
	float InfusedQi = 0.0f;

	/** Receptacle inventory for sacrificing extra items to buff dungeon enemies & loot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Gateway|Empowerment")
	USovereignInventoryComponent* SacrificeInventory;

	/** Static mesh for the gateway frame / door */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Gateway|Visual")
	UStaticMeshComponent* GatewayDoorMesh;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Gateway|Events")
	FOnGatewayUnlocked OnGatewayUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Gateway|Events")
	FOnGatewayEmpowered OnGatewayEmpowered;

	// --- Gateway Operations ---

	/** Unlocks or locks the gateway */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Gateway")
	void SetGatewayUnlocked(bool bNewUnlocked);

	/** Channels raw Qi from a character into the gateway */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Gateway")
	bool ChannelQiFromCharacter(ASovereignBaseCharacter* Character, float Amount);

	/** Calculates total mass of items currently sacrificed in the receptacle */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Gateway")
	float GetTotalSacrificedMass() const;

	/**
	 * Calculates the enemy combat buff multiplier:
	 * 1.0 + (SacrificeMass * 0.05) + (InfusedQi * 0.002)
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Gateway")
	float CalculateEnemyBuffMultiplier() const;

	/**
	 * Calculates the reward / egg mutation bonus multiplier:
	 * 1.0 + (SacrificeMass * 0.10) + (InfusedQi * 0.005)
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Gateway")
	float CalculateRewardMultiplier() const;

	/**
	 * Embarks the party through the gateway into the procedural dungeon.
	 * Returns true if the expedition was successfully launched.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Gateway")
	bool EmbarkExpedition(ASovereignBaseCharacter* PartyLeader);

	/** Empties and consumes all sacrificed items upon expedition launch or party wipe */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Gateway")
	void ConsumeSacrifices();

	// --- Level Transition & Dungeon Expedition Entry ---

	/** Target dungeon level map to load (e.g. Lvl_Dungeon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Gateway|Expedition")
	FName DungeonLevelName = FName(TEXT("Lvl_Dungeon"));

	/** Save slot name used to auto-save the garden state before transitioning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Gateway|Expedition")
	FString GardenSaveSlot = TEXT("Garden_AutoSave");

	/** Generation seed for procedural dungeon generation (0 = random) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Gateway|Expedition")
	int32 ExpeditionSeed = 0;

	/**
	 * Enters the procedural dungeon:
	 * 1. Checks if gateway is unlocked (fails if locked).
	 * 2. Saves current garden state to GardenSaveSlot JSON via USaveManager.
	 * 3. Records expedition parameters (seed, buff/reward multipliers, return map) in USovereignDungeonSubsystem.
	 * 4. Consumes sacrifices via ConsumeSacrifices().
	 * 5. Changes level to DungeonLevelName via OpenLevel.
	 * Can be clicked directly in the Unreal Editor Details Panel via CallInEditor!
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Gateway|Editor Actions")
	bool EnterDungeon(ASovereignBaseCharacter* PartyLeader);

	// --- Sovereign Base Interactable Overrides ---
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;
};
