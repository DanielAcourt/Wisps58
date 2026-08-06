// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Misc/AutomationTest.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Entities/SovereignDiagnosticBroker.h"
#include "Entities/SovereignCultivationBroker.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

/**
 * ============================================================================
 * SOVEREIGN BROKER INTEGRATION TEST - E-001 & B-016 & B-017 Verification
 * ============================================================================
 * Focus: Verifying the dynamic broker instantiation, namespace isolation,
 * and dynamic VSS/Paradox density coupling.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignBrokerIntegrationTest,
    "Sovereign.Soul.BrokerIntegration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBrokerIntegrationTest::RunTest(const FString& Parameters)
{
    // Create a transient world or use the first editor/game context
    UWorld* World = nullptr;
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        World = GEngine->GetWorldContexts()[0].World();
    }

    if (!World)
    {
        // No world available in headless unit context, let's instantiate the objects directly with dynamic outer
        USovereignSaveableEntityComponent* Soul = NewObject<USovereignSaveableEntityComponent>();
        if (!Soul)
        {
            AddError(TEXT("Failed to create USovereignSaveableEntityComponent for testing"));
            return false;
        }

        // Trigger dynamic broker initialization (Normally called on BeginPlay)
        Soul->DiagnosticBroker = NewObject<UDiagnosticBroker>(Soul);
        Soul->RegisterBroker(Soul->DiagnosticBroker);

        Soul->CultivationBroker = NewObject<UCultivationBroker>(Soul);
        Soul->RegisterBroker(Soul->CultivationBroker);

        // 1. Verify Dynamic Broker Instantiation & Registry
        TestNotNull(TEXT("DiagnosticBroker is instantiated"), Soul->DiagnosticBroker);
        TestNotNull(TEXT("CultivationBroker is instantiated"), Soul->CultivationBroker);

        // 2. Verify Namespace Isolation (Truth)
        Soul->DiagnosticBroker->SetTruthValue(TEXT("temp_c"), TEXT("25.0"));
        Soul->DiagnosticBroker->VettedBy = TEXT("Lead_Curator");

        // 3. Verify Namespace Isolation (Magic)
        Soul->CultivationBroker->QiBalance = 100.0f;
        Soul->CultivationBroker->CultivationTier = 3;

        // Capture State
        TSharedPtr<FJsonObject> State = Soul->CaptureFullEntityState();
        TestTrue(TEXT("Captured state contains Sovereign.Truth object"), State->HasField(TEXT("Sovereign.Truth")));
        TestTrue(TEXT("Captured state contains Sovereign.Magic object"), State->HasField(TEXT("Sovereign.Magic")));

        TSharedPtr<FJsonObject> TruthObj = State->GetObjectField(TEXT("Sovereign.Truth"));
        TestEqual(TEXT("Truth value set correctly"), TruthObj->GetStringField(TEXT("temp_c")), TEXT("25.0"));
        TestEqual(TEXT("Curator sign-off set correctly"), TruthObj->GetStringField(TEXT("VettedBy")), TEXT("Lead_Curator"));

        TSharedPtr<FJsonObject> MagicObj = State->GetObjectField(TEXT("Sovereign.Magic"));
        //TestEqual(TEXT("Qi balance serialized correctly"), MagicObj->GetNumberField(TEXT("QiBalance")), 100.0f);
        TestEqual(TEXT("Cultivation tier serialized correctly"), MagicObj->GetIntegerField(TEXT("CultivationTier")), 3);

        // 4. Verify VSS & Paradox Coupling
        Soul->ParadoxDensity = 0.5f; // 50% Paradox/uncertainty density

        // Unvetted System Confidence: 1.0 - 0.5 = 0.5
        Soul->DiagnosticBroker->VettedBy = TEXT(""); // Clear curation
        float UnvettedConfidence = Soul->Execute_GetSystemConfidence(Soul);
        TestEqual(TEXT("Unvetted confidence matches baseline expectation"), UnvettedConfidence, 0.5f);

        // Vetted System Confidence: 1.0 - (0.5 * 0.2) = 0.9 (80% mitigation)
        Soul->DiagnosticBroker->VettedBy = TEXT("Spirit");
        float VettedConfidence = Soul->Execute_GetSystemConfidence(Soul);
        TestEqual(TEXT("Vetted confidence applies 80% paradox mitigation"), VettedConfidence, 0.9f);

        // Round-trip verification
        TSharedPtr<FJsonObject> LoadState = MakeShared<FJsonObject>();
        TSharedPtr<FJsonObject> LoadTruth = MakeShared<FJsonObject>();
        LoadTruth->SetStringField(TEXT("LidarScanID"), TEXT("Scan_982"));
        LoadTruth->SetStringField(TEXT("VettedBy"), TEXT("Archaeologist_Theta"));
        LoadState->SetObjectField(TEXT("Sovereign.Truth"), LoadTruth);

        Soul->ApplyStateFromJsonObject(LoadState);
        TestEqual(TEXT("Load restored vetted field"), Soul->DiagnosticBroker->VettedBy, TEXT("Archaeologist_Theta"));
        TestEqual(TEXT("Load restored raw telemetry"), Soul->DiagnosticBroker->GetTruthValue(TEXT("LidarScanID")), TEXT("Scan_982"));

        return true;
    }

    // Spawn an actor inside the world to run standard actor component life cycle test
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* TempActor = World->SpawnActor<AActor>(SpawnParams);
    if (!TempActor)
    {
        AddError(TEXT("Failed to spawn transient actor for broker lifecycle testing"));
        return false;
    }

    USovereignSaveableEntityComponent* Component = NewObject<USovereignSaveableEntityComponent>(TempActor);
    TempActor->AddInstanceComponent(Component);
    Component->RegisterComponent();

    // Verification of automatic BeginPlay instantiation
    TestNotNull(TEXT("Component dynamically instantiated DiagnosticBroker"), Component->DiagnosticBroker);
    TestNotNull(TEXT("Component dynamically instantiated CultivationBroker"), Component->CultivationBroker);

    TempActor->Destroy();
    return true;
}
