// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/SovereignSaveTerminal.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "Kismet/GameplayStatics.h"

void ASovereignSaveTerminal::OnInteract_Implementation(AActor* Interactor)
{
    Editor_TriggerSave();

    UE_LOG(LogTemp, Warning, TEXT("Sovereign: Chronicle Stone activated by %s"),
        Interactor ? *Interactor->GetName() : TEXT("Unknown"));
}

void ASovereignSaveTerminal::Editor_TriggerSave()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (USaveManager* SaveManager = World->GetGameInstance()->GetSubsystem<USaveManager>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sovereign: Chronicle Stone committing to %s (JSON: %s)"),
            *TargetSlotName, bOverrideToJson ? TEXT("True") : TEXT("False"));

        SaveManager->SaveWorldState(TargetSlotName, bOverrideToJson);
        LastOperationEntityCount = SaveManager->GetSavedEntityCount(TargetSlotName, bOverrideToJson);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Sovereign: SaveManager Subsystem missing!"));
    }
}

FText ASovereignSaveTerminal::GetInteractableName_Implementation()
{
    return FText::FromString("Ancient Chronicle Stone");
}

void ASovereignSaveTerminal::Editor_TriggerLoad()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (USaveManager* SaveManager = World->GetGameInstance()->GetSubsystem<USaveManager>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sovereign: Chronicle Stone Inhaling state from [%s]"), *TargetSlotName);

        SaveManager->LoadWorldState(TargetSlotName, bOverrideToJson);
        LastOperationEntityCount = SaveManager->GetSavedEntityCount(TargetSlotName, bOverrideToJson);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Sovereign Error: SaveManager not found for Loading."));
    }
}

void ASovereignSaveTerminal::OnSecondaryInteract_Implementation(AActor* Interactor)
{
    Editor_TriggerLoad();
}

void ASovereignSaveTerminal::Editor_RefreshSavedCount()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (USaveManager* SaveManager = World->GetGameInstance()->GetSubsystem<USaveManager>())
    {
        LastOperationEntityCount = SaveManager->GetSavedEntityCount(TargetSlotName, bOverrideToJson);

        UE_LOG(LogTemp, Log, TEXT("Sovereign: File check complete. Found %d entities in %s"),
            LastOperationEntityCount, *TargetSlotName);
    }
}