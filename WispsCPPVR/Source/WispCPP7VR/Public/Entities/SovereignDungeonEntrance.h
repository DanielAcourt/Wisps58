// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-05

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "SovereignDungeonEntrance.generated.h"

/**
 * ASovereignDungeonEntrance (Ticket B-111)
 * Interactive dungeon entrance portal placed at Node 0 of procedural dungeons.
 * Allows player characters to interact with the threshold and return to the Meditation Garden.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonEntrance : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignDungeonEntrance();

	/** Target return map or save slot (queried from USovereignDungeonSubsystem if left empty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Entrance")
	FString ReturnSaveSlot = TEXT("Garden_AutoSave");

	/** User-facing display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Entrance")
	FText DisplayName;

	/** User-facing interaction hint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Entrance")
	FString InteractionHint = TEXT("Return to Meditation Garden");

	// --- ASovereignBaseInteractable Interface ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;
	virtual FText GetInteractableName_Implementation() override;

	/** Executes return to garden flow via USovereignDungeonSubsystem */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Entrance")
	bool ReturnToGarden();
};
