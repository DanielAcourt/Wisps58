// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignEncounterSubsystem.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignPartyComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

USovereignEncounterSubsystem::USovereignEncounterSubsystem()
{
	bIsEncounterActive = false;
}

bool USovereignEncounterSubsystem::TriggerEncounter(
	ASovereignBaseCharacter* PlayerLeader,
	AActor* OverlappedMob,
	const TArray<ASovereignBaseCharacter*>& EnemyUnits,
	bool bCanFlee,
	ASovereignBattleManager* CustomBattleManager)
{
	if (!PlayerLeader)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PlayerLeader)
	{
		World = PlayerLeader->GetWorld();
	}
	if (!World && CustomBattleManager)
	{
		World = CustomBattleManager->GetWorld();
	}

	// 1. Locate or Spawn Battle Manager
	ASovereignBattleManager* BattleMgr = CustomBattleManager;
	if (!BattleMgr && World)
	{
		for (TActorIterator<ASovereignBattleManager> It(World); It; ++It)
		{
			BattleMgr = *It;
			break;
		}

		if (!BattleMgr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// Spawn 5000 units above player as default arena platform if none exists in level
			FVector ArenaPos = PlayerLeader->GetActorLocation() + FVector(0.0f, 0.0f, 5000.0f);
			BattleMgr = World->SpawnActor<ASovereignBattleManager>(ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams);
		}
	}

	if (!BattleMgr)
	{
		return false;
	}

	ActiveBattleManager = BattleMgr;
	InteractedDungeonMob = OverlappedMob;

	// 2. Cache Dungeon Return Transform
	DungeonReturnTransform = PlayerLeader->GetActorTransform();

	// 3. Assemble Player Party
	ActivePlayerParty.Empty();
	TArray<ASovereignBaseCharacter*> DeployedParty;

	if (USovereignPartyComponent* PartyComp = PlayerLeader->GetPartyComponent())
	{
		DeployedParty = PartyComp->GetAllActivePartyMembers();
	}
	if (DeployedParty.Num() == 0)
	{
		DeployedParty.Add(PlayerLeader);
	}

	for (auto* Member : DeployedParty)
	{
		ActivePlayerParty.Add(Member);
	}

	// 4. Hide/Disable Collision on Roaming Dungeon Mob
	if (OverlappedMob)
	{
		OverlappedMob->SetActorHiddenInGame(true);
		OverlappedMob->SetActorEnableCollision(false);
	}

	// 5. Bind Battle Ended delegate
	BattleMgr->OnBattleEnded.RemoveDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);
	BattleMgr->OnBattleEnded.AddDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);

	// 6. Initialize Encounter on Battle Manager
	bIsEncounterActive = true;
	BattleMgr->InitializeEncounter(DeployedParty, EnemyUnits, bCanFlee);

	// 7. Activate Battle Camera & disable exploration input
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			BattleMgr->ActivateBattleCamera(PC);
			PlayerLeader->DisableInput(PC);
		}
	}

	OnEncounterTransitionStarted.Broadcast(BattleMgr);

	return true;
}

