// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Misc/AutomationTest.h"
#include "Components/SovereignBioComponent.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN BIO COMPONENT TESTS - B-035 & B-038 & B-039 Verification
 * ============================================================================
 * Focus: Verifying the Draconic Gestation, Nesting, Egg Fertility parameters,
 * and Live Gestation Progression (C++ Tick Logic) under default initialization,
 * metabolism updates, and serialization using the new Hybrid Core Enum.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignBioComponentDefaultsTest,
    "Sovereign.Bio.ComponentDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBioComponentDefaultsTest::RunTest(const FString& Parameters)
{
    // Create an instance of USovereignBioComponent
    USovereignBioComponent* BioComp = NewObject<USovereignBioComponent>();
    if (!BioComp)
    {
        AddError(TEXT("Failed to create USovereignBioComponent"));
        return false;
    }

    // Verify default draconic/reproduction parameters
    TestFalse(TEXT("Default bGestationActive is false"), BioComp->bGestationActive);
    TestEqual(TEXT("Default GestationProgress is 0.0"), BioComp->GestationProgress, 0.0f);
    TestEqual(TEXT("Default GestationRate is 1.0"), BioComp->GestationRate, 1.0f);
    TestFalse(TEXT("Default bIsNestCreated is false"), BioComp->bIsNestCreated);
    TestEqual(TEXT("Default NestSpatiotemporalVolume is 0.0"), BioComp->NestSpatiotemporalVolume, 0.0f);
    TestEqual(TEXT("Default EggFertilityState is None"), BioComp->EggFertilityState, FString(TEXT("None")));

    // B-039: Default EggState is ESovereignEggState::None
    TestTrue(TEXT("Default EggState is ESovereignEggState::None"), BioComp->EggState == ESovereignEggState::None);

    // Verify double-ticking prevention: frame-based ticking is disabled by default
    TestFalse(TEXT("Frame-based ticking should be disabled by default"), BioComp->PrimaryComponentTick.bCanEverTick);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignBioComponentSerializationTest,
    "Sovereign.Bio.ComponentSerialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBioComponentSerializationTest::RunTest(const FString& Parameters)
{
    USovereignBioComponent* BioComp = NewObject<USovereignBioComponent>();
    if (!BioComp)
    {
        AddError(TEXT("Failed to create USovereignBioComponent"));
        return false;
    }

    // Mutate parameters to non-default values
    BioComp->bGestationActive = true;
    BioComp->GestationProgress = 42.5f;
    BioComp->GestationRate = 2.5f;
    BioComp->EggState = ESovereignEggState::Gestation;
    BioComp->bIsNestCreated = true;
    BioComp->NestSpatiotemporalVolume = 1500.75f;
    BioComp->EggFertilityState = TEXT("Fertilized_Prismatic");

    // Serialize to JSON
    TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
    BioComp->OnSave(RootObj);

    // Verify JSON structure and fields
    TestTrue(TEXT("Root JSON has Bio object"), RootObj->HasField(TEXT("Bio")));
    TSharedPtr<FJsonObject> BioObj = RootObj->GetObjectField(TEXT("Bio"));
    TestNotNull(TEXT("Bio JSON object is valid"), BioObj.Get());

    TestEqual(TEXT("bGestationActive serialized correctly"), BioObj->GetBoolField(TEXT("bGestationActive")), true);
    TestEqual(TEXT("GestationProgress serialized correctly"), static_cast<float>(BioObj->GetNumberField(TEXT("GestationProgress"))), 42.5f);
    TestEqual(TEXT("GestationRate serialized correctly"), static_cast<float>(BioObj->GetNumberField(TEXT("GestationRate"))), 2.5f);
    TestEqual(TEXT("EggState serialized correctly"), static_cast<ESovereignEggState>(static_cast<uint8>(BioObj->GetNumberField(TEXT("EggState")))), ESovereignEggState::Gestation);
    TestEqual(TEXT("bIsNestCreated serialized correctly"), BioObj->GetBoolField(TEXT("bIsNestCreated")), true);
    TestEqual(TEXT("NestSpatiotemporalVolume serialized correctly"), static_cast<float>(BioObj->GetNumberField(TEXT("NestSpatiotemporalVolume"))), 1500.75f);
    TestEqual(TEXT("EggFertilityState serialized correctly"), BioObj->GetStringField(TEXT("EggFertilityState")), TEXT("Fertilized_Prismatic"));

    // Reset/modify the component fields before loading
    BioComp->bGestationActive = false;
    BioComp->GestationProgress = 0.0f;
    BioComp->GestationRate = 1.0f;
    BioComp->EggState = ESovereignEggState::None;
    BioComp->bIsNestCreated = false;
    BioComp->NestSpatiotemporalVolume = 0.0f;
    BioComp->EggFertilityState = TEXT("None");

    // Deserialize from JSON
    BioComp->OnLoad(RootObj);

    // Verify fields were restored correctly
    TestTrue(TEXT("bGestationActive restored correctly"), BioComp->bGestationActive);
    TestEqual(TEXT("GestationProgress restored correctly"), BioComp->GestationProgress, 42.5f);
    TestEqual(TEXT("GestationRate restored correctly"), BioComp->GestationRate, 2.5f);
    TestTrue(TEXT("EggState restored correctly"), BioComp->EggState == ESovereignEggState::Gestation);
    TestTrue(TEXT("bIsNestCreated restored correctly"), BioComp->bIsNestCreated);
    TestEqual(TEXT("NestSpatiotemporalVolume restored correctly"), BioComp->NestSpatiotemporalVolume, 1500.75f);
    TestEqual(TEXT("EggFertilityState restored correctly"), BioComp->EggFertilityState, FString(TEXT("Fertilized_Prismatic")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignBioComponentProgressionTest,
    "Sovereign.Bio.ComponentProgression",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBioComponentProgressionTest::RunTest(const FString& Parameters)
{
    USovereignBioComponent* BioComp = NewObject<USovereignBioComponent>();
    if (!BioComp)
    {
        AddError(TEXT("Failed to create USovereignBioComponent"));
        return false;
    }

    // Initialize inactive gestation
    BioComp->bGestationActive = false;
    BioComp->GestationProgress = 20.0f;
    BioComp->GestationRate = 1.5f;
    BioComp->EggState = ESovereignEggState::None;
    BioComp->EggFertilityState = TEXT("None");

    // Metabolism update with inactive gestation should not advance GestationProgress
    BioComp->UpdateMetabolism(10.0f);
    TestEqual(TEXT("Gestation progress remains unchanged when gestation is inactive"), BioComp->GestationProgress, 20.0f);
    TestTrue(TEXT("EggState remains None when gestation is inactive"), BioComp->EggState == ESovereignEggState::None);

    // Activate gestation
    BioComp->bGestationActive = true;

    // Advance metabolism: Progress = 20.0 + (1.5 * 10.0) = 35.0
    // Should auto-transition EggState from None -> Gestation
    BioComp->UpdateMetabolism(10.0f);
    TestEqual(TEXT("Gestation progress increments based on rate and delta time"), BioComp->GestationProgress, 35.0f);
    TestTrue(TEXT("EggState auto-transitions to Gestation"), BioComp->EggState == ESovereignEggState::Gestation);
    TestEqual(TEXT("EggFertilityState remains None before threshold is met"), BioComp->EggFertilityState, FString(TEXT("None")));

    // Advance past the 100.0 threshold: Progress = 35.0 + (1.5 * 50.0) = 110.0
    BioComp->UpdateMetabolism(50.0f);
    TestEqual(TEXT("Gestation progress ticks past 100.0"), BioComp->GestationProgress, 110.0f);
    TestTrue(TEXT("EggState transitions to ReadyToLay upon threshold completion"), BioComp->EggState == ESovereignEggState::ReadyToLay);
    TestEqual(TEXT("EggFertilityState transitions to ReadyToLay upon threshold completion"), BioComp->EggFertilityState, FString(TEXT("ReadyToLay")));

    // Ensure it continues to grow beyond 100.0 without capping and gestation remains active (to gain more power)
    BioComp->UpdateMetabolism(10.0f); // Progress = 110.0 + (1.5 * 10.0) = 125.0
    TestEqual(TEXT("Gestation progress continues to grow over-gestation power"), BioComp->GestationProgress, 125.0f);
    TestTrue(TEXT("Gestation remains active for womb nourishment power accumulation"), BioComp->bGestationActive);
    TestTrue(TEXT("EggState remains ReadyToLay"), BioComp->EggState == ESovereignEggState::ReadyToLay);

    return true;
}
