// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h" // The Boss's new parent class
#include "SaveSystem/SovereignGameData.h" // Needed for FEntitySaveData
#include "SovereignSaveManager.generated.h"


/**
 * @class USaveManager
 * @brief The "Brain" of the Sovereign Save System.
 * Lives in the GameInstance, so it survives level changes.
 */

class USovereignSaveGame;

UCLASS()
class WISPCPP7VR_API USaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:	

    // Subsystems use Initialize instead of BeginPlay
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** * @param SlotName File name
         * @param bAsJson If true, writes a readable .json. If false, writes a binary .sav
         */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|SaveSystem")
    void SaveWorldState(FString SlotName, bool bAsJson = true);

    /** * @param SlotName File name
     * @param bAsJson If true, looks for .json. If false, looks for .sav
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|SaveSystem")
    void LoadWorldState(FString SlotName, bool bAsJson = true);

    /** * Peeks into a save slot and returns the number of entities stored within.
 * Does NOT spawn or change the world state.
 */
    UFUNCTION(BlueprintPure, Category = "Sovereign|SaveSystem")
    int32 GetSavedEntityCount(FString SlotName, bool bIsJson = true);

    /** Logic: If I didn't save you, you don't exist. */
    void PurgeUnearnedEntities(class USovereignSaveGame* LoadedSuitcase);


protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

private:

    // This is where we will eventually track the active SaveSlot
    FString CurrentSaveSlot;

    // Acts as switch between saving to a json file or saving to unreal default uSavegameobject Binary system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Settings", meta = (AllowPrivateAccess = "true"))
    bool bUseJsonByDefault = true;

    // Helpers to keep the main logic clean
    FString ConvertSuitcaseToJson(USovereignSaveGame* Suitcase);
    USovereignSaveGame* ConvertJsonToSuitcase(const FString& JsonContent);

};
