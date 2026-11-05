// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Inventory/SovereignItemFragment.h"
#include "SovereignItemDefinition.generated.h"

class USovereignSpeciesData;

/**
 * USovereignItemDefinition
 * Primary DataAsset defining the immutable archetype and composable fragments of an item.
 * Registered with Unreal's AssetManager as PrimaryAssetType "SovereignItem".
 */
UCLASS(BlueprintType, Const)
class WISPCPP7VR_API USovereignItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USovereignItemDefinition();

	/** Primary GameplayTag identifying this item archetype (e.g. Item.Consumable.SpiritMushroom) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Identity")
	FGameplayTag ItemTag;

	/** User-facing display name of the item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Display")
	FText DisplayName;

	/** Optional soft reference to species data if this item represents a seed, egg, or vessel component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Species")
	TSoftObjectPtr<USovereignSpeciesData> AssociatedSpeciesData;

	/** Composable, polymorphic fragments defining this item's capabilities and behaviors */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Sovereign|Fragments")
	TArray<TObjectPtr<USovereignItemFragment>> Fragments;

	/** UPrimaryDataAsset Interface: Provides asset manager ID for discovery and async bundle management */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Templated C++ helper to query a specific fragment by type.
	 * @return Const pointer to the fragment if found, nullptr otherwise.
	 */
	template <typename T>
	const T* FindFragment() const
	{
		for (const TObjectPtr<USovereignItemFragment>& Fragment : Fragments)
		{
			if (const T* TypedFragment = Cast<T>(Fragment.Get()))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}

	/**
	 * Blueprint-callable helper to retrieve a fragment by class.
	 * @param FragmentClass Class of the fragment to search for.
	 * @return Const pointer to the fragment if found, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory|ItemDefinition", meta = (DeterminesOutputType = "FragmentClass"))
	const USovereignItemFragment* GetFragmentByClass(TSubclassOf<USovereignItemFragment> FragmentClass) const;
};
