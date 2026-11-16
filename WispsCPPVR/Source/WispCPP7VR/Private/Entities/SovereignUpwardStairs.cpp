// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#include "Entities/SovereignUpwardStairs.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ASovereignUpwardStairs::ASovereignUpwardStairs()
{
	DisplayName = NSLOCTEXT("Sovereign", "UpwardStairsName", "Upward Stairs");
	InteractionHint = TEXT("Ascend to Previous Floor");
}

void ASovereignUpwardStairs::OnInteract_Implementation(AActor* Interactor)
{
	ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor);
	AscendFloor(Character);
}

FString ASovereignUpwardStairs::GetInteractionHint_Implementation()
{
	return InteractionHint;
}

FText ASovereignUpwardStairs::GetInteractableName_Implementation()
{
	return DisplayName;
}

bool ASovereignUpwardStairs::AscendFloor(ASovereignBaseCharacter* PartyLeader)
{
	UWorld* World = GetWorld();
	if (!World && PartyLeader)
	{
		World = PartyLeader->GetWorld();
	}
	if (!World)
	{
		return false;
	}

	UGameInstance* GI = World->GetGameInstance();
	USovereignDungeonSubsystem* DungeonSub = GI ? GI->GetSubsystem<USovereignDungeonSubsystem>() : nullptr;

	if (!DungeonSub)
	{
		return false;
	}

	const int32 CurrentDepth = DungeonSub->GetCurrentDepth();

	// If ascending from Depth 1 back to Depth 0, or if Depth <= 0, return to garden
	if (CurrentDepth <= 1)
	{
		UE_LOG(LogTemp, Log, TEXT("[UpwardStairs] Ascending to Depth 0: Returning party to Meditation Garden..."));
		return DungeonSub->ReturnToGarden(World);
	}

	// Ascend one floor up
	DungeonSub->CurrentDepth--;
	DungeonSub->ActiveExpeditionSeed = DungeonSub->ActiveExpeditionSeed - 10007;

	UE_LOG(LogTemp, Log, TEXT("[UpwardStairs] Ascending to Depth %d..."), DungeonSub->CurrentDepth);

	OnUpwardStairsAscended.Broadcast(DungeonSub->CurrentDepth);

	// Regenerate dungeon in-place for previous depth
	for (TActorIterator<ASovereignDungeonGenerator> It(World); It; ++It)
	{
		ASovereignDungeonGenerator* Generator = *It;
		if (Generator)
		{
			Generator->CleanupDungeon();
			Generator->GenerationSeed = DungeonSub->ActiveExpeditionSeed;
			Generator->GenerateDungeon();
			Generator->SpawnDungeonGeometry();

			if (PartyLeader)
			{
				PartyLeader->SetActorTransform(Generator->GetEntranceTransform());
			}
			break;
		}
	}

	return true;
}
