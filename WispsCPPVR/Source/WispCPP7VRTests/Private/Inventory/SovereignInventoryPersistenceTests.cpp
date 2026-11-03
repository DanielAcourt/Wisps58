// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Inventory/SovereignInventoryBroker.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN INVENTORY PERSISTENCE TESTS - B-069 Verification
 * ============================================================================
 * Verifies USovereignInventoryBroker serialization (OnSave) into "Sovereign.Inventory"
 * JSON and deserialization (OnLoad), ensuring slots, quantities, InstanceId GUIDs,
 * dynamic tags, and dynamic properties are fully restored.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignInventoryPersistenceTest,
	"Sovereign.Inventory.PersistenceRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignInventoryPersistenceTest::RunTest(const FString& Parameters)
{
	// 1. Setup Inventory Component & Broker
	USovereignInventoryComponent* SourceInv = NewObject<USovereignInventoryComponent>();
	USovereignInventoryBroker* Broker = NewObject<USovereignInventoryBroker>();
	if (!SourceInv || !Broker)
	{
		AddError(TEXT("Failed to instantiate inventory or broker"));
		return false;
	}

	Broker->Initialize(SourceInv);
	SourceInv->MaxSlotCount = 15;

	// 2. Populate Slots with custom state
	USovereignItemDefinition* DragonEggDef = NewObject<USovereignItemDefinition>();
	DragonEggDef->DisplayName = FText::FromString(TEXT("Prismatic Dragon Egg"));

	USovereignItemDefinition* HealingSalveDef = NewObject<USovereignItemDefinition>();
	HealingSalveDef->DisplayName = FText::FromString(TEXT("Healing Salve"));

	SourceInv->AddItem(DragonEggDef, 1);
	SourceInv->AddItem(HealingSalveDef, 5);

	// Add dynamic tags and custom properties to slot 0
	FGuid EggGuid = SourceInv->InventorySlots[0].InstanceId;
	SourceInv->InventorySlots[0].DynamicTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.State.Equipped"), false));
	SourceInv->InventorySlots[0].DynamicProperties.Add(TEXT("Warmth"), TEXT("0.85"));
	SourceInv->InventorySlots[0].DynamicProperties.Add(TEXT("ParentDragon"), TEXT("SilverDraco"));

	// 3. Serialize via OnSave
	TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
	Broker->OnSave(RootJson);

	TestTrue(TEXT("RootJson should have Sovereign.Inventory key"), RootJson->HasField(TEXT("Sovereign.Inventory")));
	TSharedPtr<FJsonObject> InvJson = RootJson->GetObjectField(TEXT("Sovereign.Inventory"));
	TestNotNull(TEXT("Sovereign.Inventory object is valid"), InvJson.Get());
	TestEqual(TEXT("MaxSlotCount is 15"), InvJson->GetIntegerField(TEXT("MaxSlotCount")), 15);

	// 4. Mutate / Clear Source Inventory
	SourceInv->InventorySlots.Empty();
	SourceInv->MaxSlotCount = 5;
	TestEqual(TEXT("Cleared slots count should be 0"), SourceInv->InventorySlots.Num(), 0);

	// 5. Restore via OnLoad
	Broker->OnLoad(RootJson);

	TestEqual(TEXT("Restored MaxSlotCount should be 15"), SourceInv->MaxSlotCount, 15);
	TestEqual(TEXT("Restored slots count should be 2"), SourceInv->InventorySlots.Num(), 2);

	// 6. Verify restored slot 0 (Dragon Egg)
	const FSovereignInventoryItemInstance& RestoredEgg = SourceInv->InventorySlots[0];
	TestEqual(TEXT("Restored InstanceId matches"), RestoredEgg.InstanceId, EggGuid);
	TestEqual(TEXT("Restored Quantity is 1"), RestoredEgg.Quantity, 1);
	TestTrue(TEXT("Restored DynamicTags contains Item.State.Equipped"),
		RestoredEgg.DynamicTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Item.State.Equipped"), false)));
	TestEqual(TEXT("Restored DynamicProperties Warmth matches"), RestoredEgg.DynamicProperties.FindRef(TEXT("Warmth")), FString(TEXT("0.85")));
	TestEqual(TEXT("Restored DynamicProperties ParentDragon matches"), RestoredEgg.DynamicProperties.FindRef(TEXT("ParentDragon")), FString(TEXT("SilverDraco")));

	// 7. Verify restored slot 1 (Healing Salve)
	const FSovereignInventoryItemInstance& RestoredSalve = SourceInv->InventorySlots[1];
	TestEqual(TEXT("Restored Salve Quantity is 5"), RestoredSalve.Quantity, 5);

	return true;
}
