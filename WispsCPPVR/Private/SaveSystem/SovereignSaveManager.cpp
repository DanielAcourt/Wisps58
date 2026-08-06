// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "SaveSystem/SovereignSaveManager.h"
#include "SaveSystem/SovereignSaveGame.h"
#include "SaveSystem/SovereignGameData.h"
#include "SaveSystem/SovereignActorRegistry.h"
#include "SaveSystem/SovereignJsonUtils.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"

void USaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("SovereignSystem: SaveManager Boss Initialized."));
}

void USaveManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("SovereignSystem: SaveManager Boss Shutting Down."));
    Super::Deinitialize();
}

void USaveManager::PurgeUnearnedEntities(USovereignSaveGame* LoadedSuitcase)
{
    if (!LoadedSuitcase) 
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveSystem: PurgeUnearnedEntities called with null LoadedSuitcase"));
        return;
    }

    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;
    if (!Registry) 
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: ActorRegistry subsystem not found"));
        return;
    }

    TArray<AActor*> TrackedActors = Registry->GetTrackedActorsAsList();
    int32 PurgedCount = 0;

    for (AActor* Actor : TrackedActors)
    {
        //FIX #2: Check if actor is still valid (weak ptr may have expired)
        if (!Actor)
        {
            UE_LOG(LogTemp, Verbose, TEXT("SaveSystem: Skipping purge of destroyed actor (weak ptr expired)"));
            continue;
        }

        auto* SaveComp = Actor->FindComponentByClass<USovereignSaveableEntityComponent>();
        if (!SaveComp) 
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Tracked actor %s missing SaveableEntityComponent"), *Actor->GetName());
            continue;
        }

        bool bFound = LoadedSuitcase->SavedActors.ContainsByPredicate([&](const FEntitySaveData& S)
        {
            return S.MyGUID == SaveComp->EntityID;
        });

        if (!bFound)
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Purging untracked entity %s (GUID: %s)"), 
                *Actor->GetName(), *SaveComp->EntityID.ToString());
            Actor->Destroy();
            PurgedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Purge complete. Removed %d untracked entities."), PurgedCount);
}

void USaveManager::SaveWorldState(FString SlotName, bool bAsJson)
{
    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;

    USovereignSaveGame* SaveSuitcase = Cast<USovereignSaveGame>(
        UGameplayStatics::CreateSaveGameObject(USovereignSaveGame::StaticClass()));

    if (!Registry || !SaveSuitcase) return;

    for (auto& Elem : Registry->GetActiveRegistry())
    {
        if (AActor* TargetActor = Elem.Value.Get())
        {
            if (auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>())
            {
                FEntitySaveData Data;

                Data.MyGUID = Elem.Key;
                Data.ParentID = SaveComp->ParentID;
                Data.MotherID = SaveComp->MotherID;
                Data.FatherID = SaveComp->FatherID;
                Data.bIsFemale = SaveComp->bIsFemale;
                Data.OffspringCount = SaveComp->OffspringCount;
                Data.MatingHistory = SaveComp->MatingHistory;

                Data.WorldTransform = TargetActor->GetActorTransform();
                Data.ClassPath = TargetActor->GetClass()->GetPathName();

                TSharedPtr<FJsonObject> CapturedState = SaveComp->CaptureFullEntityState();
                if (CapturedState.IsValid())
                {
                    Data.UnknownMetaTags = CapturedState;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity %s returned null state"), *SaveComp->EntityID.ToString());
                }

                SaveSuitcase->SavedActors.Add(Data);
            }
        }
    }

    if (bAsJson)
    {
        FString JsonStr = ConvertSuitcaseToJson(SaveSuitcase);
        USovereignJsonUtils::ValidateAndSave(SlotName, JsonStr);
    }
    else
    {
        UGameplayStatics::SaveGameToSlot(SaveSuitcase, SlotName, 0);
    }

    UE_LOG(LogTemp, Log, TEXT("Sovereign: World State committed to %s. Total Entities: %d"), 
        *SlotName, SaveSuitcase->SavedActors.Num());
}

