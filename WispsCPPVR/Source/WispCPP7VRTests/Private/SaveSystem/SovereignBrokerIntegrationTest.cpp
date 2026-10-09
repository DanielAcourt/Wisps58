// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "Misc/AutomationTest.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Entities/SovereignDiagnosticBroker.h"
#include "Entities/SovereignCultivationBroker.h"
#include "Entities/SovereignBaseEntity.h"
#include "Entities/SovereignLivingEntity.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Entities/SovereignPlayerWisp.h"
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

// ============================================================================
// DYNAMIC VESSEL INFUSION & RESTORATION TEST - B-049 Verification
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignDynamicVesselInfusionTest,
    "Sovereign.Soul.DynamicVesselInfusion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignDynamicVesselInfusionTest::RunTest(const FString& Parameters)
{
    // Spawn transient actor to host dynamic infusion
    UWorld* World = nullptr;
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        World = GEngine->GetWorldContexts()[0].World();
    }

    if (!World)
    {
        AddError(TEXT("No World context available for dynamic infusion testing"));
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* TempRock = World->SpawnActor<AActor>(SpawnParams);
    if (!TempRock)
    {
        AddError(TEXT("Failed to spawn transient rock actor for infusion testing"));
        return false;
    }

    USovereignSaveableEntityComponent* Soul = NewObject<USovereignSaveableEntityComponent>(TempRock);
    TempRock->AddInstanceComponent(Soul);
    Soul->RegisterComponent();

    // 1. Initial State: Plain Rock without Magic or Life
    TestNull(TEXT("Rock initially has no QiComponent"), TempRock->FindComponentByClass<USovereignQiComponent>());
    TestNull(TEXT("Rock initially has no BioComponent"), TempRock->FindComponentByClass<USovereignBioComponent>());

    // 2. Infuse Magic & Life dynamically at runtime
    USovereignQiComponent* QiComp = Soul->InfuseMagic();
    USovereignBioComponent* BioComp = Soul->InfuseLife();

    TestNotNull(TEXT("InfuseMagic created USovereignQiComponent"), QiComp);
    TestNotNull(TEXT("InfuseLife created USovereignBioComponent"), BioComp);

    // 3. Verify state capture includes Sovereign.Magic and Sovereign.Bio
    TSharedPtr<FJsonObject> InfusedState = Soul->CaptureFullEntityState();
    TestTrue(TEXT("Infused state has Sovereign.Magic"), InfusedState->HasField(TEXT("Sovereign.Magic")));
    TestTrue(TEXT("Infused state has Sovereign.Bio"), InfusedState->HasField(TEXT("Sovereign.Bio")));

    // 4. Test Component Extraction
    bool bMagicExtracted = Soul->ExtractMagic();
    bool bLifeExtracted = Soul->ExtractLife();

    TestTrue(TEXT("ExtractMagic succeeded"), bMagicExtracted);
    TestTrue(TEXT("ExtractLife succeeded"), bLifeExtracted);
    TestNull(TEXT("Rock has no QiComponent after extraction"), TempRock->FindComponentByClass<USovereignQiComponent>());
    TestNull(TEXT("Rock has no BioComponent after extraction"), TempRock->FindComponentByClass<USovereignBioComponent>());

    // 5. Test Dynamic Component Instantiation on Load
    Soul->ApplyStateFromJsonObject(InfusedState);

    TestNotNull(TEXT("ApplyStateFromJsonObject dynamically restored USovereignQiComponent on load"), TempRock->FindComponentByClass<USovereignQiComponent>());
    TestNotNull(TEXT("ApplyStateFromJsonObject dynamically restored USovereignBioComponent on load"), TempRock->FindComponentByClass<USovereignBioComponent>());

    TempRock->Destroy();
    return true;
}

