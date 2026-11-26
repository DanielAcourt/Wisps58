// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignEncounterDefinition.h"
#include "SovereignEncounterSubsystem.generated.h"

class ASovereignBaseCharacter;
class USovereignEnemyDefinition;
class AASovereignBattleArena;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterTransitionStarted, ASovereignBattleManager*, BattleManager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterTransitionEnded, EBattleState, Outcome);

USTRUCT(BlueprintType)
struct FSovereignEncounterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Encounter")
	TSoftObjectPtr<USovereignEnemyDefinition> EnemyDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Encounter")
	int32 PointCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Encounter")
	int32 MaxPerEncounter = 4;
};

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

	/** Optional explicit battle arena override; if null, subsystem searches the world for an AASovereignBattleArena */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Encounter")
	TObjectPtr<AASovereignBattleArena> PreferredBattleArena;

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

	// --- Spawning & Lifecycle API ---

	/** Rolls a deterministic random enemy party within EncounterBudget and initializes each vessel from its Data Asset */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	TArray<ASovereignBaseCharacter*> SpawnRandomEncounterParty(
		const TArray<FSovereignEncounterEntry>& CandidatePool,
		const TArray<FTransform>& SpawnTransforms,
		int32 EncounterBudget,
		int32 RandomSeed = 1337,
		float EnemyStatMultiplier = 1.0f);

	/** Convenience entry point: rolls a random enemy party and immediately transitions into battle */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	bool TriggerRandomEncounter(
		ASovereignBaseCharacter* PlayerLeader,
		AActor* OverlappedMob,
		const TArray<FSovereignEncounterEntry>& CandidatePool,
		const TArray<FTransform>& SpawnTransforms,
		int32 EncounterBudget,
		int32 RandomSeed = 1337,
		bool bCanFlee = true,
		ASovereignBattleManager* CustomBattleManager = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	bool TriggerEncounter(
		ASovereignBaseCharacter* PlayerLeader,
		AActor* OverlappedMob,
		const TArray<ASovereignBaseCharacter*>& EnemyUnits,
		bool bCanFlee = true,
		ASovereignBattleManager* CustomBattleManager = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	bool TriggerEncounterWithDefinition(
		ASovereignBaseCharacter* PlayerLeader,
		AActor* OverlappedMob,
		USovereignEncounterDefinition* EncounterDef,
		ASovereignBattleManager* CustomBattleManager = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Encounter")
	void ReturnPartyToDungeon(EBattleState Outcome);

	UFUNCTION()
	void HandleBattleEnded(EBattleState Outcome);

};
