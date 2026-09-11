// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#include "Entities/SovereignIronKnightAgent.h"
#include "Subsystems/SovereignBridgeSubsystem.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

ASovereignIronKnightAgent::ASovereignIronKnightAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Unpossessable Guard: Ensure Player Wisp cannot inhabit or override Iron Knight
	bCanBePossessed = false;

	// 2. Initialize 3D Status Widget Component
	StatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusWidgetComponent"));
	if (StatusWidgetComponent && RootComponent)
	{
		StatusWidgetComponent->SetupAttachment(RootComponent);
		StatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
		StatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		StatusWidgetComponent->SetDrawSize(FVector2D(300.0f, 100.0f));
	}
}

void ASovereignIronKnightAgent::BeginPlay()
{
	Super::BeginPlay();

	// Set entity ID tag on soul component if valid
	if (SaveDataComponent)
	{
		SaveDataComponent->AddUnknownTag(TEXT("AgentType"), TEXT("IronKnight"));
		SaveDataComponent->AddUnknownTag(TEXT("EntityID"), TEXT("SIM_IronKnight"));
	}

	// Trigger initial AAS Handshake
	RefreshAASHandshake();

	// Start Mailbox Polling for directives addressed to SIM_IronKnight (AD-030)
	UWorld* World = GetWorld();
	if (World)
	{
		if (USovereignBridgeSubsystem* BridgeSubsystem = World->GetSubsystem<USovereignBridgeSubsystem>())
		{
			BridgeSubsystem->StartMailboxPolling(TEXT("SIM_IronKnight"));
		}
	}

	// Auto-Possession logic on start
	if (AutoPossessTargetActor)
	{
		PerformAgentPossession(AutoPossessTargetActor);
	}
	else if (!AutoPossessTargetTag.IsNone())
	{
		TArray<AActor*> TaggedActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), AutoPossessTargetTag, TaggedActors);
		if (TaggedActors.Num() > 0 && TaggedActors[0])
		{
			PerformAgentPossession(TaggedActors[0]);
		}
	}
}

void ASovereignIronKnightAgent::RefreshAASHandshake()
{
	UWorld* World = GetWorld();
	if (!World) return;

	USovereignBridgeSubsystem* BridgeSubsystem = World->GetSubsystem<USovereignBridgeSubsystem>();
	if (BridgeSubsystem)
	{
		BridgeSubsystem->ExecuteAASHandshake();
		bAASHandshakeActive = true;
		AASScore = 1.0f; // Boosted score (+0.5 VSS base 0.5)

		if (SaveDataComponent)
		{
			SaveDataComponent->AddUnknownTag(TEXT("AASHandshake"), TEXT("Active"));
			SaveDataComponent->AddUnknownTag(TEXT("AASScore"), FString::SanitizeFloat(AASScore));
		}
	}
}

bool ASovereignIronKnightAgent::PerformAgentPossession(AActor* TargetVessel)
{
	if (!TargetVessel) return false;

	PossessedTargetActor = TargetVessel;
	PossessedTargetName = TargetVessel->GetName();
	bIsPossessingTarget = true;

	if (SaveDataComponent)
	{
		SaveDataComponent->AddUnknownTag(TEXT("PossessingTarget"), PossessedTargetName);
		SaveDataComponent->AddUnknownTag(TEXT("IsPossessing"), TEXT("True"));
	}

	// B-044 Bidirectional Possession State Sync: Update target vessel's SaveDataComponent
	USovereignSaveableEntityComponent* TargetSoul = TargetVessel->FindComponentByClass<USovereignSaveableEntityComponent>();
	if (TargetSoul)
	{
		TargetSoul->bIsBeingPossessed = true;
		TargetSoul->AddUnknownTag(TEXT("PossessingEntity"), TEXT("SIM_IronKnight"));
		TargetSoul->AddUnknownTag(TEXT("PossessingAgentType"), TEXT("IronKnight"));
	}

	return true;
}

void ASovereignIronKnightAgent::EjectAgentPossession()
{
	if (PossessedTargetActor)
	{
		USovereignSaveableEntityComponent* TargetSoul = PossessedTargetActor->FindComponentByClass<USovereignSaveableEntityComponent>();
		if (TargetSoul)
		{
			TargetSoul->bIsBeingPossessed = false;
			TargetSoul->AddUnknownTag(TEXT("PossessingEntity"), TEXT("None"));
			TargetSoul->AddUnknownTag(TEXT("PossessingAgentType"), TEXT("None"));
		}
	}

	PossessedTargetActor = nullptr;
	PossessedTargetName = TEXT("None");
	bIsPossessingTarget = false;

	if (SaveDataComponent)
	{
		SaveDataComponent->AddUnknownTag(TEXT("PossessingTarget"), TEXT("None"));
		SaveDataComponent->AddUnknownTag(TEXT("IsPossessing"), TEXT("False"));
	}
}
