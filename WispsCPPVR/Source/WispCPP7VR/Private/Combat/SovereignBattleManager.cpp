// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignClassComponent.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Combat/SovereignSkillAugment.h"
#include "Combat/SovereignCombatCalculationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

ASovereignBattleManager::ASovereignBattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PlayerGrid.SetNum(GRID_SIZE);
	EnemyGrid.SetNum(GRID_SIZE);
	BattleState = EBattleState::EncounterStarting;
	bIsEscapable = true;
}

void ASovereignBattleManager::BeginPlay()
{
	Super::BeginPlay();
}

void ASovereignBattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BattleState == EBattleState::ActiveCombat)
	{
		// Process next action in queue if active combatant has finished
		if (!ActiveCombatant.IsValid() && ReadyQueue.Num() > 0)
		{
			if (ASovereignBaseCharacter* NextActor = DequeueNextCombatant())
			{
				ActiveCombatant = NextActor;
				SetBattleState(EBattleState::ActionExecuting);

				if (USovereignATBComponent* ATBComp = NextActor->GetATBComponent())
				{
					ATBComp->BeginActionSelection();
				}

				OnCombatantTurnReady.Broadcast(NextActor);
			}
		}
	}
}

void ASovereignBattleManager::InitializeEncounter(const TArray<ASovereignBaseCharacter*>& InPlayerUnits, const TArray<ASovereignBaseCharacter*>& InEnemyUnits, bool bCanFlee)
{
	bIsEscapable = bCanFlee;
	ReadyQueue.Empty();
	ActiveCombatant = nullptr;

	// Populate Player Grid
	for (int32 i = 0; i < GRID_SIZE; ++i)
	{
		PlayerGrid[i] = (i < InPlayerUnits.Num() && InPlayerUnits[i]) ? InPlayerUnits[i] : nullptr;
		if (PlayerGrid[i].IsValid())
		{
			if (USovereignATBComponent* ATB = PlayerGrid[i]->GetATBComponent())
			{
				ATB->ResetATB();
				ATB->OnATBReady.RemoveDynamic(this, &ASovereignBattleManager::OnCombatantATBReady);
				ATB->OnATBReady.AddDynamic(this, &ASovereignBattleManager::OnCombatantATBReady);
				ATB->SetComponentTickEnabled(true);
			}
		}
	}

	// Populate Enemy Grid
	for (int32 i = 0; i < GRID_SIZE; ++i)
	{
		EnemyGrid[i] = (i < InEnemyUnits.Num() && InEnemyUnits[i]) ? InEnemyUnits[i] : nullptr;
		if (EnemyGrid[i].IsValid())
		{
			if (USovereignATBComponent* ATB = EnemyGrid[i]->GetATBComponent())
			{
				ATB->ResetATB();
				ATB->OnATBReady.RemoveDynamic(this, &ASovereignBattleManager::OnCombatantATBReady);
				ATB->OnATBReady.AddDynamic(this, &ASovereignBattleManager::OnCombatantATBReady);
				ATB->SetComponentTickEnabled(true);
			}
		}
	}

	SetBattleState(EBattleState::ActiveCombat);
}

void ASovereignBattleManager::SetBattleState(EBattleState NewState)
{
	if (BattleState != NewState)
	{
		const EBattleState OldState = BattleState;
		BattleState = NewState;
		OnBattleStateChanged.Broadcast(OldState, NewState);

		if (NewState == EBattleState::ActionExecuting)
		{
			// Freeze other combatants' ATBs while action animation is executing
			SetAllCombatantsWaitMode(true);
		}
		else if (NewState == EBattleState::ActiveCombat)
		{
			// Unfreeze ATBs during active combat
			SetAllCombatantsWaitMode(false);
		}
		else if (NewState == EBattleState::Victory || NewState == EBattleState::Defeat || NewState == EBattleState::Fled)
		{
			OnBattleEnded.Broadcast(NewState);

			// Freeze ATB tickers on all combatants
			for (auto& Member : PlayerGrid)
			{
				if (Member.IsValid() && Member->GetATBComponent())
				{
					Member->GetATBComponent()->SetComponentTickEnabled(false);
				}
			}
			for (auto& Enemy : EnemyGrid)
			{
				if (Enemy.IsValid() && Enemy->GetATBComponent())
				{
					Enemy->GetATBComponent()->SetComponentTickEnabled(false);
				}
			}
		}
	}
}

void ASovereignBattleManager::OnCombatantATBReady(ASovereignBaseCharacter* Combatant)
{
	EnqueueReadyCombatant(Combatant);
}