// ============================================================================
// MODULAR BASE vs LIVING ENTITY HIERARCHY TEST - B-047 Verification
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignEntityHierarchyModularTest,
    "Sovereign.Soul.EntityHierarchyModularization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignEntityHierarchyModularTest::RunTest(const FString& Parameters)
{
    // 1. Instantiate Base Entity (Non-living simulation entity like a Rock or Terminal)
    ASovereignBaseEntity* BaseEntity = NewObject<ASovereignBaseEntity>();
    if (!BaseEntity)
    {
        AddError(TEXT("Failed to instantiate ASovereignBaseEntity"));
        return false;
    }

    // Base entity must have SaveDataComponent (Soul Hub) and EntityMesh, but NO default Bio/Attribute subobjects
    TestNotNull(TEXT("Base Entity has SaveDataComponent"), BaseEntity->GetSaveDataComponent());
    TestNull(TEXT("Base Entity has NO default BioComponent"), BaseEntity->FindComponentByClass<USovereignBioComponent>());
    TestNull(TEXT("Base Entity has NO default AttributeComponent"), BaseEntity->FindComponentByClass<USovereignAttributeComponent>());

    // Base entity implements IInteractionInterface for possession
    TestTrue(TEXT("Base Entity implements IInteractionInterface"), BaseEntity->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()));
    TestTrue(TEXT("Base Entity can be possessed by default"), IInteractionInterface::Execute_CanBePossessed(BaseEntity));

    // 2. Instantiate Living Entity (Organic creature/plant)
    ASovereignLivingEntity* LivingEntity = NewObject<ASovereignLivingEntity>();
    if (!LivingEntity)
    {
        AddError(TEXT("Failed to instantiate ASovereignLivingEntity"));
        return false;
    }

    TestTrue(TEXT("Living Entity implements IInteractionInterface"), LivingEntity->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()));
    TestTrue(TEXT("Living Entity can be possessed by default"), IInteractionInterface::Execute_CanBePossessed(LivingEntity));

    // Living entity inherits SaveDataComponent and constructs Bio, Attribute, Qi, Element subobjects by default
    TestNotNull(TEXT("Living Entity has SaveDataComponent"), LivingEntity->GetSaveDataComponent());
    TestNotNull(TEXT("Living Entity has default BioComponent"), LivingEntity->GetBioComponent());
    TestNotNull(TEXT("Living Entity has default AttributeComponent"), LivingEntity->GetAttributeComponent());
    TestNotNull(TEXT("Living Entity has default QiComponent"), LivingEntity->GetQiComponent());
    TestNotNull(TEXT("Living Entity has default ElementComponent"), LivingEntity->GetElementComponent());

    return true;
}

// ============================================================================
// SOVEREIGN STATE CACHING & THROTTLING TEST - B-042 Verification
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignStateCachingThrottlingTest,
    "Sovereign.Soul.StateCachingThrottling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignStateCachingThrottlingTest::RunTest(const FString& Parameters)
{
    // 1. Create Soul Component directly
    USovereignSaveableEntityComponent* Soul = NewObject<USovereignSaveableEntityComponent>();
    if (!Soul)
    {
        AddError(TEXT("Failed to create USovereignSaveableEntityComponent for testing"));
        return false;
    }

    // Initialize brokers to populate state
    Soul->DiagnosticBroker = NewObject<UDiagnosticBroker>(Soul);
    Soul->RegisterBroker(Soul->DiagnosticBroker);

    // Set a baseline truth value
    Soul->DiagnosticBroker->SetTruthValue(TEXT("temp_c"), TEXT("20.0"));

    // Set a known throttle interval
    Soul->UIUpdateThrottleInterval = 0.05f;

    // 2. First query: should capture the state and cache it
    FString InitialState = Soul->GetCategoryStateJson(TEXT("Sovereign.Truth"));
    TestTrue(TEXT("Initial state is not empty"), !InitialState.IsEmpty());
    TestTrue(TEXT("Initial state has truth value"), InitialState.Contains(TEXT("20.0")));

    // 3. Mutate the state directly in the broker (bypassing Soul-level mutators)
    Soul->DiagnosticBroker->SetTruthValue(TEXT("temp_c"), TEXT("30.0"));

    // 4. Query again in the same frame/time-window
    // Since we are in the same frame and time has not advanced, it MUST return the cached value (20.0) instead of the new value (30.0).
    FString CachedState = Soul->GetCategoryStateJson(TEXT("Sovereign.Truth"));
    TestEqual(TEXT("Same-frame query returns cached state"), CachedState, InitialState);
    TestTrue(TEXT("Cached state still has old value"), CachedState.Contains(TEXT("20.0")));
    TestFalse(TEXT("Cached state does not have new value"), CachedState.Contains(TEXT("30.0")));

    // 5. Trigger explicit invalidation
    Soul->InvalidateStateCache();

    // 6. Query again: should return the updated state (30.0)
    FString InvalidatedState = Soul->GetCategoryStateJson(TEXT("Sovereign.Truth"));
    TestNotEqual(TEXT("Invalidated query returns fresh state"), InvalidatedState, InitialState);
    TestTrue(TEXT("Invalidated state has new value"), InvalidatedState.Contains(TEXT("30.0")));

    // 7. Verify implicit invalidation on Soul-level mutators (e.g. AddUnknownTag)
    Soul->AddUnknownTag(TEXT("ParadoxTag"), TEXT("Confused"));

    // Changing value in broker again
    Soul->DiagnosticBroker->SetTruthValue(TEXT("temp_c"), TEXT("40.0"));

    // Since AddUnknownTag calls InvalidateStateCache(), the query MUST return the updated state (40.0)
    FString MutatedState = Soul->GetCategoryStateJson(TEXT("Sovereign.Truth"));
    TestTrue(TEXT("Mutation query returns fresh state"), MutatedState.Contains(TEXT("40.0")));

    return true;
}

