// Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "UniversalDataAsset.h"
#include "UniversalActor.generated.h"

/**
 * AUniversalActor
 * Standard agnostic base actor class provided by the UniversalDataImporter plugin.
 * Automatically holds a reference to UUniversalDataAsset and syncs 3D mesh/material visuals
 * during Editor OnConstruction and BeginPlay.
 */
UCLASS(BlueprintType, Blueprintable)
class UNIVERSALDATAIMPORTER_API AUniversalActor : public AActor
{
	GENERATED_BODY()

public:
	AUniversalActor();

	/** Root Static Mesh Component for 3D visual rendering */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Universal Actor")
	UStaticMeshComponent* MeshComponent;

	/** Primary DataAsset reference containing asset attributes and soft mesh/material pointers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Actor")
	UUniversalDataAsset* DataAssetRef;

	/** Syncs 3D Mesh and Material visuals from DataAssetRef */
	UFUNCTION(BlueprintCallable, Category = "Universal Actor")
	virtual void RefreshVisuals();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
};
