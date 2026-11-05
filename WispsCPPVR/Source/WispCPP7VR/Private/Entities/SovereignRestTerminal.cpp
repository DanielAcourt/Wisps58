// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignRestTerminal.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignPartyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ASovereignRestTerminal::ASovereignRestTerminal()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxRestCharges = 3;
	RemainingRestCharges = 3;
	bRandomizeCharges = false;
	MinRandomCharges = 1;
	MaxRandomCharges = 5;
	HealthRestorePercentage = 1.0f;
	QiRestorePercentage = 1.0f;
	bAutoDestroyOnDepletion = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCylinder(TEXT("/Engine/BasicShapes/Cylinder"));
	if (DefaultCylinder.Succeeded() && StaticMeshComponent)
	{
		StaticMeshComponent->SetStaticMesh(DefaultCylinder.Object);
		StaticMeshComponent->SetWorldScale3D(FVector(1.2f, 1.2f, 0.6f));
	}
}

void ASovereignRestTerminal::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomizeCharges)
	{
		RemainingRestCharges = FMath::RandRange(FMath::Max(1, MinRandomCharges), FMath::Max(MinRandomCharges, MaxRandomCharges));
		MaxRestCharges = RemainingRestCharges;
	}
	else
	{
		RemainingRestCharges = MaxRestCharges;
	}
}

void ASovereignRestTerminal::OnInteract_Implementation(AActor* Interactor)
{
	if (ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor))
	{
		RestParty(Character);
	}
}

FString ASovereignRestTerminal::GetInteractionHint_Implementation()
{
	if (RemainingRestCharges > 0)
	{
		return FString::Printf(TEXT("Rest and Replenish Party (%d/%d Charges Remaining)"), RemainingRestCharges, MaxRestCharges);
	}
	return TEXT("Exhausted Sanctuary Campfire (Depleted)");
}

bool ASovereignRestTerminal::RestParty(ASovereignBaseCharacter* InteractingCharacter)
{
	if (!InteractingCharacter || RemainingRestCharges <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RestTerminal] Cannot rest: No remaining charges (%d)."), RemainingRestCharges);
		return false;
	}

	TArray<ASovereignBaseCharacter*> PartyMembers;
	PartyMembers.Add(InteractingCharacter);

	// Gather party members if party component is attached
	if (USovereignPartyComponent* PartyComp = InteractingCharacter->GetPartyComponent())
	{
		for (const FSovereignPartySlot& Slot : PartyComp->GridSlots)
		{
			if (Slot.IsOccupied() && Slot.MemberCharacter.Get() && Slot.MemberCharacter.Get() != InteractingCharacter)
			{
				PartyMembers.Add(Slot.MemberCharacter.Get());
			}
		}
	}

	// Replenish Vitals, Health & Qi for all party members
	for (ASovereignBaseCharacter* Member : PartyMembers)
	{
		if (!Member)
		{
			continue;
		}

		// 1. Health
		if (USovereignAttributeComponent* Attr = Member->GetAttributeComponent())
		{
			const float HealAmount = Attr->MaxHealth * HealthRestorePercentage;
			Attr->CurrentHealth = FMath::Clamp(Attr->CurrentHealth + HealAmount, 0.0f, Attr->MaxHealth);
		}

		// 2. Qi
		if (USovereignQiComponent* QiComp = Member->GetQiComponent())
		{
			const float QiAmount = QiComp->MaxQiCapacity * QiRestorePercentage;
			QiComp->CurrentQi = FMath::Clamp(QiComp->CurrentQi + QiAmount, 0.0f, QiComp->MaxQiCapacity);
		}

		// 3. Biological Vitals
		if (USovereignBioComponent* BioComp = Member->GetBioComponent())
		{
			BioComp->Hunger = 100.0f;
			BioComp->Hydration = 100.0f;
			BioComp->Fatigue = 0.0f;
			BioComp->Toxicity = 0.0f;
		}
	}

	RemainingRestCharges--;
	UE_LOG(LogTemp, Log, TEXT("[RestTerminal] Party rested successfully! %d charges remaining."), RemainingRestCharges);
	OnRestTerminalUsed.Broadcast(RemainingRestCharges);

	if (RemainingRestCharges <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[RestTerminal] Sanctuary terminal depleted."));
		OnRestTerminalDepleted.Broadcast();

		if (bAutoDestroyOnDepletion)
		{
			Destroy();
		}
	}

	return true;
}

void ASovereignRestTerminal::ResetCharges(int32 NewChargeCount)
{
	if (NewChargeCount > 0)
	{
		MaxRestCharges = NewChargeCount;
		RemainingRestCharges = NewChargeCount;
	}
	else
	{
		RemainingRestCharges = MaxRestCharges;
	}
}
