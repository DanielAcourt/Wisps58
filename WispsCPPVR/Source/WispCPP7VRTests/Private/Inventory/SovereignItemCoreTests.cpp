// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemFragment.h"
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

	// 4. Attach base fragment subobject
	USovereignItemFragment* BaseFragment = NewObject<USovereignItemFragment>(ItemDef);
	if (!BaseFragment)
	{
		AddError(TEXT("Failed to instantiate USovereignItemFragment"));
		return false;
	}
	ItemDef->Fragments.Add(BaseFragment);

	TestEqual(TEXT("Fragments array count should be 1"), ItemDef->Fragments.Num(), 1);

	// 5. Test templated FindFragment<T>()
	const USovereignItemFragment* FoundFragment = ItemDef->FindFragment<USovereignItemFragment>();
	TestNotNull(TEXT("FindFragment<USovereignItemFragment> should find the attached fragment"), FoundFragment);

	// 6. Test Blueprint-callable GetFragmentByClass()
	const USovereignItemFragment* FoundByClass = ItemDef->GetFragmentByClass(USovereignItemFragment::StaticClass());
	TestNotNull(TEXT("GetFragmentByClass should return fragment"), FoundByClass);
	TestTrue(TEXT("FoundByClass should be of class USovereignItemFragment"), FoundByClass && FoundByClass->IsA(USovereignItemFragment::StaticClass()));

	// 7. Test GetFragmentByClass with nullptr
	const USovereignItemFragment* NullSearch = ItemDef->GetFragmentByClass(nullptr);
	TestNull(TEXT("GetFragmentByClass with nullptr should return nullptr"), NullSearch);

	// 8. Test lifecycle default virtual method execution
	TestFalse(TEXT("Default OnConsumed should return false"), BaseFragment->OnConsumed(nullptr, 1));
	TestFalse(TEXT("Default OnEquipped should return false"), BaseFragment->OnEquipped(nullptr, NAME_None));
	TestFalse(TEXT("Default OnUnequipped should return false"), BaseFragment->OnUnequipped(nullptr, NAME_None));

	return true;
}
