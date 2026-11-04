// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "CoreModules/SovereignDungeonGameMode.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Combat/SovereignBattleManager.h"
#include "Components/SovereignPartyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ASovereignDungeonGameMode::ASovereignDungeonGameMode()
{
	bLockPossessionToLeader = true;
	DefaultPawnClass = ASovereignBaseCharacter::StaticClass();
}

void ASovereignDungeonGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. Locate the dungeon generator in the world
	TArray<AActor*> FoundGenerators;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASovereignDungeonGenerator::StaticClass(), FoundGenerators);
	if (FoundGenerators.Num() > 0)
	{
		ActiveDungeonGenerator = Cast<ASovereignDungeonGenerator>(FoundGenerators[0]);
	}

	// 2. Align local player controller if already logged in
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			SetupPartyLeaderAtEntrance(PC);
		}
	}
}

void ASovereignDungeonGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	SetupPartyLeaderAtEntrance(NewPlayer);
}

void ASovereignDungeonGameMode::SetupPartyLeaderAtEntrance(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	APawn* ControlledPawn = PC->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// If player spawned as a Wisp, ensure it is possessing or transitioning into the party leader vessel
	if (ASovereignPlayerWisp* Wisp = Cast<ASovereignPlayerWisp>(ControlledPawn))
	{
		if (bLockPossessionToLeader)
		{
			// Prevent unpossession keybind from releasing the wisp in the dungeon
			Wisp->bIsPossessing = true;
		}

		if (Wisp->CurrentHost)
		{
			DungeonPartyLeader = Cast<ASovereignBaseCharacter>(Wisp->CurrentHost);
		}
	}
	else if (ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(ControlledPawn))
	{
		DungeonPartyLeader = Character;
	}

	// Teleport the party leader to the Entrance Room WorldTransform
	if (ActiveDungeonGenerator)
	{
		const FTransform EntranceTransform = ActiveDungeonGenerator->GetEntranceTransform();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorTransform(EntranceTransform, false, nullptr, ETeleportType::TeleportPhysics);
			PC->SetControlRotation(EntranceTransform.Rotator());
			UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Teleported party leader to Entrance Room (%s)"),
				*EntranceTransform.GetLocation().ToString());
		}
	}

	ConfigurePartyForExploration();
}

void ASovereignDungeonGameMode::ConfigurePartyForExploration()
{
	if (!DungeonPartyLeader)
	{
		return;
	}

	// Query party members: keep leader active, hide other party members and anchor them on the battle arena
	USovereignPartyComponent* PartyComp = DungeonPartyLeader->GetPartyComponent();
	if (!PartyComp)
	{
		return;
	}

	ASovereignBattleManager* BattleArena = ActiveDungeonGenerator ? ActiveDungeonGenerator->GetBattleArenaForBiome(0) : nullptr;

	for (int32 SlotIdx = 0; SlotIdx < PartyComp->PartyMembers.Num(); ++SlotIdx)
	{
		const FSovereignPartySlot& Slot = PartyComp->PartyMembers[SlotIdx];
		if (!Slot.bIsOccupied || !Slot.AssignedCharacter.IsValid())
		{
			continue;
		}

		ASovereignBaseCharacter* Member = Slot.AssignedCharacter.Get();
		if (Member == DungeonPartyLeader)
		{
			// Leader is visible and active exploring the dungeon corridors
			Member->SetActorHiddenInGame(false);
			Member->SetActorEnableCollision(true);
		}
		else
		{
			// Non-leader party members remain hidden during dungeon corridor traversal
			Member->SetActorHiddenInGame(true);
			Member->SetActorEnableCollision(false);

			// Pre-anchor to battle platform slot transforms so they are in position when combat triggers
			if (BattleArena && BattleArena->PlayerSlotTransforms.IsValidIndex(SlotIdx))
			{
				Member->SetActorTransform(BattleArena->PlayerSlotTransforms[SlotIdx], false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
}
