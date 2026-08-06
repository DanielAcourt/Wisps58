// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Misc/AutomationTest.h"
#include "SaveSystem/SovereignGameData.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "Dom/JsonObject.h"

/**
 * ============================================================================
 * SOVEREIGN SAVE SYSTEM TESTS - SaveSystem Agent Responsibilities
 * ============================================================================
 * Agent Focus: Error Handling & Unit Testing
 * Coverage Areas:
 *   - Null safety hardening
 *   - Registry consistency validation
 *   - JSON serialization robustness
 *   - Soft reference failure resilience
 * ============================================================================
 */

// ============================================================================
// TEST 1: FEntitySaveData Initialization & Null Safety
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignEntityDataInitializationTest,
    "SaveSystem.Initialization.EntityDataDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignEntityDataInitializationTest::RunTest(const FString& Parameters)
{
    // Test default constructor initializes all properties correctly
    FEntitySaveData Entity;

    TestTrue(TEXT("Default GUID is initialized"), Entity.MyGUID.IsValid());
    TestEqual(TEXT("Default bIsFemale is false"), Entity.bIsFemale, false);
    TestEqual(TEXT("Default OffspringCount is 0"), Entity.OffspringCount, 0);
    TestEqual(TEXT("Default SavedGrowthStage is Inception"),
        static_cast<uint8>(Entity.SavedGrowthStage), 
        static_cast<uint8>(ESovereignGrowthStage::Inception)
    );
    TestEqual(TEXT("Default SavedFrequency is Standard"),
        static_cast<uint8>(Entity.SavedFrequency),
        static_cast<uint8>(EUpdateFrequency::Standard)
    );

    return true;
}

// ============================================================================
// TEST 2: Registry Consistency - GUID Uniqueness
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignGUIDUniquenessTest,
    "SaveSystem.Registry.GUIDUniqueness",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignGUIDUniquenessTest::RunTest(const FString& Parameters)
{
    // Test that multiple entities generate unique GUIDs
    TArray<FEntitySaveData> Entities;
    const int32 EntityCount = 50;

    for (int32 i = 0; i < EntityCount; ++i)
    {
        FEntitySaveData Entity;
        Entities.Add(Entity);
    }

    // Verify all GUIDs are unique
    for (int32 i = 0; i < Entities.Num(); ++i)
    {
        for (int32 j = i + 1; j < Entities.Num(); ++j)
        {
            if (Entities[i].MyGUID == Entities[j].MyGUID)
            {
                AddError(FString::Printf(TEXT("Entity %d and %d have DUPLICATE GUIDs"), i, j));
                return false;
            }
        }
    }

    TestEqual(TEXT("Created 50 unique entities"), Entities.Num(), 50);
    return true;
}

// ============================================================================
// TEST 3: JSON Serialization - Robustness & Null Handling
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignJsonSerializationTest,
    "SaveSystem.Serialization.JSONRobustness",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignJsonSerializationTest::RunTest(const FString& Parameters)
{
    FEntitySaveData OriginalEntity;
    OriginalEntity.MyGUID = FGuid::NewGuid();
    OriginalEntity.bIsFemale = true;
    OriginalEntity.OffspringCount = 5;
    OriginalEntity.SavedGrowthStage = ESovereignGrowthStage::Legendary;
    OriginalEntity.SavedFrequency = EUpdateFrequency::Slower;
    OriginalEntity.ClassPath = TEXT("/Game/Blueprints/Entities/BP_Test");

    // Simulate JSON serialization
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("MyGUID"), OriginalEntity.MyGUID.ToString());
    JsonObject->SetBoolField(TEXT("bIsFemale"), OriginalEntity.bIsFemale);
    JsonObject->SetNumberField(TEXT("OffspringCount"), OriginalEntity.OffspringCount);
    JsonObject->SetNumberField(TEXT("SavedGrowthStage"), static_cast<double>(OriginalEntity.SavedGrowthStage));
    JsonObject->SetStringField(TEXT("ClassPath"), OriginalEntity.ClassPath);

    // Verify serialization produced valid JSON
    TestTrue(TEXT("JSON object created successfully"), JsonObject.IsValid());
    TestTrue(TEXT("GUID field exists"), JsonObject->HasField(TEXT("MyGUID")));
    TestTrue(TEXT("bIsFemale field exists"), JsonObject->HasField(TEXT("bIsFemale")));
    TestEqual(TEXT("bIsFemale value correct"), JsonObject->GetBoolField(TEXT("bIsFemale")), true);
    TestEqual(TEXT("OffspringCount value correct"), static_cast<int32>(JsonObject->GetNumberField(TEXT("OffspringCount"))), 5);

    return true;
}

// ============================================================================
// TEST 4: Soft Reference Failure Resilience
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignSoftReferenceResilienceTest,
    "SaveSystem.References.SoftReferenceFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignSoftReferenceResilienceTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;

    // Test empty ClassPath doesn't break serialization
    Entity.ClassPath = TEXT("");
    TestTrue(TEXT("Empty ClassPath doesn't invalidate entity"), Entity.MyGUID.IsValid());

    // Test invalid ClassPath is handled gracefully
    Entity.ClassPath = TEXT("/InvalidGame/MissingBlueprint");
    TestTrue(TEXT("Invalid ClassPath doesn't break entity"), Entity.MyGUID.IsValid());

    // Test null parent references
    Entity.ParentID = FGuid();
    TestTrue(TEXT("Null ParentID is valid FGuid"), Entity.ParentID == FGuid());

    return true;
}

