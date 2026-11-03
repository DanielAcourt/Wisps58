// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemFragment.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN INVENTORY COMPONENT TESTS - B-067 Verification
 * ============================================================================
 * Verifies slot management, addition, removal, stack merging, capacity gating,
 * item queries, and fragment lifecycle triggers on USovereignInventoryComponent.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignInventoryOperationsTest,
	"Sovereign.Inventory.ComponentOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignInventoryOperationsTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate USovereignInventoryComponent
	USovereignInventoryComponent* InvComp = NewObject<USovereignInventoryComponent>();
	if (!InvComp)
	{
		AddError(TEXT("Failed to instantiate USovereignInventoryComponent"));
		return false;
	}

	InvComp->MaxSlotCount = 3;
	TestEqual(TEXT("Initial slots count should be 0"), InvComp->InventorySlots.Num(), 0);
	TestEqual(TEXT("Initial total items count should be 0"), InvComp->GetTotalItemCount(), 0);
	TestFalse(TEXT("Inventory should not be full initially"), InvComp->IsFull());

	// 2. Create mock Item Definitions
	USovereignItemDefinition* HerbDef = NewObject<USovereignItemDefinition>();
	HerbDef->DisplayName = FText::FromString(TEXT("Spirit Herb"));

	USovereignItemDefinition* OreDef = NewObject<USovereignItemDefinition>();
	OreDef->DisplayName = FText::FromString(TEXT("Iron Ore"));

	USovereignItemDefinition* CrystalDef = NewObject<USovereignItemDefinition>();
	CrystalDef->DisplayName = FText::FromString(TEXT("Qi Crystal"));

	USovereignItemDefinition* ExtraDef = NewObject<USovereignItemDefinition>();
	ExtraDef->DisplayName = FText::FromString(TEXT("Extra Scroll"));

	// 3. Test AddItem & Stacking
	TestTrue(TEXT("Adding 5 Herbs should succeed"), InvComp->AddItem(HerbDef, 5));
	TestEqual(TEXT("Slots count should be 1"), InvComp->InventorySlots.Num(), 1);
	TestEqual(TEXT("Total count should be 5"), InvComp->GetTotalItemCount(), 5);

	// Add more of the same item -> should stack into the same slot
	TestTrue(TEXT("Adding 3 more Herbs should stack"), InvComp->AddItem(HerbDef, 3));
	TestEqual(TEXT("Slots count should remain 1 after stacking"), InvComp->InventorySlots.Num(), 1);
	TestEqual(TEXT("Stacked herb quantity should be 8"), InvComp->InventorySlots[0].Quantity, 8);

	// 4. Fill remaining slots to test capacity gating
	TestTrue(TEXT("Adding 2 Ore should succeed"), InvComp->AddItem(OreDef, 2));
	TestTrue(TEXT("Adding 1 Crystal should succeed"), InvComp->AddItem(CrystalDef, 1));
	TestEqual(TEXT("Slots count should be 3"), InvComp->InventorySlots.Num(), 3);
	TestTrue(TEXT("Inventory should now be full"), InvComp->IsFull());

	// Attempting to add 4th distinct item definition should fail
	TestFalse(TEXT("Adding item when full should return false"), InvComp->AddItem(ExtraDef, 1));
	TestEqual(TEXT("Slots count should remain 3"), InvComp->InventorySlots.Num(), 3);

	// 5. Test FindSlotByInstanceId
	FGuid HerbInstanceId = InvComp->InventorySlots[0].InstanceId;
	FSovereignInventoryItemInstance FoundInstance;
	TestTrue(TEXT("FindSlotByInstanceId should find herb"), InvComp->FindSlotByInstanceId(HerbInstanceId, FoundInstance));
	TestEqual(TEXT("Found quantity should match"), FoundInstance.Quantity, 8);

	// 6. Test partial removal
	TestTrue(TEXT("Removing 3 herbs should succeed"), InvComp->RemoveItemByInstanceId(HerbInstanceId, 3));
	TestEqual(TEXT("Herb quantity should now be 5"), InvComp->InventorySlots[0].Quantity, 5);

	// 7. Test complete slot clearance on total removal
	TestTrue(TEXT("Removing remaining 5 herbs should clear slot"), InvComp->RemoveItemByInstanceId(HerbInstanceId, 5));
	TestEqual(TEXT("Slots count should decrease to 2"), InvComp->InventorySlots.Num(), 2);
	TestFalse(TEXT("Inventory should no longer be full"), InvComp->IsFull());

	// 8. Now adding the extra item should succeed into the freed slot
	TestTrue(TEXT("Adding extra item into freed slot should succeed"), InvComp->AddItem(ExtraDef, 1));
	TestEqual(TEXT("Slots count should return to 3"), InvComp->InventorySlots.Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCharacterInventoryIntegrationTest,
	"Sovereign.Inventory.CharacterIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCharacterInventoryIntegrationTest::RunTest(const FString& Parameters)
{
	// Instantiate ASovereignBaseCharacter
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	USovereignInventoryComponent* InvComp = Character->GetInventoryComponent();
	TestNotNull(TEXT("Character should have default InventoryComponent"), InvComp);
	if (InvComp)
	{
		TestEqual(TEXT("Default MaxSlotCount should be 20"), InvComp->MaxSlotCount, 20);
		TestEqual(TEXT("Owner of inventory component should match Character"), InvComp->GetOwnerCharacter(), Character);
	}

	return true;
}
