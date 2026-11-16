// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-05

#include "Entities/SovereignDungeonEntrance.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ASovereignDungeonEntrance::ASovereignDungeonEntrance()
{
	DisplayName = NSLOCTEXT("Sovereign", "DungeonEntranceName", "Garden Return Gateway");
	InteractionHint = TEXT("Return to Meditation Garden");
	bIsInteractable = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultMeshFinder.Succeeded() && PhysicalVessel)
	{
		PhysicalVessel->SetStaticMesh(DefaultMeshFinder.Object);
		PhysicalVessel->SetRelativeScale3D(FVector(0.3f, 4.0f, 5.0f));
	}
}

void ASovereignDungeonEntrance::OnInteract_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ASovereignDungeonEntrance] Interacted with by %s. Initiating return to garden."),
		Interactor ? *Interactor->GetName() : TEXT("Unknown"));

	ReturnToGarden();
}

FString ASovereignDungeonEntrance::GetInteractionHint_Implementation()
{
	return InteractionHint;
}

FText ASovereignDungeonEntrance::GetInteractableName_Implementation()
{
	return DisplayName;
}

bool ASovereignDungeonEntrance::ReturnToGarden()
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
