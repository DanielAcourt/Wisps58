// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignClassComponent.h"
#include "Components/SovereignSkillComponent.h"
#include "Components/SovereignLevelComponent.h"
#include "Components/SovereignStatusComponent.h"
#include "Combat/SovereignSkillDefinition.h"
#include "Combat/SovereignSkillAugment.h"
#include "Combat/SovereignCombatCalculationLibrary.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASovereignBattleManager::ASovereignBattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PlayerGrid.SetNum(GRID_SIZE);
	EnemyGrid.SetNum(GRID_SIZE);
	BattleState = EBattleState::EncounterStarting;
	bIsEscapable = true;
	CurrentEscapeProgress = 0.0f;
	EscapeProgressIncrementPerFail = 25.0f;
}

void ASovereignBattleManager::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerSlotTransforms.Num() < GRID_SIZE || EnemySlotTransforms.Num() < GRID_SIZE)
	{
		GenerateDefaultGridTransforms();
	}
}

void ASovereignBattleManager::GenerateDefaultGridTransforms()
{
	const FVector Origin = GetActorLocation();
	const FRotator BaseRotation = GetActorRotation();

	PlayerSlotTransforms.SetNum(GRID_SIZE);
	EnemySlotTransforms.SetNum(GRID_SIZE);

	// Formation metrics: 3 ranks (X axis: Front, Mid, Rear) x 3 columns (Y axis: Left, Center, Right)
	// Player party sits at -X (facing +X); Enemies sit at +X (facing -X)
	constexpr float RankSpacing = 200.0f;
	constexpr float ColumnSpacing = 180.0f;
	constexpr float CenterOffset = 400.0f;

	for (int32 Slot = 0; Slot < GRID_SIZE; ++Slot)
	{
		const int32 Rank = Slot / 3;    // 0 = Front, 1 = Mid, 2 = Rear
		const int32 Col = (Slot % 3) - 1; // -1 = Left, 0 = Center, +1 = Right

		// Player Grid
		FVector PlayerPos = Origin + FVector(-(CenterOffset + (Rank * RankSpacing)), Col * ColumnSpacing, 0.0f);
		PlayerSlotTransforms[Slot] = FTransform(BaseRotation, PlayerPos);

		// Enemy Grid (Mirrored on +X, rotated 180 degrees)
		FRotator EnemyRot = BaseRotation;
		EnemyRot.Yaw += 180.0f;
		FVector EnemyPos = Origin + FVector(CenterOffset + (Rank * RankSpacing), Col * ColumnSpacing, 0.0f);
		EnemySlotTransforms[Slot] = FTransform(EnemyRot, EnemyPos);
	}

	// Action Camera Transform (Side elevated view framing both sides in FF7 perspective)
	FVector CamPos = Origin + FVector(0.0f, -800.0f, 350.0f);
	FRotator CamRot = FRotationMatrix::MakeFromX(Origin - CamPos).Rotator();
	ArenaCameraTransform = FTransform(CamRot, CamPos);

	if (BattleCameraActor)
	{
		BattleCameraActor->SetActorTransform(ArenaCameraTransform);
	}
}

