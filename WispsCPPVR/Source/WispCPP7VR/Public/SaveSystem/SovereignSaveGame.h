// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

/*2. The SaveGame Object (UWispsSaveGame)
Class Type: Inherits from SaveGame

Location: Public/SaveSystem/ & Private/SaveSystem/

Purpose: This is the actual file format. It will contain a FGardenSaveData variable. It acts as the "Suitcase" that travels to the hard drive.
The Container: A standard Unreal class used to store high-level data like "Last Played Date" or "Player Level."
Image Tag:*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem/SovereignGameData.h"

#include "SovereignSaveGame.generated.h"

/**
 * 
 */USTRUCT(BlueprintType)
     struct FSovereignActorData
 {
     GENERATED_BODY()

     // The unique ID we generated in the Component
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
     FGuid EntityID;

     // The blueprint path so we can respawn this object if it's missing
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
     FString ClassPath;

     // The "Unknown Tags" we harvested (e.g., "GrowthStage:5")
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
     TMap<FString, FString> MetaTags;

     // The physical location/rotation in the world
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
     FTransform Transform;
 };

 /**
  * @class USovereignSaveGame
  * @brief The "Suitcase" file that gets written to the hard drive.
  */
 UCLASS()
     class WISPCPP7VR_API USovereignSaveGame : public USaveGame
 {
     GENERATED_BODY()

 public:
     // The master list of every saved entity in the world
     UPROPERTY()
     TArray<FEntitySaveData> SavedActors; // <--- This must match the struct in SovereignGameData.h

     // High-level metadata (Optional but helpful for Isla)
     UPROPERTY(BlueprintReadWrite, Category = "Sovereign")
     FString SaveSlotName;

     UPROPERTY(BlueprintReadWrite, Category = "Sovereign")
     FDateTime SaveTime;

     // You can add global world data here later, like:
    // UPROPERTY()
    // float TimeOfDay;
 };