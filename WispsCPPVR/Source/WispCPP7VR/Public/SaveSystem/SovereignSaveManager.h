// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h" // The Boss's new parent class
#include "SaveSystem/SovereignGameData.h" // Needed for FEntitySaveData
#include "SaveSystem/SovereignDungeonDelta.h"
#include "SovereignSaveManager.generated.h"

/**
 * @class USaveManager
 * @brief The "Brain" of the Sovereign Save System.
 * Lives in the GameInstance, so it survives level changes.
 */

class USovereignSaveGame;
class ASovereignBaseInteractable;

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

    /**
     * Checks if an interactable is an unmutated procedural dungeon actor (Ticket B-115).
     * If true, it should be excluded from full entity serialization to prevent saving thousands
     * of static room actors in 750+ room mega-dungeons.
     */
    static bool ShouldBypassProceduralActorSerialization(AActor* InActor);

    /**
     * Applies lightweight dungeon deltas onto active procedural actors in the world (Ticket B-115).
     */
    void ApplyDungeonDeltas(const FSovereignDungeonDeltaState& DeltaState);

    // Helpers to serialize and deserialize save suitcases to/from JSON (Ticket B-115 & B-118)
    static FString ConvertSuitcaseToJson(USovereignSaveGame* Suitcase);
    static USovereignSaveGame* ConvertJsonToSuitcase(const FString& JsonContent);

    /**
     * Updates or merges an individual entity's serialized state into a saved world slot (Ticket B-123).
     * Used by dungeon exit portals to synchronize character progression back to the Garden save file.
     */
    static bool UpdateEntityInSavedSlot(const FString& SlotName, const FGuid& EntityGuid, const TSharedPtr<FJsonObject>& NewEntityState);

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

private:

    // This is where we will eventually track the active SaveSlot
    FString CurrentSaveSlot;

    // Acts as switch between saving to a json file or saving to unreal default uSavegameobject Binary system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Settings", meta = (AllowPrivateAccess = "true"))
    bool bUseJsonByDefault = true;

};
