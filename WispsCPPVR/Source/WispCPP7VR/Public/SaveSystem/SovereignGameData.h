// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

/*1. The Data Structs (GardenData.h)
Class Type: "None" (Header Only)
The Data Model: This defines what a "Saved Actor" actually looks like (GUID, Transform, Tags). 
It’s the blueprint for the JSON.
Location: Public/SaveSystem/

Purpose: To define the "Language" of your save system. By putting Enums (GrowthState) and Structs (FEntitySaveData) here, you prevent circular dependencies.

Status: Do this first.*/

#pragma once

#include"CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Dom/JsonObject.h"
#include "SovereignGameData.generated.h"

class AWispsGameModeBase; // Just type this line above the struct

USTRUCT(BlueprintType)
struct FItemDefaults : public FTableRowBase
{
    GENERATED_BODY()

    FItemDefaults()
        : SpeciesVesselData(nullptr)
    {}

    // we need this gameplay to be made or matched in some way!
    //Can be called Gameplaytag and be more genic as its not specific to a species but anything in the engine we define
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Identity")
    FGameplayTag SpeciesTag;

    // My first custom "SpeciesClass"
    // This should package things a bit nicer
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Data")
    class USovereignSpeciesData* SpeciesVesselData;
};

UENUM(BlueprintType)
enum class ESovereignGrowthStage : uint8
{
    // 0-2: The Beginning (High Vulnerability)
    Inception,   // Concept/Ghost/Egg-Phase 

    //1 of  2 a half possible for clones with increase mutattion?

    //Has mother and father
    Seed,        // Dormant/Egg
    Sprout,      // Just emerged/Hatchling

    // 3-5: The Development (Rapid Growth)
    Juvenile,     // Juvenile/Young/Sapling
    Adolescent,       // Adolescent/Fledgling/Shrub
    Adult,        // Adult/Prime Can make babies 

    // 6-9: The Beyond (High Power)
    Elder, // Ancient/Venerable Can make babies but has a hit in chance and increased danger for females
    Mature,
    Retired,
    Deteriorating,

    GoldenCore, 
   
    Mythical,
    Legendary,   // Mythic (Replacing Seed1-3 with Tier labels) rare to have children
    
    PrimalSpirit,
    Eternal,     // Peak Evolution Almost imposibble to have children
    Godly,      // Cant Have children no matter the buffs or abilities can rebirth and use those stats in another form

    MAX UMETA(Hidden)
};

// --- 1. GLOBAL SCOPE ENUMS (This must be OUTSIDE of any class/struct) ---
//Version 3.0 Expanded

UENUM(BlueprintType)
enum class EUpdateFrequency : uint8
{
    //check my math
    Faster      UMETA(DisplayName = "Every Frame (Wisps/NPCS)"),
    Realtime    UMETA(DisplayName = "Every Frame (Wisps/NPCS)"),  // Key things NPCS,Dragons Wisps etc very limted

    Standard    UMETA(DisplayName = "Every 1 Second (Animals/Active)"), // active hunter more dexitiry

    Slow        UMETA(DisplayName = "Every 10 Seconds (Animals/Passive)"), //animals herbivor slow grazers

    Slower      UMETA(DisplayName = "Every 30 Seconds (Creatures/Bees)"), //30 seconds

    Slowest     UMETA(DisplayName = "Every 60 Seconds (Plants/Rocks)"), //minute / Poissible bee level?
    Glacier     UMETA(DisplayName = "Every 360 Seconds (Geological/Rocks)"), //hour fice numbers
    Dormant     UMETA(DisplayName = "Only on Interaction (Events/Fences)")
};


//Should i conside this the bae lement and expirment?

UENUM(BlueprintType)
enum class ESovereignElement : uint8
{
    None,
    // Alignment
    Light, Dark, Grey,
    // Core Natural
    Fire, Water, Earth,
    // Not implemented in element component
    Nature, Air,
    // Special
    // Not defined in any save compoent or element component ideally it needs to be handled with tags
    Electric, Fairy, Dragon
};

USTRUCT(BlueprintType)
struct FEntitySaveData
{
    GENERATED_BODY()

    FEntitySaveData()
        : MyGUID(FGuid::NewGuid())
        , ParentID(FGuid())
        , MotherID(FGuid())
        , FatherID(FGuid())
        , bIsFemale(false)
        , OffspringCount(0)
        , SavedGrowthStage(ESovereignGrowthStage::Inception)
        , WorldTransform(FTransform::Identity)
        , ClassPath(TEXT(""))
        , SavedFrequency(EUpdateFrequency::Standard)
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IgnoreForMemberInitializationTest))
    FGuid MyGUID;

    /** The GUID of the entity (Wisp, Plant, or Tool) that created this object. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Lineage")
    FGuid ParentID;

    // --- Optinial i might rethink this approach as a Seperate component to "living things" DO WE ADD THESE so early on?? ---
    UPROPERTY()
    FGuid MotherID;

    UPROPERTY()
    FGuid FatherID;

    UPROPERTY()
    bool bIsFemale;

    UPROPERTY()
    int32 OffspringCount;

    UPROPERTY()
    TArray<FGuid> MatingHistory;

    // Inside SovereignGameData.h
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|SaveData")
    ESovereignGrowthStage SavedGrowthStage; // Update the type name here!
    // -----------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform WorldTransform;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
    //EGrowthState CurrentState;

    /** * THE SECRET SAUCE: Handles tags the system can't know in advance.
     * This is where your "Key:Value" scraper saves its data.
     */

    // SHould this be a Bleuprint readwrite option
    TSharedPtr<FJsonObject> UnknownMetaTags;


    /** * The blueprint path of the actor.
     * Essential if Player needs to respawn an object that was deleted.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
    FString ClassPath;

    // We save the frequency so the heartbeat restores correctly!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign")
    EUpdateFrequency SavedFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Elements")
    TMap<ESovereignElement, float> ElementalAffinities;
};

/**
 * @struct FBlackBoxEntry
 * @brief A single timestamped "Truth Event" for a specific telemetry key.
 */
USTRUCT(BlueprintType)
struct FBlackBoxEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|BlackBox")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|BlackBox")
    FString Key;

    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|BlackBox")
    float Value;

    /** Optional string-based event data for non-numeric logs. */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|BlackBox")
    FString EventData;

    FBlackBoxEntry()
        : Timestamp(FDateTime::Now())
        , Key(TEXT(""))
        , Value(0.0f)
        , EventData(TEXT(""))
    {}

    FBlackBoxEntry(const FString& InKey, float InValue, const FString& InEventData = TEXT(""))
        : Timestamp(FDateTime::Now())
        , Key(InKey)
        , Value(InValue)
        , EventData(InEventData)
    {}
};
