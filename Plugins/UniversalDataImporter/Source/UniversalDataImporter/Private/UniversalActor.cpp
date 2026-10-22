// Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.

#include "UniversalActor.h"

AUniversalActor::AUniversalActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetMobility(EComponentMobility::Movable);

	DataAssetRef = nullptr;
}

void AUniversalActor::RefreshVisuals()
{
	if (!DataAssetRef || !MeshComponent)
	{
		return;
	}

	// 1. Load Static Mesh if specified
	if (!DataAssetRef->Mesh.IsNull())
	{
		if (UStaticMesh* LoadedMesh = DataAssetRef->Mesh.LoadSynchronous())
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}

	// 2. Load Material override if specified
	if (!DataAssetRef->Material.IsNull())
	{
		if (UMaterialInterface* LoadedMat = DataAssetRef->Material.LoadSynchronous())
		{
			MeshComponent->SetMaterial(0, LoadedMat);
		}
	}
}

void AUniversalActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisuals();
}

void AUniversalActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshVisuals();
}
