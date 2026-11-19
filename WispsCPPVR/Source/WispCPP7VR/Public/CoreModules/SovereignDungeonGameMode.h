// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "CoreModules/WispsGameModeBase.h"
#include "SovereignDungeonGameMode.generated.h"

class ASovereignBaseCharacter;
class ASovereignDungeonGenerator;
class ASovereignBattleManager;
class USovereignEncounterDefinition;
class USovereignEncounterSubsystem;

/**
 * ASovereignDungeonGameMode
 * Dedicated GameMode for subterranean procedural dungeon expeditions.
 * Enforces uniform enhanced input controls, locks the player wisp into the possessed
 * party leader dragon vessel (preventing unpossession while exploring the dungeon),
 * automatically positions the party leader at the dungeon entrance node,
 * and maintains non-leader party members hidden on the battle arena grid until combat triggers.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonGameMode : public AWispsGameModeBase
{
	GENERATED_BODY()

public:
	ASovereignDungeonGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	ASovereignBattleManager* GetBattleManager(int32 BiomeIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	USovereignEncounterSubsystem* GetEncounterSubsystem() const;

	// Trigger Combat entry point
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	bool TriggerCombatEncounter(USovereignEncounterDefinition* EncounterDefinition, AActor* SourceActor = nullptr, int32 BiomeIndex = 0);

	//Player Controller overrides
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	/** Whether unpossession back into wisp spirit form is strictly prohibited in the dungeon (Defaults to false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	bool bLockPossessionToLeader = false;

	/** Class of dungeon generator to auto-spawn if none is found in the scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	TSubclassOf<ASovereignDungeonGenerator> DungeonGeneratorClass;

	/** Fallback BattleManager class to spawn only if DungeonGenerator did not create any biome arenas */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	TSubclassOf<ASovereignBattleManager> FallbackBattleManagerClass;

	/** Cached pointer to the procedural dungeon generator in the active level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|State")
	TObjectPtr<ASovereignDungeonGenerator> ActiveDungeonGenerator;

	/** Cached pointer to the active party leader vessel currently exploring the dungeon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|State")
	TObjectPtr<ASovereignBaseCharacter> DungeonPartyLeader;

	/**
	 * Aligns the party leader vessel to the Entrance Room node of the active dungeon generator.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void SetupPartyLeaderAtEntrance(APlayerController* PC);

	/**
	 * Configures party member visibility and positions non-leader party members on the arena grid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void ConfigurePartyForExploration();

protected:

	/** Verifies that BattleManager arenas and EncounterSubsystem are online when the dungeon loads */
	void InitializeDungeonSystems();

private:
	/** Cached pointer to the active BattleManager for the current dungeon expedition */
	UPROPERTY()
	TObjectPtr<ASovereignBattleManager> CachedFallbackBattleManager;
};
