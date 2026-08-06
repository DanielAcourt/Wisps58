// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Entities/SovereignBlackBoxComponent.h"
#include "Subsystems/SovereignBlackBoxSubsystem.h"
#include "SaveSystem/SovereignPSTAConfig.h"
#include "SaveSystem/SovereignBlackBoxExporter.h"
#include "Subsystems/SovereignBlackBoxHeartbeat.h"
#include "Subsystems/SovereignBlackBoxReplaySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FSovereignBlackBoxSpec, "Sovereign.BlackBox", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    UWorld* World;
    AActor* TestActor;
    USovereignBlackBoxComponent* BBComp;
    USovereignBlackBoxSubsystem* BBSubsystem;

    // Helper methods
    FString GetBlackBoxFilePath() const;
    bool VerifyFileExists(const FString& FilePath) const;
    TSharedPtr<FJsonObject> LoadJsonFile(const FString& FilePath) const;
    void CleanupBlackBoxFile(const FString& FilePath) const;
    void SafeDestroyActor(AActor* Actor);
END_DEFINE_SPEC(FSovereignBlackBoxSpec)

FString FSovereignBlackBoxSpec::GetBlackBoxFilePath() const
{
    if (!BBComp)
    {
        return FString();
    }
    return FPaths::ProjectSavedDir() / TEXT("BlackBox") / FString::Printf(TEXT("BB_%s.json"), *BBComp->EntityID.ToString());
}

bool FSovereignBlackBoxSpec::VerifyFileExists(const FString& FilePath) const
{
    if (FilePath.IsEmpty())
    {
        return false;
    }
    return FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath);
}

TSharedPtr<FJsonObject> FSovereignBlackBoxSpec::LoadJsonFile(const FString& FilePath) const
{
    if (!VerifyFileExists(FilePath))
    {
        return nullptr;
    }

    FString JsonContent;
    if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
    {
        return nullptr;
    }

    TSharedPtr<FJsonObject> JsonObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        return JsonObject;
    }

    return nullptr;
}

void FSovereignBlackBoxSpec::CleanupBlackBoxFile(const FString& FilePath) const
{
    if (VerifyFileExists(FilePath))
    {
        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*FilePath);
    }
}

void FSovereignBlackBoxSpec::SafeDestroyActor(AActor* Actor)
{
    if (Actor && IsValid(Actor) && !Actor->IsActorBeingDestroyed())
    {
        Actor->Destroy(false, false);
    }
}

