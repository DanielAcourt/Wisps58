// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Components/SovereignBaseComponent.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignElementComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Interaction/SovereignUIInspectable.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignUIInspectableInterfaceTest,
    "Sovereign.UI.InspectableInterface",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignUIInspectableInterfaceTest::RunTest(const FString& Parameters)
{
    // Test Bio Component
    USovereignBioComponent* BioComp = NewObject<USovereignBioComponent>();
    if (TestNotNull(TEXT("BioComponent created"), BioComp))
    {
        TestTrue(TEXT("BioComponent implements ISovereignUIInspectable"), BioComp->GetClass()->ImplementsInterface(USovereignUIInspectable::StaticClass()));
        FText DisplayName = ISovereignUIInspectable::Execute_GetInspectorDisplayName(BioComp);
        TestFalse(TEXT("Display name is not empty"), DisplayName.IsEmpty());
        FString Category = ISovereignUIInspectable::Execute_GetInspectorCategory(BioComp);
        TestFalse(TEXT("Category is not empty"), Category.IsEmpty());
        TMap<FString, FString> Props = ISovereignUIInspectable::Execute_GetInspectorProperties(BioComp);
        TestTrue(TEXT("Properties extracted via reflection"), Props.Num() > 0);
        FString JsonData = ISovereignUIInspectable::Execute_GetInspectorDataJson(BioComp);
        TestFalse(TEXT("JSON string is not empty"), JsonData.IsEmpty());
    }

    // Test Attribute Component
    USovereignAttributeComponent* AttrComp = NewObject<USovereignAttributeComponent>();
    if (TestNotNull(TEXT("AttributeComponent created"), AttrComp))
    {
        TestTrue(TEXT("AttributeComponent implements ISovereignUIInspectable"), AttrComp->GetClass()->ImplementsInterface(USovereignUIInspectable::StaticClass()));
        TMap<FString, FString> Props = ISovereignUIInspectable::Execute_GetInspectorProperties(AttrComp);
        TestTrue(TEXT("Attribute properties extracted via reflection"), Props.Num() > 0);
    }

    // Test Element Component
    USovereignElementComponent* ElemComp = NewObject<USovereignElementComponent>();
    if (TestNotNull(TEXT("ElementComponent created"), ElemComp))
    {
        TestTrue(TEXT("ElementComponent implements ISovereignUIInspectable"), ElemComp->GetClass()->ImplementsInterface(USovereignUIInspectable::StaticClass()));
    }

    // Test Qi Component
    USovereignQiComponent* QiComp = NewObject<USovereignQiComponent>();
    if (TestNotNull(TEXT("QiComponent created"), QiComp))
    {
        TestTrue(TEXT("QiComponent implements ISovereignUIInspectable"), QiComp->GetClass()->ImplementsInterface(USovereignUIInspectable::StaticClass()));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSovereignUIInspectorDiscoveryTest,
    "Sovereign.UI.InspectorDiscovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FSovereignUIInspectorDiscoveryTest::RunTest(const FString& Parameters)
{
    // Null safety check
    TArray<UActorComponent*> NullResults = USovereignSaveableEntityComponent::GetInspectableComponents(nullptr);
    TestEqual(TEXT("Null actor returns empty inspectable components"), NullResults.Num(), 0);

    FString NullJson = USovereignSaveableEntityComponent::GetAggregatedInspectionJson(nullptr);
    TestEqual(TEXT("Null actor returns empty JSON object"), NullJson, FString(TEXT("{}")));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
