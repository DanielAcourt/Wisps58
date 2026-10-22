// Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "UniversalDataAsset.generated.h"

/**
 * UUniversalDataAsset
 * Standard agnostic Primary DataAsset class provided by the UniversalDataImporter plugin out-of-the-box.
 * Serves as the default DataAsset target for CSV template generation and asset synthesis.
 */
UCLASS(BlueprintType, Blueprintable)
class UNIVERSALDATAIMPORTER_API UUniversalDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UUniversalDataAsset();

	/** Display name for UI and inventory screens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	FText DisplayName;

	/** Primary Gameplay Tag signature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	FGameplayTag IdentityTag;

	/** Gameplay Tag Container for traits and capabilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	FGameplayTagContainer Tags;

	/** Soft Actor Class reference for synthesized Blueprint actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	TSoftClassPtr<AActor> ActorClass;

	/** Soft Texture2D icon pointer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Detailed description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data")
	FText Description;

	/** Base Health stat (Proof of Concept Stat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Data|Stats")
	int32 Health;
};
