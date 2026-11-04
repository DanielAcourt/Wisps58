// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SovereignDungeonSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnExpeditionLaunched, int32, Seed, float, EnemyBuff, float, RewardMult, FName, TargetDungeonLevel);

/**
 * USovereignDungeonSubsystem
 * GameInstance Subsystem managing persistent dungeon expedition state across level transitions.
 * Survives OpenLevel transitions between the Meditation Garden and procedural Dungeon levels.
 * Stores expedition seeds, risk-vs-reward sacrifice multipliers, return garden level info,
 * and handles returning to the garden with state restoration.
 */
UCLASS(BlueprintType)
class WISPCPP7VR_API USovereignDungeonSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Generation seed for procedural dungeon generation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	int32 ActiveExpeditionSeed = 1337;

	/** Combat stat multiplier applied to enemies inside the dungeon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	float ActiveEnemyBuffMultiplier = 1.0f;

	/** Loot drop / dragon egg mutation multiplier applied to dungeon rewards */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	float ActiveRewardMultiplier = 1.0f;

	/** Name of the Garden level to return to upon expedition completion or flight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	FName GardenReturnMapName = NAME_None;

	/** Save slot name used for auto-saving the garden prior to embarking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	FString GardenSaveSlot = TEXT("Garden_AutoSave");

	/** Whether an expedition is currently ongoing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	bool bHasActiveExpedition = false;

	/** Delegate broadcast when a new expedition is launched */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Events")
	FOnExpeditionLaunched OnExpeditionLaunched;

	/**
	 * Records expedition state in the GameInstance Subsystem prior to level transition.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	void RecordExpedition(int32 Seed, float EnemyBuff, float RewardMult, FName ReturnMapName, const FString& SaveSlot = TEXT("Garden_AutoSave"));

	/**
	 * Clears active expedition parameters upon returning to the garden.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	void ClearExpedition();

	/**
	 * Returns the player party to the Meditation Garden and restores saved world state.
	 * Calls OpenLevel to load the GardenReturnMapName, and loads GardenSaveSlot state via USaveManager.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition", meta = (WorldContext = "WorldContextObject"))
	bool ReturnToGarden(const UObject* WorldContextObject);
};