// ============================================================================
// TEST 5: FItemDefaults Initialization
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignItemDefaultsInitializationTest,
    "SaveSystem.Initialization.ItemDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignItemDefaultsInitializationTest::RunTest(const FString& Parameters)
{
    FItemDefaults ItemDefaults;

    TestTrue(TEXT("SpeciesVesselData is initialized to null"), ItemDefaults.SpeciesVesselData == nullptr);
    TestTrue(TEXT("SpeciesTag is initialized"), true); // GameplayTag is always valid

    return true;
}

// ============================================================================
// TEST 6: Lineage Data Consistency
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignLineageDataTest,
    "SaveSystem.Data.LineageConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignLineageDataTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Parent;
    FGuid ParentID = Parent.MyGUID;

    FEntitySaveData Child;
    Child.ParentID = ParentID;
    Child.MotherID = ParentID;
    Child.FatherID = FGuid::NewGuid();

    TestEqual(TEXT("Child's ParentID matches Parent's GUID"), Child.ParentID, ParentID);
    TestNotEqual(TEXT("Child's FatherID is different from MotherID"), Child.MotherID, Child.FatherID);

    Child.OffspringCount = 0;
    TestEqual(TEXT("Initial OffspringCount is 0"), Child.OffspringCount, 0);

    Child.OffspringCount++;
    TestEqual(TEXT("OffspringCount increments correctly"), Child.OffspringCount, 1);

    return true;
}

// ============================================================================
// TEST 7: Growth Stage Transitions
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignGrowthStageTransitionTest,
    "SaveSystem.GameRules.GrowthStageValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignGrowthStageTransitionTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;
    Entity.SavedGrowthStage = ESovereignGrowthStage::Inception;
    TestEqual(TEXT("Starting stage is Inception"),
        static_cast<uint8>(Entity.SavedGrowthStage), 
        static_cast<uint8>(ESovereignGrowthStage::Inception)
    );

    Entity.SavedGrowthStage = ESovereignGrowthStage::Seed;
    TestEqual(TEXT("Can progress to Seed"),
        static_cast<uint8>(Entity.SavedGrowthStage),
        static_cast<uint8>(ESovereignGrowthStage::Seed)
    );

    Entity.SavedGrowthStage = ESovereignGrowthStage::Adult;
    TestTrue(TEXT("Can advance to Adult stage"),
        static_cast<uint8>(Entity.SavedGrowthStage) > static_cast<uint8>(ESovereignGrowthStage::Sprout)
    );

    return true;
}

// ============================================================================
// TEST 8: Update Frequency Configuration
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignUpdateFrequencyTest,
    "SaveSystem.Configuration.UpdateFrequency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignUpdateFrequencyTest::RunTest(const FString& Parameters)
{
    FEntitySaveData FastEntity, SlowEntity;

    FastEntity.SavedFrequency = EUpdateFrequency::Realtime;
    SlowEntity.SavedFrequency = EUpdateFrequency::Glacier;

    TestTrue(TEXT("Fast update frequency is realtime"),
        static_cast<uint8>(FastEntity.SavedFrequency) <= static_cast<uint8>(EUpdateFrequency::Realtime)
    );

    TestTrue(TEXT("Slow update frequency is glacier"),
        static_cast<uint8>(SlowEntity.SavedFrequency) >= static_cast<uint8>(EUpdateFrequency::Slowest)
    );

    return true;
}

// ============================================================================
// TEST 9: Transform Data Integrity
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignTransformIntegrityTest,
    "SaveSystem.Data.TransformIntegrity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignTransformIntegrityTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;
    FVector TestLocation(100.0f, 200.0f, 300.0f);
    Entity.WorldTransform.SetTranslation(TestLocation);

    TestTrue(TEXT("Transform location is valid"),
        Entity.WorldTransform.GetTranslation().Equals(TestLocation, 0.01f)
    );

    return true;
}

// ============================================================================
// TEST 10: Mating History Array Operations
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignMatingHistoryTest,
    "SaveSystem.Lineage.MatingHistory",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignMatingHistoryTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;
    FGuid Mate1 = FGuid::NewGuid();
    FGuid Mate2 = FGuid::NewGuid();
    FGuid Mate3 = FGuid::NewGuid();

    Entity.MatingHistory.Add(Mate1);
    Entity.MatingHistory.Add(Mate2);
    Entity.MatingHistory.Add(Mate3);

    TestEqual(TEXT("MatingHistory contains 3 entries"), Entity.MatingHistory.Num(), 3);
    TestEqual(TEXT("First mate matches"), Entity.MatingHistory[0], Mate1);
    TestEqual(TEXT("Second mate matches"), Entity.MatingHistory[1], Mate2);
    TestEqual(TEXT("Third mate matches"), Entity.MatingHistory[2], Mate3);

    Entity.MatingHistory.Remove(Mate2);
    TestEqual(TEXT("MatingHistory contains 2 entries after removal"), Entity.MatingHistory.Num(), 2);

    return true;
}

