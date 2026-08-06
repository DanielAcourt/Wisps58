// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "SaveSystem/SovereignSaveManager.h"
#include "SaveSystem/SovereignSaveGame.h"
#include "SaveSystem/SovereignGameData.h"
#include "SaveSystem/SovereignActorRegistry.h"
#include "SaveSystem/SovereignJsonUtils.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h" // Requires "JsonUtilities" in Build.cs
#include "Components/SovereignBioComponent.h"

void USaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // This is where you'd set up global save settings 
    // when the game first launches.
    UE_LOG(LogTemp, Log, TEXT("SovereignSystem: SaveManager Boss Initialized."));
}
void USaveManager::Deinitialize()
{
    // Clean up any temporary save data before the game closes
    UE_LOG(LogTemp, Log, TEXT("SovereignSystem: SaveManager Boss Shutting Down."));

    Super::Deinitialize();
}

void USaveManager::PurgeUnearnedEntities(USovereignSaveGame* LoadedSuitcase)
{
    if (!LoadedSuitcase) return;

    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;
    if (!Registry) return;

    // Get only the actors we are actually tracking
    TArray<AActor*> TrackedActors = Registry->GetTrackedActorsAsList();

    for (AActor* Actor : TrackedActors)
    {
        auto* SaveComp = Actor->FindComponentByClass<USovereignSaveableEntityComponent>();
        if (!SaveComp) continue;

        // Use a Lambda or FindByPredicate for better performance than a nested loop
        bool bFound = LoadedSuitcase->SavedActors.ContainsByPredicate([&](const FEntitySaveData& S) {
            return S.MyGUID == SaveComp->EntityID;
            });

        if (!bFound)
        {
            UE_LOG(LogTemp, Warning, TEXT("Sovereign: Purging %s"), *Actor->GetName());
            Actor->Destroy(); // Registry cleanup happens automatically in Actor's EndPlay!
        }
    }
}

