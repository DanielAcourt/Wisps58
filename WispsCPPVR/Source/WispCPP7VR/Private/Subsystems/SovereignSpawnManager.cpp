// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SovereignSpawnManager.h"
#include "Entities/SovereignBaseEntity.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "SaveSystem/SovereignActorRegistry.h"
#include "GameplayTagContainer.h"

void USovereignSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("Sovereign Spawn Manager Online: PSTAS Logic Engaged"));
}

void USovereignSpawnManager::Deinitialize()
{
	Super::Deinitialize();
}

void USovereignSpawnManager::RequestSpawn(const USovereignSpeciesData* SpeciesData, const FTransform& Transform, const FGuid& MotherID, const FGuid& FatherID)
{
	if (!SpeciesData)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: SpeciesData is null!"));
		return;
	}

	int32 RequestID = NextRequestID++;
	FSpawnRequest NewRequest(RequestID, SpeciesData, Transform, MotherID, FatherID);
	SpawnQueue.Add(RequestID, NewRequest);

	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	StreamableManager.RequestAsyncLoad(SpeciesData->ActorClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &USovereignSpawnManager::OnClassLoaded, RequestID));
}

void USovereignSpawnManager::OnClassLoaded(int32 RequestID)
{
	FSpawnRequest* Request = SpawnQueue.Find(RequestID);
	if (!Request)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: OnClassLoaded called with invalid RequestID!"));
		return;
	}

	const USovereignSpeciesData* SpeciesData = Request->SpeciesData;
	if (!SpeciesData)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: OnClassLoaded called with null SpeciesData!"));
		return;
	}

	TSubclassOf<ASovereignBaseEntity> ClassToSpawn = FallbackUnknownClass;
	TSubclassOf<ASovereignBaseEntity> LoadedClass = SpeciesData->ActorClass.Get();

	if (LoadedClass)
	{
		const ASovereignBaseEntity* CDO = LoadedClass->GetDefaultObject<ASovereignBaseEntity>();
		if (CDO && CDO->IdentitySignature == SpeciesData->IdentitySignature)
		{
			ClassToSpawn = LoadedClass;
		}
	}

	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: ClassToSpawn is null and no fallback is set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// "Drone-Laying Penalty" (Ontological Parent Check)
	// If the parent ID is valid but the parent actor is not found in the registry,
	// it is considered an ontological failure (e.g. the parent has died or been culled).
	bool bHasParentFailure = false;
	if (Request->MotherID.IsValid())
	{
		UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
		if (Registry && !Registry->FindActor(Request->MotherID))
		{
			bHasParentFailure = true;
		}
	}

	ASovereignBaseEntity* NewEntity = World->SpawnActor<ASovereignBaseEntity>(ClassToSpawn, Request->Transform);

	if (NewEntity)
	{
		NewEntity->PostSpawnInitialize(SpeciesData, Request->MotherID, Request->FatherID);
		if (bHasParentFailure)
		{
			FGameplayTag PenaltyTag = FGameplayTag::RequestGameplayTag(FName("State.Biological.Penalty"), false);
			if (PenaltyTag.IsValid())
			{
				NewEntity->GameplayTags.AddTag(PenaltyTag);
			}
		}
		if (ClassToSpawn == FallbackUnknownClass)
		{
			FGameplayTag UnknownTag = FGameplayTag::RequestGameplayTag(FName("Transient.Unknown"), false);
			if (UnknownTag.IsValid())
			{
				NewEntity->GameplayTags.AddTag(UnknownTag);
			}
		}
	}

	// Remove the request from the queue
	SpawnQueue.Remove(RequestID);
}
