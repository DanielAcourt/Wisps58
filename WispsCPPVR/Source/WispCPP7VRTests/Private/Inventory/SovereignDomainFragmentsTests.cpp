// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignLevelComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN DOMAIN FRAGMENTS TESTS - B-068 Verification
 * ============================================================================
 * Verifies USovereignItemFragment_Consumable replenishes BioComponent Hunger
 * and feeds attribute XP into USovereignLevelComponent, verifies Weight calculations,
 * and verifies USovereignItemFragment_Spiritual adds Qi to USovereignQiComponent.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignConsumableFragmentTest,
	"Sovereign.Inventory.ConsumableFragment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignConsumableFragmentTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate Character Vessel
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	USovereignBioComponent* Bio = Character->GetBioComponent();
	USovereignLevelComponent* Level = Character->GetLevelComponent();
	USovereignInventoryComponent* Inv = Character->GetInventoryComponent();

	TestNotNull(TEXT("BioComponent exists"), Bio);
	TestNotNull(TEXT("LevelComponent exists"), Level);
	TestNotNull(TEXT("InventoryComponent exists"), Inv);

	if (!Bio || !Level || !Inv)
	{
		return false;
	}

	// Set baseline vitals
	Bio->Hunger = 20.0f;
	Level->StrengthXP = 0.0;

	// 2. Create Item Definition with Consumable Fragment
	USovereignItemDefinition* RoastedMeatDef = NewObject<USovereignItemDefinition>();
	RoastedMeatDef->DisplayName = FText::FromString(TEXT("Roasted Prime Beast Meat"));

	USovereignItemFragment_Consumable* ConsumableFrag = NewObject<USovereignItemFragment_Consumable>(RoastedMeatDef);
	ConsumableFrag->HungerRestore = 35.0f;
	ConsumableFrag->StrengthXP = 2.5;
	RoastedMeatDef->Fragments.Add(ConsumableFrag);

	// 3. Add to inventory
	TestTrue(TEXT("Adding roasted meat should succeed"), Inv->AddItem(RoastedMeatDef, 2));
	FGuid MeatId = Inv->InventorySlots[0].InstanceId;

	// 4. Consume 1 unit
	TestTrue(TEXT("Consuming roasted meat should succeed"), Inv->ConsumeItem(MeatId, 1));

	// 5. Verify Bio Hunger increased from 20 to 55 (20 + 35)
	TestEqual(TEXT("Hunger should replenish by 35.0f"), Bio->Hunger, 55.0f);

	// 6. Verify LevelComponent rollover: 2.5 XP awards 2 full levels into Strength and leaves 0.5 XP in accumulator
	TestEqual(TEXT("StrengthXP after rollover should be 0.5"), Level->StrengthXP, 0.5);
	if (USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent())
	{
		TestEqual(TEXT("Strength attribute increased by 2 from integer rollover"), AttrComp->Strength, 12);
	}

	// 7. Verify inventory quantity decremented from 2 to 1
	TestEqual(TEXT("Remaining quantity should be 1"), Inv->InventorySlots[0].Quantity, 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignWeightFragmentTest,
	"Sovereign.Inventory.WeightFragment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignWeightFragmentTest::RunTest(const FString& Parameters)
{
	USovereignItemFragment_Weight* WeightFrag = NewObject<USovereignItemFragment_Weight>();
	if (!WeightFrag)
	{
		AddError(TEXT("Failed to instantiate USovereignItemFragment_Weight"));
		return false;
	}

	WeightFrag->UnitWeight = 2.5f;
	WeightFrag->MaxStackSize = 50;

	TestEqual(TEXT("Stack weight for 1 unit"), WeightFrag->GetStackWeight(1), 2.5f);
	TestEqual(TEXT("Stack weight for 10 units"), WeightFrag->GetStackWeight(10), 25.0f);
	TestEqual(TEXT("Stack weight for 0 units clamps to 1"), WeightFrag->GetStackWeight(0), 2.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignSpiritualFragmentTest,
	"Sovereign.Inventory.SpiritualFragment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignSpiritualFragmentTest::RunTest(const FString& Parameters)
{
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	USovereignQiComponent* QiComp = Character->FindComponentByClass<USovereignQiComponent>();
	TestNotNull(TEXT("QiComponent exists"), QiComp);

	if (!QiComp)
	{
		return false;
	}

	QiComp->CurrentQi = 10.0f;
	QiComp->MaxQiCapacity = 100.0f;

	USovereignItemFragment_Spiritual* SpiritualFrag = NewObject<USovereignItemFragment_Spiritual>();
	SpiritualFrag->QiGive = 25.0f;

	// Consume 1 unit directly
	TestTrue(TEXT("Spiritual fragment OnConsumed should succeed"), SpiritualFrag->OnConsumed(Character, 1));
	TestEqual(TEXT("CurrentQi should increase by 25"), QiComp->CurrentQi, 35.0f);

	return true;
}
