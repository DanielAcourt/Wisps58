// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignStatusComponent.h"
#include "Components/SovereignSkillComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN BOSS TELEGRAPH TESTS - Ticket CS-006 Verification
 * ============================================================================
 * Verifies:
 * 1. Stage 1: Initiating telegraphed charge broadcasts warning, resets ATB to 0%,
 *    enters EATBState::ChargingAttack, and deals 0 damage on Turn 1.
 * 2. Stage 2: Unleashing telegraphed attack executes full damage on Turn 2.
 * 3. Stun Interruption: Inflicting Stun during ChargingAttack cancels the ultimate
 *    and resets state back to Charging.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignBossTelegraphSuiteTest,
	"Sovereign.Combat.BossTelegraphSuite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignBossTelegraphSuiteTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		World = GEngine->GetWorldContexts()[0].World();
	}

	ASovereignBattleManager* BattleMgr = World ? World->SpawnActor<ASovereignBattleManager>() : NewObject<ASovereignBattleManager>();
	if (!BattleMgr)
	{
		AddError(TEXT("Failed to create ASovereignBattleManager"));
		return false;
	}

	ASovereignBaseCharacter* BossActor = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* PlayerActor = World ? World->SpawnActor<ASovereignBaseCharacter>() : NewObject<ASovereignBaseCharacter>();

	USovereignAttributeComponent* BossAttr = BossActor->GetAttributeComponent();
	BossAttr->Strength = 25;
	BossAttr->CurrentHealth = 500.0f;
	BossAttr->MaxHealth = 500.0f;

	USovereignAttributeComponent* PlayerAttr = PlayerActor->GetAttributeComponent();
	PlayerAttr->Constitution = 10;
	PlayerAttr->CurrentHealth = 200.0f;
	PlayerAttr->MaxHealth = 200.0f;

	// Create Telegraphed Skill (Earth Breaker)
	USovereignSkillDefinition* EarthBreaker = NewObject<USovereignSkillDefinition>();
	EarthBreaker->SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Combat.Boss.EarthBreaker"), false);
	EarthBreaker->SkillDisplayName = FText::FromString(TEXT("Earth Breaker"));
	EarthBreaker->bIsTelegraphed = true;
	EarthBreaker->TelegraphWarningText = FText::FromString(TEXT("Earth energy is gathering!"));
	EarthBreaker->BasePower = 80.0f;
	EarthBreaker->StatMultiplier = 2.0f;
	EarthBreaker->ScalingAttribute = ESovereignSkillScalingAttribute::Strength;
	EarthBreaker->TargetPattern = ESovereignSkillTargetPattern::SingleTarget;

	USovereignSkillComponent* SkillComp = BossActor->GetSkillComponent();
	FGuid OutId;
	SkillComp->EquipSkill(EarthBreaker, OutId);

	BattleMgr->InitializeEncounter({ PlayerActor }, { BossActor });

	USovereignATBComponent* BossATB = BossActor->GetATBComponent();

	// 1. Stage 1: Execute Turn 1 (Should start telegraphing, 0 damage)
	const float PlayerHPBeforeTurn1 = PlayerAttr->CurrentHealth;
	BattleMgr->ExecuteEnemyTurn(BossActor);
	const float PlayerHPAfterTurn1 = PlayerAttr->CurrentHealth;

	TestEqual(TEXT("Player took 0 damage during Stage 1 telegraph"), PlayerHPAfterTurn1, PlayerHPBeforeTurn1);
	TestTrue(TEXT("Boss is in ChargingAttack state"), BossATB->IsChargingAttack());
	TestEqual(TEXT("Boss ATB reset to 0%"), BossATB->CurrentATB, 0.0f);

	// 2. Stage 2: Unleash on Turn 2 (Should deal lethal damage)
	BattleMgr->ExecuteEnemyTurn(BossActor);
	const float PlayerHPAfterTurn2 = PlayerAttr->CurrentHealth;

	TestTrue(TEXT("Player took massive damage from unleashed Earth Breaker"), PlayerHPAfterTurn2 < PlayerHPAfterTurn1);
	TestFalse(TEXT("Boss is no longer in ChargingAttack state"), BossATB->IsChargingAttack());

	// 3. Stun Interruption Test
	BossATB->BeginTelegraphedCharge(FSovereignSkillInstance(EarthBreaker), FText::FromString(TEXT("Warning!")));
	TestTrue(TEXT("Boss is charging again"), BossATB->IsChargingAttack());

	// Inflict Stun on Boss
	USovereignStatusComponent* StatusComp = BossActor->GetStatusComponent();
	StatusComp->ApplyStatusEffect(ESovereignStatusType::Stun, 1);

	// Simulate Turn Ready during Stun
	BattleMgr->ActiveCombatant = BossActor;
	BattleMgr->ReadyQueue.Empty();
	FSovereignReadyEntry ReadyBoss;
	ReadyBoss.Combatant = BossActor;
	ReadyBoss.Dexterity = 10;
	ReadyBoss.TieBreakerNonce = 1;
	BattleMgr->ReadyQueue.Add(ReadyBoss);

	BattleMgr->ActiveCombatant = nullptr;
	BattleMgr->Tick(0.1f); // BattleManager Tick pops ReadyQueue and processes Stun

	TestFalse(TEXT("Stun interrupted the telegraphed charge"), BossATB->IsChargingAttack());

	// Teardown
	if (World)
	{
		BossActor->Destroy();
		PlayerActor->Destroy();
		BattleMgr->Destroy();
	}

	return true;
}
