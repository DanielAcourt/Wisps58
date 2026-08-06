// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTables/SovereignSpeciesData.h"
#include "Subsystems/SovereignSpawnmanager.h"
#include "SovereignSpawnManager.generated.h"

class ASovereignBaseEntity;

/** * FSpawnRequest: A 'Data Package' that holds all the instructions needed to create a lifeform.
 * This is queued so the engine can process it without hitching (Asynchronous loading).
 */
USTRUCT(BlueprintType)
struct FSpawnRequest
{
	GENERATED_BODY()

	/** Unique ID for this specific spawn attempt, used for tracking in the queue */
	UPROPERTY()
	int32 RequestID;

	/** The 'Genetic Template' from your Directory Scanner/Registry (Oak, Bee, Dragon) */
	UPROPERTY()
	const USovereignSpeciesData* SpeciesData;

	/** Where in the 3D Chunk this entity should appear */
	UPROPERTY()
	FTransform Transform;

	/** * Refers to the 'Gene Seed' transform.
	 * Note: Using FTransform for a ParentID is unusual; typically this would be a GUID
	 * or a link to a static DNA table.
	 */
	UPROPERTY()
	FTransform ParentID;

	/** The GUID of the biological Mother. Essential for lineage and trait inheritance. */
	UPROPERTY()
	FGuid MotherID; //Refers to the individual Mother

	/** The GUID of the biological Father. Allows for multi-parent trait mixing. */
	UPROPERTY()
	FGuid FatherID; //Refers to the individual father

	/** * The 'Truth' Payload: This is the TMap of key-value pairs from your JSON.
		 * It carries the 'Sandwich' tags, Qi levels, and health from the last save.
		 */
	UPROPERTY()
	TMap<FString, FString> InitialMetadata;

	// --- Constructors for clean initialization ---
	FSpawnRequest() : RequestID(0), SpeciesData(nullptr), Transform(FTransform::Identity), MotherID(FGuid()), FatherID(FGuid()) {}

	FSpawnRequest(int32 InID, const USovereignSpeciesData* InData, const FTransform& InTransform, const FGuid& InMotherID, const FGuid& InFatherID = FGuid())
		: RequestID(InID), SpeciesData(InData), Transform(InTransform), MotherID(InMotherID), FatherID(InFatherID) {}
};

/**
 * USovereignSpawnManager: The 'Factory' that manufactures Sovereign Souls.
 * It manages the SpawnQueue to ensure that thousands of entities don't crash the game
 * by trying to spawn all at the exact same frame.
 */
UCLASS()
class WISPCPP7VR_API USovereignSpawnManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** * The Primary Command: Use this to tell the machine to create an entity.
	 * This is what your JSON parser calls for every entry in the "Entities" array.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Spawning")
	void RequestSpawn(const USovereignSpeciesData* SpeciesData, const FTransform& Transform, const FGuid& MotherID, const FGuid& FatherID = FGuid());

private:
	UPROPERTY()
	TMap<int32, FSpawnRequest> SpawnQueue;

	int32 NextRequestID = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Sovereign|Spawning")
	TSubclassOf<ASovereignBaseEntity> FallbackUnknownClass;

	void OnClassLoaded(int32 RequestID);
};
