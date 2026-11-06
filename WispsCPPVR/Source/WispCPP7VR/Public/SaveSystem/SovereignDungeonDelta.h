// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#pragma once

#include "CoreMinimal.h"
#include "SovereignDungeonDelta.generated.h"

/**
 * FSovereignDungeonDeltaState
 * Lightweight struct capturing mutated state deltas and generation seed for mega-dungeons (Ticket B-115).
 * Prevents saving thousands of unmutated procedural actors while preserving full world fidelity.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonDeltaState
{
	GENERATED_BODY()

	/** Generation seed for deterministic layout reconstruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	int32 ExpeditionSeed = 0;

	/** Enemy combat scaling multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	float EnemyBuffMultiplier = 1.0f;

	/** Reward scaling multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	float RewardMultiplier = 1.0f;

	/** Map of Door GUID to boolean is open */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	TMap<FGuid, bool> DoorStates;

	/** Map of Chest GUID to boolean is opened */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	TMap<FGuid, bool> ChestStates;

	/** Map of Rest Terminal GUID to remaining charges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	TMap<FGuid, int32> RestTerminalCharges;

	/** Set of collected Key Pickup GUIDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	TArray<FGuid> CollectedKeyGuids;

	/** Whether the boss was defeated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData|Dungeon")
	bool bBossDefeated = false;
};
