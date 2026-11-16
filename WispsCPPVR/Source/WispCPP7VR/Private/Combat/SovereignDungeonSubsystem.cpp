// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignDungeonSubsystem.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/SovereignPartyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void USovereignDungeonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveExpeditionSeed = 1337;
	ActiveEnemyBuffMultiplier = 1.0f;
	ActiveRewardMultiplier = 1.0f;
	CurrentDepth = 0;
	DepthEnemyBuffRate = 0.25f;
	DepthRewardRate = 0.20f;
	PartyLeaderClassPath.Empty();
	PartyLeaderGUID.Invalidate();
	PartyLeaderStateJson.Empty();
	ActivePartyMembersData.Empty();
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

void USovereignDungeonSubsystem::RecordPartyLeaderState(const FString& InClassPath, const FGuid& InGuid, const FString& InStateJson)
{
	PartyLeaderClassPath = InClassPath;
	PartyLeaderGUID = InGuid;
	PartyLeaderStateJson = InStateJson;

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Party Leader State recorded: Class=%s, GUID=%s, StateLen=%d"),
		*PartyLeaderClassPath, *PartyLeaderGUID.ToString(), PartyLeaderStateJson.Len());
}

void USovereignDungeonSubsystem::RecordPartyMemberState(int32 SlotIndex, const FString& InClassPath, const FGuid& InGuid, const FString& InStateJson, bool bIsLeader)
{
	ActivePartyMembersData.RemoveAll([SlotIndex](const FSovereignPartyMemberData& Existing) {
		return Existing.SlotIndex == SlotIndex;
	});

	FSovereignPartyMemberData MemberData;
	MemberData.SlotIndex = SlotIndex;
	MemberData.ClassPath = InClassPath;
	MemberData.MemberGUID = InGuid;
	MemberData.StateJson = InStateJson;
	MemberData.bIsLeader = bIsLeader;

	ActivePartyMembersData.Add(MemberData);

	UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Recorded party member [Slot %d]: Class [%s], GUID [%s], Leader: %s"),
		SlotIndex, *InClassPath, *InGuid.ToString(), bIsLeader ? TEXT("True") : TEXT("False"));
}

void USovereignDungeonSubsystem::ClearPartyMembersState()
{
	ActivePartyMembersData.Empty();
}

void USovereignDungeonSubsystem::ClearExpedition()
{
	ActiveExpeditionSeed = 1337;
	DungeonGeneratorClassPath.Empty();
	DungeonGeneratorClassOverride = nullptr;
	ActiveEnemyBuffMultiplier = 1.0f;
	ActiveRewardMultiplier = 1.0f;
	CurrentDepth = 0;
	CollectedKeyGuids.Empty();
	PartyLeaderClassPath.Empty();
	PartyLeaderGUID.Invalidate();
	PartyLeaderStateJson.Empty();
	ActivePartyMembersData.Empty();
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

	// Ticket B-123: Bidirectional Progression Synchronization
	// Capture the current runtime state of the active party leader and party members in the dungeon world
	// and merge their updated progression (XP, levels, attributes, loot) into the Garden save file.
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (USaveManager* SaveManager = GI->GetSubsystem<USaveManager>())
		{
			ASovereignBaseCharacter* LeaderChar = nullptr;
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				LeaderChar = Cast<ASovereignBaseCharacter>(PC->GetPawn());
			}

			if (LeaderChar)
			{
				if (USovereignSaveableEntityComponent* LeaderSave = LeaderChar->FindComponentByClass<USovereignSaveableEntityComponent>())
				{
					TSharedPtr<FJsonObject> LeaderState = LeaderSave->CaptureFullEntityState();
					USaveManager::UpdateEntityInSavedSlot(GardenSaveSlot, LeaderSave->EntityID, LeaderState);
					UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Synchronized Leader [%s] progression to slot [%s]."),
						*LeaderChar->GetName(), *GardenSaveSlot);
				}

				if (USovereignPartyComponent* PartyComp = LeaderChar->GetPartyComponent())
				{
					for (const FSovereignPartySlot& Slot : PartyComp->GridSlots)
					{
						if (Slot.MemberCharacter.IsValid() && Slot.MemberCharacter.Get() != LeaderChar)
						{
							ASovereignBaseCharacter* MemberChar = Slot.MemberCharacter.Get();
							if (USovereignSaveableEntityComponent* MemberSave = MemberChar->FindComponentByClass<USovereignSaveableEntityComponent>())
							{
								TSharedPtr<FJsonObject> MemberState = MemberSave->CaptureFullEntityState();
								USaveManager::UpdateEntityInSavedSlot(GardenSaveSlot, MemberSave->EntityID, MemberState);
								UE_LOG(LogTemp, Log, TEXT("[SovereignDungeonSubsystem] Synchronized Member [%s] progression to slot [%s]."),
									*MemberChar->GetName(), *GardenSaveSlot);
							}
						}
					}
				}
			}
		}
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
