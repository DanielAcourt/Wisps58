// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemFragment.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN ITEM CORE TESTS - B-066 Verification
 * ============================================================================
 * Verifies USovereignItemDefinition creation, PrimaryAssetId generation,
 * fragment attachment, and templated / class-based fragment retrieval.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignItemCoreTest,
	"Sovereign.Inventory.ItemCore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignItemCoreTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate USovereignItemDefinition
	USovereignItemDefinition* ItemDef = NewObject<USovereignItemDefinition>();
	if (!ItemDef)
	{
		AddError(TEXT("Failed to instantiate USovereignItemDefinition"));
		return false;
	}

	// 2. Verify PrimaryAssetId registration
	FPrimaryAssetId AssetId = ItemDef->GetPrimaryAssetId();
	TestEqual(TEXT("PrimaryAssetType should be SovereignItem"), AssetId.PrimaryAssetType.GetName(), FName(TEXT("SovereignItem")));

	// 3. Verify empty fragment search returns nullptr
	const USovereignItemFragment* MissingFragment = ItemDef->FindFragment<USovereignItemFragment>();
	TestNull(TEXT("Searching empty fragments array should return nullptr"), MissingFragment);

	// 4. Attach concrete UI fragment subobject (abstract base cannot be instantiated directly)
	USovereignItemFragment_UIData* ConcreteFragment = NewObject<USovereignItemFragment_UIData>(ItemDef);
	if (!ConcreteFragment)
	{
		AddError(TEXT("Failed to instantiate USovereignItemFragment_UIData"));
		return false;
	}
	ConcreteFragment->Description = FText::FromString(TEXT("Test Description"));
	ItemDef->Fragments.Add(ConcreteFragment);

	TestEqual(TEXT("Fragments array count should be 1"), ItemDef->Fragments.Num(), 1);

	// 5. Test templated FindFragment<T>() for concrete type and base type
	const USovereignItemFragment_UIData* FoundConcrete = ItemDef->FindFragment<USovereignItemFragment_UIData>();
	TestNotNull(TEXT("FindFragment<USovereignItemFragment_UIData> should find the attached fragment"), FoundConcrete);

	const USovereignItemFragment* FoundBase = ItemDef->FindFragment<USovereignItemFragment>();
	TestNotNull(TEXT("FindFragment<USovereignItemFragment> should find base fragment"), FoundBase);

	// 6. Test Blueprint-callable GetFragmentByClass()
	const USovereignItemFragment* FoundByClass = ItemDef->GetFragmentByClass(USovereignItemFragment::StaticClass());
	TestNotNull(TEXT("GetFragmentByClass should return fragment"), FoundByClass);
	TestTrue(TEXT("FoundByClass should be of class USovereignItemFragment"), FoundByClass && FoundByClass->IsA(USovereignItemFragment::StaticClass()));

	// 7. Test GetFragmentByClass with nullptr
	const USovereignItemFragment* NullSearch = ItemDef->GetFragmentByClass(nullptr);
	TestNull(TEXT("GetFragmentByClass with nullptr should return nullptr"), NullSearch);

	// 8. Test lifecycle default virtual method execution
	TestFalse(TEXT("Default OnConsumed should return false"), ConcreteFragment->OnConsumed(nullptr, 1));
	TestFalse(TEXT("Default OnEquipped should return false"), ConcreteFragment->OnEquipped(nullptr, NAME_None));
	TestFalse(TEXT("Default OnUnequipped should return false"), ConcreteFragment->OnUnequipped(nullptr, NAME_None));

	return true;
}
