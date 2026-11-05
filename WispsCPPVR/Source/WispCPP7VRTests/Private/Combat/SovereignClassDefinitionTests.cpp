// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21

#include "Misc/AutomationTest.h"
#include "Combat/SovereignClassDefinition.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Components/SovereignClassComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "GameplayTagsManager.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN CLASS DEFINITION & PREREQUISITE TESTS - Ticket CS-001 Verification
 * ============================================================================
 * Verifies USovereignClassDefinition creation, PrimaryAssetId generation,
 * attribute thresholds and class level prerequisite validation, and unlocking
 * directly via USovereignClassComponent::UnlockClassFromDefinition.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignClassDefinitionPrereqTest,
	"Sovereign.Combat.ClassDefinitionPrereq",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignClassDefinitionPrereqTest::RunTest(const FString& Parameters)
{
	// 1. Register tags for isolated test execution
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Class.Combat.Adventurer"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Class.Combat.Warrior"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Class.Combat.Rogue"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Class.Combat.Ninja"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Skill.Combat.ShadowStrike"));

	FGameplayTag RogueTag = FGameplayTag::RequestGameplayTag(FName("Class.Combat.Rogue"));
	FGameplayTag WarriorTag = FGameplayTag::RequestGameplayTag(FName("Class.Combat.Warrior"));
	FGameplayTag NinjaTag = FGameplayTag::RequestGameplayTag(FName("Class.Combat.Ninja"));
	FGameplayTag ShadowStrikeTag = FGameplayTag::RequestGameplayTag(FName("Skill.Combat.ShadowStrike"));

	// 2. Instantiate Skill and Class Definitions
	USovereignSkillDefinition* SkillDef = NewObject<USovereignSkillDefinition>();
	TestNotNull(TEXT("SkillDef should be created"), SkillDef);
	const_cast<FGameplayTag&>(SkillDef->SkillTag) = ShadowStrikeTag;
	const_cast<FText&>(SkillDef->SkillDisplayName) = FText::FromString(TEXT("Shadow Strike"));

	USovereignClassDefinition* NinjaClassDef = NewObject<USovereignClassDefinition>();
	TestNotNull(TEXT("NinjaClassDef should be created"), NinjaClassDef);

	// Setup Ninja Definition
	const_cast<FGameplayTag&>(NinjaClassDef->ClassTag) = NinjaTag;
	const_cast<FText&>(NinjaClassDef->ClassDisplayName) = FText::FromString(TEXT("Ninja"));
	const_cast<int32&>(NinjaClassDef->CPCost) = 2;
	const_cast<float&>(NinjaClassDef->DexterityMultiplier) = 1.30f;
	const_cast<float&>(NinjaClassDef->StrengthMultiplier) = 1.10f;

	// Requirements: Rogue Level 5, Warrior Level 5, STR 12, DEX 15
	TArray<FSovereignClassPrerequisite> Prereqs;
	Prereqs.Add(FSovereignClassPrerequisite(RogueTag, 5));
	Prereqs.Add(FSovereignClassPrerequisite(WarriorTag, 5));
	const_cast<TArray<FSovereignClassPrerequisite>&>(NinjaClassDef->PrerequisiteClasses) = Prereqs;
	const_cast<int32&>(NinjaClassDef->RequiredStrength) = 12;
	const_cast<int32&>(NinjaClassDef->RequiredDexterity) = 15;

	// Granted skill soft pointer
	const_cast<TArray<TSoftObjectPtr<USovereignSkillDefinition>>&>(NinjaClassDef->GrantedSkills).Add(SkillDef);

	// 3. Verify Primary Asset ID
	FPrimaryAssetId PrimaryId = NinjaClassDef->GetPrimaryAssetId();
	TestEqual(TEXT("PrimaryAssetType must be SovereignClass"), PrimaryId.PrimaryAssetType.ToString(), TEXT("SovereignClass"));
	TestEqual(TEXT("PrimaryAssetName matches asset name"), PrimaryId.PrimaryAssetName, NinjaClassDef->GetFName());

	// 4. Create Character and Test Prerequisite Failure / Success
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	TestNotNull(TEXT("Character must be valid"), Character);

	USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent();
	TestNotNull(TEXT("AttributeComponent must be present"), AttrComp);

	USovereignClassComponent* ClassComp = Character->GetClassComponent();
	TestNotNull(TEXT("ClassComponent must be present"), ClassComp);

	// Initially: Attributes are below requirements (STR < 12, DEX < 15) and Rogue/Warrior not unlocked
	TestFalse(TEXT("Cannot unlock Ninja when requirements not met"), NinjaClassDef->CanUnlockClass(Character));

	// Raise attributes to meet minimums
	AttrComp->SetStrength(14);
	AttrComp->SetDexterity(16);

	// Still fails because Rogue and Warrior are not unlocked yet
	TestFalse(TEXT("Cannot unlock Ninja without prerequisite classes"), NinjaClassDef->CanUnlockClass(Character));

	// Unlock Rogue at level 1 and Warrior at level 1
	FSovereignClassData RogueData;
	RogueData.ClassTag = RogueTag;
	RogueData.ClassLevel = 1;
	ClassComp->UnlockClass(RogueData);

	FSovereignClassData WarriorData;
	WarriorData.ClassTag = WarriorTag;
	WarriorData.ClassLevel = 1;
	ClassComp->UnlockClass(WarriorData);

	// Still fails because levels are 1, but requirement is 5
	TestFalse(TEXT("Cannot unlock Ninja when Rogue/Warrior level is below 5"), NinjaClassDef->CanUnlockClass(Character));

	// Level up Rogue to 5, Warrior still at 1 -> still fails
	ClassComp->AddClassLevel(RogueTag, 4); // 1 + 4 = 5
	TestFalse(TEXT("Cannot unlock Ninja when Warrior level is below 5"), NinjaClassDef->CanUnlockClass(Character));

	// Level up Warrior to 5 -> Now all prerequisites are satisfied!
	ClassComp->AddClassLevel(WarriorTag, 4); // 1 + 4 = 5
	TestTrue(TEXT("Can unlock Ninja when both Rogue lvl 5, Warrior lvl 5, STR 14, DEX 16 are met"), NinjaClassDef->CanUnlockClass(Character));

	// 5. Verify UnlockClassFromDefinition
	bool bUnlocked = ClassComp->UnlockClassFromDefinition(NinjaClassDef);
	TestTrue(TEXT("UnlockClassFromDefinition succeeds"), bUnlocked);
	TestTrue(TEXT("Ninja is now unlocked in ClassComponent"), ClassComp->UnlockedClasses.Contains(NinjaTag));

	const FSovereignClassData* UnlockedNinja = ClassComp->UnlockedClasses.Find(NinjaTag);
	TestNotNull(TEXT("UnlockedNinja data must exist"), UnlockedNinja);
	TestEqual(TEXT("Ninja CPCost matches definition"), UnlockedNinja->CPCost, 2);
	TestEqual(TEXT("Ninja DexterityMultiplier matches definition"), UnlockedNinja->DexterityMultiplier, 1.30f);
	TestEqual(TEXT("Ninja StrengthMultiplier matches definition"), UnlockedNinja->StrengthMultiplier, 1.10f);

	return true;
}
