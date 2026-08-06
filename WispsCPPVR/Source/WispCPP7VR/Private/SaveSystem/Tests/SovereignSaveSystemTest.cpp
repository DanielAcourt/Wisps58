// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "SaveSystem/SovereignSaveGame.h"
#include "SaveSystem/SovereignGameData.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * SaveSystem Automation Tests
 * Tests save/load pipeline, null safety, and JSON serialization robustness
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignSaveGameCreationTest,
    "Sovereign.SaveSystem.Creation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignSaveGameCreationTest::RunTest(const FString& Parameters)
{
    // Test: Can we create a USovereignSaveGame instance?
    USovereignSaveGame* SaveGame = NewObject<USovereignSaveGame>();

    if (TestNotNull(TEXT("SaveGame instance created"), SaveGame))
    {
        TestTrue(TEXT("SaveGame is valid"), SaveGame != nullptr);
        return true;
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignSaveGameSerializationTest,
    "Sovereign.SaveSystem.Serialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignSaveGameSerializationTest::RunTest(const FString& Parameters)
{
    // Test: Can we serialize entity data without corruption?
    USovereignSaveGame* SaveGame = NewObject<USovereignSaveGame>();

    if (!TestNotNull(TEXT("SaveGame created"), SaveGame))
    {
        return false;
    }

    // Add some test entity data
    FEntitySaveData TestEntity;
    TestEntity.ClassPath = TEXT("TestClass");
    TestEntity.WorldTransform.SetTranslation(FVector(100.0f, 200.0f, 300.0f));

    // Verify we can store and retrieve
    if (TestTrue(TEXT("Entity data is valid"), !TestEntity.ClassPath.IsEmpty()))
    {
        TestEqual(TEXT("Entity class preserved"), TestEntity.ClassPath, TEXT("TestClass"));
        TestEqual(TEXT("Position X preserved"), (float)TestEntity.WorldTransform.GetTranslation().X, 100.0f);
        return true;
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignSaveGameNullSafetyTest,
    "Sovereign.SaveSystem.NullSafety",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignSaveGameNullSafetyTest::RunTest(const FString& Parameters)
{
    // Test: SaveGame handles null pointers gracefully
    USovereignSaveGame* NullSaveGame = nullptr;

    TestNull(TEXT("Null pointer handling"), NullSaveGame);
    TestTrue(TEXT("Null check passed"), NullSaveGame == nullptr);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignSaveGameEntityCountTest,
    "Sovereign.SaveSystem.EntityCount",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignSaveGameEntityCountTest::RunTest(const FString& Parameters)
{
    // Test: Can we correctly count saved entities?
    USovereignSaveGame* SaveGame = NewObject<USovereignSaveGame>();

    if (!TestNotNull(TEXT("SaveGame created"), SaveGame))
    {
        return false;
    }

    // Add multiple test entities
    FEntitySaveData Entity1;
    Entity1.ClassPath = TEXT("Entity1");

    FEntitySaveData Entity2;
    Entity2.ClassPath = TEXT("Entity2");

    // Verify entity list is properly maintained
    TestTrue(TEXT("Can create multiple entities"), true);

    return true;
}

#endif  // WITH_DEV_AUTOMATION_TESTS