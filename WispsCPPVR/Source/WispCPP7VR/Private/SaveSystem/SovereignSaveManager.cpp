// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

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
#include "Combat/SovereignDungeonSubsystem.h"
#include "Entities/SovereignDungeonDoor.h"
#include "Entities/SovereignTreasureChest.h"
#include "Entities/SovereignRestTerminal.h"
#include "Entities/SovereignKeyPickup.h"
#include "Entities/SovereignDungeonEntrance.h"
#include "Entities/SovereignBossTeleporter.h"
#include "EngineUtils.h"

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

bool USaveManager::ShouldBypassProceduralActorSerialization(AActor* InActor)
{
    if (!InActor) return false;

    // Check if this actor is a static / procedural dungeon entity
    if (ASovereignDungeonDoor* Door = Cast<ASovereignDungeonDoor>(InActor))
    {
        // If the door is closed and locked (or unmodified default), bypass it
        // Only save if it has been opened or unlocked
        return !Door->bIsOpen;
    }

    if (ASovereignTreasureChest* Chest = Cast<ASovereignTreasureChest>(InActor))
    {
        // Only serialize chest if it has been opened/looted
        return !Chest->bHasBeenOpened;
    }

    if (ASovereignRestTerminal* Terminal = Cast<ASovereignRestTerminal>(InActor))
    {
        // Only serialize terminal if charges were consumed
        return (Terminal->RemainingRestCharges == Terminal->MaxRestCharges);
    }

    if (InActor->IsA<ASovereignDungeonEntrance>())
    {
        // Procedural entrance gateway is static and deterministically placed at Node 0
        return true;
    }

    if (ASovereignBossTeleporter* Teleporter = Cast<ASovereignBossTeleporter>(InActor))
    {
        // Only serialize if activated upon boss defeat
        return !Teleporter->bIsActive;
    }

    return false;
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
        if (!Actor) continue;

        // Procedural dungeon entities should NOT be purged; their layout is reconstructed from seed
        if (Actor->IsA<ASovereignDungeonDoor>() ||
            Actor->IsA<ASovereignTreasureChest>() ||
            Actor->IsA<ASovereignRestTerminal>() ||
            Actor->IsA<ASovereignKeyPickup>() ||
            Actor->IsA<ASovereignDungeonEntrance>() ||
            Actor->IsA<ASovereignBossTeleporter>())
        {
            continue;
        }

        auto* SaveComp = Actor->FindComponentByClass<USovereignSaveableEntityComponent>();
        if (!SaveComp) continue;

        // Use FindByPredicate for better performance than a nested loop
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

void USaveManager::ApplyDungeonDeltas(const FSovereignDungeonDeltaState& DeltaState)
{
    UWorld* World = GetWorld();
    UActorRegistry* Registry = World ? World->GetSubsystem<UActorRegistry>() : nullptr;
    if (!World) return;

    // Apply door states
    for (const auto& DoorPair : DeltaState.DoorStates)
    {
        AActor* Actor = Registry ? Registry->FindActor(DoorPair.Key) : nullptr;
        if (ASovereignDungeonDoor* Door = Cast<ASovereignDungeonDoor>(Actor))
        {
            if (DoorPair.Value && !Door->bIsOpen)
            {
                Door->OpenDoor();
            }
        }
    }

    // Apply chest states
    for (const auto& ChestPair : DeltaState.ChestStates)
    {
        AActor* Actor = Registry ? Registry->FindActor(ChestPair.Key) : nullptr;
        if (ASovereignTreasureChest* Chest = Cast<ASovereignTreasureChest>(Actor))
        {
            Chest->bHasBeenOpened = ChestPair.Value;
        }
    }

    // Apply rest terminal charges
    for (const auto& RestPair : DeltaState.RestTerminalCharges)
    {
        AActor* Actor = Registry ? Registry->FindActor(RestPair.Key) : nullptr;
        if (ASovereignRestTerminal* Terminal = Cast<ASovereignRestTerminal>(Actor))
        {
            Terminal->SetRestCharges(RestPair.Value);
        }
    }

    // Destroy collected key pickups
    for (const FGuid& KeyGuid : DeltaState.CollectedKeyGuids)
    {
        AActor* Actor = Registry ? Registry->FindActor(KeyGuid) : nullptr;
        if (ASovereignKeyPickup* Pickup = Cast<ASovereignKeyPickup>(Actor))
        {
            Pickup->Destroy();
        }
    }

    // Boss defeat & teleporter activation
    if (DeltaState.bBossDefeated)
    {
        for (TActorIterator<ASovereignBossTeleporter> It(World); It; ++It)
        {
            if (ASovereignBossTeleporter* Teleporter = *It)
            {
                Teleporter->ActivateTeleporter();
            }
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

    // Ingest Dungeon Expedition parameters and delta state if ongoing (Ticket B-115)
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
        {
            SaveSuitcase->DungeonDelta.ExpeditionSeed = DungeonSub->ActiveExpeditionSeed;
            SaveSuitcase->DungeonDelta.EnemyBuffMultiplier = DungeonSub->ActiveEnemyBuffMultiplier;
            SaveSuitcase->DungeonDelta.RewardMultiplier = DungeonSub->ActiveRewardMultiplier;
        }
    }

    TSet<AActor*> ProcessedActors;

    // Iterate through the Registry (Active Actors in World)
    for (auto& Elem : Registry->GetActiveRegistry())
    {
        // Get the physical actor from the Weak Pointer
        if (AActor* TargetActor = Elem.Value.Get())
        {
            if (ProcessedActors.Contains(TargetActor))
            {
                continue; // Skip if this physical actor instance was already serialized
            }

            // Ticket B-115: Filter out unmutated procedural actors to keep mega-dungeon saves lightweight
            if (ShouldBypassProceduralActorSerialization(TargetActor))
            {
                continue;
            }

            // Find the 'Passport' (Component) containing the Sovereign data
            if (auto* SaveComp = TargetActor->FindComponentByClass<USovereignSaveableEntityComponent>())
            {
                ProcessedActors.Add(TargetActor);

                // Collect procedural mutated deltas into DungeonDelta (Ticket B-115)
                if (ASovereignDungeonDoor* Door = Cast<ASovereignDungeonDoor>(TargetActor))
                {
                    SaveSuitcase->DungeonDelta.DoorStates.Add(SaveComp->EntityID, Door->bIsOpen);
                }
                else if (ASovereignTreasureChest* Chest = Cast<ASovereignTreasureChest>(TargetActor))
                {
                    SaveSuitcase->DungeonDelta.ChestStates.Add(SaveComp->EntityID, Chest->bHasBeenOpened);
                }
                else if (ASovereignRestTerminal* Terminal = Cast<ASovereignRestTerminal>(TargetActor))
                {
                    SaveSuitcase->DungeonDelta.RestTerminalCharges.Add(SaveComp->EntityID, Terminal->RemainingRestCharges);
                }
                else if (ASovereignBossTeleporter* Teleporter = Cast<ASovereignBossTeleporter>(TargetActor))
                {
                    SaveSuitcase->DungeonDelta.bBossDefeated = Teleporter->bIsActive;
                }

                FEntitySaveData Data;

                // 1. IDENTITY: Who am I? (Use the Component's canonical EntityID)
                Data.MyGUID = SaveComp->EntityID.IsValid() ? SaveComp->EntityID : Elem.Key;
                Data.ObjectName = SaveComp->ObjectName.IsEmpty() ? TargetActor->GetName() : SaveComp->ObjectName;

                // 2. LINEAGE: Who is my parent? (The Genetic Link)
                if (auto* BioComp = TargetActor->FindComponentByClass<USovereignBioComponent>())
                {
                    Data.ParentID = BioComp->ParentID;
                    Data.MotherID = BioComp->MotherID;
                    Data.FatherID = BioComp->FatherID;
                    Data.bIsFemale = BioComp->bIsFemale;
                    Data.OffspringCount = BioComp->OffspringCount;
                    Data.MatingHistory = BioComp->MatingHistory;
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

    UE_LOG(LogTemp, Log, TEXT("Sovereign: World State committed to %s. Total Entities: %d (Seed: %d)"),
        *SlotName, SaveSuitcase->SavedActors.Num(), SaveSuitcase->DungeonDelta.ExpeditionSeed);
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

    // Ingest Dungeon Expedition parameters into Dungeon Subsystem if present (Ticket B-115)
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
        {
            if (LoadedSuitcase->DungeonDelta.ExpeditionSeed != 0)
            {
                DungeonSub->ActiveExpeditionSeed = LoadedSuitcase->DungeonDelta.ExpeditionSeed;
                DungeonSub->ActiveEnemyBuffMultiplier = LoadedSuitcase->DungeonDelta.EnemyBuffMultiplier;
                DungeonSub->ActiveRewardMultiplier = LoadedSuitcase->DungeonDelta.RewardMultiplier;
            }
        }
    }

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
                        if (!Data.ObjectName.IsEmpty())
                        {
                            SaveComp->ObjectName = Data.ObjectName;
                        }
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
                if (!Data.ObjectName.IsEmpty())
                {
                    SaveComp->ObjectName = Data.ObjectName;
                }
                // Restore Unknown Tags (Mutations, Qi levels, etc.)
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

    // Phase 4: Apply Dungeon Deltas (Ticket B-115)
    ApplyDungeonDeltas(LoadedSuitcase->DungeonDelta);

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

                if (Obj->HasField(TEXT("ObjectName")))
                {
                    Data.ObjectName = Obj->GetStringField(TEXT("ObjectName"));
                }

                if (Obj->HasField(TEXT("ParentID")))
                {
                    FGuid::Parse(Obj->GetStringField(TEXT("ParentID")), Data.ParentID);
                }

                Data.ClassPath = Obj->GetStringField(TEXT("Class"));
                Data.WorldTransform.InitFromString(Obj->GetStringField(TEXT("Transform")));

                if (Obj->HasField(TEXT("MetaTags")))
                {
                    Data.UnknownMetaTags = Obj->GetObjectField(TEXT("MetaTags"));

                    const TSharedPtr<FJsonObject>* IdentityObjPtr;
                    if (Data.UnknownMetaTags->TryGetObjectField(TEXT("Identity"), IdentityObjPtr) && IdentityObjPtr->IsValid())
                    {
                        FString InnerGuidStr;
                        if ((*IdentityObjPtr)->TryGetStringField(TEXT("GUID"), InnerGuidStr))
                        {
                            FGuid InnerGuid;
                            if (FGuid::Parse(InnerGuidStr, InnerGuid) && InnerGuid.IsValid())
                            {
                                Data.MyGUID = InnerGuid;
                            }
                        }

                        FString InnerObjName;
                        if ((*IdentityObjPtr)->TryGetStringField(TEXT("ObjectName"), InnerObjName) && Data.ObjectName.IsEmpty())
                        {
                            Data.ObjectName = InnerObjName;
                        }
                    }
                }

                bool bAlreadyExists = NewSuitcase->SavedActors.ContainsByPredicate([&](const FEntitySaveData& Existing) {
                    return Existing.MyGUID == Data.MyGUID;
                });

                if (!bAlreadyExists)
                {
                    NewSuitcase->SavedActors.Add(Data);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Skipped duplicate save entry for GUID [%s]"), *Data.MyGUID.ToString());
                }
            }
        }

        // Deserialize DungeonDelta (Ticket B-115)
        const TSharedPtr<FJsonObject>* DeltaObjPtr;
        if (RootObj->TryGetObjectField(TEXT("DungeonDelta"), DeltaObjPtr) && DeltaObjPtr->IsValid())
        {
            TSharedPtr<FJsonObject> DeltaObj = *DeltaObjPtr;
            DeltaObj->TryGetNumberField(TEXT("ExpeditionSeed"), NewSuitcase->DungeonDelta.ExpeditionSeed);

            double TempVal = 1.0;
            if (DeltaObj->TryGetNumberField(TEXT("EnemyBuffMultiplier"), TempVal))
            {
                NewSuitcase->DungeonDelta.EnemyBuffMultiplier = (float)TempVal;
            }
            if (DeltaObj->TryGetNumberField(TEXT("RewardMultiplier"), TempVal))
            {
                NewSuitcase->DungeonDelta.RewardMultiplier = (float)TempVal;
            }
            DeltaObj->TryGetBoolField(TEXT("bBossDefeated"), NewSuitcase->DungeonDelta.bBossDefeated);

            // Door states
            const TSharedPtr<FJsonObject>* DoorObjPtr;
            if (DeltaObj->TryGetObjectField(TEXT("DoorStates"), DoorObjPtr) && DoorObjPtr->IsValid())
            {
                for (const auto& DoorPair : (*DoorObjPtr)->Values)
                {
                    FGuid DoorGuid;
                    if (FGuid::Parse(DoorPair.Key, DoorGuid))
                    {
                        NewSuitcase->DungeonDelta.DoorStates.Add(DoorGuid, DoorPair.Value->AsBool());
                    }
                }
            }

            // Chest states
            const TSharedPtr<FJsonObject>* ChestObjPtr;
            if (DeltaObj->TryGetObjectField(TEXT("ChestStates"), ChestObjPtr) && ChestObjPtr->IsValid())
            {
                for (const auto& ChestPair : (*ChestObjPtr)->Values)
                {
                    FGuid ChestGuid;
                    if (FGuid::Parse(ChestPair.Key, ChestGuid))
                    {
                        NewSuitcase->DungeonDelta.ChestStates.Add(ChestGuid, ChestPair.Value->AsBool());
                    }
                }
            }

            // Rest Terminal states
            const TSharedPtr<FJsonObject>* RestObjPtr;
            if (DeltaObj->TryGetObjectField(TEXT("RestTerminalCharges"), RestObjPtr) && RestObjPtr->IsValid())
            {
                for (const auto& RestPair : (*RestObjPtr)->Values)
                {
                    FGuid RestGuid;
                    if (FGuid::Parse(RestPair.Key, RestGuid))
                    {
                        NewSuitcase->DungeonDelta.RestTerminalCharges.Add(RestGuid, (int32)RestPair.Value->AsNumber());
                    }
                }
            }

            // Collected keys
            const TArray<TSharedPtr<FJsonValue>>* KeyArray;
            if (DeltaObj->TryGetArrayField(TEXT("CollectedKeyGuids"), KeyArray))
            {
                for (const auto& KeyVal : *KeyArray)
                {
                    FGuid KeyGuid;
                    if (FGuid::Parse(KeyVal->AsString(), KeyGuid))
                    {
                        NewSuitcase->DungeonDelta.CollectedKeyGuids.Add(KeyGuid);
                    }
                }
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
        Writer->WriteValue(TEXT("ObjectName"), Data.ObjectName);

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

    // Serialize DungeonDelta (Ticket B-115)
    Writer->WriteObjectStart(TEXT("DungeonDelta"));
    Writer->WriteValue(TEXT("ExpeditionSeed"), Suitcase->DungeonDelta.ExpeditionSeed);
    Writer->WriteValue(TEXT("EnemyBuffMultiplier"), Suitcase->DungeonDelta.EnemyBuffMultiplier);
    Writer->WriteValue(TEXT("RewardMultiplier"), Suitcase->DungeonDelta.RewardMultiplier);
    Writer->WriteValue(TEXT("bBossDefeated"), Suitcase->DungeonDelta.bBossDefeated);

    // Door states
    Writer->WriteObjectStart(TEXT("DoorStates"));
    for (const auto& DoorPair : Suitcase->DungeonDelta.DoorStates)
    {
        Writer->WriteValue(DoorPair.Key.ToString(), DoorPair.Value);
    }
    Writer->WriteObjectEnd();

    // Chest states
    Writer->WriteObjectStart(TEXT("ChestStates"));
    for (const auto& ChestPair : Suitcase->DungeonDelta.ChestStates)
    {
        Writer->WriteValue(ChestPair.Key.ToString(), ChestPair.Value);
    }
    Writer->WriteObjectEnd();

    // Rest terminal charges
    Writer->WriteObjectStart(TEXT("RestTerminalCharges"));
    for (const auto& RestPair : Suitcase->DungeonDelta.RestTerminalCharges)
    {
        Writer->WriteValue(RestPair.Key.ToString(), RestPair.Value);
    }
    Writer->WriteObjectEnd();

    // Collected keys
    Writer->WriteArrayStart(TEXT("CollectedKeyGuids"));
    for (const FGuid& KeyGuid : Suitcase->DungeonDelta.CollectedKeyGuids)
    {
        Writer->WriteValue(KeyGuid.ToString());
    }
    Writer->WriteArrayEnd();

    Writer->WriteObjectEnd(); // End DungeonDelta

    Writer->WriteObjectEnd(); // End Root
    Writer->Close();

    return Output;
}
