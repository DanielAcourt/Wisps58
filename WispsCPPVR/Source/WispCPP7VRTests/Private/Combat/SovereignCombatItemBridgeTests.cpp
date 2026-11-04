// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "GameplayTagsManager.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN IN-COMBAT ITEM BRIDGE TESTS - B-081 Verification
 * ============================================================================
 * Verifies friendly healing/Qi potion usage on allies, offensive grenade usage
 * on enemies, target alignment enforcement, inventory stack decrements, and turn conclusion.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCombatItemBridgeTest,
	"Sovereign.Combat.ItemBridgeOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCombatItemBridgeTest::RunTest(const FString& Parameters)
{
	ASovereignBattleManager* BattleMgr = NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to instantiate ASovereignBattleManager"));
		return false;
	}

	// 1. Setup Combatants
	ASovereignBaseCharacter* DragonUser = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* WoundedAlly = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* EnemyTarget = NewObject<ASovereignBaseCharacter>();

	USovereignInventoryComponent* Inventory = DragonUser->GetInventoryComponent();
	if (!Inventory)
	{
		AddError(TEXT("DragonUser must possess USovereignInventoryComponent"));
		return false;
	}

	// Configure Wounded Ally
	if (USovereignAttributeComponent* Attr = WoundedAlly->GetAttributeComponent())
	{
		Attr->MaxHealth = 100.0f;
		Attr->CurrentHealth = 40.0f; // Needs 60 healing
	}

	// Configure Enemy Target
	if (USovereignAttributeComponent* Attr = EnemyTarget->GetAttributeComponent())
	{
		Attr->MaxHealth = 150.0f;
		Attr->CurrentHealth = 150.0f;
	}

	TArray<ASovereignBaseCharacter*> Players = { DragonUser, WoundedAlly };
	TArray<ASovereignBaseCharacter*> Enemies = { EnemyTarget };
	BattleMgr->InitializeEncounter(Players, Enemies, true);

	// 2. Setup Healing Potion Item
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Item.Potion.Health"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Item.Weapon.Grenade"));

	USovereignItemDefinition* PotionDef = NewObject<USovereignItemDefinition>();
	PotionDef->ItemTag = FGameplayTag::RequestGameplayTag(TEXT("Item.Potion.Health"), false);
	PotionDef->DisplayName = FText::FromString(TEXT("Healing Draught"));

	USovereignItemFragment_CombatEffect* PotionCombatFrag = NewObject<USovereignItemFragment_CombatEffect>(PotionDef);
	PotionCombatFrag->DirectHealing = 40.0f;
	PotionCombatFrag->bCanTargetAllies = true;
	PotionCombatFrag->bCanTargetEnemies = false;
	PotionDef->Fragments.Add(PotionCombatFrag);

	// Setup Offensive Spirit Grenade Item
	USovereignItemDefinition* GrenadeDef = NewObject<USovereignItemDefinition>();
	GrenadeDef->ItemTag = FGameplayTag::RequestGameplayTag(TEXT("Item.Weapon.Grenade"), false);
	GrenadeDef->DisplayName = FText::FromString(TEXT("Spirit Fire Bomb"));

	USovereignItemFragment_CombatEffect* GrenadeCombatFrag = NewObject<USovereignItemFragment_CombatEffect>(GrenadeDef);
	GrenadeCombatFrag->DirectDamage = 50.0f;
	GrenadeCombatFrag->bCanTargetAllies = false;
	GrenadeCombatFrag->bCanTargetEnemies = true;
	GrenadeDef->Fragments.Add(GrenadeCombatFrag);

	// 3. Add Items to DragonUser's Inventory
	Inventory->AddItem(PotionDef, 2);  // 2 potions
	Inventory->AddItem(GrenadeDef, 1); // 1 grenade

	TestEqual(TEXT("Inventory should contain 2 slots"), Inventory->InventorySlots.Num(), 2);
	const FGuid PotionGuid = Inventory->InventorySlots[0].InstanceId;
	const FGuid GrenadeGuid = Inventory->InventorySlots[1].InstanceId;

	// 4. Test Target Alignment Violation: Using Grenade on Friendly Ally must fail
	const bool bIllegalGrenade = BattleMgr->ExecuteItemAction(DragonUser, GrenadeGuid, WoundedAlly);
	TestFalse(TEXT("Cannot throw grenade at friendly ally"), bIllegalGrenade);
	TestEqual(TEXT("Grenade quantity must remain 1"), Inventory->InventorySlots[1].Quantity, 1);
	TestEqual(TEXT("Ally health remains 40.0"), WoundedAlly->GetAttributeComponent()->CurrentHealth, 40.0f);

	// 5. Test Friendly Healing Potion on Ally
	const bool bPotionSuccess = BattleMgr->ExecuteItemAction(DragonUser, PotionGuid, WoundedAlly);
	TestTrue(TEXT("Healing potion successfully used on ally"), bPotionSuccess);
	TestEqual(TEXT("Ally health restored by 40 (now 80.0)"), WoundedAlly->GetAttributeComponent()->CurrentHealth, 80.0f);
	TestEqual(TEXT("Potion stack decremented by 1 (now 1)"), Inventory->InventorySlots[0].Quantity, 1);

	// 6. Test Offensive Grenade on Enemy
	const bool bGrenadeSuccess = BattleMgr->ExecuteItemAction(DragonUser, GrenadeGuid, EnemyTarget);
	TestTrue(TEXT("Grenade successfully thrown at enemy"), bGrenadeSuccess);
	TestEqual(TEXT("Enemy health damaged by 50 (150 -> 100.0)"), EnemyTarget->GetAttributeComponent()->CurrentHealth, 100.0f);

	// Grenade quantity was 1, so the slot should now be completely removed from inventory
	TestEqual(TEXT("Inventory should now have 1 slot left after grenade was exhausted"), Inventory->InventorySlots.Num(), 1);

	// 7. Test Attempting to use exhausted item fails
	const bool bExhaustedSuccess = BattleMgr->ExecuteItemAction(DragonUser, GrenadeGuid, EnemyTarget);
	TestFalse(TEXT("Cannot use exhausted item instance"), bExhaustedSuccess);

	return true;
}
