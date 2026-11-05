// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "SovereignRestTerminal.generated.h"

class ASovereignBaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRestTerminalUsed, int32, RemainingCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestTerminalDepleted);

/**
 * ASovereignRestTerminal
 * Interactive rest terminal / sanctuary campfire placed in procedural dungeon Rest Sanctuaries (Ticket B-103).
 * Restores party health, Qi, and metabolic vitals, and auto-destroys / extinguishes once all charges are exhausted.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignRestTerminal : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignRestTerminal();

	virtual void BeginPlay() override;

	/** Total maximum rest charges this terminal can grant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	int32 MaxRestCharges = 3;

	/** Remaining rest charges before depletion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Rest")
	int32 RemainingRestCharges = 3;

	/** If true, randomizes rest charges between MinRandomCharges and MaxRandomCharges on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	bool bRandomizeCharges = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest", meta = (EditCondition = "bRandomizeCharges"))
	int32 MinRandomCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest", meta = (EditCondition = "bRandomizeCharges"))
	int32 MaxRandomCharges = 5;

	/** Percentage of Max Health restored to the party per rest (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	float HealthRestorePercentage = 1.0f;

	/** Percentage of Max Qi restored to the party per rest (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	float QiRestorePercentage = 1.0f;

	/** Whether this terminal should auto-destroy upon exhausting all charges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	bool bAutoDestroyOnDepletion = true;

	/** Whether resting triggers a level-isolated checkpoint save in the dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	bool bTriggerCheckpointSave = true;

	/** Isolated checkpoint save slot (Strictly separate from Garden_AutoSave so garden state is never overwritten!) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Rest")
	FString CheckpointSaveSlot = TEXT("Dungeon_Checkpoint");

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Rest")
	FOnRestTerminalUsed OnRestTerminalUsed;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Rest")
	FOnRestTerminalDepleted OnRestTerminalDepleted;

	// --- Interaction API ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;

	/**
	 * Executes rest logic on the interacting character and their active party.
	 * Can also be triggered directly from Blueprints or Details panel via CallInEditor!
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Rest")
	bool RestParty(ASovereignBaseCharacter* InteractingCharacter);

	/** Resets charges back to MaxRestCharges */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Rest")
	void ResetCharges(int32 NewChargeCount = -1);
};
