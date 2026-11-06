// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-05

#include "Entities/SovereignBossTeleporter.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ASovereignBossTeleporter::ASovereignBossTeleporter()
{
	DisplayName = NSLOCTEXT("Sovereign", "BossTeleporterName", "Vault Extraction Portal");
	DormantHint = TEXT("Dormant Portal - Defeat the Vault Boss to activate");
	ActiveHint = TEXT("Teleport to Meditation Garden");
	bIsInteractable = true;
	bIsActive = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (DefaultMeshFinder.Succeeded() && PhysicalVessel)
	{
		PhysicalVessel->SetStaticMesh(DefaultMeshFinder.Object);
		PhysicalVessel->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.2f));
	}

	PortalLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PortalLightComponent"));
	if (PortalLightComponent)
	{
		PortalLightComponent->SetupAttachment(RootComponent);
		PortalLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
		PortalLightComponent->SetLightColor(FLinearColor(0.2f, 0.8f, 1.0f)); // Cyan glow
		PortalLightComponent->SetIntensity(5000.0f);
		PortalLightComponent->SetAttenuationRadius(1200.0f);
		PortalLightComponent->SetVisibility(false); // Dormant initially
	}
}

void ASovereignBossTeleporter::BeginPlay()
{
	Super::BeginPlay();

	// Ensure visual state matches bIsActive
	if (PortalLightComponent)
	{
		PortalLightComponent->SetVisibility(bIsActive);
	}
}

bool ASovereignBossTeleporter::CanInteract_Implementation(AActor* Interactor)
{
	// Only interactable if teleporter has been activated following boss defeat
	if (!bIsActive)
	{
		return false;
	}

	return Super::CanInteract_Implementation(Interactor);
}

void ASovereignBossTeleporter::OnInteract_Implementation(AActor* Interactor)
{
	if (!bIsActive || !CanInteract_Implementation(Interactor))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ASovereignBossTeleporter] Active teleporter used by %s. Teleporting to garden."),
		Interactor ? *Interactor->GetName() : TEXT("Unknown"));

	ReturnToGarden();
}

FString ASovereignBossTeleporter::GetInteractionHint_Implementation()
{
	return bIsActive ? ActiveHint : DormantHint;
}

FText ASovereignBossTeleporter::GetInteractableName_Implementation()
{
	return DisplayName;
}

void ASovereignBossTeleporter::ActivateTeleporter()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;

	if (PortalLightComponent)
	{
		PortalLightComponent->SetVisibility(true);
	}

	UE_LOG(LogTemp, Log, TEXT("[ASovereignBossTeleporter] Teleporter has been ACTIVATED following boss defeat."));

	OnBossTeleporterActivated.Broadcast();
}

void ASovereignBossTeleporter::NotifyBossDefeated()
{
	ActivateTeleporter();
}

bool ASovereignBossTeleporter::ReturnToGarden()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSubsystem = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				return DungeonSubsystem->ReturnToGarden(this);
			}
		}
	}
	return false;
}
