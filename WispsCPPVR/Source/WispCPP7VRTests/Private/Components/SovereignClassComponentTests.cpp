// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Components/SovereignClassComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Dom/JsonObject.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN CLASS COMPONENT TESTS - B-073 Verification
 * ============================================================================
 * Verifies class unlocking, CP capacity gating, simultaneous multi-classing,
 * combat multiplier aggregation, and atomic JSON persistence roundtrip.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignClassComponentTest,
	"Sovereign.CombatClass.ComponentOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignClassComponentTest::RunTest(const FString& Parameters)
{
	USovereignClassComponent* ClassComp = NewObject<USovereignClassComponent>();
	if (!ClassComp)
	{
		AddError(TEXT("Failed to instantiate USovereignClassComponent"));
		return false;
	}

	ClassComp->MaxCP = 5;

	// Ensure tags are added to manager if running in isolated automation harness
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Class.Combat.Warrior"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Class.Combat.Ninja"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Class.Combat.Archmage"));

	// 1. Define Test Classes
	FGameplayTag WarriorTag = FGameplayTag::RequestGameplayTag(TEXT("Class.Combat.Warrior"), false);
	FSovereignClassData WarriorData;
	WarriorData.ClassTag = WarriorTag;
	WarriorData.ClassDisplayName = FText::FromString(TEXT("Warrior"));
	WarriorData.ClassLevel = 5;
	WarriorData.CPCost = 2;
	WarriorData.StrengthMultiplier = 1.25f;
	WarriorData.DexterityMultiplier = 1.0f;

	FGameplayTag NinjaTag = FGameplayTag::RequestGameplayTag(TEXT("Class.Combat.Ninja"), false);
	FSovereignClassData NinjaData;
	NinjaData.ClassTag = NinjaTag;
	NinjaData.ClassDisplayName = FText::FromString(TEXT("Ninja"));
	NinjaData.ClassLevel = 3;
	NinjaData.CPCost = 3;
	NinjaData.StrengthMultiplier = 1.0f;
	NinjaData.DexterityMultiplier = 1.30f;

	FGameplayTag ArchmageTag = FGameplayTag::RequestGameplayTag(TEXT("Class.Combat.Archmage"), false);
	FSovereignClassData ArchmageData;
	ArchmageData.ClassTag = ArchmageTag;
	ArchmageData.ClassDisplayName = FText::FromString(TEXT("Archmage"));
	ArchmageData.ClassLevel = 1;
	ArchmageData.CPCost = 4;
	ArchmageData.IntelligenceMultiplier = 1.50f;

	// 2. Unlock Classes
	TestTrue(TEXT("Unlocking Warrior should succeed"), ClassComp->UnlockClass(WarriorData));
	TestTrue(TEXT("Unlocking Ninja should succeed"), ClassComp->UnlockClass(NinjaData));
	TestTrue(TEXT("Unlocking Archmage should succeed"), ClassComp->UnlockClass(ArchmageData));
	TestEqual(TEXT("Unlocked classes count should be 3"), ClassComp->UnlockedClasses.Num(), 3);

	// 3. Test Multi-Class Equipping & CP Capacity
	TestEqual(TEXT("Initial Used CP should be 0"), ClassComp->GetUsedCP(), 0);
	TestEqual(TEXT("Initial Available CP should be 5"), ClassComp->GetAvailableCP(), 5);

	// Equip Warrior (Cost: 2) -> Used: 2, Avail: 3
	TestTrue(TEXT("Equipping Warrior should succeed"), ClassComp->EquipClass(WarriorTag));
	TestEqual(TEXT("Used CP after Warrior should be 2"), ClassComp->GetUsedCP(), 2);
	TestEqual(TEXT("Available CP should be 3"), ClassComp->GetAvailableCP(), 3);
	TestTrue(TEXT("Warrior is equipped"), ClassComp->IsClassEquipped(WarriorTag));

	// Equip Ninja (Cost: 3) -> Used: 5, Avail: 0
	TestTrue(TEXT("Equipping Ninja should succeed (Multi-Classing)"), ClassComp->EquipClass(NinjaTag));
	TestEqual(TEXT("Used CP after Ninja should be 5"), ClassComp->GetUsedCP(), 5);
	TestEqual(TEXT("Available CP should be 0"), ClassComp->GetAvailableCP(), 0);

	// Attempting to equip Archmage (Cost: 4) should fail due to CP cap
	TestFalse(TEXT("Equipping Archmage should fail due to exceeding MaxCP"), ClassComp->EquipClass(ArchmageTag));

	// 4. Verify Stat Multipliers from Multi-Classing
	TestEqual(TEXT("Total Strength Multiplier should be 1.25x"), ClassComp->GetTotalStrengthMultiplier(), 1.25f);
	TestEqual(TEXT("Total Dexterity Multiplier should be 1.30x"), ClassComp->GetTotalDexterityMultiplier(), 1.30f);

	// 5. Test Unequipping
	TestTrue(TEXT("Unequipping Warrior should succeed"), ClassComp->UnequipClass(WarriorTag));
	TestEqual(TEXT("Used CP after unequipping Warrior should be 3"), ClassComp->GetUsedCP(), 3);
	TestEqual(TEXT("Available CP should be 2"), ClassComp->GetAvailableCP(), 2);

	// 6. Test JSON Persistence Roundtrip
	TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
	ClassComp->OnSave(RootJson);

	TestTrue(TEXT("RootJson should have Sovereign.Classes key"), RootJson->HasField(TEXT("Sovereign.Classes")));

	// Mutate component before load
	ClassComp->EquippedClasses.Reset();
	ClassComp->UnlockedClasses.Empty();
	ClassComp->MaxCP = 1;

	// Load from JSON
	ClassComp->OnLoad(RootJson);

	TestEqual(TEXT("Restored MaxCP is 5"), ClassComp->MaxCP, 5);
	TestEqual(TEXT("Restored UnlockedClasses count is 3"), ClassComp->UnlockedClasses.Num(), 3);
	TestTrue(TEXT("Restored EquippedClasses has NinjaTag"), ClassComp->IsClassEquipped(NinjaTag));
	TestFalse(TEXT("Restored EquippedClasses does not have WarriorTag"), ClassComp->IsClassEquipped(WarriorTag));
	const FSovereignClassData* RestoredNinja = ClassComp->UnlockedClasses.Find(NinjaTag);
	TestNotNull(TEXT("Restored Ninja ClassData should be found in UnlockedClasses"), RestoredNinja);
	if (RestoredNinja)
	{
		TestEqual(TEXT("Restored Ninja ClassLevel is 3"), RestoredNinja->ClassLevel, 3);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCharacterClassIntegrationTest,
	"Sovereign.CombatClass.CharacterIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCharacterClassIntegrationTest::RunTest(const FString& Parameters)
{
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	USovereignClassComponent* ClassComp = Character->GetClassComponent();
	TestNotNull(TEXT("Character should instantiate ClassComponent by default"), ClassComp);
	if (ClassComp)
	{
		TestEqual(TEXT("Default MaxCP should be 5"), ClassComp->MaxCP, 5);
	}

	return true;
}
