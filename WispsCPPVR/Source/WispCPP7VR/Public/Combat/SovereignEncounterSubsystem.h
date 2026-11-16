// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignEncounterDefinition.h"
#include "SovereignEncounterSubsystem.generated.h"

class ASovereignBaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterTransitionStarted, ASovereignBattleManager*, BattleManager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterTransitionEnded, EBattleState, Outcome);

/**
 * USovereignEncounterSubsystem
 * Global world subsystem managing the transition lifecycle between real-time 3D dungeon
 * navigation and the Dedicated 3D Battle Arena platform.
 * Caches field transforms, teleports the party, listens for battle completion,
 * destroys defeated mobs on Victory, and restores field navigation.
 */
UCLASS()
class WISPCPP7VR_API USovereignEncounterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USovereignEncounterSubsystem();

	/** World transform where the party was exploring before entering battle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Encounter")
	FTransform DungeonReturnTransform;

	/** Reference to the dungeon mob actor that triggered the encounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Encounter")
	TWeakObjectPtr<AActor> InteractedDungeonMob;

	/** Active player party members in the current encounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Encounter")
	TArray<TObjectPtr<ASovereignBaseCharacter>> ActivePlayerParty;

	/** Reference to active battle manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Encounter")
	TWeakObjectPtr<ASovereignBattleManager> ActiveBattleManager;

	/** Whether a combat encounter is currently active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Encounter")
	bool bIsEncounterActive = false;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Encounter|Events")
	FOnEncounterTransitionStarted OnEncounterTransitionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Encounter|Events")
	FOnEncounterTransitionEnded OnEncounterTransitionEnded;

	// --- Lifecycle API ---

	/**
	 * Initiates a battle encounter from roaming dungeon exploration.
	 * Caches player transform, teleports party to the arena, initializes the battle manager,
	 * and hides the roaming mob.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	bool TriggerEncounter(
		ASovereignBaseCharacter* PlayerLeader,
		AActor* OverlappedMob,
		const TArray<ASovereignBaseCharacter*>& EnemyUnits,
		bool bCanFlee = true,
		ASovereignBattleManager* CustomBattleManager = nullptr);

	/**
	 * Initiates a battle encounter from an authored USovereignEncounterDefinition.
	 * Dynamically spawns enemy vessels in formation, teleports party, activates battle camera,
	 * and disables player field exploration input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	bool TriggerEncounterWithDefinition(
		ASovereignBaseCharacter* PlayerLeader,
		AActor* OverlappedMob,
		USovereignEncounterDefinition* EncounterDef,
		ASovereignBattleManager* CustomBattleManager = nullptr);

	/**
	 * Concludes the encounter, returning party members to the dungeon floor.
	 * Destroys the defeated dungeon mob on Victory; preserves mob on Fled.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	void ReturnPartyToDungeon(EBattleState Outcome);

	/** Delegate hook bound to ASovereignBattleManager::OnBattleEnded */
	UFUNCTION()
	void HandleBattleEnded(EBattleState Outcome);
};
