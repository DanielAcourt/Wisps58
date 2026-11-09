// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignKeyPickup.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagsManager.h"
#include "UObject/ConstructorHelpers.h"

ASovereignKeyPickup::ASovereignKeyPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RotationSpeed = 60.0f;
	BobAmplitude = 12.0f;
	BobFrequency = 2.0f;
	RunningTime = 0.0f;

	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Item.Key.BossGate"));
	KeyTag = FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
	KeyDisplayName = NSLOCTEXT("Sovereign", "BossKeyName", "Vault Guardian Key");

	KeyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMeshComponent"));
	if (PhysicalVessel)
	{
		KeyMeshComponent->SetupAttachment(PhysicalVessel);
	}
	else
	{
		SetRootComponent(KeyMeshComponent);
	}

	KeyMeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCylinder(TEXT("/Engine/BasicShapes/Cylinder"));
	if (DefaultCylinder.Succeeded() && KeyMeshComponent)
	{
		KeyMeshComponent->SetStaticMesh(DefaultCylinder.Object);
		KeyMeshComponent->SetWorldScale3D(FVector(0.2f, 0.2f, 0.6f));
	}
}

void ASovereignKeyPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
}

void ASovereignKeyPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// Idle floating animation: yaw rotation + sine wave vertical bob
	AddActorLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));

	const float DeltaZ = FMath::Sin(RunningTime * BobFrequency * PI) * BobAmplitude;
	SetActorLocation(BaseLocation + FVector(0.0f, 0.0f, DeltaZ));
}

void ASovereignKeyPickup::OnInteract_Implementation(AActor* Interactor)
{
	if (ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor))
	{
		PickupKey(Character);
	}
}

FString ASovereignKeyPickup::GetInteractionHint_Implementation()
{
	return FString::Printf(TEXT("Pick Up %s"), *KeyDisplayName.ToString());
}

bool ASovereignKeyPickup::PickupKey(ASovereignBaseCharacter* Looter)
{
	if (!Looter)
	{
		return false;
	}

	if (USovereignInventoryComponent* Inv = Looter->GetInventoryComponent())
	{
		// If explicit item definition assigned, add it to inventory
		USovereignItemDefinition* KeyDefToUse = KeyItemDefinition ? KeyItemDefinition.Get() : nullptr;
		if (!KeyDefToUse)
		{
			// Construct transient item definition with KeyTag if unassigned (outered to inventory component for stability)
			USovereignItemDefinition* FallbackKeyDef = NewObject<USovereignItemDefinition>(Inv);
			FallbackKeyDef->ItemTag = KeyTag;
			FallbackKeyDef->DisplayName = KeyDisplayName;
			KeyDefToUse = FallbackKeyDef;
		}

		Inv->AddItem(KeyDefToUse, 1);

		// Record key pickup GUID in DungeonSubsystem before destruction (Ticket B-118)
		FGuid KeyGuid;
		if (SaveDataComponent && SaveDataComponent->EntityID.IsValid())
		{
			KeyGuid = SaveDataComponent->EntityID;
		}

		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
				{
					if (KeyGuid.IsValid())
					{
						DungeonSub->RecordKeyCollected(KeyGuid);
					}
				}
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[KeyPickup] %s (GUID: %s) picked up by %s! Tag: %s"),
			*GetName(), *KeyGuid.ToString(), *Looter->GetName(), *KeyTag.ToString());

		OnKeyPickedUp.Broadcast(Looter);
		bIsPickedUp = true;
		Destroy();
		return true;
	}

	return false;
}