// ============================================================================
// SOVEREIGN ATTRIBUTE BROKER & REGISTRATION TEST - B-043 Verification
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignAttributeRegistrationTest,
    "Sovereign.Soul.AttributeRegistrationAndSync",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignAttributeRegistrationTest::RunTest(const FString& Parameters)
{
    // 1. Create Soul Component and Attribute Component
    USovereignSaveableEntityComponent* Soul = NewObject<USovereignSaveableEntityComponent>();
    USovereignAttributeComponent* AttrComp = NewObject<USovereignAttributeComponent>();

    if (!Soul || !AttrComp)
    {
        AddError(TEXT("Failed to create components for B-043 testing"));
        return false;
    }

    // 2. Test manual & automatic broker registration via Soul authority
    TScriptInterface<ISovereignBrokerInterface> BrokerInterface(AttrComp);
    Soul->RegisterBroker(BrokerInterface);

    // 3. Verify custom D&D values & experience accumulators
    AttrComp->Strength = 18;
    AttrComp->StrengthExperience = 1250.50;
    AttrComp->Dexterity = 14;
    AttrComp->DexterityExperience = 850.25;
    AttrComp->Constitution = 16;
    AttrComp->ConstitutionExperience = 990.00;
    AttrComp->Intelligence = 15;
    AttrComp->IntelligenceExperience = 450.00;
    AttrComp->Wisdom = 12;
    AttrComp->WisdomExperience = 300.00;
    AttrComp->Charisma = 10;
    AttrComp->CharismaExperience = 100.00;
    AttrComp->Luck = 20;
    AttrComp->LuckExperience = 5000.00;

    AttrComp->ArmourClass = 15;
    AttrComp->CurrentHealth = 160.0f;
    AttrComp->MaxHealth = 160.0f;
    AttrComp->CurrentStamina = 100.0f;
    AttrComp->MaxStamina = 100.0f;

    AttrComp->PhysicalResistance = 0.2f;
    AttrComp->MagicalResistance = 0.15f;
    AttrComp->MentalResistance = 0.10f;
    AttrComp->PoisonResistance = 0.05f;
    AttrComp->SlowResistance = 0.0f;

    // 4. Capture state and verify Sovereign.Attributes JSON field
    TSharedPtr<FJsonObject> State = Soul->CaptureFullEntityState();
    TestTrue(TEXT("Captured state contains Sovereign.Attributes object"), State->HasField(TEXT("Sovereign.Attributes")));

    TSharedPtr<FJsonObject> AttrObj = State->GetObjectField(TEXT("Sovereign.Attributes"));
    TestNotNull(TEXT("Sovereign.Attributes object is valid"), AttrObj.Get());

    TestEqual(TEXT("Strength level correct"), AttrObj->GetIntegerField(TEXT("Strength")), 18);
    TestEqual(TEXT("StrengthExperience correct"), AttrObj->GetNumberField(TEXT("StrengthExperience")), 1250.50);
    TestEqual(TEXT("Dexterity level correct"), AttrObj->GetIntegerField(TEXT("Dexterity")), 14);
    TestEqual(TEXT("MaxHealth correct"), AttrObj->GetNumberField(TEXT("MaxHealth")), 160.0);
    TestEqual(TEXT("PhysicalResistance correct"), AttrObj->GetNumberField(TEXT("PhysicalResistance")), 0.2);

    // 5. Verify round-trip OnLoad restoration
    USovereignAttributeComponent* RestoredComp = NewObject<USovereignAttributeComponent>();
    RestoredComp->OnLoad(State);

    TestEqual(TEXT("Restored Strength level matches"), RestoredComp->Strength, 18);
    TestEqual(TEXT("Restored Strength XP matches"), RestoredComp->StrengthExperience, 1250.50);
    TestEqual(TEXT("Restored Dexterity level matches"), RestoredComp->Dexterity, 14);
    TestEqual(TEXT("Restored Luck level matches"), RestoredComp->Luck, 20);
    TestEqual(TEXT("Restored Luck XP matches"), RestoredComp->LuckExperience, 5000.00);
    TestEqual(TEXT("Restored MaxHealth matches"), RestoredComp->MaxHealth, 160.0f);
    TestEqual(TEXT("Restored PhysicalResistance matches"), RestoredComp->PhysicalResistance, 0.2f);

    return true;
}