void ASovereignBattleManager::PositionCombatantsOnGrid()
{
	if (PlayerSlotTransforms.Num() < GRID_SIZE || EnemySlotTransforms.Num() < GRID_SIZE)
	{
		GenerateDefaultGridTransforms();
	}

	for (int32 i = 0; i < GRID_SIZE; ++i)
	{
		if (PlayerGrid[i])
		{
			PlayerGrid[i]->SetActorTransform(PlayerSlotTransforms[i], false, nullptr, ETeleportType::TeleportPhysics);
		}
		if (EnemyGrid[i])
		{
			EnemyGrid[i]->SetActorTransform(EnemySlotTransforms[i], false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void ASovereignBattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BattleState == EBattleState::ActiveCombat)
	{
		// Process next action in queue if active combatant has finished
		if (!ActiveCombatant && ReadyQueue.Num() > 0)
		{
			if (ASovereignBaseCharacter* NextActor = DequeueNextCombatant())
			{
				ActiveCombatant = NextActor;
				SetBattleState(EBattleState::ActionExecuting);

				// Check Stun condition (Ticket CS-005)
				if (USovereignStatusComponent* StatusComp = NextActor->GetStatusComponent())
				{
					if (StatusComp->IsStunned())
					{
						UE_LOG(LogTemp, Log, TEXT("[BattleManager] %s is Stunned and loses their turn!"), *NextActor->GetName());
						ConcludeActiveTurn();
						continue;
					}
				}

				if (USovereignATBComponent* ATBComp = NextActor->GetATBComponent())
				{
					ATBComp->BeginActionSelection();
				}

				OnCombatantTurnReady.Broadcast(NextActor);

				// If the acting combatant belongs to the enemy formation, execute automated AI turn
				bool bIsEnemy = false;
				for (const auto& EnemyUnit : EnemyGrid)
				{
					if (EnemyUnit.Get() == NextActor)
					{
						bIsEnemy = true;
						break;
					}
				}

				if (bIsEnemy)
				{
					ExecuteEnemyTurn(NextActor);
				}
			}
		}
	}
}

void ASovereignBattleManager::InitializeEncounterFromDefinition(const TArray<ASovereignBaseCharacter*>& InPlayerUnits, USovereignEncounterDefinition* EncounterDef)
{
	if (!EncounterDef)
	{
		InitializeEncounter(InPlayerUnits, TArray<ASovereignBaseCharacter*>(), true);
		return;
	}

	ActiveEncounterDefinition = EncounterDef;
	CleanupSpawnedEnemies();

	UWorld* World = GetWorld();
	TArray<ASovereignBaseCharacter*> SpawnedEnemies;
	SpawnedEnemies.SetNumZeroed(GRID_SIZE);

	if (World)
	{
		if (PlayerSlotTransforms.Num() < GRID_SIZE || EnemySlotTransforms.Num() < GRID_SIZE)
		{
			GenerateDefaultGridTransforms();
		}

		for (const FSovereignEncounterSlot& SlotConfig : EncounterDef->EnemySlots)
		{
			if (SlotConfig.EnemyDefinition && SlotConfig.SlotIndex >= 0 && SlotConfig.SlotIndex < GRID_SIZE)
			{
				TSubclassOf<ASovereignBaseCharacter> ClassToSpawn = SlotConfig.EnemyDefinition->CharacterClass;
				if (!ClassToSpawn)
				{
					ClassToSpawn = ASovereignBaseCharacter::StaticClass();
				}

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FTransform& SpawnTransform = EnemySlotTransforms[SlotConfig.SlotIndex];
				ASovereignBaseCharacter* NewEnemy = World->SpawnActor<ASovereignBaseCharacter>(
					ClassToSpawn,
					SpawnTransform.GetLocation(),
					SpawnTransform.GetRotation().Rotator(),
					SpawnParams);

				if (NewEnemy)
				{
					SlotConfig.EnemyDefinition->InitializeEnemyVessel(NewEnemy);
					EnemyDefinitionsMap.Add(NewEnemy, SlotConfig.EnemyDefinition);
					SpawnedEnemyActors.Add(NewEnemy);
					SpawnedEnemies[SlotConfig.SlotIndex] = NewEnemy;
				}
			}
		}
	}

	InitializeEncounter(InPlayerUnits, SpawnedEnemies, EncounterDef->bCanFlee);
}

void ASovereignBattleManager::CleanupSpawnedEnemies()
{
	for (TObjectPtr<ASovereignBaseCharacter> EnemyActor : SpawnedEnemyActors)
	{
		if (EnemyActor)
		{
			EnemyActor->Destroy();
		}
	}
	SpawnedEnemyActors.Empty();
	EnemyDefinitionsMap.Empty();
}

void ASovereignBattleManager::ActivateBattleCamera(APlayerController* PC, float BlendTime)
{
	if (!PC)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!BattleCameraActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		BattleCameraActor = World->SpawnActor<ACameraActor>(
			ACameraActor::StaticClass(),
			ArenaCameraTransform.GetLocation(),
			ArenaCameraTransform.GetRotation().Rotator(),
			SpawnParams);
	}
	else
	{
		BattleCameraActor->SetActorTransform(ArenaCameraTransform);
	}

	if (BattleCameraActor)
	{
		PC->SetViewTargetWithBlend(BattleCameraActor, BlendTime, VTBlend_EaseInOut, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("[BattleManager] Switched ViewTarget to BattleCameraActor [%s] with blend time %f"),
			*BattleCameraActor->GetName(), BlendTime);
	}
}

void ASovereignBattleManager::RestorePlayerCamera(APlayerController* PC, AActor* TargetActor, float BlendTime)
{
	if (!PC || !TargetActor)
	{
		return;
	}

	PC->SetViewTargetWithBlend(TargetActor, BlendTime, VTBlend_EaseInOut, 2.0f);
	UE_LOG(LogTemp, Log, TEXT("[BattleManager] Restored ViewTarget to %s with blend time %f"),
		*TargetActor->GetName(), BlendTime);
}

void ASovereignBattleManager::ExecuteEnemyTurn(ASovereignBaseCharacter* EnemyCombatant)
{
	if (!EnemyCombatant || !IsCombatantAlive(EnemyCombatant))
	{
		ConcludeActiveTurn();
		return;
	}

	const TArray<ASovereignBaseCharacter*> LivingPlayers = GetLivingPlayerUnits();
	if (LivingPlayers.Num() == 0)
	{
		ConcludeActiveTurn();
		return;
	}

	// Determine AI Role (Ticket CS-003)
	ESovereignEnemyRole Role = ESovereignEnemyRole::Default;
	if (const TObjectPtr<USovereignEnemyDefinition>* FoundDef = EnemyDefinitionsMap.Find(EnemyCombatant))
	{
		if (*FoundDef)
		{
			Role = (*FoundDef)->EnemyRole;
		}
	}

	USovereignSkillComponent* SkillComp = EnemyCombatant->GetSkillComponent();

	// 1. Role-specific decision trees
	FSovereignSkillInstance SelectedSkillInstance;
	bool bFoundUsableSkill = false;
	TArray<ASovereignBaseCharacter*> Targets;

	// Helper lambda: creates transient basic strike
	auto MakeFallbackStrike = [this]() -> FSovereignSkillInstance
	{
		USovereignSkillDefinition* FallbackStrike = NewObject<USovereignSkillDefinition>(this);
		FallbackStrike->SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Combat.BasicStrike"), false);
		FallbackStrike->SkillDisplayName = FText::FromString(TEXT("Strike"));
		FallbackStrike->BasePower = 15.0f;
		FallbackStrike->StatMultiplier = 1.0f;
		FallbackStrike->QiCost = 0.0f;
		FallbackStrike->ScalingAttribute = ESovereignSkillScalingAttribute::Strength;
		FallbackStrike->TargetPattern = ESovereignSkillTargetPattern::SingleTarget;
		return FSovereignSkillInstance(FallbackStrike);
	};

	// --- A. Support Healer Decision Tree ---
	if (Role == ESovereignEnemyRole::SupportHealer)
	{
		// Check living allies for health < 50%
		ASovereignBaseCharacter* MostWoundedAlly = nullptr;
		float LowestHealthPercent = 0.50f;

		for (ASovereignBaseCharacter* Ally : GetLivingEnemyUnits())
		{
			if (Ally)
			{
				if (const USovereignAttributeComponent* Attr = Ally->GetAttributeComponent())
				{
					if (Attr->MaxHealth > 0.0f)
					{
						const float HealthPct = Attr->CurrentHealth / Attr->MaxHealth;
						if (HealthPct < LowestHealthPercent)
						{
							LowestHealthPercent = HealthPct;
							MostWoundedAlly = Ally;
						}
					}
				}
			}
		}

		if (MostWoundedAlly && SkillComp)
		{
			// Search for healing skill
			for (const FSovereignSkillInstance& Candidate : SkillComp->EquippedSkills)
			{
				const USovereignSkillDefinition* CandidateDef = Candidate.SkillDefinition.Get();
				if (!CandidateDef) CandidateDef = Candidate.SkillDefinition.LoadSynchronous();

				if (CandidateDef && CandidateDef->ScalingAttribute == ESovereignSkillScalingAttribute::Wisdom &&
					(CandidateDef->TargetPattern == ESovereignSkillTargetPattern::SingleTarget ||
					 CandidateDef->TargetPattern == ESovereignSkillTargetPattern::SelfOnly ||
					 CandidateDef->TargetPattern == ESovereignSkillTargetPattern::AllAllies))
				{
					if (CanCastSkill(EnemyCombatant, CandidateDef))
					{
						SelectedSkillInstance = Candidate;
						bFoundUsableSkill = true;
						if (CandidateDef->TargetPattern == ESovereignSkillTargetPattern::AllAllies)
						{
							Targets = GetLivingEnemyUnits();
						}
						else
						{
							Targets.Add(MostWoundedAlly);
						}
						break;
					}
				}
			}
		}
	}

	// --- B. Tactical Leader Decision Tree (AoE or Highest Threat) ---
	if (!bFoundUsableSkill && Role == ESovereignEnemyRole::TacticalLeader && SkillComp)
	{
		// If 2+ living players, prefer full matrix AoE or cleave
		if (LivingPlayers.Num() >= 2)
		{
			for (const FSovereignSkillInstance& Candidate : SkillComp->EquippedSkills)
			{
				const USovereignSkillDefinition* CandidateDef = Candidate.SkillDefinition.Get();
				if (!CandidateDef) CandidateDef = Candidate.SkillDefinition.LoadSynchronous();

				if (CandidateDef && (CandidateDef->TargetPattern == ESovereignSkillTargetPattern::FullMatrixAOE ||
									 CandidateDef->TargetPattern == ESovereignSkillTargetPattern::FrontRowSweep))
				{
					if (CanCastSkill(EnemyCombatant, CandidateDef))
					{
						SelectedSkillInstance = Candidate;
						bFoundUsableSkill = true;
						Targets = LivingPlayers;
						break;
					}
				}
			}
		}

		// Otherwise target player with highest Strength (threat)
		if (!bFoundUsableSkill)
		{
			ASovereignBaseCharacter* HighestThreatPlayer = nullptr;
			int32 MaxStrength = -1;
			for (ASovereignBaseCharacter* Player : LivingPlayers)
			{
				if (Player)
				{
					if (const USovereignAttributeComponent* Attr = Player->GetAttributeComponent())
					{
						if (Attr->Strength > MaxStrength)
						{
							MaxStrength = Attr->Strength;
							HighestThreatPlayer = Player;
						}
					}
				}
			}
			if (HighestThreatPlayer)
			{
				Targets.Add(HighestThreatPlayer);
			}
		}
	}

	// --- C. Aggressive Brute Decision Tree (Front Rank Focus) ---
	if (Targets.Num() == 0 && Role == ESovereignEnemyRole::AggressiveBrute)
	{
		// Prioritize Front Rank (Slots 0, 1, 2)
		TArray<ASovereignBaseCharacter*> FrontRankPlayers;
		for (int32 Slot = 0; Slot < 3; ++Slot)
		{
			if (PlayerGrid[Slot] && IsCombatantAlive(PlayerGrid[Slot].Get()))
			{
				FrontRankPlayers.Add(PlayerGrid[Slot].Get());
			}
		}

		if (FrontRankPlayers.Num() > 0)
		{
			const int32 Pick = FMath::RandRange(0, FrontRankPlayers.Num() - 1);
			Targets.Add(FrontRankPlayers[Pick]);
		}
	}

	// --- D. Skirmisher / Sniper Decision Tree (Execute Lowest HP or Rear Rank) ---
	if (Targets.Num() == 0 && Role == ESovereignEnemyRole::SkirmisherSniper)
	{
		// Find living player with lowest absolute HP
		ASovereignBaseCharacter* LowestHPPlayer = nullptr;
		float MinHP = MAX_FLT;

		for (ASovereignBaseCharacter* Player : LivingPlayers)
		{
			if (Player)
			{
				if (const USovereignAttributeComponent* Attr = Player->GetAttributeComponent())
				{
					if (Attr->CurrentHealth < MinHP)
					{
						MinHP = Attr->CurrentHealth;
						LowestHPPlayer = Player;
					}
				}
			}
		}

		if (LowestHPPlayer)
		{
			Targets.Add(LowestHPPlayer);
		}
	}

	// --- E. Standard Skill Selection if not chosen above ---
	if (!bFoundUsableSkill && SkillComp)
	{
		for (const FSovereignSkillInstance& Candidate : SkillComp->EquippedSkills)
		{
			const USovereignSkillDefinition* CandidateDef = Candidate.SkillDefinition.Get();
			if (!CandidateDef) CandidateDef = Candidate.SkillDefinition.LoadSynchronous();

			if (CandidateDef && CanCastSkill(EnemyCombatant, CandidateDef))
			{
				// Avoid selecting healing skills if targeting offensive players
				if (CandidateDef->ScalingAttribute != ESovereignSkillScalingAttribute::Wisdom)
				{
					SelectedSkillInstance = Candidate;
					bFoundUsableSkill = true;
					break;
				}
			}
		}
	}

	if (!bFoundUsableSkill)
	{
		SelectedSkillInstance = MakeFallbackStrike();
	}

	// --- F. Fallback Target Selection if preferred slot was empty ---
	if (Targets.Num() == 0)
	{
		const int32 Pick = FMath::RandRange(0, LivingPlayers.Num() - 1);
		Targets.Add(LivingPlayers[Pick]);
	}

	// 2. Execute Action
	ExecuteSkillAction(EnemyCombatant, SelectedSkillInstance, Targets);
}

void ASovereignBattleManager::InitializeEncounter(const TArray<ASovereignBaseCharacter*>& InPlayerUnits, const TArray<ASovereignBaseCharacter*>& InEnemyUnits, bool bCanFlee)
{
	bIsEscapable = bCanFlee;
	CurrentEscapeProgress = 0.0f;
	ReadyQueue.Empty();
	ActiveCombatant = nullptr;

	if (PlayerSlotTransforms.Num() < GRID_SIZE || EnemySlotTransforms.Num() < GRID_SIZE)
	{
		GenerateDefaultGridTransforms();
	}

	// Populate Player Grid
	for (int32 i = 0; i < GRID_SIZE; ++i)
	{
		PlayerGrid[i] = (i < InPlayerUnits.Num() && InPlayerUnits[i]) ? InPlayerUnits[i] : nullptr;
		if (PlayerGrid[i])
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
		if (EnemyGrid[i])
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

	PositionCombatantsOnGrid();
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
			if (NewState == EBattleState::Victory)
			{
				DistributeVictoryRewards();
			}

			OnBattleEnded.Broadcast(NewState);

			// Freeze ATB tickers on all combatants
			for (auto& Member : PlayerGrid)
			{
				if (Member && Member->GetATBComponent())
				{
					Member->GetATBComponent()->SetComponentTickEnabled(false);
				}
			}
			for (auto& Enemy : EnemyGrid)
			{
				if (Enemy && Enemy->GetATBComponent())
				{
					Enemy->GetATBComponent()->SetComponentTickEnabled(false);
				}
			}

			// Clean up dynamically spawned enemies so the arena is pristine for the next encounter
			CleanupSpawnedEnemies();
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
		if (Member && Member->GetATBComponent())
		{
			Member->GetATBComponent()->SetWaitModeActive(bWait);
		}
	}
	for (auto& Enemy : EnemyGrid)
	{
		if (Enemy && Enemy->GetATBComponent())
		{
			Enemy->GetATBComponent()->SetWaitModeActive(bWait);
		}
	}
}

void ASovereignBattleManager::ConcludeActiveTurn()
{
	if (ActiveCombatant)
	{
		// Process turn-end status effects (Poison damage, Regen healing, turn decrement) - Ticket CS-005
		if (USovereignStatusComponent* StatusComp = ActiveCombatant->GetStatusComponent())
		{
			StatusComp->ProcessTurnEnd();
		}

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

void ASovereignBattleManager::DistributeVictoryRewards()
{
	float TotalExp = 0.0f;
	int32 TotalClassExp = 0;
	float TotalQi = 0.0f;

	if (ActiveEncounterDefinition)
	{
		TotalExp = ActiveEncounterDefinition->CalculateTotalExperience();
		TotalClassExp = ActiveEncounterDefinition->CalculateTotalClassExperience();
		TotalQi = ActiveEncounterDefinition->CalculateTotalQi();
	}
	else
	{
		// Fallback reward calculation based on number of enemies in EnemyGrid
		int32 EnemyCount = 0;
		for (const auto& Enemy : EnemyGrid)
		{
			if (Enemy)
			{
				EnemyCount++;
			}
		}
		EnemyCount = FMath::Max(1, EnemyCount);
		TotalExp = EnemyCount * 50.0f;
		TotalClassExp = EnemyCount * 10;
		TotalQi = EnemyCount * 15.0f;
	}

	const TArray<ASovereignBaseCharacter*> LivingMembers = GetLivingPlayerUnits();
	if (LivingMembers.Num() == 0)
	{
		return;
	}

	// Option A: Award experience to living party members
	// Distribute EXP across living members and feed USovereignLevelComponent + USovereignClassComponent
	const double MemberExp = TotalExp / LivingMembers.Num();
	const float MemberQi = TotalQi / LivingMembers.Num();

	for (ASovereignBaseCharacter* Member : LivingMembers)
	{
		if (!Member)
		{
			continue;
		}

		// 1. Level Component Attribute XP (feathers into active training focus)
		if (USovereignLevelComponent* LevelComp = Member->GetLevelComponent())
		{
			LevelComp->AddAttributeXP(LevelComp->ActiveTrainingFocus, MemberExp);
		}

		// 2. Class Component CP / Class Level Progression
		if (USovereignClassComponent* ClassComp = Member->GetClassComponent())
		{
			for (const FGameplayTag& EquippedTag : ClassComp->EquippedClasses)
			{
				ClassComp->AddClassLevel(EquippedTag, FMath::Max(1, TotalClassExp / 10));
			}
		}

		// 3. Qi replenishment from defeated enemies
		if (USovereignQiComponent* QiComp = Member->GetQiComponent())
		{
			QiComp->CurrentQi = FMath::Clamp(QiComp->CurrentQi + MemberQi, 0.0f, QiComp->MaxQiCapacity);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleManager] Distributed Victory Rewards: TotalExp: %.1f, ClassExp: %d, TotalQi: %.1f across %d members."),
		TotalExp, TotalClassExp, TotalQi, LivingMembers.Num());

	OnVictoryRewardsDistributed.Broadcast(TotalExp, TotalClassExp, TotalQi, LivingMembers.Num());
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
		if (Unit && IsCombatantAlive(Unit.Get()))
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

	// Check Silence status effect (Ticket CS-005)
	if (const USovereignStatusComponent* StatusComp = Caster->GetStatusComponent())
	{
		if (StatusComp->IsSilenced())
		{
			return false; // Silenced combatants cannot cast Qi skills
		}
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

		// Elemental properties (Ticket CS-004)
		if (const TObjectPtr<USovereignEnemyDefinition>* FoundDef = EnemyDefinitionsMap.Find(const_cast<ASovereignBaseCharacter*>(Combatant)))
		{
			if (*FoundDef)
			{
				Snapshot.PrimaryElement = (*FoundDef)->PrimaryElement;
				Snapshot.WeaknessElement = (*FoundDef)->WeaknessElement;
				Snapshot.ResistanceElement = (*FoundDef)->ResistanceElement;
			}
		}
		else if (const USovereignElementComponent* ElemComp = Combatant->FindComponentByClass<USovereignElementComponent>())
		{
			Snapshot.PrimaryElement = ElemComp->BodySocket;
		}

		// Status conditions (Ticket CS-005)
		if (const USovereignStatusComponent* StatusComp = Combatant->GetStatusComponent())
		{
			Snapshot.bIsBlinded = StatusComp->IsBlinded();
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
				ClassMult = ClassComp->GetTotalIntelligenceMultiplier();
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

		// Apply Status Effects if defined on the skill (Ticket CS-005)
		if (SkillDef->InflictedStatus != ESovereignStatusType::None)
		{
			if (USovereignStatusComponent* TargetStatus = Target->GetStatusComponent())
			{
				TargetStatus->ApplyStatusEffect(SkillDef->InflictedStatus, SkillDef->StatusDurationTurns, SkillDef->StatusPotency, Caster);
			}
		}
	}

	OnSkillActionExecuted.Broadcast(Caster, SkillDef->SkillTag, TotalActionResult, bIsHealing);

	// Conclude turn and cycle combat state
	ConcludeActiveTurn();
	return true;
}

bool ASovereignBattleManager::ExecuteItemAction(ASovereignBaseCharacter* User, const FGuid& ItemInstanceId, ASovereignBaseCharacter* Target)
{
	if (!User || !Target || !ItemInstanceId.IsValid())
	{
		return false;
	}

	USovereignInventoryComponent* Inventory = User->GetInventoryComponent();
	if (!Inventory)
	{
		return false;
	}

	// 1. Locate Item Slot in User's Inventory
	FSovereignInventoryItemInstance* FoundSlot = nullptr;
	for (FSovereignInventoryItemInstance& Slot : Inventory->InventorySlots)
	{
		if (Slot.InstanceId == ItemInstanceId)
		{
			FoundSlot = &Slot;
			break;
		}
	}

	if (!FoundSlot || FoundSlot->Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleManager] %s attempted to use item instance %s but has none."),
			*User->GetName(), *ItemInstanceId.ToString());
		return false;
	}

	const USovereignItemDefinition* ItemDef = FoundSlot->ItemDefinition.Get();
	if (!ItemDef)
	{
		ItemDef = FoundSlot->ItemDefinition.LoadSynchronous();
	}
	if (!ItemDef)
	{
		return false;
	}

	// 2. Validate Target Alignment (Friendly vs Enemy)
	bool bIsTargetFriendly = false;
	for (const auto& Member : PlayerGrid)
	{
		if (Member.Get() == Target)
		{
			bIsTargetFriendly = true;
			break;
		}
	}

	if (const USovereignItemFragment_CombatEffect* CombatFragment = ItemDef->FindFragment<USovereignItemFragment_CombatEffect>())
	{
		if (bIsTargetFriendly && !CombatFragment->bCanTargetAllies)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Item %s cannot target friendly allies!"), *ItemDef->DisplayName.ToString());
			return false; // Retain turn
		}
		if (!bIsTargetFriendly && !CombatFragment->bCanTargetEnemies)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Item %s cannot target enemies!"), *ItemDef->DisplayName.ToString());
			return false; // Retain turn
		}
	}

	// 3. Execute Item Consumption Hooks on Target
	bool bExecutionSuccess = false;
	for (const TObjectPtr<USovereignItemFragment>& Fragment : ItemDef->Fragments)
	{
		if (Fragment)
		{
			if (Fragment->OnConsumed(Target, 1))
			{
				bExecutionSuccess = true;
			}
		}
	}

	if (!bExecutionSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Item %s produced no effect on target %s."),
			*ItemDef->DisplayName.ToString(), *Target->GetName());
		return false;
	}

	// 4. Decrement 1 Quantity from User Inventory
	FoundSlot->Quantity -= 1;
	if (FoundSlot->Quantity <= 0)
	{
		Inventory->RemoveItemByInstanceId(ItemInstanceId, 1);
	}
	else
	{
		Inventory->OnInventoryUpdated.Broadcast();
	}

	OnItemActionExecuted.Broadcast(User, ItemDef->ItemTag, Target);

	// 5. Conclude Active Turn
	ConcludeActiveTurn();
	return true;
}