void USaveManager::LoadWorldState(FString SlotName, bool bAsJson)
{
    USovereignSaveGame* LoadedSuitcase = nullptr;

    if (bAsJson)
    {
        FString JsonContent;
        if (!USovereignJsonUtils::LoadStringFromFile(SlotName, JsonContent))
        {
            UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to load JSON file: %s"), *SlotName);
            return;
        }
        LoadedSuitcase = ConvertJsonToSuitcase(JsonContent);
    }
    else
    {
        LoadedSuitcase = Cast<USovereignSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    if (!LoadedSuitcase)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to load save data from slot: %s"), *SlotName);
        return;
    }

    PurgeUnearnedEntities(LoadedSuitcase);

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: GetWorld() returned null during load"));
        return;
    }

    UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: ActorRegistry subsystem not available"));
        return;
    }

    int32 SpawnedCount = 0;
    int32 SkippedCount = 0;

    for (const FEntitySaveData& Data : LoadedSuitcase->SavedActors)
    {
        if (!Data.MyGUID.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity has invalid GUID. Skipping."));
            SkippedCount++;
            continue;
        }

        AActor* TargetActor = Registry->FindActor(Data.MyGUID);

        if (!TargetActor)
        {
            if (Data.ClassPath.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity %s has empty ClassPath. Skipping."), 
                    *Data.MyGUID.ToString());
                SkippedCount++;
                continue;
            }

            UClass* ActorClass = LoadObject<UClass>(nullptr, *Data.ClassPath);
            if (!ActorClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Could not load class %s for entity %s"), 
                    *Data.ClassPath, *Data.MyGUID.ToString());
                SkippedCount++;
                continue;
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // FIX #1: Check if spawn succeeded
            TargetActor = World->SpawnActor<AActor>(ActorClass, Data.WorldTransform, SpawnParams);

            if (!TargetActor)
            {
                UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to spawn entity of class %s at %s"), 
                    *Data.ClassPath, *Data.WorldTransform.ToString());
                SkippedCount++;
                continue;
            }

            auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>();
            if (!SaveComp)
            {
                UE_LOG(LogTemp, Error, TEXT("SaveSystem: Spawned actor %s missing SaveableEntityComponent"), 
                    *TargetActor->GetName());
                TargetActor->Destroy();
                SkippedCount++;
                continue;
            }

            SaveComp->EntityID = Data.MyGUID;
            SaveComp->ParentID = Data.ParentID;
            Registry->RegisterActor(Data.MyGUID, TargetActor);
            SpawnedCount++;
        }

        if (TargetActor)
        {
            TargetActor->SetActorTransform(Data.WorldTransform);
            auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>();
            if (SaveComp)
            {
                SaveComp->ParentID = Data.ParentID;

                if (Data.UnknownMetaTags.IsValid())
                {
                    SaveComp->ApplyStateFromJsonObject(Data.UnknownMetaTags);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Load complete. Spawned=%d, Skipped=%d. Symmetry restored from %s."), 
        SpawnedCount, SkippedCount, *SlotName);
}

int32 USaveManager::GetSavedEntityCount(FString SlotName, bool bIsJson)
{
    USovereignSaveGame* TempSuitcase = nullptr;

    if (bIsJson)
    {
        FString JsonContent;
        if (USovereignJsonUtils::LoadStringFromFile(SlotName, JsonContent))
        {
            TempSuitcase = ConvertJsonToSuitcase(JsonContent);
        }
    }
    else
    {
        TempSuitcase = Cast<USovereignSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    return (TempSuitcase) ? TempSuitcase->SavedActors.Num() : 0;
}

USovereignSaveGame* USaveManager::ConvertJsonToSuitcase(const FString& JsonContent)
{
    if (JsonContent.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: ConvertJsonToSuitcase called with empty content"));
        return nullptr;
    }

    USovereignSaveGame* NewSuitcase = Cast<USovereignSaveGame>(
        UGameplayStatics::CreateSaveGameObject(USovereignSaveGame::StaticClass()));

    if (!NewSuitcase)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to create SaveGame object"));
        return nullptr;
    }

    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);

    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to deserialize JSON content"));
        return nullptr;
    }

    const TArray<TSharedPtr<FJsonValue>>* EntityArray = nullptr;
    if (!RootObj->TryGetArrayField(TEXT("Entities"), EntityArray))
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: JSON missing 'Entities' array"));
        return nullptr;
    }

    if (!EntityArray)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: 'Entities' field is not a valid array"));
        return nullptr;
    }

    int32 ParsedCount = 0;
    int32 SkippedCount = 0;

    for (const auto& Val : *EntityArray)
    {
        TSharedPtr<FJsonObject> Obj = Val->AsObject();
        if (!Obj.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity at index %d is not a valid JSON object"), 
                ParsedCount + SkippedCount);
            SkippedCount++;
            continue;
        }

        FEntitySaveData Data;

        // FIX #3: Safe GUID parsing
        FString GuidString;
        if (!Obj->TryGetStringField(TEXT("GUID"), GuidString) || GuidString.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity missing GUID field. Skipping."));
            SkippedCount++;
            continue;
        }

        if (!FGuid::Parse(GuidString, Data.MyGUID))
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Invalid GUID format: %s. Skipping."), *GuidString);
            SkippedCount++;
            continue;
        }

        // Safe ClassPath parsing
        FString ClassPath;
        if (!Obj->TryGetStringField(TEXT("Class"), ClassPath) || ClassPath.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity %s missing Class field"), *Data.MyGUID.ToString());
            SkippedCount++;
            continue;
        }
        Data.ClassPath = ClassPath;

        // Safe Transform parsing
        FString TransformString;
        if (Obj->TryGetStringField(TEXT("Transform"), TransformString))
        {
            Data.WorldTransform.InitFromString(TransformString);
        }
        else
        {
            Data.WorldTransform = FTransform::Identity;
        }

        // Safe ParentID parsing
        FString ParentIDString;
        if (Obj->TryGetStringField(TEXT("ParentID"), ParentIDString) && !ParentIDString.IsEmpty())
        {
            FGuid::Parse(ParentIDString, Data.ParentID);
        }

        // Safe metadata parsing
        if (Obj->HasField(TEXT("MetaTags")))
        {
            Data.UnknownMetaTags = Obj->GetObjectField(TEXT("MetaTags"));
        }

        NewSuitcase->SavedActors.Add(Data);
        ParsedCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Parsed %d entities, skipped %d invalid"), ParsedCount, SkippedCount);
    return NewSuitcase;
}

FString USaveManager::ConvertSuitcaseToJson(USovereignSaveGame* Suitcase)
{
    if (!Suitcase) return "";

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    Writer->WriteObjectStart();
    Writer->WriteArrayStart(TEXT("Entities"));

    for (const FEntitySaveData& Data : Suitcase->SavedActors)
    {
        Writer->WriteObjectStart();

        Writer->WriteValue(TEXT("GUID"), Data.MyGUID.ToString());
        Writer->WriteValue(TEXT("ParentID"), Data.ParentID.ToString());
        Writer->WriteValue(TEXT("Class"), Data.ClassPath);
        Writer->WriteValue(TEXT("Transform"), Data.WorldTransform.ToString());

        if (Data.UnknownMetaTags.IsValid())
        {
            Writer->WriteIdentifierPrefix(TEXT("MetaTags"));
            FJsonSerializer::Serialize(Data.UnknownMetaTags.ToSharedRef(), Writer);
        }

        Writer->WriteObjectEnd();
    }

    Writer->WriteArrayEnd();
    Writer->WriteObjectEnd();
    Writer->Close();

    return Output;
}