void USaveManager::SaveWorldState(FString SlotName, bool bAsJson)
{
    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;

    // Create the 'Suitcase' (SaveGame Object) to hold our data
    USovereignSaveGame* SaveSuitcase = Cast<USovereignSaveGame>(UGameplayStatics::CreateSaveGameObject(USovereignSaveGame::StaticClass()));

    if (!Registry || !SaveSuitcase) return;

    // Iterate through the Registry (Active Actors in the Garden)
    for (auto& Elem : Registry->GetActiveRegistry())
    {
        // Get the physical actor from the Weak Pointer
        if (AActor* TargetActor = Elem.Value.Get())
        {
            // Find the 'Passport' (Component) containing the Sovereign data
            if (auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>())
            {
                FEntitySaveData Data;

                // 1. IDENTITY: Who am I?
                Data.MyGUID = Elem.Key;

                // 2. LINEAGE: Who is my parent? (The Genetic Link)
                // Access lineage data from Bio component
                if (auto* BioComp = TargetActor->FindComponentByClass<USovereignBioComponent>())
                {
                    Data.ParentID = BioComp->ParentID;
                    Data.MotherID = BioComp->MotherID;
                    Data.FatherID = BioComp->FatherID;
                    Data.bIsFemale = BioComp->bIsFemale;
                    Data.OffspringCount = BioComp->OffspringCount;
                    Data.MatingHistory = BioComp->MatingHistory; // Note: This is TArray<FGuid>, may need conversion
                }

                // 3. PHYSICALITY: Where am I and what am I?
                Data.WorldTransform = TargetActor->GetActorTransform();
                Data.ClassPath = TargetActor->GetClass()->GetPathName();

                // 4. THE SOVEREIGN BRIDGE : The Master Scrape
                Data.UnknownMetaTags = SaveComp->CaptureFullEntityState();

                // Pack this entity into the suitcase
                SaveSuitcase->SavedActors.Add(Data);
            }
        }
    }

    // FINAL COMMIT: Standard Save Slot or Human-Readable JSON
    if (bAsJson)
    {
        FString JsonStr = ConvertSuitcaseToJson(SaveSuitcase);
        USovereignJsonUtils::ValidateAndSave(SlotName, JsonStr);
    }
    else
    {
        UGameplayStatics::SaveGameToSlot(SaveSuitcase, SlotName, 0);
    }

    UE_LOG(LogTemp, Log, TEXT("Sovereign: World State committed to %s. Total Entities: %d"), *SlotName, SaveSuitcase->SavedActors.Num());
}
void USaveManager::LoadWorldState(FString SlotName, bool bAsJson)
{
    USovereignSaveGame* LoadedSuitcase = nullptr;

    if (bAsJson)
    {
        FString JsonContent;
        if (USovereignJsonUtils::LoadStringFromFile(SlotName, JsonContent))
        {
            LoadedSuitcase = ConvertJsonToSuitcase(JsonContent);
        }
    }
    else
    {
        LoadedSuitcase = Cast<USovereignSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    if (!LoadedSuitcase) return;

    // Phase 1: Clean the world of anything not in the save file
    PurgeUnearnedEntities(LoadedSuitcase);

    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;
    if (!World || !Registry) return;

    // Phase 2: Reconstitution
    for (const FEntitySaveData& Data : LoadedSuitcase->SavedActors)
    {
        AActor* TargetActor = Registry->FindActor(Data.MyGUID);

        // Spawn if missing
        if (!TargetActor)
        {
            if (Data.ClassPath.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Entity %s has empty ClassPath. Skipping spawn."), *Data.MyGUID.ToString());
                continue;
            }

            UClass* ActorClass = LoadObject<UClass>(nullptr, *Data.ClassPath);
            if (ActorClass)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                TargetActor = World->SpawnActor<AActor>(ActorClass, Data.WorldTransform, SpawnParams);

                if (TargetActor)
                {
                    if (auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>())
                    {
                        // Restore Identity immediately upon birth
                        SaveComp->EntityID = Data.MyGUID;
                        Registry->RegisterActor(Data.MyGUID, TargetActor);
                    }
                    
                    // Restore Lineage data to Bio component
                    if (auto* BioComp = TargetActor->FindComponentByClass<USovereignBioComponent>())
                    {
                        BioComp->ParentID = Data.ParentID; // The Genetic Link
                        BioComp->MotherID = Data.MotherID;
                        BioComp->FatherID = Data.FatherID;
                        BioComp->bIsFemale = Data.bIsFemale;
                    }
                }
            }
        }
        // Phase 3. APPLY STATE: Update physical and metabolic data for all actors (newly spawned or existing)
        if (TargetActor)
        {
            TargetActor->SetActorTransform(Data.WorldTransform);
            if (auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>())
            {
                // Restore [2025-12-20] Unknown Tags (Mutations, Qi levels, etc.)
                if (Data.UnknownMetaTags.IsValid())
                {
                    SaveComp->ApplyStateFromJsonObject(Data.UnknownMetaTags);
                }
            }
            
            // Ensure Bio component state is synced for existing actors
            if (auto* BioComp = TargetActor->FindComponentByClass<USovereignBioComponent>())
            {
                BioComp->ParentID = Data.ParentID;
                BioComp->MotherID = Data.MotherID;
                BioComp->FatherID = Data.FatherID;
                BioComp->bIsFemale = Data.bIsFemale;
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Sovereign: Symmetry Restored. World synchronized with %s."), *SlotName);
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

    // Return the count if found, otherwise 0
    return (TempSuitcase) ? TempSuitcase->SavedActors.Num() : 0;
}
USovereignSaveGame* USaveManager::ConvertJsonToSuitcase(const FString& JsonContent)
{
    USovereignSaveGame* NewSuitcase = Cast<USovereignSaveGame>(UGameplayStatics::CreateSaveGameObject(USovereignSaveGame::StaticClass()));
    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);

    if (FJsonSerializer::Deserialize(Reader, RootObj) && RootObj.IsValid())
    {
        const TArray<TSharedPtr<FJsonValue>>* EntityArray;
        if (RootObj->TryGetArrayField(TEXT("Entities"), EntityArray))
        {
            for (auto& Val : *EntityArray)
            {
                TSharedPtr<FJsonObject> Obj = Val->AsObject();
                if (!Obj.IsValid()) continue;

                FEntitySaveData Data;
                FGuid::Parse(Obj->GetStringField(TEXT("GUID")), Data.MyGUID);

                // --- ADDED: Parse ParentID if it exists ---
                if (Obj->HasField(TEXT("ParentID")))
                {
                    FGuid::Parse(Obj->GetStringField(TEXT("ParentID")), Data.ParentID);
                }

                Data.ClassPath = Obj->GetStringField(TEXT("Class"));
                Data.WorldTransform.InitFromString(Obj->GetStringField(TEXT("Transform")));

                if (Obj->HasField(TEXT("MetaTags")))
                {
                    Data.UnknownMetaTags = Obj->GetObjectField(TEXT("MetaTags"));
                }

                NewSuitcase->SavedActors.Add(Data);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to deserialize JSON content."));
    }

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

        // 1. Identity
        Writer->WriteValue(TEXT("GUID"), Data.MyGUID.ToString());

        // 2. Lineage (The Genetic Link)
        Writer->WriteValue(TEXT("ParentID"), Data.ParentID.ToString());

        // 3. Physicals
        Writer->WriteValue(TEXT("Class"), Data.ClassPath);
        Writer->WriteValue(TEXT("Transform"), Data.WorldTransform.ToString());

        // 4. Metabolics (Isla's Tags)
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