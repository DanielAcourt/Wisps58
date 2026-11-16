// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "SovereignUpwardStairs.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpwardStairsAscended, int32, TargetDepth);

/**
 * ASovereignUpwardStairs (Ticket B-117)
 * Interactive upward stairs spawned at Node 0 of procedural dungeons when Depth >= 1.
 * Allows the player party to ascend back to the previous floor (Depth - 1),
 * while Depth 0 retains ASovereignDungeonEntrance for returning to the Meditation Garden.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignUpwardStairs : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignUpwardStairs();

	/** Display name for UI / interaction prompt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Stairs")
	FText DisplayName;

	/** User interaction hint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Stairs")
	FString InteractionHint = TEXT("Ascend to Previous Floor");

	/** Delegate broadcast upon ascending */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Stairs")
	FOnUpwardStairsAscended OnUpwardStairsAscended;

	// --- ASovereignBaseInteractable Interface ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;
	virtual FText GetInteractableName_Implementation() override;

	/**
	 * Ascends to the previous depth level (or returns to garden if Depth becomes 0).
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Stairs")
	bool AscendFloor(ASovereignBaseCharacter* PartyLeader);
};
