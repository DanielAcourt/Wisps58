// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).

#include "Entities/SovereignLivingEntity.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignElementComponent.h"
#include "DataTables/SovereignSpeciesData.h"

ASovereignLivingEntity::ASovereignLivingEntity()
{
	BioComponent = CreateDefaultSubobject<USovereignBioComponent>(TEXT("BioComponent"));
	QiComponent = CreateDefaultSubobject<USovereignQiComponent>(TEXT("QiComponent"));
	ElementComponent = CreateDefaultSubobject<USovereignElementComponent>(TEXT("ElementComponent"));
	AttributeComponent = CreateDefaultSubobject<USovereignAttributeComponent>(TEXT("AttributeComponent"));
}

void ASovereignLivingEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BioComponent)
	{
		BioComponent->MaturityProgress += (BioComponent->MaturityRate * DeltaTime);
		BioComponent->UpdateMetabolism(DeltaTime);
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
}

void ASovereignLivingEntity::InitializeFromSovereignData(USovereignSpeciesData* InData)
{
	Super::InitializeFromSovereignData(InData);

	if (AttributeComponent && InData)
	{
		AttributeComponent->SyncStatsFromEntity();
	}
}
