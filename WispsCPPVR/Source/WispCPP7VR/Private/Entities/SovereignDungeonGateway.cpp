// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignDungeonGateway.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignQiComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Inventory/SovereignInventoryBroker.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/StaticMeshComponent.h"

ASovereignDungeonGateway::ASovereignDungeonGateway()
{
	bIsUnlocked = false;
	bAutoUnlockOnHatch = true;
	InfusedQi = 0.0f;

	GatewayDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatewayDoorMesh"));
	if (RootComponent)
	{
		GatewayDoorMesh->SetupAttachment(RootComponent);
	}
	else
	{
		RootComponent = GatewayDoorMesh;
	}

	SacrificeInventory = CreateDefaultSubobject<USovereignInventoryComponent>(TEXT("SacrificeInventory"));
	SacrificeInventory->MaxSlotCount = 10;
}

void ASovereignDungeonGateway::BeginPlay()
{
	Super::BeginPlay();

	// Register SacrificeInventoryBroker with SaveDataComponent hub for atomic save persistence
	if (SaveDataComponent && SacrificeInventory)
	{
		SacrificeInventoryBroker = NewObject<USovereignInventoryBroker>(this, TEXT("SacrificeInventoryBroker"));
		SacrificeInventoryBroker->Initialize(SacrificeInventory);
		SaveDataComponent->RegisterBroker(SacrificeInventoryBroker);
	}
}

void ASovereignDungeonGateway::SetGatewayUnlocked(bool bNewUnlocked)
{
	if (bIsUnlocked != bNewUnlocked)
	{
		bIsUnlocked = bNewUnlocked;
		OnGatewayUnlocked.Broadcast(bIsUnlocked);

		if (GatewayDoorMesh)
		{
			// When unlocked, door collision opens
			GatewayDoorMesh->SetCollisionEnabled(bIsUnlocked ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
		}
	}
}

bool ASovereignDungeonGateway::ChannelQiFromCharacter(ASovereignBaseCharacter* Character, float Amount)
{
	if (!Character || Amount <= 0.0f)
	{
		return false;
	}

	USovereignQiComponent* QiComp = Character->GetQiComponent();
	if (!QiComp || QiComp->CurrentQi < Amount)
	{
		return false;
	}

	QiComp->CurrentQi -= Amount;
	InfusedQi += Amount;

	OnGatewayEmpowered.Broadcast(CalculateEnemyBuffMultiplier(), CalculateRewardMultiplier());
	return true;
}

float ASovereignDungeonGateway::GetTotalSacrificedMass() const
{
	if (!SacrificeInventory)
	{
		return 0.0f;
	}

	float TotalMass = 0.0f;
	for (const FSovereignInventoryItemInstance& Item : SacrificeInventory->InventorySlots)
	{
		const USovereignItemDefinition* Def = Item.ItemDefinition.Get();
		if (!Def)
		{
			Def = Item.ItemDefinition.LoadSynchronous();
		}

		if (Def)
		{
			if (const USovereignItemFragment_Weight* WeightFrag = Def->FindFragment<USovereignItemFragment_Weight>())
			{
				TotalMass += WeightFrag->GetStackWeight(Item.Quantity);
			}
			else
			{
				TotalMass += 1.0f * Item.Quantity;
			}
		}
	}
	return TotalMass;
}

float ASovereignDungeonGateway::CalculateEnemyBuffMultiplier() const
{
	const float Mass = GetTotalSacrificedMass();
	// 1.0 + (SacrificeMass * 0.05) + (InfusedQi * 0.002)
	return 1.0f + (Mass * 0.05f) + (InfusedQi * 0.002f);
}

float ASovereignDungeonGateway::CalculateRewardMultiplier() const
{
	const float Mass = GetTotalSacrificedMass();
	// 1.0 + (SacrificeMass * 0.10) + (InfusedQi * 0.005)
	return 1.0f + (Mass * 0.10f) + (InfusedQi * 0.005f);
}

bool ASovereignDungeonGateway::EmbarkExpedition(ASovereignBaseCharacter* PartyLeader)
{
	if (!bIsUnlocked || !PartyLeader)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGateway] Cannot embark: Gateway is sealed or Leader is invalid."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] Expedition launched! Enemy Buff: x%f, Reward Mult: x%f"),
		CalculateEnemyBuffMultiplier(), CalculateRewardMultiplier());

	return true;
}

void ASovereignDungeonGateway::ConsumeSacrifices()
{
	if (SacrificeInventory)
	{
		SacrificeInventory->InventorySlots.Empty();
		SacrificeInventory->OnInventoryUpdated.Broadcast();
	}
	InfusedQi = 0.0f;
	OnGatewayEmpowered.Broadcast(1.0f, 1.0f);
}

void ASovereignDungeonGateway::OnInteract_Implementation(AActor* Interactor)
{
	Super::OnInteract_Implementation(Interactor);

	// Toggle/unlock if interacting player has unlocked authority
	if (!bIsUnlocked)
	{
		SetGatewayUnlocked(true);
	}
}

FString ASovereignDungeonGateway::GetInteractionHint_Implementation()
{
	return bIsUnlocked ? TEXT("Embark into Dungeon") : TEXT("Unseal Dungeon Gateway");
}

TMap<FString, FString> ASovereignDungeonGateway::GetSaveData()
{
	TMap<FString, FString> Data;
	Data.Add(TEXT("Gateway.bIsUnlocked"), bIsUnlocked ? TEXT("true") : TEXT("false"));
	Data.Add(TEXT("Gateway.InfusedQi"), FString::SanitizeFloat(InfusedQi));
	return Data;
}

void ASovereignDungeonGateway::RestoreSaveData(const TMap<FString, FString>& Data)
{
	if (const FString* UnlockedStr = Data.Find(TEXT("Gateway.bIsUnlocked")))
	{
		SetGatewayUnlocked(UnlockedStr->Equals(TEXT("true"), ESearchCase::IgnoreCase));
	}
	if (const FString* QiStr = Data.Find(TEXT("Gateway.InfusedQi")))
	{
		InfusedQi = FCString::Atof(**QiStr);
	}
}
