/**
 * @file SaveSystem_Hardened_Fixes.cpp (REFERENCE ONLY - DO NOT COMPILE)
 *
 * This file serves as a guide for the Sovereign Save System's "Hardened"
 * implementation. It demonstrates the critical code changes required to
 * ensure data integrity and prevent engine crashes during Save/Load operations.
 */

#include "SovereignGameData.h"
#include "SovereignSaveManager.h"
#include "SovereignJsonUtils.h"

// -----------------------------------------------------------------------------
// 🟢 SECTION 1: Hardening the Save Data Struct
// -----------------------------------------------------------------------------
// Use explicit constructors to initialize non-UObject members.
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FEntitySaveData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClassPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFemale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OffspringCount;

    // ... other properties ...

    // CRITICAL: Explicit constructor to prevent garbage values
    FEntitySaveData()
        : bIsFemale(false)
        , OffspringCount(0)
    {
        // ... initialize all other non-UObject properties ...
    }
};

// -----------------------------------------------------------------------------
// 🟢 SECTION 2: Hardening the Save Manager (Spawning)
// -----------------------------------------------------------------------------
// Defensive checks for ClassPath strings before spawning.
// -----------------------------------------------------------------------------
void USaveManager::LoadWorldState(const USovereignSaveGame* Suitcase) {
    if (!Suitcase) return;

    for (const auto& Data : Suitcase->SavedEntities) {
        // DEFENSIVE CHECK: Prevent crash from empty or malformed class paths
        if (Data.ClassPath.IsEmpty()) {
            UE_LOG(LogTemp, Warning, TEXT("USaveManager: Skipping spawn for empty ClassPath."));
            continue;
        }

        UClass* EntityClass = LoadObject<UClass>(nullptr, *Data.ClassPath);
        if (!EntityClass) {
            UE_LOG(LogTemp, Error, TEXT("USaveManager: Failed to load class at path: %s"), *Data.ClassPath);
            continue;
        }

        // ... proceed with spawning logic ...
    }
}

// -----------------------------------------------------------------------------
// 🟢 SECTION 3: Hardening the JSON Serialization
// -----------------------------------------------------------------------------
// Explicit error logging for deserialization failures.
// -----------------------------------------------------------------------------
USovereignSaveGame* USovereignJsonUtils::ConvertJsonToSuitcase(const FString& JsonString) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    TSharedPtr<FJsonObject> JsonObject;

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
        // LOG ERROR: Provide clear feedback instead of silent failure
        UE_LOG(LogTemp, Error, TEXT("USovereignJsonUtils: CRITICAL - JSON Deserialization failed. Save file may be corrupt."));
        return nullptr;
    }

    // ... mapping JSON properties to Suitcase (USovereignSaveGame) ...
}
