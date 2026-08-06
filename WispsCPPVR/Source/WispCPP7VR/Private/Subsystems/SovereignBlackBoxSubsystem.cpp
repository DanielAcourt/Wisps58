// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Subsystems/SovereignBlackBoxSubsystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFileManager.h"

void USovereignBlackBoxSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString Dir = GetBlackBoxDirectory();
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (!PlatformFile.DirectoryExists(*Dir))
    {
        PlatformFile.CreateDirectoryTree(*Dir);
        UE_LOG(LogTemp, Warning, TEXT("BlackBoxSubsystem: Created directory %s"), *Dir);
    }
}

void USovereignBlackBoxSubsystem::RecordEntries(const FGuid& EntityID, const TArray<FBlackBoxEntry>& Entries)
{
    AppendEntriesToFile(EntityID, Entries);
}

FString USovereignBlackBoxSubsystem::GetBlackBoxDirectory() const
{
    return FPaths::ProjectSavedDir() / TEXT("BlackBox");
}

FString USovereignBlackBoxSubsystem::GetFilePathForEntity(const FGuid& EntityID) const
{
    return GetBlackBoxDirectory() / FString::Printf(TEXT("BB_%s.json"), *EntityID.ToString());
}

void USovereignBlackBoxSubsystem::AppendEntriesToFile(const FGuid& EntityID, const TArray<FBlackBoxEntry>& NewEntries)
{
    if (NewEntries.Num() == 0) return;

    FString FilePath = GetFilePathForEntity(EntityID);
    FString JsonString;

    TArray<TSharedPtr<FJsonValue>> LogArray;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    // 1. Load existing if it exists
    if (PlatformFile.FileExists(*FilePath))
    {
        FString ExistingContent;
        if (FFileHelper::LoadFileToString(ExistingContent, *FilePath))
        {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingContent);
            TSharedPtr<FJsonObject> RootObj;
            if (FJsonSerializer::Deserialize(Reader, RootObj) && RootObj.IsValid())
            {
                const TArray<TSharedPtr<FJsonValue>>* ExistingLogs;
                if (RootObj->TryGetArrayField(TEXT("Logs"), ExistingLogs))
                {
                    LogArray = *ExistingLogs;
                }
            }
        }
    }

    // 2. Add new entries
    for (const FBlackBoxEntry& Entry : NewEntries)
    {
        TSharedPtr<FJsonObject> EntryObj = MakeShareable(new FJsonObject());
        EntryObj->SetStringField(TEXT("Timestamp"), Entry.Timestamp.ToString());
        EntryObj->SetStringField(TEXT("Key"), Entry.Key);
        EntryObj->SetNumberField(TEXT("Value"), Entry.Value);

        if (!Entry.EventData.IsEmpty())
        {
            EntryObj->SetStringField(TEXT("EventData"), Entry.EventData);
        }

        LogArray.Add(MakeShareable(new FJsonValueObject(EntryObj)));
    }

    // 3. PERSISTENCE: Save the truth back to disk using an Atomic Write pattern.
    TSharedPtr<FJsonObject> FinalRoot = MakeShareable(new FJsonObject());
    FinalRoot->SetArrayField(TEXT("Logs"), LogArray);

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(FinalRoot.ToSharedRef(), Writer);

    // ATOMIC WRITE: We write to a temporary file first. If the write is interrupted (crash/power loss),
    // the original log file remains intact, preventing total data corruption.
    FString TempFilePath = FilePath + TEXT(".tmp");
    if (FFileHelper::SaveStringToFile(JsonString, *TempFilePath))
    {
        // WINDOWS HARDENING: Explicitly delete the destination file before moving.
        // On some platforms (Windows/UE), MoveFile can fail if the destination already exists or has an open handle.
        // Explicit deletion mitigates "Access Denied" (Result 5) errors.
        if (PlatformFile.FileExists(*FilePath))
        {
            PlatformFile.DeleteFile(*FilePath);
        }

        if (!PlatformFile.MoveFile(*FilePath, *TempFilePath))
        {
            UE_LOG(LogTemp, Error, TEXT("BlackBoxSubsystem: Failed to move temp file %s to %s"), *TempFilePath, *FilePath);
        }
    }
}
