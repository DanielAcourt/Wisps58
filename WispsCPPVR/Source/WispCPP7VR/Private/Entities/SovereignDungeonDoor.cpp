// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignDungeonDoor.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagsManager.h"
#include "UObject/ConstructorHelpers.h"

ASovereignDungeonDoor::ASovereignDungeonDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bIsOpen = false;
	bIsLocked = false;
	UnlockCondition = EBossGateUnlockCondition::None;
	OpenAngle = 90.0f;
	SwingSpeed = 4.0f;
	bStayOpenOnceOpened = true;
	RequiredQiToUnlock = 50.0f;
	ChanneledQi = 0.0f;

	// Outer frame
	DoorFrameMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMeshComponent"));
	if (PhysicalVessel)
	{
		DoorFrameMeshComponent->SetupAttachment(PhysicalVessel);
	}
	else
	{
		SetRootComponent(DoorFrameMeshComponent);
	}

	// Movable door leaf attached to frame
	DoorLeafMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeafMeshComponent"));
	DoorLeafMeshComponent->SetupAttachment(DoorFrameMeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCube(TEXT("/Engine/BasicShapes/Cube"));
	if (DefaultCube.Succeeded())
	{
		if (DoorFrameMeshComponent)
		{
			DoorFrameMeshComponent->SetStaticMesh(DefaultCube.Object);
			DoorFrameMeshComponent->SetWorldScale3D(FVector(0.4f, 4.0f, 5.0f));
		}
		if (DoorLeafMeshComponent)
		{
			DoorLeafMeshComponent->SetStaticMesh(DefaultCube.Object);
			DoorLeafMeshComponent->SetWorldScale3D(FVector(0.35f, 3.8f, 4.8f));
		}
	}
}

void ASovereignDungeonDoor::BeginPlay()
{
	Super::BeginPlay();

	if (DoorLeafMeshComponent)
	{
		ClosedRotation = DoorLeafMeshComponent->GetRelativeRotation();
		TargetRotation = bIsOpen ? (ClosedRotation + FRotator(0.0f, OpenAngle, 0.0f)) : ClosedRotation;
	}
}

void ASovereignDungeonDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DoorLeafMeshComponent)
	{
		const FRotator CurrentRot = DoorLeafMeshComponent->GetRelativeRotation();
		if (!CurrentRot.Equals(TargetRotation, 0.1f))
		{
			const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, SwingSpeed);
			DoorLeafMeshComponent->SetRelativeRotation(NewRot);
		}
	}
}

void ASovereignDungeonDoor::OnInteract_Implementation(AActor* Interactor)
{
	Super::OnInteract_Implementation(Interactor);

	UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] %s interacted with by %s (IsOpen: %d, IsLocked: %d)."),
		*GetName(), Interactor ? *Interactor->GetName() : TEXT("Unknown"), bIsOpen ? 1 : 0, bIsLocked ? 1 : 0);

	if (bIsOpen && bStayOpenOnceOpened)
	{
		return;
	}

	ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor);

	// Check locking conditions
	if (bIsLocked)
	{
		if (UnlockCondition == EBossGateUnlockCondition::RequiresBossKey && Character)
		{
			FGameplayTag KeyTagToCheck = RequiredKeyTag;
			if (!KeyTagToCheck.IsValid())
			{
				UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Item.Key.BossGate"));
				KeyTagToCheck = FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
			}

			if (USovereignInventoryComponent* Inv = Character->GetInventoryComponent())
			{
				if (KeyTagToCheck.IsValid() && Inv->GetItemCountByTag(KeyTagToCheck) > 0)
				{
					UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] Key %s found in player inventory! Unlocking door."), *KeyTagToCheck.ToString());
					if (bConsumeKeyOnUnlock)
					{
						Inv->RemoveItemByTag(KeyTagToCheck, 1);
						UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] Consumed 1 unit of key %s from player inventory."), *KeyTagToCheck.ToString());
					}
					UnlockDoor();
					OpenDoor();
					return;
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("[DungeonDoor] Locked: Interactor lacks required key tag %s."), *KeyTagToCheck.ToString());
			return;
		}
		else if (UnlockCondition == EBossGateUnlockCondition::RequiresQiEmpowerment && Character)
		{
			ChannelQiToUnlock(Character, 25.0f);
			if (!bIsLocked)
			{
				OpenDoor();
			}
			return;
		}
		else if (UnlockCondition == EBossGateUnlockCondition::DefeatEliteGuardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonDoor] Sealed: Defeat the Elite Guardian to unseal this gate!"));
			return;
		}
		return;
	}

	ToggleDoor();
}

FString ASovereignDungeonDoor::GetInteractionHint_Implementation()
{
	if (bIsLocked)
	{
		switch (UnlockCondition)
		{
		case EBossGateUnlockCondition::RequiresBossKey:
			return FString::Printf(TEXT("Locked: Requires Key [%s]"), *RequiredKeyTag.ToString());
		case EBossGateUnlockCondition::RequiresQiEmpowerment:
			return FString::Printf(TEXT("Channel Qi to Unseal (%.0f/%.0f Qi)"), ChanneledQi, RequiredQiToUnlock);
		case EBossGateUnlockCondition::DefeatEliteGuardian:
			return TEXT("Sealed by Elite Guardian");
		default:
			return TEXT("Locked");
		}
	}

	if (bIsOpen)
	{
		return bStayOpenOnceOpened ? TEXT("Open Passage") : TEXT("Close Door");
	}
	return TEXT("Open Door");
}

void ASovereignDungeonDoor::ToggleDoor()
{
	if (bIsOpen)
	{
		CloseDoor();
	}
	else
	{
		OpenDoor();
	}
}

void ASovereignDungeonDoor::OpenDoor()
{
	if (bIsLocked)
	{
		return;
	}

	bIsOpen = true;
	TargetRotation = ClosedRotation + FRotator(0.0f, OpenAngle, 0.0f);
	OnDoorStateChanged.Broadcast(true);
	UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] %s opened."), *GetName());
}

void ASovereignDungeonDoor::CloseDoor()
{
	if (bStayOpenOnceOpened)
	{
		return;
	}

	bIsOpen = false;
	TargetRotation = ClosedRotation;
	OnDoorStateChanged.Broadcast(false);
	UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] %s closed."), *GetName());
}

void ASovereignDungeonDoor::UnlockDoor()
{
	bIsLocked = false;
	UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] %s unlocked!"), *GetName());
}

bool ASovereignDungeonDoor::ChannelQiToUnlock(ASovereignBaseCharacter* Cultivator, float Amount)
{
	if (!Cultivator || !bIsLocked || Amount <= 0.0f)
	{
		return false;
	}

	USovereignQiComponent* QiComp = Cultivator->GetQiComponent();
	if (!QiComp || QiComp->CurrentQi <= 0.0f)
	{
		return false;
	}

	const float DrainAmount = FMath::Min(Amount, QiComp->CurrentQi);
	QiComp->CurrentQi -= DrainAmount;
	ChanneledQi += DrainAmount;

	UE_LOG(LogTemp, Log, TEXT("[DungeonDoor] Channeled %.1f Qi towards door unsealing (%.1f / %.1f)."),
		DrainAmount, ChanneledQi, RequiredQiToUnlock);

	if (ChanneledQi >= RequiredQiToUnlock)
	{
		UnlockDoor();
	}

	return true;
}
