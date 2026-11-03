// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "SovereignPolymorphComponent.generated.h"

class ASovereignBaseCharacter;
class USkeletalMesh;
class UMaterialInterface;
class UAnimInstance;

/**
 * ESovereignVesselForm
 * Visual and physical form of a shapeshifting vessel entity.
 */
UENUM(BlueprintType)
enum class ESovereignVesselForm : uint8
{
	TrueForm        UMETA(DisplayName = "True Form (Beast / Dragon)"),
	HumanoidVisage  UMETA(DisplayName = "Humanoid Visage")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVesselFormChanged, ESovereignVesselForm, NewForm);

/**
 * USovereignPolymorphComponent
 * Modular component attached strictly to shapeshifting entities (Dragons, Spirit Beasts).
 * Enables adult entities to transform into a Humanoid Visage for dungeon exploration
 * and turn-based combat class utilization, preserving base D&D attributes, identity, and inventory.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignPolymorphComponent : public USovereignBaseComponent
{
	GENERATED_BODY()

public:
	USovereignPolymorphComponent();

	/** --- 1. CURRENT STATE & CONFIGURATION --- */

	/** Active physical and visual form of this vessel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Polymorph")
	ESovereignVesselForm CurrentForm = ESovereignVesselForm::TrueForm;

	/** Minimum growth stage required to unlock polymorph capabilities (e.g. Stage 7 Adult) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph")
	int32 RequiredGrowthStage = 7;

	/** Humanoid Visage Skeletal Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	TSoftObjectPtr<USkeletalMesh> HumanoidSkeletalMesh;

	/** Optional Material override for Humanoid form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	TSoftObjectPtr<UMaterialInterface> HumanoidMaterial;

	/** Optional Animation Blueprint class for Humanoid form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	TSubclassOf<UAnimInstance> HumanoidAnimClass;

	/** Capsule collision dimensions when in Humanoid form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	float HumanoidCapsuleRadius = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	float HumanoidCapsuleHalfHeight = 88.0f;

	/** Ground movement speed in Humanoid form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Polymorph|Humanoid")
	float HumanoidWalkSpeed = 600.0f;

	/** Broadcast when the vessel transitions between TrueForm and HumanoidVisage */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Polymorph|Events")
	FOnVesselFormChanged OnVesselFormChanged;

	/** --- 2. PUBLIC API --- */

	/** Checks whether this vessel meets all requirements to polymorph (e.g. adult maturity) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Polymorph")
	bool CanPolymorph() const;

	/** Toggles form between TrueForm and HumanoidVisage */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Polymorph")
	bool TogglePolymorph();

	/** Explicitly sets the vessel form */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Polymorph")
	bool SetVesselForm(ESovereignVesselForm NewForm);

	/** Helper to retrieve owner character vessel */
	ASovereignBaseCharacter* GetOwnerCharacter() const;

protected:
	virtual void BeginPlay() override;

	/** Caches the True Form's mesh, collision, and movement properties */
	void CacheTrueFormDefaults();

	/** Cached True Form defaults */
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> CachedTrueMesh;

	UPROPERTY()
	TSoftObjectPtr<UMaterialInterface> CachedTrueMaterial;

	UPROPERTY()
	TSubclassOf<UAnimInstance> CachedTrueAnimClass;

	UPROPERTY()
	float CachedTrueCapsuleRadius = 60.0f;

	UPROPERTY()
	float CachedTrueCapsuleHalfHeight = 150.0f;

	UPROPERTY()
	float CachedTrueMaxWalkSpeed = 800.0f;

	UPROPERTY()
	float CachedTrueMaxFlySpeed = 1200.0f;

	UPROPERTY()
	uint8 CachedTrueMovementMode = 5; // MOVE_Flying default

	UPROPERTY()
	bool bTrueDefaultsCached = false;
};