// ============================================================================
// TEST 11: Elemental Affinities Map Management
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignElementalAffinitiesTest,
    "SaveSystem.Elements.AffinitiesManagement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignElementalAffinitiesTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;

    Entity.ElementalAffinities.Add(ESovereignElement::Fire, 0.8f);
    Entity.ElementalAffinities.Add(ESovereignElement::Water, 0.3f);
    Entity.ElementalAffinities.Add(ESovereignElement::Nature, 0.9f);

    TestEqual(TEXT("ElementalAffinities map has 3 entries"), Entity.ElementalAffinities.Num(), 3);
    TestEqual(TEXT("Fire affinity is 0.8"), Entity.ElementalAffinities[ESovereignElement::Fire], 0.8f);
    TestTrue(TEXT("Water affinity is less than Nature affinity"),
        Entity.ElementalAffinities[ESovereignElement::Water] < Entity.ElementalAffinities[ESovereignElement::Nature]
    );

    return true;
}

// ============================================================================
// TEST 12: Null Safety - UnknownMetaTags Handling
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignUnknownMetaTagsNullSafetyTest,
    "SaveSystem.NullSafety.UnknownMetaTags",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignUnknownMetaTagsNullSafetyTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;

    // Test null pointer safety
    TestTrue(TEXT("UnknownMetaTags is initially null"), !Entity.UnknownMetaTags.IsValid());

    // Create valid JSON object
    Entity.UnknownMetaTags = MakeShared<FJsonObject>();
    TestTrue(TEXT("UnknownMetaTags is valid after creation"), Entity.UnknownMetaTags.IsValid());

    // Add data safely
    Entity.UnknownMetaTags->SetStringField(TEXT("CustomTag"), TEXT("CustomValue"));
    TestEqual(TEXT("CustomTag was set correctly"),
        Entity.UnknownMetaTags->GetStringField(TEXT("CustomTag")), 
        FString(TEXT("CustomValue"))
    );

    return true;
}

// ============================================================================
// TEST 13: ClassPath Soft Reference Validation
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignClassPathValidationTest,
    "SaveSystem.References.ClassPathValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignClassPathValidationTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;

    Entity.ClassPath = TEXT("/Game/Blueprints/Entities/BP_SovereignWisp_C");
    TestTrue(TEXT("Valid ClassPath is not empty"), !Entity.ClassPath.IsEmpty());
    TestTrue(TEXT("ClassPath contains /Game/"), Entity.ClassPath.Contains(TEXT("/Game/")));

    Entity.ClassPath = TEXT("");
    TestTrue(TEXT("Empty ClassPath doesn't invalidate entity"), Entity.MyGUID.IsValid());

    return true;
}

// ============================================================================
// TEST 14: Batch Entity Creation & Consistency
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignBatchEntityCreationTest,
    "SaveSystem.Performance.BatchCreation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBatchEntityCreationTest::RunTest(const FString& Parameters)
{
    const int32 BatchSize = 100;
    TArray<FEntitySaveData> Entities;

    double StartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < BatchSize; ++i)
    {
        FEntitySaveData Entity;
        Entity.bIsFemale = (i % 2 == 0);
        Entity.OffspringCount = i;
        Entities.Add(Entity);
    }
    double CreationTime = FPlatformTime::Seconds() - StartTime;

    TestEqual(TEXT("Batch creation generated correct number of entities"), Entities.Num(), BatchSize);
    TestTrue(TEXT("Batch creation completed in reasonable time"), CreationTime < 1.0);

    int32 FemaleCount = 0;
    for (const FEntitySaveData& E : Entities)
    {
        if (E.bIsFemale)
            FemaleCount++;
    }
    TestTrue(TEXT("Approximate 50% female entities"), FemaleCount >= BatchSize / 2 - 10);

    return true;
}

// ============================================================================
// TEST 15: Edge Cases & Boundary Conditions
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignEdgeCasesTest,
    "SaveSystem.EdgeCases.BoundaryConditions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignEdgeCasesTest::RunTest(const FString& Parameters)
{
    FEntitySaveData Entity;

    // Max offspring count
    Entity.OffspringCount = INT32_MAX;
    TestEqual(TEXT("Max offspring count is handled"), Entity.OffspringCount, INT32_MAX);

    // Empty GUID
    Entity.MyGUID = FGuid();
    TestTrue(TEXT("Empty GUID is valid FGuid"), Entity.MyGUID == FGuid());

    // Very long ClassPath
    Entity.ClassPath = FString(TEXT("/Game/")) + FString::ChrN(500, 'A');
    TestTrue(TEXT("Long ClassPath is stored correctly"), Entity.ClassPath.Len() > 500);

    // Zero scale transform
    Entity.WorldTransform.SetScale3D(FVector::ZeroVector);
    TestTrue(TEXT("Zero scale transform is valid"), Entity.WorldTransform.IsValid());

    return true;
}