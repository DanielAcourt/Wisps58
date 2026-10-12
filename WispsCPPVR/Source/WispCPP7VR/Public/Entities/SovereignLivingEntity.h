// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseEntity.h"
#include "SovereignLivingEntity.generated.h"

class USovereignBioComponent;
class USovereignQiComponent;
class USovereignElementComponent;
class USovereignAttributeComponent;

/**
 * ASovereignLivingEntity
 * Derived base class for all organic, biological, or living simulation entities (Plants, Animals, Monsters).
 * Instantiates Bio, Attribute, Qi, and Element components by default.
 */
UCLASS()
class WISPCPP7VR_API ASovereignLivingEntity : public ASovereignBaseEntity
{
	GENERATED_BODY()

public:
	ASovereignLivingEntity();

	// --- Component Accessors ---
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Living")
	USovereignBioComponent* GetBioComponent() const { return BioComponent; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Living")
	USovereignAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Living")
	USovereignQiComponent* GetQiComponent() const { return QiComponent; }

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Living")
	USovereignElementComponent* GetElementComponent() const { return ElementComponent; }

	// --- Overrides ---
	virtual void Tick(float DeltaTime) override;
	virtual void Evolve() override;
	virtual void PostSpawnInitialize(const USovereignSpeciesData* InSpeciesData, const FGuid& InMotherID, const FGuid& InFatherID) override;

protected:
	virtual void OnSovereignHeartbeat() override;
	virtual void VerifySymmetryLevel();
	/** The Biological engine: Metabolism, Growth, Lineage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignBioComponent* BioComponent;

	/** The Spiritual engine: Magic, Alignment, Qi */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignQiComponent* QiComponent;

	/** The Physical nature: Elemental resistances and sockets */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignElementComponent* ElementComponent;

	/** The Attribute engine: Strength, Intelligence, HP */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	USovereignAttributeComponent* AttributeComponent;

	virtual void InitializeFromSovereignData(USovereignSpeciesData* InData) override;
};