void FSovereignBlackBoxSpec::Define()
{
    BeforeEach([this]()
    {
        // retrieve the existing stable editor/game world context
        World = nullptr;
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            World = GEngine->GetWorldContexts()[0].World();
        }

        TestTrue("Test World should be valid", World != nullptr);
        if (!World) 
        {
            return;
        }

        BBSubsystem = World->GetSubsystem<USovereignBlackBoxSubsystem>();
        TestTrue("BlackBox subsystem should be valid", BBSubsystem != nullptr);

        // Create a simple test actor (not abstract)
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        TestActor = World->SpawnActor<AActor>(SpawnParams);
        TestTrue("Test actor should be spawned", TestActor != nullptr);

        if (!TestActor)
        {
            return;
        }

        // Create and attach the BlackBox component
        BBComp = NewObject<USovereignBlackBoxComponent>(TestActor);
        TestTrue("BlackBox component should be created", BBComp != nullptr);

        if (BBComp)
        {
            BBComp->RegisterComponent();
            BBComp->EntityID = FGuid::NewGuid();
        }
    });

    // ============================================================
    // CORE FUNCTIONALITY TESTS
    // ============================================================

    It("Should create BlackBox file on first snapshot", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr && World != nullptr);
        if (!BBComp || !World) return;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Simulate an actor with temperature data
        TestActor->Tags.Add(TEXT("Telemetry.temp_c:50"));

        // Act
        BBComp->RecordTruthSnapshot();

        // Assert
        TestTrue("BlackBox file should exist after first snapshot", VerifyFileExists(FilePath));
    });

    It("Should record valid JSON structure", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act
        BBComp->RecordTruthSnapshot();

        // Assert
        const TSharedPtr<FJsonObject> JsonObject = LoadJsonFile(FilePath);
        TestTrue("JSON should be parseable", JsonObject.IsValid());
        
        if (JsonObject.IsValid())
        {
            TestTrue("JSON should contain EntityID field", JsonObject->HasField(TEXT("EntityID")));
            TestTrue("JSON should contain Logs field", JsonObject->HasField(TEXT("Logs")));
        }
    });

    It("Should record events in JSON", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act
        BBComp->RecordEvent(TEXT("TestEvent"), TEXT("TestValue"));
        BBComp->RecordTruthSnapshot();

        // Assert
        const TSharedPtr<FJsonObject> JsonObject = LoadJsonFile(FilePath);
        TestTrue("JSON should be parseable", JsonObject.IsValid());

        if (JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonObject->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                bool bFoundEvent = false;
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->HasField(TEXT("Key")))
                    {
                        FString Key = LogEntry->GetStringField(TEXT("Key"));
                        if (Key == TEXT("TestEvent"))
                        {
                            bFoundEvent = true;
                            break;
                        }
                    }
                }
                TestTrue("Should have recorded the test event", bFoundEvent);
            }
        }
    });

    // ============================================================
    // CSV EXPORT TESTS
    // ============================================================

    It("Should export logs to CSV using Exporter", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        BBComp->RecordEvent(TEXT("ExportTestEvent"), TEXT("ExportTestData"));
        BBComp->RecordTruthSnapshot();

        // Act
        FString CsvPath;
        bool bSuccess = USovereignBlackBoxExporter::ExportEntityLogToCSV(BBComp->EntityID, CsvPath);

        // Assert
        TestTrue("CSV Export should be successful", bSuccess);
        TestTrue("CSV file should exist", VerifyFileExists(CsvPath));

        if (bSuccess && VerifyFileExists(CsvPath))
        {
            FString Content;
            if (FFileHelper::LoadFileToString(Content, *CsvPath))
            {
                TestTrue("CSV should contain event data", Content.Contains(TEXT("ExportTestEvent")));
            }
        }
    });

    // ============================================================
    // PSTA TESTS
    // ============================================================

    It("Should record PSTA dimension health correctly", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        USovereignPSTAConfig* Config = NewObject<USovereignPSTAConfig>();
        if (!Config) return;

        // Setup PSTA tag mapping for temperature
        FPSTATagMapping Mapping;
        Mapping.TagKey = TEXT("Telemetry.temp_c");
        Mapping.Dimension = EPSTADimension::Technical;
        Mapping.Weight = 1.0f;
        Mapping.RangeMin = 0.0f;
        Mapping.RangeMax = 100.0f;
        Config->TagMappings.Add(Mapping);

        // Setup dimension weights
        Config->DimensionWeights.Add(EPSTADimension::Technical, 1.0f);
        Config->DimensionWeights.Add(EPSTADimension::Psychological, 0.0f);
        Config->DimensionWeights.Add(EPSTADimension::Social, 0.0f);
        Config->DimensionWeights.Add(EPSTADimension::Administrative, 0.0f);

        BBComp->PSTAConfig = Config;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act
        BBComp->RecordTruthSnapshot();

        // Assert
        const TSharedPtr<FJsonObject> JsonObject = LoadJsonFile(FilePath);
        TestTrue("JSON should be valid", JsonObject.IsValid());

        if (JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonObject->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                bool bFoundDi = false;
                float DiValue = -1.0f;
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->GetStringField(TEXT("Key")).Equals(TEXT("PSTA.Di.2"))) // Technical = 2
                    {
                        bFoundDi = true;
                        DiValue = LogEntry->GetNumberField(TEXT("Value"));
                        break;
                    }
                }
                TestTrue("Should have recorded PSTA.Di.2", bFoundDi);
                TestEqual("Technical health should be 0.5 (50/100)", DiValue, 0.5f);
            }
        }
    });

    It("Should implement the Unified Safety Formula (VSS) and non-compensatory failure", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        USovereignPSTAConfig* Config = NewObject<USovereignPSTAConfig>();
        if (!Config) return;

        // Setup two dimensions: Technical and Social
        FPSTATagMapping TechnicalMapping;
        TechnicalMapping.TagKey = TEXT("Telemetry.TechnicalValue");
        TechnicalMapping.Dimension = EPSTADimension::Technical;
        TechnicalMapping.Weight = 1.0f;
        TechnicalMapping.RangeMin = 0.0f;
        TechnicalMapping.RangeMax = 100.0f;
        Config->TagMappings.Add(TechnicalMapping);

        FPSTATagMapping SocialMapping;
        SocialMapping.TagKey = TEXT("Telemetry.SocialValue");
        SocialMapping.Dimension = EPSTADimension::Social;
        SocialMapping.Weight = 1.0f;
        SocialMapping.RangeMin = 0.0f;
        SocialMapping.RangeMax = 100.0f;
        Config->TagMappings.Add(SocialMapping);

        // Assign weights (50/50)
        Config->DimensionWeights.Add(EPSTADimension::Technical, 0.5f);
        Config->DimensionWeights.Add(EPSTADimension::Social, 0.5f);
        Config->DimensionWeights.Add(EPSTADimension::Psychological, 0.0f);
        Config->DimensionWeights.Add(EPSTADimension::Administrative, 0.0f);

        // Set failure thresholds (0.3)
        Config->DimensionFailureThresholds.Add(EPSTADimension::Technical, 0.3f);
        Config->DimensionFailureThresholds.Add(EPSTADimension::Social, 0.3f);

        BBComp->PSTAConfig = Config;

        const FString FilePath = GetBlackBoxFilePath();

        // SCENARIO 1: Nominal (Both 100%) -> VSS should be 1.0
        TestActor->Tags.Empty();
        TestActor->Tags.Add(TEXT("Telemetry.TechnicalValue:100"));
        TestActor->Tags.Add(TEXT("Telemetry.SocialValue:100"));

        CleanupBlackBoxFile(FilePath);
        BBComp->RecordTruthSnapshot();

        TSharedPtr<FJsonObject> JsonNominal = LoadJsonFile(FilePath);
        if (JsonNominal.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonNominal->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                bool bFoundVSS = false;
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->GetStringField(TEXT("Key")).Equals(TEXT("PSTA.VSS")))
                    {
                        bFoundVSS = true;
                        TestEqual("Nominal VSS should be 1.0", (float)LogEntry->GetNumberField(TEXT("Value")), 1.0f);
                    }
                }
                TestTrue("Should have recorded PSTA.VSS (Nominal)", bFoundVSS);
            }
        }

        // SCENARIO 2: Partial Failure (Social at 20% < 30% Threshold) -> VSS should collapse to 0.0
        // even though Technical is still 100% (Weighted Average would be 0.6)
        TestActor->Tags.Empty();
        TestActor->Tags.Add(TEXT("Telemetry.TechnicalValue:100"));
        TestActor->Tags.Add(TEXT("Telemetry.SocialValue:20"));

        CleanupBlackBoxFile(FilePath);
        BBComp->RecordTruthSnapshot();

        TSharedPtr<FJsonObject> JsonFail = LoadJsonFile(FilePath);
        if (JsonFail.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonFail->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                bool bFoundVSS = false;
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->GetStringField(TEXT("Key")).Equals(TEXT("PSTA.VSS")))
                    {
                        bFoundVSS = true;
                        TestEqual("VSS should COLLAPSE to 0.0 due to non-compensatory Social failure", (float)LogEntry->GetNumberField(TEXT("Value")), 0.0f);
                    }
                }
                TestTrue("Should have recorded PSTA.VSS (Failure)", bFoundVSS);
            }
        }
    });

    It("Should enforce PSTA Anchor Tag failure and Void Safety", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        USovereignPSTAConfig* Config = NewObject<USovereignPSTAConfig>();
        if (!Config) return;

        // 1. ANCHOR TAG SETUP: Technical dimension has one normal sensor and one Anchor sensor.
        FPSTATagMapping NormalMapping;
        NormalMapping.TagKey = TEXT("Telemetry.NormalSensor");
        NormalMapping.Dimension = EPSTADimension::Technical;
        NormalMapping.Weight = 1.0f;
        NormalMapping.RangeMin = 0.0f;
        NormalMapping.RangeMax = 100.0f;
        Config->TagMappings.Add(NormalMapping);

        FPSTATagMapping AnchorMapping;
        AnchorMapping.TagKey = TEXT("Telemetry.CriticalAnchor");
        AnchorMapping.Dimension = EPSTADimension::Technical;
        AnchorMapping.Weight = 1.0f;
        AnchorMapping.RangeMin = 0.0f;
        AnchorMapping.RangeMax = 100.0f;
        AnchorMapping.bIsAnchorTag = true;
        Config->TagMappings.Add(AnchorMapping);

        BBComp->PSTAConfig = Config;

        // Simulate normal sensor at 100% (Healthy) but Anchor at 0% (CRITICAL FAILURE)
        TestActor->Tags.Empty();
        TestActor->Tags.Add(TEXT("Telemetry.NormalSensor:100"));
        TestActor->Tags.Add(TEXT("Telemetry.CriticalAnchor:0"));

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act
        BBComp->RecordTruthSnapshot();

        // Assert Anchor Failure
        const TSharedPtr<FJsonObject> JsonObject = LoadJsonFile(FilePath);
        TestTrue("JSON should be valid", JsonObject.IsValid());

        if (JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonObject->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->GetStringField(TEXT("Key")).Equals(TEXT("PSTA.Di.2")))
                    {
                        float DiValue = LogEntry->GetNumberField(TEXT("Value"));
                        TestEqual("Technical health MUST be 0.0 due to Anchor failure, despite normal sensor", DiValue, 0.0f);
                    }
                }
            }
        }

        // 2. VOID SAFETY SETUP: Social dimension has NO mappings.
        // We trigger another snapshot.
        CleanupBlackBoxFile(FilePath);
        BBComp->RecordTruthSnapshot();

        const TSharedPtr<FJsonObject> JsonObjectVoid = LoadJsonFile(FilePath);
        if (JsonObjectVoid.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonObjectVoid->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    // Social = 1
                    if (LogEntry && LogEntry->GetStringField(TEXT("Key")).Equals(TEXT("PSTA.Di.1")))
                    {
                        float DiValue = LogEntry->GetNumberField(TEXT("Value"));
                        TestEqual("Social health MUST be 0.0 (Void Safety), as it has no sensors", DiValue, 0.0f);
                    }
                }
            }
        }
    });

    // ============================================================
    // THRESHOLD TESTS
    // ============================================================

    It("Should track file changes on snapshots", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act - Take first snapshot
        BBComp->RecordTruthSnapshot();
        int64 FirstFileSize = FPlatformFileManager::Get().GetPlatformFile().FileSize(*FilePath);

        // Take another snapshot without changes
        BBComp->RecordTruthSnapshot();
        int64 SecondFileSize = FPlatformFileManager::Get().GetPlatformFile().FileSize(*FilePath);

        // Assert
        TestTrue("First snapshot should create file", FirstFileSize > 0);
        TestTrue("Second snapshot should exist", SecondFileSize > 0);
    });

    // ============================================================
    // INGESTION TESTS
    // ============================================================

    It("Should ingest external telemetry entries", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr);
        if (!BBComp) return;

        FBlackBoxEntry Entry;
        Entry.Key = TEXT("ExternalTelemetry.test_value");
        Entry.Value = 42.5f;

        // Act
        BBComp->IngestBlackBoxEntry(Entry);
        BBComp->RecordTruthSnapshot();

        const FString FilePath = GetBlackBoxFilePath();

        // Assert
        const TSharedPtr<FJsonObject> JsonObject = LoadJsonFile(FilePath);
        TestTrue("JSON should be valid after ingestion", JsonObject.IsValid());

        if (JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
            if (JsonObject->TryGetArrayField(TEXT("Logs"), Logs) && Logs)
            {
                bool bFoundIngested = false;
                for (const auto& LogVal : *Logs)
                {
                    const TSharedPtr<FJsonObject> LogEntry = LogVal->AsObject();
                    if (LogEntry && LogEntry->HasField(TEXT("Key")))
                    {
                        FString Key = LogEntry->GetStringField(TEXT("Key"));
                        if (Key == TEXT("ExternalTelemetry.test_value"))
                        {
                            bFoundIngested = true;
                            if (LogEntry->HasField(TEXT("Value")))
                            {
                                double Value = LogEntry->GetNumberField(TEXT("Value"));
                                TestEqual("Ingested value should match", Value, 42.5);
                            }
                            break;
                        }
                    }
                }
                TestTrue("Should have recorded ingested telemetry", bFoundIngested);
            }
        }
    });

    // ============================================================
    // HEARTBEAT TESTS
    // ============================================================

    It("Should register and tick via Heartbeat subsystem", [this]()
    {
        // Arrange
        TestTrue("Setup should be valid", BBComp != nullptr && World != nullptr);
        if (!BBComp || !World) return;

        USovereignBlackBoxHeartbeat* Heartbeat = World->GetSubsystem<USovereignBlackBoxHeartbeat>();
        TestTrue("Heartbeat subsystem should exist", Heartbeat != nullptr);
        
        if (!Heartbeat) return;

        BBComp->UpdateFrequency = EUpdateFrequency::Realtime;

        const FString FilePath = GetBlackBoxFilePath();
        CleanupBlackBoxFile(FilePath);

        // Act
        Heartbeat->RegisterComponent(BBComp, EUpdateFrequency::Realtime);
        Heartbeat->Tick(0.2f); // Simulate 200ms passing

        // Assert - Should have created a snapshot file
        TestTrue("Heartbeat should have triggered snapshot", VerifyFileExists(FilePath));
    });

    // ============================================================
    // SUBSYSTEM TESTS
    // ============================================================

    It("Should have valid BlackBox subsystem in world", [this]()
    {
        // Arrange & Act
        TestTrue("Setup should be valid", World != nullptr);
        if (!World) return;

        USovereignBlackBoxSubsystem* Subsystem = World->GetSubsystem<USovereignBlackBoxSubsystem>();

        // Assert
        TestTrue("BlackBox subsystem should be valid", Subsystem != nullptr);
    });

    It("Should have valid Replay subsystem in world", [this]()
    {
        // Arrange & Act
        TestTrue("Setup should be valid", World != nullptr);
        if (!World) return;

        USovereignBlackBoxReplaySubsystem* ReplaySubsystem = World->GetSubsystem<USovereignBlackBoxReplaySubsystem>();

        // Assert
        TestTrue("Replay subsystem should exist", ReplaySubsystem != nullptr);
    });

    // ============================================================
    // CLEANUP
    // ============================================================

    AfterEach([this]()
    {
        // Clean up the BlackBox file
        if (BBComp)
        {
            const FString FilePath = GetBlackBoxFilePath();
            CleanupBlackBoxFile(FilePath);
        }

        // Destroy test actor if it exists
        if (TestActor)
        {
            SafeDestroyActor(TestActor);
            TestActor = nullptr;
        }

        BBComp = nullptr;
        BBSubsystem = nullptr;

        // DO NOT destroy the global context/world here as it is shared!
        World = nullptr;
    });
}

#endif  // WITH_DEV_AUTOMATION_TESTS
