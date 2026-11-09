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

	/** Current dungeon floor depth (0 = entrance floor) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	int32 CurrentDepth = 0;

	/** Set of Key Pickup GUIDs collected during the current expedition */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Expedition")
	TArray<FGuid> CollectedKeyGuids;

	/** Difficulty scaling rate per depth floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Expedition")
	float DepthEnemyBuffRate = 0.25f;

	/** Reward multiplier scaling rate per depth floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Expedition")
	float DepthRewardRate = 0.20f;

	/** Delegate broadcast when a new expedition is launched */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Events")
	FOnExpeditionLaunched OnExpeditionLaunched;

	/**
	 * Records expedition state in the GameInstance Subsystem prior to level transition.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	void RecordExpedition(int32 Seed, float EnemyBuff, float RewardMult, FName ReturnMapName, const FString& SaveSlot = TEXT("Garden_AutoSave"));

	/**
	 * Advances floor depth and applies scaling multipliers (Ticket B-116 & B-117).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	int32 AdvanceFloorDepth(float ExtraSacrificeBuff = 1.0f, float ExtraSacrificeReward = 1.0f);

	/**
	 * Returns the current floor depth.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Expedition")
	int32 GetCurrentDepth() const { return CurrentDepth; }

	/**
	 * Clears active expedition parameters upon returning to the garden.
	 */
	/**
	 * Records a key pickup as collected by its GUID.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	void RecordKeyCollected(const FGuid& KeyGuid);

	/**
	 * Checks if a key pickup has been collected.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Expedition")
	bool IsKeyCollected(const FGuid& KeyGuid) const;

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition")
	void ClearExpedition();

	/**
	 * Returns the player party to the Meditation Garden and restores saved world state.
	 * Calls OpenLevel to load the GardenReturnMapName, and loads GardenSaveSlot state via USaveManager.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Expedition", meta = (WorldContext = "WorldContextObject"))
	bool ReturnToGarden(const UObject* WorldContextObject);
};
