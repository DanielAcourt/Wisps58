// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignDungeonGateway.h"
#include "SovereignDungeonStairs.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonStairsDescended, int32, NewDepth);

/**
 * ASovereignDungeonStairs (Ticket B-116: DU-Add stairs)
 * Interactive procedural descent stairs descending to deeper dungeon floors.
 * Inherits from ASovereignDungeonGateway, providing full sacrifice inventory and Qi channeling
 * to power up enemy scaling and loot multiplier rewards on the next floor.
 * Supports seamless in-level procedural floor regeneration (Option B).
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonStairs : public ASovereignDungeonGateway
{
	GENERATED_BODY()

public:
	ASovereignDungeonStairs();

	/** Display title for the descent stairs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Stairs")
	FText StairsDisplayName;

	/** If true, stairs start sealed/locked until the floor boss is defeated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Stairs")
	bool bRequireBossDefeat = true;

	/** Delegate broadcast when party descends to the next floor */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Stairs")
	FOnDungeonStairsDescended OnDungeonStairsDescended;

	// --- ASovereignBaseInteractable & ASovereignDungeonGateway Overrides ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;
	virtual FText GetInteractableName_Implementation() override;

	/** Activates/unseals the stairs upon boss defeat */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Stairs")
	void NotifyBossDefeated();

	/**
	 * Executes seamless descent to the next dungeon depth:
	 * 1. Consumes sacrifices and adds sacrifice multipliers to the active expedition.
	 * 2. Increments depth in USovereignDungeonSubsystem.
	 * 3. Triggers procedural dungeon regeneration in-place with a depth-derived seed.
	 * 4. Teleports the player party to the new Node 0 entrance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Stairs")
	bool DescendToNextFloor(ASovereignBaseCharacter* PartyLeader);
};