float ASovereignBattleManager::CalculateEscapeChance(const ASovereignBaseCharacter* Fleer) const
{
	if (!bIsEscapable)
	{
		return 0.0f;
	}

	// 1. Calculate Player Party Average DEX
	const TArray<ASovereignBaseCharacter*> LivingPlayers = GetLivingPlayerUnits();
	float PlayerAvgDex = 10.0f;
	if (LivingPlayers.Num() > 0)
	{
		float TotalDex = 0.0f;
		for (const auto* Char : LivingPlayers)
		{
			if (const USovereignAttributeComponent* Attr = Char->GetAttributeComponent())
			{
				TotalDex += Attr->Dexterity;
			}
		}
		PlayerAvgDex = TotalDex / LivingPlayers.Num();
	}

	// 2. Calculate Enemy Average DEX
	const TArray<ASovereignBaseCharacter*> LivingEnemies = GetLivingEnemyUnits();
	float EnemyAvgDex = 10.0f;
	if (LivingEnemies.Num() > 0)
	{
		float TotalDex = 0.0f;
		for (const auto* Char : LivingEnemies)
		{
			if (const USovereignAttributeComponent* Attr = Char->GetAttributeComponent())
			{
				TotalDex += Attr->Dexterity;
			}
		}
		EnemyAvgDex = TotalDex / LivingEnemies.Num();
	}

	// 3. Fleer Luck Contribution
	float FleerLuck = 10.0f;
	if (Fleer)
	{
		if (const USovereignAttributeComponent* Attr = Fleer->GetAttributeComponent())
		{
			FleerLuck = Attr->Luck;
		}
	}

	// Formula: 50% base + (PartyAvgDex - EnemyAvgDex) * 2% + FleerLuck * 0.5% + CurrentEscapeProgress
	const float BaseChance = 50.0f + ((PlayerAvgDex - EnemyAvgDex) * 2.0f) + (FleerLuck * 0.5f) + CurrentEscapeProgress;
	return FMath::Clamp(BaseChance, 5.0f, 100.0f);
}

