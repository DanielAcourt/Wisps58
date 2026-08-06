// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Misc/AutomationTest.h"
#include "Entities/SovereignBaseInteractable.h"
#include "Entities/SovereignSaveTerminal.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

/**
 * ============================================================================
 * SOVEREIGN TELEMETRY MAPPING TEST - B-001 Verification
 * ============================================================================
 * Focus: Verifying the Digital Twin IoT mapping (temp_c, ph_val, water_depth_mm)
 * from JSON "Soul" state to the "Vessel" Actor properties.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignTelemetryMappingTest,
    "SaveSystem.DigitalTwin.TelemetryMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignTelemetryMappingTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        World = GEngine->GetWorldContexts()[0].World();
    }

    if (!World)
    {
        // If no world, we can't spawn actors for integration test.
        return true;
    }

    // 1. Setup the Vessel
    // Using ASovereignSaveTerminal as a concrete implementation of ASovereignBaseInteractable
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ASovereignSaveTerminal* Vessel = World->SpawnActor<ASovereignSaveTerminal>(SpawnParams);

    if (!Vessel)
    {
        AddError(TEXT("Failed to spawn SovereignSaveTerminal for testing"));
        return false;
    }

    USovereignSaveableEntityComponent* Soul = Vessel->FindComponentByClass<USovereignSaveableEntityComponent>();
    if (!Soul)
    {
        AddError(TEXT("Vessel is missing a Sovereign Soul component"));
        Vessel->Destroy();
        return false;
    }

    // 2. Define the "Current Truth" (IoT Telemetry from JSON)
    TSharedPtr<FJsonObject> TelemetryJson = MakeShared<FJsonObject>();
    TelemetryJson->SetStringField(TEXT("Telemetry.temp_c"), TEXT("24.5"));
    TelemetryJson->SetStringField(TEXT("Telemetry.ph_val"), TEXT("8.2"));
    TelemetryJson->SetStringField(TEXT("Telemetry.water_depth_mm"), TEXT("450.0"));

    // 3. Apply state to the Soul
    Soul->ApplyStateFromJsonObject(TelemetryJson);

    // 4. Verify the Vessel was updated via the Bridge
    TestEqual(TEXT("Temperature mapped correctly"), Vessel->TemperatureCelsius, 24.5f);
    TestEqual(TEXT("pH Value mapped correctly"), Vessel->PhValue, 8.2f);
    TestEqual(TEXT("Water Depth mapped correctly"), Vessel->WaterDepthMM, 450.0f);

    // 5. Verify round-trip (Capture)
    TSharedPtr<FJsonObject> CapturedState = Soul->CaptureFullEntityState();

    TestTrue(TEXT("Captured state has Telemetry.temp_c"), CapturedState->HasField(TEXT("Telemetry.temp_c")));

    // Support both %f and sanitized float formats
    FString ActualTemp = CapturedState->GetStringField(TEXT("Telemetry.temp_c"));
    TestTrue(TEXT("Captured temp_c matches"), ActualTemp == TEXT("24.500000") || ActualTemp == TEXT("24.5"));

    FString ActualPh = CapturedState->GetStringField(TEXT("Telemetry.ph_val"));
    TestTrue(TEXT("Captured ph_val matches"), ActualPh == TEXT("8.200000") || ActualPh == TEXT("8.2"));

    FString ActualDepth = CapturedState->GetStringField(TEXT("Telemetry.water_depth_mm"));
    TestTrue(TEXT("Captured depth matches"), ActualDepth == TEXT("450.000000") || ActualDepth == TEXT("450.0"));

    // Cleanup
    Vessel->Destroy();

    return true;
}
