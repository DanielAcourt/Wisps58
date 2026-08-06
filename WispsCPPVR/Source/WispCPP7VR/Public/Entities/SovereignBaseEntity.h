// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SaveSystem/SovereignGameData.h"
#include "DataTables/SovereignSpeciesData.h"

#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h" 

#include "Interaction/SovereignEntityInterface.h"

#include "SovereignBaseEntity.generated.h"

class USovereignSaveableEntityComponent;
class USovereignBioComponent;
class USovereignQiComponent;
class USovereignElementComponent;
class USovereignAttributeComponent;
class USovereignSpeciesData;
class UStaticMeshComponent;

/**
 * ASovereignBaseEntity: Base class for all possessable simulation entities.
 */
UCLASS()
class WISPCPP7VR_API ASovereignBaseEntity : public APawn, public IGameplayTagAssetInterface, public ISovereignEntityInterface
{
	GENERATED_BODY()

public:
	ASovereignBaseEntity();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Soul")
	USovereignSaveableEntityComponent* GetSaveDataComponent() const { return SaveDataComponent; }

	/** Returns the Sovereign Soul component for this entity */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Soul")
	USovereignSaveableEntityComponent* GetSovereignSoul_Implementation() const;

	/** Returns the unique Save System ID for this specific entity */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Entity")
	FGuid GetSovereignID() const;

	/** The Unique Identity Signature for this class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Identity")
	FGameplayTag IdentitySignature;


	/** The Gameplay Tags for this entity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity")
	FGameplayTagContainer GameplayTags;

	/** The Gameplay Tags that this Enity is checking against. */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Tags")
	void IngestSovereignTag(FString IncomingTagString);

	// --- IGameplayTagAssetInterface Implementation ---
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	// --- Lifecycle ---
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Primary logic for moving from one growth stage to the next */
	virtual void Evolve();


	/** Called by the Spawn Manager after the actor has been spawned. */
	virtual void PostSpawnInitialize(const USovereignSpeciesData* InSpeciesData, const FGuid& InMotherID, const FGuid& InFatherID);

protected:
	/** Can this entity be possessed by a Sovereign Spirit? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Possession")
	bool bCanBePossessed = true;

	/** Calculates the float delay based on the UpdateFrequency enum */
	float GetHeartbeatInterval() const;

	// --- Components ---

	/** The physical body of the entity (The Oak Tree, the Bee, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Visuals")
	UStaticMeshComponent* EntityMesh;

	/** The Soul of the Actor: Contains the GUID and Metadata tags */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignSaveableEntityComponent* SaveDataComponent;

	/** The Biological engine: Health, Stamina, Lineage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignBioComponent* BioComponent;

	/** The Spiritual engine: Magic, Alignment, Qi */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignQiComponent* QiComponent;

	/** The Physical nature: Elemental resistances and sockets */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignElementComponent* ElementComponent;

	/** The Attribute engine: Strength, Intelligence, HP */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|SaveSystem")
	USovereignAttributeComponent* AttributeComponent;

	/** Array of 8 meshes representing the growth stages (0-7) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Visuals")
	TArray<UStaticMesh*> GrowthMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Security")
	int32 TrustSignature = 0;

	/** 1.0 is default size. 2.0 is double size. Allows for growth without new meshes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Growth")
	float VisualScale = 1.0f;

	/** Base health/stamina multiplier for this stage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Growth")
	float BaseConstitution = 1.0f;

	// --- Data & Evolution ---

	/** Logic to update the entity based on new Species Data */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Identity")
	virtual void InitializeFromSovereignData(USovereignSpeciesData* InData);


protected:
	void VerifySymmetryLevel();


	/** The 'Passport' for this entity's species and trust level */
	/** The "Advanced" data asset defining growth stages, health, and species attributes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Data", meta = (AllowPrivateAccess = "true"))
	class USovereignSpeciesData* SpeciesData;

	/** Which of the 8 growth stages are we currently in? (0 to 7) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Growth")
	int32 CurrentGrowthStage = 0;

	/** How often this entity processes logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Performance")
	EUpdateFrequency UpdateFrequency = EUpdateFrequency::Standard;

	/** Swaps the Static Mesh based on the current Growth Stage index */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Visuals")
	void RefreshVisuals();

	// --- Internal Logic ---

	/** The recurring timer handle for the heartbeat logic */
	FTimerHandle HeartbeatTimerHandle;

	/** If true, the Soul uses the ManualBirthDate instead of the current time on first spawn */
	UPROPERTY(EditAnywhere, Category = "Sovereign|Identity")
	bool bUseManualBirthDate = false;

	/** Format: "2017.03.23-16.00.00" (Year.Month.Day-Hour.Minute.Second) */
	UPROPERTY(EditAnywhere, Category = "Sovereign|Identity", meta = (EditCondition = "bUseManualBirthDate"))
	FString ManualBirthDate = "2017.03.23-16.00.00";

	/** The logic function that runs growth progress and checks for Evolution */
	void OnSovereignHeartbeat();

	//can i evolve?
	void CheckForEvolution();

	/** Callback function for when a mesh has been asynchronously loaded. */
	void OnMeshLoaded(TSoftObjectPtr<UStaticMesh> LoadedMeshPtr);
};
