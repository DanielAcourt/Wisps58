// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignCultivationBroker.generated.h"

/**
 * UCultivationBroker: Handles the "Magic" - gamified growth, Qi, and evolution.
 */
UCLASS()
class WISPCPP7VR_API UCultivationBroker : public UObject, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	// ISovereignBrokerInterface
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

	/**
	 * Qi Balance (The Progression Balance).
	 * Measures the character's optional gamified spiritual energy reserves.
	 * Decoupled in this broker to isolate magical/RPG systems from core physical coordinates.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Magic")
	float QiBalance = 0.0f;

	/**
	 * Cultivation Tier (The Evolutionary Rank).
	 * Measures the character's spiritual development tier.
	 * By encapsulating progress tiers in this broker, we ensure that if magic systems are disabled
	 * (since magic spells are banned in the Unreal Engine simulation standard), the physical entity
	 * can still load and exist flawlessly with its core coordinates and properties intact.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Magic")
	int32 CultivationTier = 1;

	/**
	 * BlueprintCallable modifier to safely award or consume Qi.
	 * Provides a direct, safe interface for possession triggers, meditation, or quest rewards
	 * to alter spiritual metrics without manual variable mapping.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Magic")
	void AddQi(float Amount);

	/**
	 * BlueprintCallable modifier to progress the entity to the next evolutionary rank.
	 * This facilitates downstream visual components (e.g., Niagara particle systems or dynamic
	 * material overrides) to react to tier upgrades cleanly without polluting core physics.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Magic")
	void IncrementTier();
};
