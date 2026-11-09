// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignDungeonSubsystem.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void USovereignDungeonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveExpeditionSeed = 1337;
	ActiveEnemyBuffMultiplier = 1.0f;
	ActiveRewardMultiplier = 1.0f;
	CurrentDepth = 0;
	DepthEnemyBuffRate = 0.25f;
	DepthRewardRate = 0.20f;
	GardenReturnMapName = NAME_None;
	GardenSaveSlot = TEXT("Garden_AutoSave");
	bHasActiveExpedition = false;

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Initialized GameInstance Subsystem."));
}

void USovereignDungeonSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USovereignDungeonSubsystem::RecordExpedition(int32 Seed, float EnemyBuff, float RewardMult, FName ReturnMapName, const FString& SaveSlot)
{
	ActiveExpeditionSeed = Seed;
	CurrentDepth = 0;
	ActiveEnemyBuffMultiplier = FMath::Max(1.0f, EnemyBuff);
	ActiveRewardMultiplier = FMath::Max(1.0f, RewardMult);
	GardenReturnMapName = ReturnMapName;
	GardenSaveSlot = SaveSlot.IsEmpty() ? TEXT("Garden_AutoSave") : SaveSlot;
	bHasActiveExpedition = true;

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Expedition Recorded: Seed=%d, EnemyBuff=x%f, RewardMult=x%f, ReturnMap=%s, SaveSlot=%s"),
		ActiveExpeditionSeed, ActiveEnemyBuffMultiplier, ActiveRewardMultiplier, *GardenReturnMapName.ToString(), *GardenSaveSlot);

	OnExpeditionLaunched.Broadcast(ActiveExpeditionSeed, ActiveEnemyBuffMultiplier, ActiveRewardMultiplier, GardenReturnMapName);
}

int32 USovereignDungeonSubsystem::AdvanceFloorDepth(float ExtraSacrificeBuff, float ExtraSacrificeReward)
{
	CurrentDepth++;
	// Seed changes deterministically per floor
	ActiveExpeditionSeed = ActiveExpeditionSeed + 10007;

	// Scale difficulty and rewards with depth
	const float DepthBuffFactor = 1.0f + (CurrentDepth * DepthEnemyBuffRate);
	const float DepthRewardFactor = 1.0f + (CurrentDepth * DepthRewardRate);

	ActiveEnemyBuffMultiplier = FMath::Max(1.0f, (ActiveEnemyBuffMultiplier * ExtraSacrificeBuff) * DepthBuffFactor);
	ActiveRewardMultiplier = FMath::Max(1.0f, (ActiveRewardMultiplier * ExtraSacrificeReward) * DepthRewardFactor);

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Advanced to Depth %d! New Seed: %d, EnemyBuff: x%f, RewardMult: x%f"),
		CurrentDepth, ActiveExpeditionSeed, ActiveEnemyBuffMultiplier, ActiveRewardMultiplier);

	return CurrentDepth;
}

void USovereignDungeonSubsystem::RecordKeyCollected(const FGuid& KeyGuid)
{
	if (KeyGuid.IsValid() && !CollectedKeyGuids.Contains(KeyGuid))
	{
		CollectedKeyGuids.Add(KeyGuid);
		UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Key recorded as collected: %s"), *KeyGuid.ToString());
	}
}

bool USovereignDungeonSubsystem::IsKeyCollected(const FGuid& KeyGuid) const
{
	return KeyGuid.IsValid() && CollectedKeyGuids.Contains(KeyGuid);
}

void USovereignDungeonSubsystem::ClearExpedition()
{
	ActiveExpeditionSeed = 1337;
	ActiveEnemyBuffMultiplier = 1.0f;
	ActiveRewardMultiplier = 1.0f;
	CurrentDepth = 0;
	CollectedKeyGuids.Empty();
	bHasActiveExpedition = false;

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Expedition state cleared."));
}

bool USovereignDungeonSubsystem::ReturnToGarden(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SovereignDungeonSubsystem] ReturnToGarden: Invalid WorldContextObject."));
		return false;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		World = WorldContextObject->GetWorld();
	}

	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SovereignDungeonSubsystem] ReturnToGarden: Unable to retrieve World context."));
		return false;
	}

	// Determine map name to open
	FName TargetMap = GardenReturnMapName;
	if (TargetMap.IsNone())
	{
		TargetMap = FName(TEXT("TestWorld")); // Fallback to primary world map if unspecified
	}

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Returning to Garden map [%s]..."), *TargetMap.ToString());

	// If we have an active game instance, we can trigger level load
	UGameplayStatics::OpenLevel(World, TargetMap);

	// In the loaded map, the USaveManager subsystem can restore the state
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (USaveManager* SaveManager = GI->GetSubsystem<USaveManager>())
		{
			// Mark to restore on next map load or trigger load immediately if in same level
			SaveManager->LoadWorldState(GardenSaveSlot, true);
		}
	}

	ClearExpedition();
	return true;
}