bool USovereignEncounterSubsystem::TriggerEncounterWithDefinition(
	ASovereignBaseCharacter* PlayerLeader,
	AActor* OverlappedMob,
	USovereignEncounterDefinition* EncounterDef,
	ASovereignBattleManager* CustomBattleManager)
{
	if (!PlayerLeader || !EncounterDef)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PlayerLeader)
	{
		World = PlayerLeader->GetWorld();
	}
	if (!World && CustomBattleManager)
	{
		World = CustomBattleManager->GetWorld();
	}

	// 1. Locate or Spawn Battle Manager
	ASovereignBattleManager* BattleMgr = CustomBattleManager;
	if (!BattleMgr && World)
	{
		for (TActorIterator<ASovereignBattleManager> It(World); It; ++It)
		{
			BattleMgr = *It;
			break;
		}

		if (!BattleMgr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector ArenaPos = PlayerLeader->GetActorLocation() + FVector(0.0f, 0.0f, 5000.0f);
			BattleMgr = World->SpawnActor<ASovereignBattleManager>(ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams);
		}
	}

	if (!BattleMgr)
	{
		return false;
	}

	ActiveBattleManager = BattleMgr;
	InteractedDungeonMob = OverlappedMob;

	// 2. Cache Dungeon Return Transform
	DungeonReturnTransform = PlayerLeader->GetActorTransform();

	// 3. Assemble Player Party
	ActivePlayerParty.Empty();
	TArray<ASovereignBaseCharacter*> DeployedParty;

	if (USovereignPartyComponent* PartyComp = PlayerLeader->GetPartyComponent())
	{
		DeployedParty = PartyComp->GetAllActivePartyMembers();
	}
	if (DeployedParty.Num() == 0)
	{
		DeployedParty.Add(PlayerLeader);
	}

	for (auto* Member : DeployedParty)
	{
		ActivePlayerParty.Add(Member);
	}

	// 4. Hide/Disable Collision on Roaming Dungeon Mob
	if (OverlappedMob)
	{
		OverlappedMob->SetActorHiddenInGame(true);
		OverlappedMob->SetActorEnableCollision(false);
	}

	// 5. Bind Battle Ended delegate
	BattleMgr->OnBattleEnded.RemoveDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);
	BattleMgr->OnBattleEnded.AddDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);

	// 6. Initialize Encounter on Battle Manager from definition
	bIsEncounterActive = true;
	BattleMgr->InitializeEncounterFromDefinition(DeployedParty, EncounterDef);

	// 7. Activate Battle Camera & disable exploration input
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			BattleMgr->ActivateBattleCamera(PC);
			PlayerLeader->DisableInput(PC);
		}
	}

	OnEncounterTransitionStarted.Broadcast(BattleMgr);
	return true;
}

void USovereignEncounterSubsystem::HandleBattleEnded(EBattleState Outcome)
{
	ReturnPartyToDungeon(Outcome);
}

void USovereignEncounterSubsystem::ReturnPartyToDungeon(EBattleState Outcome)
{
	if (!bIsEncounterActive)
	{
		return;
	}

	// 0. Restore Camera & Re-enable Input
	UWorld* World = GetWorld();
	if (ActiveBattleManager.IsValid() && ActivePlayerParty.Num() > 0 && ActivePlayerParty[0])
	{
		if (World)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				ActiveBattleManager->RestorePlayerCamera(PC, ActivePlayerParty[0]);
				ActivePlayerParty[0]->EnableInput(PC);
			}
		}
	}

	// 1. Teleport Living Party Members Back to Dungeon Floor
	for (int32 i = 0; i < ActivePlayerParty.Num(); ++i)
	{
		if (ActivePlayerParty[i])
		{
			// Offset party members slightly around the leader return transform
			FVector Offset = (i == 0) ? FVector::ZeroVector : FVector(0.0f, (i % 2 == 0 ? 1.0f : -1.0f) * ((i + 1) / 2) * 100.0f, 0.0f);
			FTransform TargetTransform = DungeonReturnTransform;
			TargetTransform.AddToTranslation(Offset);

			ActivePlayerParty[i]->SetActorTransform(TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// 2. Mob Cleanup vs Restoration
	if (InteractedDungeonMob.IsValid())
	{
		if (Outcome == EBattleState::Victory)
		{
			// Mob defeated: destroy from world
			InteractedDungeonMob->Destroy();
		}
		else if (Outcome == EBattleState::Fled)
		{
			// Fled: unhide mob so player can re-encounter or navigate around it
			InteractedDungeonMob->SetActorHiddenInGame(false);
			InteractedDungeonMob->SetActorEnableCollision(true);
		}
	}

	bIsEncounterActive = false;
	OnEncounterTransitionEnded.Broadcast(Outcome);
}
