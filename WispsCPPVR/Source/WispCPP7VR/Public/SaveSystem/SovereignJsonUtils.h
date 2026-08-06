// Fill out your copyright notice in the Description page of Project Settings.
//The Tool: The "Static" library that handles the heavy lifting of turning Maps into JSON strings and writing them to the hard drive.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h" //// This include is vital for the JSON types we will use in the .cpp
#include "SovereignJsonUtils.generated.h"

/**
* @class USovereignJsonUtils
* @brief Global utility for serializing ABaseEntity data and file I / O.
*/

UCLASS()
class WISPCPP7VR_API USovereignJsonUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    /** * WHY: The "Workhorse" of the system.
       * WHAT: Takes raw text (JSON) and pushes it to the hard drive.
       * It handles the file pathing and folder creation automatically.
       */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|SaveSystem")
    static bool SaveStringToFile(FString FileName, FString Content);

    /** * /** * WHY: To bring Player's garden back to life. 
     * WHAT: Finds a file by name and reads the text inside back into a string 
     * that our JSON parser can understand.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|SaveSystem")
    static bool LoadStringFromFile(FString FileName, FString& OutContent);

    /** * /** * WHY: Handles [2025-12-20] requirement for "Unknown Tags."
     * WHAT: Takes a flexible C++ Map (Key/Value pairs) and turns it into a 
     * structured JSON object that can be saved.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|SaveSystem")
    static bool ValidateAndSave(FString FileName, FString Content);

    static TSharedPtr<FJsonObject> ParseJsonString(FString JsonString);
};

