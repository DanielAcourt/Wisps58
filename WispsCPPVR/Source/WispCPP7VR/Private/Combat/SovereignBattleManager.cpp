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
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
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

	// Action Camera Transform (Side elevated view)
	FVector CamPos = Origin + FVector(0.0f, -800.0f, 350.0f);
	FRotator CamRot = FRotationMatrix::MakeFromX(Origin - CamPos).Rotator();
	ArenaCameraTransform = FTransform(CamRot, CamPos);
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
	for (const USovereignItemFragment* Fragment : ItemDef->Fragments)
	{
		if (Fragment)
		{
			if (Fragment->Execute_OnConsumed(Fragment, Target, 1))
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
