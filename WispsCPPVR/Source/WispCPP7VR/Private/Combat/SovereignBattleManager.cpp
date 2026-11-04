// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignATBComponent.h"
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
