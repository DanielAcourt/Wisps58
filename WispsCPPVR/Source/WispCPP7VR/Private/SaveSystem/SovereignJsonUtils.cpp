// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/SovereignJsonUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

bool USovereignJsonUtils::SaveStringToFile(FString FileName, FString Content)
{
    // Ensure file extension is present to prevent "naked" files on the OS
        if (!FileName.EndsWith(".json")) { FileName += ".json"; }

        // Use ProjectSavedDir to keep user data out of the Engine/Source folders
    FString SaveDir = FPaths::ProjectSavedDir() + "SovereignSaves/";
    FString FullPath = SaveDir + FileName;

    // DIRECTORY GUARD: Checks if the folder exists on the Windows/Linux file system
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (!PlatformFile.DirectoryExists(*SaveDir))
    {
        PlatformFile.CreateDirectory(*SaveDir);

        // If the folder is missing, we create it so the save doesn't fail
        UE_LOG(LogTemp, Warning, TEXT("SovereignSystem: Created missing directory at %s"), *SaveDir);
    }

    // ForceUTF8 ensures that special characters (like Erisis's elven name) don't break
    return FFileHelper::SaveStringToFile(Content, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8);
}

bool USovereignJsonUtils::LoadStringFromFile(FString FileName, FString& OutContent)
{
    if (!FileName.EndsWith(".json")) { FileName += ".json"; }
    FString FullPath = FPaths::ProjectSavedDir() + "SovereignSaves/" + FileName;

    // SANITY CHECK:VALIDATION Don't try to read a file that isn't there
    if (!FPaths::FileExists(FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("SovereignSystem: Failed to load. File not found: %s"), *FullPath);
        return false;
    }

    return FFileHelper::LoadFileToString(OutContent, *FullPath);
}

//New Functionality ALlows me to Validate a save, we might need to do more logic here so it makes sense when being used
bool USovereignJsonUtils::ValidateAndSave(FString FileName, FString Content)
{
    // 1. JSON VALIDATION
    // We try to parse the string. If it's missing a bracket or has a typo, we stop here.
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Sovereign: Validation Failed! Content is not valid JSON. Save Aborted."));
        return false;
    }

    // 2. FILENAME CLEANING
    // Ensures PLayer's save files don't break Windows/Linux file naming rules
    FString CleanFileName = FileName;
    CleanFileName.ReplaceInline(TEXT("/"), TEXT("_"));
    CleanFileName.ReplaceInline(TEXT("\\"), TEXT("_"));
    CleanFileName.ReplaceInline(TEXT(" "), TEXT("_"));

    // 3. LOGGING
    UE_LOG(LogTemp, Log, TEXT("Sovereign: Validated JSON for %s. Committing to disk..."), *CleanFileName);

    // 4. COMMIT TO DISK
    // We call SaveStringToFile once using our cleaned name.
    return SaveStringToFile(CleanFileName, Content);
}
/*
TSharedPtr<FJsonObject> USovereignJsonUtils::MapToJsonObject(const TMap<FString, FString>& TagMap)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    for (const auto& Entry : TagMap)
    {
        JsonObject->SetStringField(Entry.Key, Entry.Value);
    }
    return JsonObject;
}
TMap<FString, FString> USovereignJsonUtils::JsonObjectToMap(TSharedPtr<FJsonObject> JsonObject)
{
    TMap<FString, FString> OutMap;

    if (JsonObject.IsValid())
    {
        for (auto& Entry : JsonObject->Values)
        {
            // Convert any JSON field back into our "Sovereign" string map
            OutMap.Add(Entry.Key, Entry.Value->AsString());
        }
    }

    return OutMap;
}
/*
TSharedPtr<FJsonObject> USovereignJsonUtils::ParseJsonString(FString JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        return JsonObject;
    }

    UE_LOG(LogTemp, Error, TEXT("Sovereign: Failed to parse JSON string!"));
    return nullptr;
}
*/
/*Export: Map -> MapToJsonObject -> SaveStringToFileImport:
LoadStringFromFile -> ParseJsonString -> JsonObjectToMap -> Map*/

// Updated to ensure we have a clean parsing tool for the SaveManager to use
TSharedPtr<FJsonObject> USovereignJsonUtils::ParseJsonString(FString JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        return JsonObject;
    }

    UE_LOG(LogTemp, Error, TEXT("Sovereign: Failed to parse JSON string!"));
    return nullptr;
}
