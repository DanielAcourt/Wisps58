// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SovereignSpawnerUtils.generated.h"
/**
 * Sovereign Utility Library
 * Handles the "D&D" style attribute inheritance and the Genetic Recombination system.
 */
UCLASS()
class WISPCPP7VR_API USovereignSpawnerUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Spawner")
    static void SpawnEarnedEntity(UObject* WorldContextObject, const USovereignSpeciesData* SpeciesData, FTransform SpawnTransform, FGuid ParentID);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Spawner")
    static void SpawnEarnedEntityInherited(UObject* WorldContextObject, const USovereignSpeciesData* SpeciesData, FTransform SpawnTransform, AActor* ParentActor);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Spawner")
    static void SpawnHybridEntity(UObject* WorldContextObject, const USovereignSpeciesData* SpeciesData, FTransform SpawnTransform, AActor* Mother, AActor* Father);

    /** ?? COMPATIBILITY: Checks if two actors can produce offspring (Gender check, etc) */
    // Might need to exapand the logic so they are both type "[plant} for example
    UFUNCTION(BlueprintPure, Category = "Sovereign|Genetics")
    static bool CanBreed(AActor* ParentA, AActor* ParentB);

    // Warning ! This is an interesting chance for a simple yet scalable game mecanic where eat generation get better over the past.(question is how high can you go and how many generations do you want to store in memory so warning!)
    static TMap<FString, FString> RecombineDNA(const TMap<FString, FString>& MotherDNA, const TMap<FString, FString>& FatherDNA, float MutationChance);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Genetics")
    static TMap<FString, FString> GenerateMutatedTags(TMap<FString, FString> SourceTags, float MutationChance = 0.05f);

    UFUNCTION(BlueprintPure, Category = "Sovereign|Debug")
    static FString GetEntityAncestry(AActor* TargetActor);

    UFUNCTION(BlueprintPure, Category = "Sovereign|Genetics")
    static TArray<FString> ScanForMutations(AActor* ChildActor, AActor* ParentActor);
};
