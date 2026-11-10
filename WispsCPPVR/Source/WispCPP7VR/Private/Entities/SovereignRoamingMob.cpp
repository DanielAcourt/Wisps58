// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignRoamingMob.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignEncounterSubsystem.h"

ASovereignRoamingMob::ASovereignRoamingMob()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetSphereRadius(120.0f);
	TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
	TriggerSphere->SetGenerateOverlapEvents(true);
}

void ASovereignRoamingMob::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerSphere)
	{
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ASovereignRoamingMob::OnOverlapBegin);
	}
}

void ASovereignRoamingMob::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		InitiateBattle(OtherActor);
	}
}

void ASovereignRoamingMob::OnInteract_Implementation(AActor* Interactor)
{
	InitiateBattle(Interactor);
}

FString ASovereignRoamingMob::GetInteractionHint_Implementation()
{
	if (EncounterDefinition && !EncounterDefinition->EncounterDisplayName.IsEmpty())
	{
		return FString::Printf(TEXT("Engage: %s"), *EncounterDefinition->EncounterDisplayName.ToString());
	}
	return TEXT("Engage Enemy");
}

bool ASovereignRoamingMob::InitiateBattle(AActor* Interactor)
{
	if (!Interactor)
	{
		return false;
	}

	ASovereignBaseCharacter* LeaderChar = Cast<ASovereignBaseCharacter>(Interactor);
	if (!LeaderChar)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	USovereignEncounterSubsystem* EncounterSubsystem = World->GetSubsystem<USovereignEncounterSubsystem>();
	if (!EncounterSubsystem || EncounterSubsystem->bIsEncounterActive)
	{
		return false;
	}

	OnEncounterTriggered.Broadcast(this);

	if (EncounterDefinition)
	{
		return EncounterSubsystem->TriggerEncounterWithDefinition(LeaderChar, this, EncounterDefinition);
	}

	// Fallback if no definition: instantiate raw fallback classes
	TArray<ASovereignBaseCharacter*> FallbackEnemies;
	for (const TSubclassOf<ASovereignBaseCharacter>& EnemyClass : FallbackEnemyClasses)
	{
		if (EnemyClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (ASovereignBaseCharacter* EnemyActor = World->SpawnActor<ASovereignBaseCharacter>(EnemyClass, FVector(0, 0, -50000.0f), FRotator::ZeroRotator, SpawnParams))
			{
				FallbackEnemies.Add(EnemyActor);
			}
		}
	}

	return EncounterSubsystem->TriggerEncounter(LeaderChar, this, FallbackEnemies, true);
}