void ASovereignBattleManager::EnqueueReadyCombatant(ASovereignBaseCharacter* Combatant)
{
	if (!Combatant || !IsCombatantAlive(Combatant))
	{
		return;
	}

	// Prevent duplicate enqueue
	for (const FSovereignReadyEntry& Entry : ReadyQueue)
	{
		if (Entry.Combatant.Get() == Combatant)
		{
			return;
		}
	}

	FSovereignReadyEntry NewEntry;
	NewEntry.Combatant = Combatant;
	NewEntry.Dexterity = 0;
	if (const USovereignAttributeComponent* Attr = Combatant->GetAttributeComponent())
	{
		NewEntry.Dexterity = Attr->Dexterity;
	}
	// Assign deterministic tie-breaker nonce at arrival time (combining random roll with unique sequence)
	NewEntry.TieBreakerNonce = FMath::RandRange(1, 1000000);

	ReadyQueue.Add(NewEntry);

	// Sort queue using Strict Weak Ordering (deterministic on Dexterity, with arrival TieBreakerNonce for ties)
	ReadyQueue.Sort([](const FSovereignReadyEntry& A, const FSovereignReadyEntry& B)
	{
		if (A.Dexterity != B.Dexterity)
		{
			return A.Dexterity > B.Dexterity; // Higher DEX acts first
		}
		return A.TieBreakerNonce > B.TieBreakerNonce;
	});
}

ASovereignBaseCharacter* ASovereignBattleManager::DequeueNextCombatant()
{
	while (ReadyQueue.Num() > 0)
	{
		FSovereignReadyEntry Candidate = ReadyQueue[0];
		ReadyQueue.RemoveAt(0);

		if (Candidate.Combatant.IsValid() && IsCombatantAlive(Candidate.Combatant.Get()))
		{
			return Candidate.Combatant.Get();
		}
	}
	return nullptr;
}

void ASovereignBattleManager::SetAllCombatantsWaitMode(bool bWait)
{
	for (auto& Member : PlayerGrid)
	{
		if (Member.IsValid() && Member->GetATBComponent())
		{
			Member->GetATBComponent()->SetWaitModeActive(bWait);
		}
	}
	for (auto& Enemy : EnemyGrid)
	{
		if (Enemy.IsValid() && Enemy->GetATBComponent())
		{
			Enemy->GetATBComponent()->SetWaitModeActive(bWait);
		}
	}
}

void ASovereignBattleManager::ConcludeActiveTurn()
{
	if (ActiveCombatant.IsValid())
	{
		if (USovereignATBComponent* ATB = ActiveCombatant->GetATBComponent())
		{
			ATB->CompleteActionExecution();
		}
	}

	ActiveCombatant = nullptr;

	// Check if this action concluded the battle
	if (!CheckCombatOutcome())
	{
		SetBattleState(EBattleState::ActiveCombat);
	}
}

bool ASovereignBattleManager::CheckCombatOutcome()
{
	const TArray<ASovereignBaseCharacter*> LivingPlayers = GetLivingPlayerUnits();
	const TArray<ASovereignBaseCharacter*> LivingEnemies = GetLivingEnemyUnits();

	if (LivingEnemies.Num() == 0)
	{
		SetBattleState(EBattleState::Victory);
		return true;
	}

	if (LivingPlayers.Num() == 0)
	{
		SetBattleState(EBattleState::Defeat);
		return true;
	}

	return false;
}

void ASovereignBattleManager::ConcludeFled()
{
	if (bIsEscapable)
	{
		SetBattleState(EBattleState::Fled);
	}
}

bool ASovereignBattleManager::IsCombatantAlive(const ASovereignBaseCharacter* Combatant)
{
	if (!Combatant) return false;

	if (const USovereignAttributeComponent* AttrComp = Combatant->GetAttributeComponent())
	{
		return AttrComp->CurrentHealth > 0.0f;
	}
	return true;
}

TArray<ASovereignBaseCharacter*> ASovereignBattleManager::GetLivingPlayerUnits() const
{
	TArray<ASovereignBaseCharacter*> Living;
	for (const auto& Unit : PlayerGrid)
	{
		if (Unit.IsValid() && IsCombatantAlive(Unit.Get()))
		{
			Living.Add(Unit.Get());
		}
	}
	return Living;
}

bool ASovereignBattleManager::CanCastSkill(const ASovereignBaseCharacter* Caster, const USovereignSkillDefinition* SkillDef) const
{
	if (!Caster || !SkillDef)
	{
		return false;
	}

	if (SkillDef->QiCost <= 0.0f)
	{
		return true;
	}

	if (const USovereignQiComponent* QiComp = Caster->GetQiComponent())
	{
		return QiComp->CurrentQi >= SkillDef->QiCost;
	}

	return false;
}

FSovereignCombatEntitySnapshot ASovereignBattleManager::CreateSnapshotForCombatant(const ASovereignBaseCharacter* Combatant, int32 SlotIndex) const
{
	FSovereignCombatEntitySnapshot Snapshot;
	// Libraries use 1-based slots (1-9)
	Snapshot.FormationSlot = SlotIndex + 1;
	Snapshot.bIsAlive = IsCombatantAlive(Combatant);

	if (Combatant)
	{
		if (const USovereignAttributeComponent* Attr = Combatant->GetAttributeComponent())
		{
			Snapshot.Strength = Attr->Strength;
			Snapshot.Dexterity = Attr->Dexterity;
			Snapshot.Constitution = Attr->Constitution;
			Snapshot.Intelligence = Attr->Intelligence;
			Snapshot.Wisdom = Attr->Wisdom;
			Snapshot.Luck = Attr->Luck;
		}
	}
	return Snapshot;
}

