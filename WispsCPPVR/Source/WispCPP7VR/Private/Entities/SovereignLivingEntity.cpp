// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).

#include "Entities/SovereignLivingEntity.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignElementComponent.h"
#include "DataTables/SovereignSpeciesData.h"
#include "SaveSystem/SovereignActorRegistry.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASovereignLivingEntity::ASovereignLivingEntity()
{
	BioComponent = CreateDefaultSubobject<USovereignBioComponent>(TEXT("BioComponent"));
	QiComponent = CreateDefaultSubobject<USovereignQiComponent>(TEXT("QiComponent"));
	ElementComponent = CreateDefaultSubobject<USovereignElementComponent>(TEXT("ElementComponent"));
	AttributeComponent = CreateDefaultSubobject<USovereignAttributeComponent>(TEXT("AttributeComponent"));
}

void ASovereignLivingEntity::OnSovereignHeartbeat()
{
	Super::OnSovereignHeartbeat();

	float HeartbeatSeconds = GetWorldTimerManager().GetTimerRate(HeartbeatTimerHandle);
	int32 WisdomVal = AttributeComponent ? AttributeComponent->Wisdom : 10;

	// 1. BIOLOGICAL GROWTH & CONSUMPTION
	if (BioComponent)
	{
		BioComponent->MaturityProgress += (BioComponent->MaturityRate);
		BioComponent->UpdateMetabolism(HeartbeatSeconds);
	}

	// 2. SPIRITUAL FLOW
	if (QiComponent)
	{
		QiComponent->ProcessQiFlow(HeartbeatSeconds, WisdomVal);
	}

	// 3. EVOLUTION CHECK
	if (BioComponent && BioComponent->MaturityProgress >= 1.0f)
	{
		BioComponent->MaturityProgress = 0.0f;
		Evolve();
	}
}

void ASovereignLivingEntity::VerifySymmetryLevel()
{
	// Check trust signature and Luck/Charisma attributes on AttributeComponent
	if (TrustSignature > 1000 && AttributeComponent && AttributeComponent->Luck > 50)
	{
		UE_LOG(LogTemp, Log, TEXT("Sovereign: %s verified high symmetry level."), *GetName());
	}
}

void ASovereignLivingEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Simulate biology and Qi flow every frame if set to Realtime update frequency
	if (UpdateFrequency == EUpdateFrequency::Realtime)
	{
		if (BioComponent)
		{
			BioComponent->MaturityProgress += (BioComponent->MaturityRate * DeltaTime);
			BioComponent->UpdateMetabolism(DeltaTime);
		}

		if (QiComponent)
		{
			int32 WisdomVal = AttributeComponent ? AttributeComponent->Wisdom : 10;
			QiComponent->ProcessQiFlow(DeltaTime, WisdomVal);
		}
	}
}

void ASovereignLivingEntity::Evolve()
{
	Super::Evolve();

	if (BioComponent)
	{
		BioComponent->NutrientReserves.Empty();
		BioComponent->Hunger = 0.0f;
		BioComponent->Entropy += 10.0f;
		BioComponent->MassExperience += 5.0;
		BioComponent->Mass = FMath::FloorToInt(BioComponent->MassExperience);
	}
}

void ASovereignLivingEntity::PostSpawnInitialize(const USovereignSpeciesData* InSpeciesData, const FGuid& InMotherID, const FGuid& InFatherID)
{
	Super::PostSpawnInitialize(InSpeciesData, InMotherID, InFatherID);

	if (BioComponent)
	{
		BioComponent->MotherID = InMotherID;
		BioComponent->FatherID = InFatherID;
	}

	if (InFatherID.IsValid() && GetWorld())
	{
		if (UActorRegistry* Registry = GetWorld()->GetSubsystem<UActorRegistry>())
		{
			AActor* Mother = Registry->FindActor(InMotherID);
			AActor* Father = Registry->FindActor(InFatherID);
			if (Mother && Father)
			{
				float CurrentTime = GetWorld()->GetTimeSeconds();
				if (auto* MomBio = Mother->FindComponentByClass<USovereignBioComponent>())
				{
					MomBio->LastMatingTimestamp = CurrentTime;
				}
				if (auto* DadBio = Father->FindComponentByClass<USovereignBioComponent>())
				{
					DadBio->LastMatingTimestamp = CurrentTime;
				}
			}
		}
	}
}

void ASovereignLivingEntity::InitializeFromSovereignData(USovereignSpeciesData* InData)
{
	Super::InitializeFromSovereignData(InData);

	if (AttributeComponent && InData)
	{
		AttributeComponent->SyncStatsFromEntity();
	}
}
