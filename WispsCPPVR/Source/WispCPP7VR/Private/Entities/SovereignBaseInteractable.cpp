// Fill out your copyright notice in the Description page of Project Settings.
/*
The "Sovereign" Inheritance Chain
Think of it like a family tree where each generation adds a new capability:

AActor (Unreal Engine): Gives it a place in the world (Transform).

ASovereignBaseEntity (Your Identity Layer):
- Adds the GUID
- Adds the Saveable Component
- Registers with the Actor Registry

ASovereignBaseInteractable (Your Physical Layer):
- Adds a Static Mesh (so it exists physically)
- Implements the Interaction Interface (so it can be touched)
*/


#include "Entities/SovereignBaseInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Entities/SovereignBaseCharacter.h"

ASovereignBaseInteractable::ASovereignBaseInteractable()
{
    // 1. Performance
    PrimaryActorTick.bCanEverTick = false;

    // 2. Physical Layer
    PhysicalVessel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicalVessel"));
    RootComponent = PhysicalVessel;

    // 3. Collision Setup (Applied to Root)
    PhysicalVessel->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicalVessel->SetCollisionResponseToAllChannels(ECR_Block);
    PhysicalVessel->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // 4. Soul Layer
    // We create it here so it is always available
    SaveDataComponent = CreateDefaultSubobject<USovereignSaveableEntityComponent>(TEXT("SovereignSoul"));
}

bool ASovereignBaseInteractable::CanInteract_Implementation(AActor* Interactor)
{
    return bIsInteractable && IsValid(Interactor);
}
void ASovereignBaseInteractable::OnInteract_Implementation(AActor* Interactor)
{
    // 1. Core Safety Check
    if (!CanInteract_Implementation(Interactor))
    {
        return;
    }

    // 2. The Handshake: Pass 'this' (the interactable) back to the Wisp
    if (ASovereignBaseCharacter* BaseChar = Cast<ASovereignBaseCharacter>(Interactor))
    {
        // This triggers the 'OnActorSensed' Event in your Blueprint
        BaseChar->OnActorSensed.Broadcast(this);
    }

    // 3. Feedback: Cyan for success, logging the specific names
    FString DebugMessage = FString::Printf(
        TEXT("INTERACTED: %s is now focused on %s"),
        *Interactor->GetName(),
        *GetName()
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DebugMessage);
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMessage);
}


void ASovereignBaseInteractable::OnBeginHover_Implementation()
{
    // Optional: highlight, sound cue, UI prompt
}
void ASovereignBaseInteractable::OnEndHover_Implementation()
{
    // Optional: remove highlight, stop sound
}

//Interable Name
FText ASovereignBaseInteractable::GetInteractableName_Implementation()
{
    return FText::FromString(TEXT("Interactable Object"));
}
FString ASovereignBaseInteractable::GetInteractionHint_Implementation()
{
    return TEXT("Interact");
}

void ASovereignBaseInteractable::OnSecondaryInteract_Implementation(AActor* Interactor)
{
    // Optional secondary action (long press, grip, etc.)
}

//Possession Functions
bool ASovereignBaseInteractable::CanBePossessed_Implementation()
{
    // Note: bCanBePossessed is a protected variable inherited from ASovereignBaseEntity
    return bCanBePossessed;
}
//Changes 06/06/2026. We want to use the bCanBePossessed on the unpossession check so it handles object too
void ASovereignBaseInteractable::RequestPossession_Implementation(AController* RequestingController)
{
    APlayerController* PC = Cast<APlayerController>(RequestingController);
    if (!PC || !CanBePossessed_Implementation()) return;

    // 1. UPDATE THE SOUL STATE
    if (SaveDataComponent)
    {
        SaveDataComponent->bIsBeingPossessed = true;
    }

    // 2. TRANSFER INPUT
    if (APawn* WispPawn = PC->GetPawn())
    {
        WispPawn->DisableInput(PC);
    }
    this->EnableInput(PC);

    UE_LOG(LogTemp, Warning, TEXT("Spirit Link established: %s is now soul-bound."), *GetName());
}

USceneComponent* ASovereignBaseInteractable::GetPossessionAttachmentComponent_Implementation()
{
    return PhysicalVessel;
}

/* =========================
   ISovereignSaveInterface
   ========================= */

TMap<FString, FString> ASovereignBaseInteractable::GetSaveData()
{
    TMap<FString, FString> Data;
    // Map internal properties to the "Telemetry." namespace for the Digital Twin
    Data.Add(TEXT("Telemetry.temp_c"), FString::SanitizeFloat(TemperatureCelsius));
    Data.Add(TEXT("Telemetry.ph_val"), FString::SanitizeFloat(PhValue));
    Data.Add(TEXT("Telemetry.water_depth_mm"), FString::SanitizeFloat(WaterDepthMM));
    return Data;
}

void ASovereignBaseInteractable::RestoreSaveData(const TMap<FString, FString>& Data)
{
    // Scrape the "suitcase" for telemetry keys and update the physical vessel
    if (const FString* Val = Data.Find(TEXT("Telemetry.temp_c")))
    {
        TemperatureCelsius = FCString::Atof(**Val);
    }

    if (const FString* Val = Data.Find(TEXT("Telemetry.ph_val")))
    {
        PhValue = FCString::Atof(**Val);
    }

    if (const FString* Val = Data.Find(TEXT("Telemetry.water_depth_mm")))
    {
        WaterDepthMM = FCString::Atof(**Val);
    }
}