bool ASovereignBattleManager::ExecuteSkillAction(ASovereignBaseCharacter* Caster, const FSovereignSkillInstance& SkillInstance, const TArray<ASovereignBaseCharacter*>& Targets)
{
	if (!Caster || Targets.Num() == 0)
	{
		return false;
	}

	const USovereignSkillDefinition* SkillDef = SkillInstance.SkillDefinition.Get();
	if (!SkillDef)
	{
		SkillDef = SkillInstance.SkillDefinition.LoadSynchronous();
	}
	if (!SkillDef)
	{
		return false;
	}

	// 1. Verify and deduct Qi
	if (SkillDef->QiCost > 0.0f)
	{
		USovereignQiComponent* QiComp = Caster->GetQiComponent();
		if (!QiComp || QiComp->CurrentQi < SkillDef->QiCost)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BattleManager] %s has insufficient Qi (%f / %f) for skill %s"),
				*Caster->GetName(), QiComp ? QiComp->CurrentQi : 0.0f, SkillDef->QiCost, *SkillDef->SkillDisplayName.ToString());
			return false; // Turn is retained, action rejected
		}
		QiComp->CurrentQi = FMath::Max(0.0f, QiComp->CurrentQi - SkillDef->QiCost);
	}

	// 2. Identify Caster Slot in their formation
	int32 CasterSlot = 0;
	bool bIsPlayerCaster = false;
	for (int32 i = 0; i < GRID_SIZE; ++i)
	{
		if (PlayerGrid[i].Get() == Caster)
		{
			CasterSlot = i;
			bIsPlayerCaster = true;
			break;
		}
		if (EnemyGrid[i].Get() == Caster)
		{
			CasterSlot = i;
			break;
		}
	}
	FSovereignCombatEntitySnapshot CasterSnapshot = CreateSnapshotForCombatant(Caster, CasterSlot);

	// 3. Determine if Healing vs Damaging
	const bool bIsHealing = (SkillDef->ScalingAttribute == ESovereignSkillScalingAttribute::Wisdom &&
		(SkillDef->TargetPattern == ESovereignSkillTargetPattern::SelfOnly || SkillDef->TargetPattern == ESovereignSkillTargetPattern::AllAllies));

	float TotalActionResult = 0.0f;

	for (ASovereignBaseCharacter* Target : Targets)
	{
		if (!Target) continue;

		USovereignAttributeComponent* TargetAttr = Target->GetAttributeComponent();
		if (!TargetAttr) continue;

		int32 TargetSlot = 0;
		for (int32 i = 0; i < GRID_SIZE; ++i)
		{
			if (PlayerGrid[i].Get() == Target || EnemyGrid[i].Get() == Target)
			{
				TargetSlot = i;
				break;
			}
		}
		FSovereignCombatEntitySnapshot TargetSnapshot = CreateSnapshotForCombatant(Target, TargetSlot);

		if (bIsHealing)
		{
			// Healing calculation: raw wisdom power
			float ClassMult = 1.0f;
			if (const USovereignClassComponent* ClassComp = Caster->GetClassComponent())
			{
				ClassMult = ClassComp->MagicDamageMultiplier;
			}
			float HealAmount = SkillDef->CalculateRawPower(CasterSnapshot.Wisdom, ClassMult);
			TargetAttr->CurrentHealth = FMath::Clamp(TargetAttr->CurrentHealth + HealAmount, 0.0f, TargetAttr->MaxHealth);
			TotalActionResult += HealAmount;
		}
		else
		{
			// Damage calculation with percentage mitigation curve
			FSovereignDamageCalculationResult Result = USovereignCombatCalculationLibrary::CalculateCombatDamage(
				CasterSnapshot, TargetSnapshot, SkillInstance
			);

			float FinalDmg = Result.FinalDamage;

			// Check target Defend stance mitigation
			if (USovereignATBComponent* TargetATB = Target->GetATBComponent())
			{
				if (TargetATB->bIsDefending)
				{
					FinalDmg = FMath::Max(1.0f, FinalDmg * TargetATB->DefendDamageMultiplier);
				}
			}

			TargetAttr->CurrentHealth = FMath::Max(0.0f, TargetAttr->CurrentHealth - FinalDmg);
			TotalActionResult += FinalDmg;
		}
	}

	OnSkillActionExecuted.Broadcast(Caster, SkillDef->SkillTag, TotalActionResult, bIsHealing);

	// Conclude turn and cycle combat state
	ConcludeActiveTurn();
	return true;
}

TArray<ASovereignBaseCharacter*> ASovereignBattleManager::GetLivingEnemyUnits() const
{
	TArray<ASovereignBaseCharacter*> Living;
	for (const auto& Unit : EnemyGrid)
	{
		if (Unit.IsValid() && IsCombatantAlive(Unit.Get()))
		{
			Living.Add(Unit.Get());
		}
	}
	return Living;
}
