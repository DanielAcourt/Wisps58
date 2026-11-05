// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignTreasureChest.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASovereignTreasureChest::ASovereignTreasureChest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bHasBeenOpened = false;
	LidOpenAngle = -90.0f;
	LidOpenSpeed = 5.0f;

	// Chest Base
	ChestBaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBaseMeshComponent"));
	if (PhysicalVessel)
	{
		ChestBaseMeshComponent->SetupAttachment(PhysicalVessel);
	}
	else
	{
		SetRootComponent(ChestBaseMeshComponent);
	}

	// Chest Lid
	ChestLidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLidMeshComponent"));
	ChestLidMeshComponent->SetupAttachment(ChestBaseMeshComponent);

	// Loot inventory
	LootInventory = CreateDefaultSubobject<USovereignInventoryComponent>(TEXT("LootInventory"));
	LootInventory->MaxSlotCount = 8;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCube(TEXT("/Engine/BasicShapes/Cube"));
	if (DefaultCube.Succeeded())
	{
		if (ChestBaseMeshComponent)
		{
			ChestBaseMeshComponent->SetStaticMesh(DefaultCube.Object);
			ChestBaseMeshComponent->SetWorldScale3D(FVector(0.8f, 1.2f, 0.5f));
		}
		if (ChestLidMeshComponent)
		{
			ChestLidMeshComponent->SetStaticMesh(DefaultCube.Object);
			ChestLidMeshComponent->SetRelativeLocation(FVector(-35.0f, 0.0f, 25.0f));
			ChestLidMeshComponent->SetWorldScale3D(FVector(0.8f, 1.2f, 0.2f));
		}
	}
}

void ASovereignTreasureChest::BeginPlay()
{
	Super::BeginPlay();

	if (ChestLidMeshComponent)
	{
		ClosedLidRotation = ChestLidMeshComponent->GetRelativeRotation();
		TargetLidRotation = bHasBeenOpened ? (ClosedLidRotation + FRotator(LidOpenAngle, 0.0f, 0.0f)) : ClosedLidRotation;
	}
}

void ASovereignTreasureChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ChestLidMeshComponent)
	{
		const FRotator CurrentRot = ChestLidMeshComponent->GetRelativeRotation();
		if (!CurrentRot.Equals(TargetLidRotation, 0.1f))
		{
			const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetLidRotation, DeltaTime, LidOpenSpeed);
			ChestLidMeshComponent->SetRelativeRotation(NewRot);
		}
	}
}

void ASovereignTreasureChest::OnInteract_Implementation(AActor* Interactor)
{
	if (bHasBeenOpened)
	{
		return;
	}

	if (ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor))
	{
		OpenChest(Character);
	}
}

FString ASovereignTreasureChest::GetInteractionHint_Implementation()
{
	if (bHasBeenOpened)
	{
		return TEXT("Empty Treasure Chest (Already Looted)");
	}
	return TEXT("Open Treasure Chest");
}

bool ASovereignTreasureChest::OpenChest(ASovereignBaseCharacter* Looter)
{
	if (bHasBeenOpened)
	{
		return false;
	}

	bHasBeenOpened = true;
	TargetLidRotation = ClosedLidRotation + FRotator(LidOpenAngle, 0.0f, 0.0f);

	// Transfer all loot items from chest inventory into looter's inventory
	int32 TransferredCount = 0;
	if (Looter && LootInventory)
	{
		if (USovereignInventoryComponent* PlayerInv = Looter->GetInventoryComponent())
		{
			for (int32 SlotIdx = LootInventory->InventorySlots.Num() - 1; SlotIdx >= 0; --SlotIdx)
			{
				const FSovereignInventoryItemInstance& Item = LootInventory->InventorySlots[SlotIdx];
				if (Item.ItemDefinition.IsValid() && Item.Quantity > 0)
				{
					PlayerInv->AddItem(Item.ItemDefinition.Get(), Item.Quantity);
					TransferredCount++;
				}
			}
			LootInventory->InventorySlots.Empty();
		}
	}

	bIsInteractable = false; // Disable further looting
	OnChestOpened.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[TreasureChest] Chest %s opened! Transferred %d item stacks to player."),
		*GetName(), TransferredCount);

	return true;
}
