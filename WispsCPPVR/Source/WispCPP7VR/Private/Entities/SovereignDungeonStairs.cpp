// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#include "Entities/SovereignDungeonStairs.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ASovereignDungeonStairs::ASovereignDungeonStairs()
{
	bIsUnlocked = false;
	bRequireBossDefeat = true;
	StairsDisplayName = NSLOCTEXT("Sovereign", "DescentStairsName", "Descent Stairs to Deeper Vault");
}

void ASovereignDungeonStairs::NotifyBossDefeated()
{
	SetGatewayUnlocked(true);
	UE_LOG(LogTemp, Log, TEXT("[DungeonStairs] %s unsealed following Vault Guardian defeat!"), *GetName());
}

FString ASovereignDungeonStairs::GetInteractionHint_Implementation()
{
	if (!bIsUnlocked)
	{
		return TEXT("Sealed Stairs - Defeat Vault Boss to open");
	}
	return TEXT("Sacrifice Items or Descend Deeper");
}

FText ASovereignDungeonStairs::GetInteractableName_Implementation()
{
	return StairsDisplayName;
}

void ASovereignDungeonStairs::OnInteract_Implementation(AActor* Interactor)
{
	if (!bIsUnlocked && !bRequireBossDefeat)
	{
		SetGatewayUnlocked(true);
	}

	if (bIsUnlocked)
	{
		ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor);
		DescendToNextFloor(Character);
	}
	else
	{
		Super::OnInteract_Implementation(Interactor);
	}
}

bool ASovereignDungeonStairs::DescendToNextFloor(ASovereignBaseCharacter* PartyLeader)
{
	if (!bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonStairs] Cannot descend: Stairs are sealed."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PartyLeader)
	{
		World = PartyLeader->GetWorld();
	}
	if (!World)
	{
		return false;
	}

	// 1. Calculate sacrifice bonuses
	const float EnemyBuff = CalculateEnemyBuffMultiplier();
	const float RewardMult = CalculateRewardMultiplier();

	// 2. Query Dungeon Subsystem and increment depth
	int32 NewDepth = 1;
	UGameInstance* GI = World->GetGameInstance();
	USovereignDungeonSubsystem* DungeonSub = GI ? GI->GetSubsystem<USovereignDungeonSubsystem>() : nullptr;
	if (DungeonSub)
	{
		DungeonSub->AdvanceFloorDepth(EnemyBuff, RewardMult);
		NewDepth = DungeonSub->GetCurrentDepth();
	}

	// 3. Consume sacrifices
	ConsumeSacrifices();

	// If player carries a Boss Key, delete it upon using descent stairs
	if (PartyLeader)
	{
		if (USovereignInventoryComponent* LeaderInv = PartyLeader->GetInventoryComponent())
		{
			const FGameplayTag BossKeyTag = FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
			if (BossKeyTag.IsValid() && LeaderInv->GetItemCountByTag(BossKeyTag) > 0)
			{
				const int32 KeysDeleted = LeaderInv->GetItemCountByTag(BossKeyTag);
				LeaderInv->RemoveItemByTag(BossKeyTag, KeysDeleted);
				UE_LOG(LogTemp, Log, TEXT("[DungeonStairs] Deleted %d Boss Key(s) from inventory upon descending."), KeysDeleted);
			}
		}
	}

	OnDungeonStairsDescended.Broadcast(NewDepth);

	// 4. Find active dungeon generator for in-place procedural regeneration (Option B)
	ASovereignDungeonGenerator* Generator = nullptr;
	for (TActorIterator<ASovereignDungeonGenerator> It(World); It; ++It)
	{
		Generator = *It;
		break;
	}

	if (Generator)
	{
		UE_LOG(LogTemp, Log, TEXT("[DungeonStairs] Regenerating dungeon in-place for Depth %d..."), NewDepth);

		// Clean up existing layout
		Generator->CleanupDungeon();

		// Update generation seed based on depth
		if (DungeonSub)
		{
			Generator->GenerationSeed = DungeonSub->ActiveExpeditionSeed;
		}

		// Generate and spawn new floor
		Generator->GenerateDungeon();
		Generator->SpawnDungeonGeometry();

		// Teleport player party to new Node 0 entrance
		if (PartyLeader)
		{
			FTransform EntranceXform = Generator->GetEntranceTransform();
			PartyLeader->SetActorTransform(EntranceXform);
		}
	}

	return true;
}
