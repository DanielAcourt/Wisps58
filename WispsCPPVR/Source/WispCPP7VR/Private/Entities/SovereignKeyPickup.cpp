// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignKeyPickup.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASovereignKeyPickup::ASovereignKeyPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RotationSpeed = 60.0f;
	BobAmplitude = 12.0f;
	BobFrequency = 2.0f;
	RunningTime = 0.0f;

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
		if (KeyItemDefinition)
		{
			Inv->AddItem(KeyItemDefinition.Get(), 1);
		}
		else
		{
			// Construct transient item definition with KeyTag if unassigned
			USovereignItemDefinition* FallbackKeyDef = NewObject<USovereignItemDefinition>(this);
			FallbackKeyDef->ItemTag = KeyTag;
			FallbackKeyDef->DisplayName = KeyDisplayName;
			Inv->AddItem(FallbackKeyDef, 1);
		}

		UE_LOG(LogTemp, Log, TEXT("[KeyPickup] %s picked up by %s! Tag: %s"),
			*GetName(), *Looter->GetName(), *KeyTag.ToString());

		OnKeyPickedUp.Broadcast(Looter);
		Destroy();
		return true;
	}

	return false;
}
