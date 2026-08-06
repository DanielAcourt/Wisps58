// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#include "Components/SovereignBioComponent.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Dom/JsonObject.h"

USovereignBioComponent::USovereignBioComponent()
{
    // B-038: Disable general frame-based ticking by default to rely solely on the entity heartbeat
    PrimaryComponentTick.bCanEverTick = false;

    // Default Vitals
    Hunger = 50.0f;
    Hydration = 50.0f;
    Fatigue = 0.0f;
    Tiredness = 0.0f;
    Toxicity = 0.0f;
    WasteLevel = 0.0f;
    Entropy = 0.0f;
    Mass = 1;
    MassExperience = 1.0;

    // B-038: Default gestation rate
    GestationRate = 1.0f;

    // B-039: Initialize Hybrid Enum state
    EggState = ESovereignEggState::None;
}

void USovereignBioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Auto-register with the Soul Hub
    if (AActor* Owner = GetOwner())
    {
        if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
        {
            SoulHub->RegisterBroker(this);
        }
    }
}

void USovereignBioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AActor* Owner = GetOwner())
    {
        if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
        {
            SoulHub->UnregisterBroker(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void USovereignBioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateMetabolism(DeltaTime);
}

void USovereignBioComponent::UpdateMetabolism(float DeltaTime)
{
    // Total cost of existence
    float NetDrain = 0.1f * DeltaTime;

    if (Hunger > 0.0f)
    {
        Hunger -= NetDrain;
        Hydration -= (NetDrain * 1.2f);
    }

    WasteLevel += (NetDrain * 0.5f);

    // B-038 & B-039: Live Gestation Progression with Hybrid Core Enum
    if (bGestationActive)
    {
        // Auto-transition to Gestation state if currently None
        if (EggState == ESovereignEggState::None)
        {
            EggState = ESovereignEggState::Gestation;
        }

        GestationProgress += GestationRate * DeltaTime;

        // Transition to ReadyToLay once threshold is met (if not already laid or customized)
        if (GestationProgress >= 100.0f && EggState == ESovereignEggState::Gestation)
        {
            EggState = ESovereignEggState::ReadyToLay;
            if (EggFertilityState == TEXT("None"))
            {
                EggFertilityState = TEXT("ReadyToLay");
            }
        }
    }
}

void USovereignBioComponent::HandleBiologicalTransition(float DeltaTime)
{
}

/** --- 07 BROKER IMPLEMENTATION --- */

void USovereignBioComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
    TSharedPtr<FJsonObject> BioObj = MakeShareable(new FJsonObject());

    // 1. Vitals
    BioObj->SetNumberField(TEXT("Hunger"), Hunger);
    BioObj->SetNumberField(TEXT("Hydration"), Hydration);
    BioObj->SetNumberField(TEXT("Fatigue"), Fatigue);
    BioObj->SetNumberField(TEXT("Tiredness"), Tiredness);
    BioObj->SetNumberField(TEXT("Toxicity"), Toxicity);
    BioObj->SetNumberField(TEXT("WasteLevel"), WasteLevel);

    // 2. Growth
    BioObj->SetNumberField(TEXT("Mass"), Mass);
    BioObj->SetNumberField(TEXT("MassExperience"), MassExperience);
    BioObj->SetNumberField(TEXT("MaturityProgress"), MaturityProgress);
    BioObj->SetNumberField(TEXT("Entropy"), Entropy);

    // 3. Lineage
    BioObj->SetBoolField(TEXT("bIsFemale"), bIsFemale);
    BioObj->SetStringField(TEXT("ParentID"), ParentID.ToString());
    BioObj->SetStringField(TEXT("MotherID"), MotherID.ToString());
    BioObj->SetStringField(TEXT("FatherID"), FatherID.ToString());
    BioObj->SetNumberField(TEXT("OffspringCount"), OffspringCount);

    // Draconic Gestation & Nesting [B-035]
    BioObj->SetBoolField(TEXT("bGestationActive"), bGestationActive);
    BioObj->SetNumberField(TEXT("GestationProgress"), GestationProgress);
    BioObj->SetNumberField(TEXT("GestationRate"), GestationRate); // B-038
    BioObj->SetNumberField(TEXT("EggState"), static_cast<double>(EggState)); // B-039
    BioObj->SetBoolField(TEXT("bIsNestCreated"), bIsNestCreated);
    BioObj->SetNumberField(TEXT("NestSpatiotemporalVolume"), NestSpatiotemporalVolume);
    BioObj->SetStringField(TEXT("EggFertilityState"), EggFertilityState);

    TArray<TSharedPtr<FJsonValue>> MatingArray;
    for (const FGuid& Id : MatingHistory)
    {
        MatingArray.Add(MakeShareable(new FJsonValueString(Id.ToString())));
    }
    BioObj->SetArrayField(TEXT("MatingHistory"), MatingArray);

    OutJson->SetObjectField(TEXT("Bio"), BioObj);
}

void USovereignBioComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
    const TSharedPtr<FJsonObject>* BioObj;
    if (InJson->TryGetObjectField(TEXT("Bio"), BioObj))
    {
        double TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Hunger"), TempVal)) Hunger = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Hydration"), TempVal)) Hydration = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Fatigue"), TempVal)) Fatigue = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Tiredness"), TempVal)) Tiredness = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Toxicity"), TempVal)) Toxicity = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("WasteLevel"), TempVal)) WasteLevel = (float)TempVal;

        if ((*BioObj)->TryGetNumberField(TEXT("Mass"), TempVal)) Mass = (int32)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("MassExperience"), MassExperience)) {} // MassExperience is double
        if ((*BioObj)->TryGetNumberField(TEXT("MaturityProgress"), TempVal)) MaturityProgress = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("Entropy"), TempVal)) Entropy = (float)TempVal;

        (*BioObj)->TryGetBoolField(TEXT("bIsFemale"), bIsFemale);

        FString IdStr;
        if ((*BioObj)->TryGetStringField(TEXT("ParentID"), IdStr)) FGuid::Parse(IdStr, ParentID);
        if ((*BioObj)->TryGetStringField(TEXT("MotherID"), IdStr)) FGuid::Parse(IdStr, MotherID);
        if ((*BioObj)->TryGetStringField(TEXT("FatherID"), IdStr)) FGuid::Parse(IdStr, FatherID);

        if ((*BioObj)->TryGetNumberField(TEXT("OffspringCount"), TempVal)) OffspringCount = (int32)TempVal;

        // Draconic Gestation & Nesting [B-035]
        (*BioObj)->TryGetBoolField(TEXT("bGestationActive"), bGestationActive);
        if ((*BioObj)->TryGetNumberField(TEXT("GestationProgress"), TempVal)) GestationProgress = (float)TempVal;
        if ((*BioObj)->TryGetNumberField(TEXT("GestationRate"), TempVal)) GestationRate = (float)TempVal; // B-038
        if ((*BioObj)->TryGetNumberField(TEXT("EggState"), TempVal)) EggState = static_cast<ESovereignEggState>((uint8)TempVal); // B-039
        (*BioObj)->TryGetBoolField(TEXT("bIsNestCreated"), bIsNestCreated);
        if ((*BioObj)->TryGetNumberField(TEXT("NestSpatiotemporalVolume"), TempVal)) NestSpatiotemporalVolume = (float)TempVal;
        (*BioObj)->TryGetStringField(TEXT("EggFertilityState"), EggFertilityState);

        const TArray<TSharedPtr<FJsonValue>>* MatingArray;
        if ((*BioObj)->TryGetArrayField(TEXT("MatingHistory"), MatingArray))
        {
            MatingHistory.Empty();
            for (auto& Val : *MatingArray)
            {
                FGuid Id;
                if (FGuid::Parse(Val->AsString(), Id)) MatingHistory.Add(Id);
            }
        }
    }
}

void USovereignBioComponent::OnProcessData(const TMap<FString, FString>& Data)
{
    // Handle external biological injections
    if (Data.Contains(TEXT("Bio.Heal")))
    {
        Toxicity = FMath::Clamp(Toxicity - FCString::Atof(*Data[TEXT("Bio.Heal")]), 0.0f, 100.0f);
    }
}