bool ASovereignBattleManager::AttemptFlee(ASovereignBaseCharacter* Fleer, float ForcedRoll)
{
	if (!Fleer)
	{
		return false;
	}

	// 1. Boss / Story Restraint Check
	if (!bIsEscapable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Cannot flee from this boss/guardian encounter!"));
		OnFleeAttemptFailed.Broadcast(Fleer, CurrentEscapeProgress);
		return false; // Turn is retained when attempting to flee an unescapable boss
	}

	// 2. Roll Escape
	const float EscapeChance = CalculateEscapeChance(Fleer);
	const float Roll = (ForcedRoll >= 0.0f) ? ForcedRoll : FMath::FRandRange(0.0f, 100.0f);

	if (Roll <= EscapeChance)
	{
		UE_LOG(LogTemp, Log, TEXT("[BattleManager] %s successfully fled combat! (Roll: %f <= Chance: %f)"),
			*Fleer->GetName(), Roll, EscapeChance);
		SetBattleState(EBattleState::Fled);
		return true;
	}

	// 3. Escape Failed: Increment escape bar fallback and consume turn
	CurrentEscapeProgress = FMath::Clamp(CurrentEscapeProgress + EscapeProgressIncrementPerFail, 0.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("[BattleManager] %s failed to flee! (Roll: %f > Chance: %f). Escape bar now %f%%"),
		*Fleer->GetName(), Roll, EscapeChance, CurrentEscapeProgress);

	OnFleeAttemptFailed.Broadcast(Fleer, CurrentEscapeProgress);

	// Consumes the acting dragon's turn (resets ATB to 0%)
	ConcludeActiveTurn();
	return false;
}

TArray<ASovereignBaseCharacter*> ASovereignBattleManager::GetLivingEnemyUnits() const
{
	TArray<ASovereignBaseCharacter*> Living;
	for (const auto& Unit : EnemyGrid)
	{
		if (Unit && IsCombatantAlive(Unit.Get()))
		{
			Living.Add(Unit.Get());
		}
	}
	return Living;
}
