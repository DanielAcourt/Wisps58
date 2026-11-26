// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-19

#include "Combat/SovereignEncounterSubsystem.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Combat/SovereignEnemyDefinition.h"
#include "Combat/ASovereignBattleArena.h"
#include "Combat/SovereignBattleManager.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignPartyComponent.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	static void LogRandomEncounterSummary(
		int32 Seed,
		int32 InitialBudget,
		int32 RemainingBudget,
		int32 PoolCount,
		int32 TransformCount,
		float StatMultiplier,
		const TArray<ASovereignBaseCharacter*>& SpawnedEnemies,
		const TArray<FString>& SpawnedLines)
	{
		UE_LOG(LogTemp, Display, TEXT("================================================================================"));
		UE_LOG(LogTemp, Display, TEXT("⚔️  [SOVEREIGN ENCOUNTER SPAWNER] PARTY GENERATION COMPLETE (Seed %d, Buff x%.2f)"),
			Seed, StatMultiplier);
		UE_LOG(LogTemp, Display, TEXT("================================================================================"));
		UE_LOG(LogTemp, Display, TEXT("  Spawned Count: %d | Transforms Provided: %d | Candidate Pool Entries: %d"),
			SpawnedEnemies.Num(), TransformCount, PoolCount);
		UE_LOG(LogTemp, Display, TEXT("  Budget: %d -> Remaining: %d (Spent: %d)"),
			InitialBudget, RemainingBudget, FMath::Max(0, InitialBudget - RemainingBudget));
		UE_LOG(LogTemp, Display, TEXT("--------------------------------------------------------------------------------"));
		UE_LOG(LogTemp, Display, TEXT("  👾 SPAWNED ENEMY ROSTER (%d Total):"), SpawnedEnemies.Num());

		if (SpawnedLines.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("    • [NONE SPAWNED] Check CandidatePool entries, EncounterBudget, or SpawnTransforms.Num()!"));
		}
		else
		{
			for (const FString& Line : SpawnedLines)
			{
				UE_LOG(LogTemp, Display, TEXT("    • %s"), *Line);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	}

	static AASovereignBattleArena* ResolveBattleArena(UWorld* World, AASovereignBattleArena* PreferredArena)
	{
		if (IsValid(PreferredArena))
		{
			return PreferredArena;
		}

		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<AASovereignBattleArena> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				return *It;
			}
		}

		return nullptr;
	}
}

USovereignEncounterSubsystem::USovereignEncounterSubsystem()
{
	bIsEncounterActive = false;
}

TArray<ASovereignBaseCharacter*> USovereignEncounterSubsystem::SpawnRandomEncounterParty(
	const TArray<FSovereignEncounterEntry>& CandidatePool,
	const TArray<FTransform>& SpawnTransforms,
	int32 EncounterBudget,
	int32 RandomSeed,
	float EnemyStatMultiplier)
{
	TArray<ASovereignBaseCharacter*> SpawnedEnemies;
	TArray<FString> SpawnedLines;

	UWorld* World = GetWorld();
	if (!World || CandidatePool.IsEmpty() || SpawnTransforms.IsEmpty() || EncounterBudget <= 0)
	{
		LogRandomEncounterSummary(RandomSeed, EncounterBudget, EncounterBudget, CandidatePool.Num(), SpawnTransforms.Num(), EnemyStatMultiplier, SpawnedEnemies, SpawnedLines);
		return SpawnedEnemies;
	}

	// Expand single transform into 6 distinct transforms spaced 200 units apart
	// so single-anchor callers (e.g. door blueprints) generate a full squad up to budget
	TArray<FTransform> EffectiveTransforms = SpawnTransforms;
	if (EffectiveTransforms.Num() == 1)
	{
		const FTransform BaseXform = EffectiveTransforms[0];
		const FVector BaseLoc = BaseXform.GetLocation();
		const FRotator BaseRot = BaseXform.GetRotation().Rotator();
		const FVector RightDir = BaseXform.GetRotation().GetRightVector();
		const FVector FwdDir = BaseXform.GetRotation().GetForwardVector();

		EffectiveTransforms.Empty();
		for (int32 Row = 0; Row < 2; ++Row)
		{
			for (int32 Col = -1; Col <= 1; ++Col)
			{
				const FVector Offset = (FwdDir * (Row * 200.0f)) + (RightDir * (Col * 200.0f));
				EffectiveTransforms.Add(FTransform(BaseRot, BaseLoc + Offset));
			}
		}
	}

	FRandomStream Stream(RandomSeed);
	int32 RemainingBudget = EncounterBudget;
	int32 SpawnIndex = 0;
	TMap<FSoftObjectPath, int32> SpawnedCountsByDef;

	while (RemainingBudget > 0 && SpawnIndex < EffectiveTransforms.Num())
	{
		TArray<FSovereignEncounterEntry> Affordable;
		for (const FSovereignEncounterEntry& Entry : CandidatePool)
		{
			if (Entry.EnemyDefinition.IsNull() || Entry.PointCost > RemainingBudget)
			{
				continue;
			}

			const int32 ExistingCount = SpawnedCountsByDef.FindRef(Entry.EnemyDefinition.ToSoftObjectPath());
			if (Entry.MaxPerEncounter <= 0 || ExistingCount < Entry.MaxPerEncounter)
			{
				Affordable.Add(Entry);
			}
		}

		if (Affordable.IsEmpty())
		{
			break;
		}

		const FSovereignEncounterEntry& Picked = Affordable[Stream.RandRange(0, Affordable.Num() - 1)];
		USovereignEnemyDefinition* Def = Picked.EnemyDefinition.LoadSynchronous();
		if (!Def)
		{
			break;
		}

		UClass* SpawnClass = Def->CharacterClass ? Def->CharacterClass.Get() : ASovereignBaseCharacter::StaticClass();

		const FTransform& SpawnXform = EffectiveTransforms[SpawnIndex];
		ASovereignBaseCharacter* EnemyChar = World->SpawnActorDeferred<ASovereignBaseCharacter>(
			SpawnClass,
			SpawnXform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (EnemyChar)
		{
			EnemyChar->InitializeFromEnemyDefinition(Def, EnemyStatMultiplier);
			EnemyChar->FinishSpawning(SpawnXform);

			SpawnedEnemies.Add(EnemyChar);
			SpawnedCountsByDef.FindOrAdd(Picked.EnemyDefinition.ToSoftObjectPath())++;

			const FString EnemyName = Def->EnemyDisplayName.IsEmpty() ? Def->GetName() : Def->EnemyDisplayName.ToString();
			const FString RoleStr = StaticEnum<ESovereignEnemyRole>()->GetNameStringByValue(static_cast<int64>(Def->EnemyRole));
			const FVector SpawnLoc = SpawnXform.GetLocation();
			float HP = Def->MaxHealth * EnemyStatMultiplier;
			float MaxHP = HP;
			if (USovereignAttributeComponent* Attr = EnemyChar->GetAttributeComponent())
			{
				HP = Attr->CurrentHealth;
				MaxHP = Attr->MaxHealth;
			}

			SpawnedLines.Add(FString::Printf(
				TEXT("[Slot %d] %s (Def: %s, Cost: %d, Role: %s) | HP: %.1f/%.1f | Loc: (X=%.0f, Y=%.0f, Z=%.0f)"),
				SpawnIndex, *EnemyName, *Def->GetName(), Picked.PointCost, *RoleStr, HP, MaxHP,
				SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z));

			RemainingBudget -= FMath::Max(1, Picked.PointCost);
			++SpawnIndex;
		}
		else
		{
			break;
		}
	}

	LogRandomEncounterSummary(RandomSeed, EncounterBudget, RemainingBudget, CandidatePool.Num(), EffectiveTransforms.Num(), EnemyStatMultiplier, SpawnedEnemies, SpawnedLines);

	return SpawnedEnemies;
}

bool USovereignEncounterSubsystem::TriggerRandomEncounter(
	ASovereignBaseCharacter* PlayerLeader,
	AActor* OverlappedMob,
	const TArray<FSovereignEncounterEntry>& CandidatePool,
	const TArray<FTransform>& SpawnTransforms,
	int32 EncounterBudget,
	int32 RandomSeed,
	bool bCanFlee,
	ASovereignBattleManager* CustomBattleManager)
{
	if (bIsEncounterActive)
	{
		return false;
	}

	UWorld* World = GetWorld();
	float StatMultiplier = 1.0f;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				if (DungeonSub->bHasActiveExpedition)
				{
					StatMultiplier = DungeonSub->ActiveEnemyBuffMultiplier;
				}
			}
		}
	}

	TArray<FTransform> EffectiveSpawnTransforms = SpawnTransforms;
	if (AASovereignBattleArena* Arena = ResolveBattleArena(World, PreferredBattleArena))
	{
		const TArray<FTransform> ArenaEnemySlots = Arena->GetEnemySpawnTransforms();
		if (ArenaEnemySlots.Num() > 0)
		{
			EffectiveSpawnTransforms = ArenaEnemySlots;
		}
	}

	TArray<ASovereignBaseCharacter*> EnemyUnits = SpawnRandomEncounterParty(
		CandidatePool,
		EffectiveSpawnTransforms,
		EncounterBudget,
		RandomSeed,
		StatMultiplier);

	if (EnemyUnits.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SovereignEncounterSubsystem] TriggerRandomEncounter failed to spawn any enemies."));
		return false;
	}

	return TriggerEncounter(PlayerLeader, OverlappedMob, EnemyUnits, bCanFlee, CustomBattleManager);
}

bool USovereignEncounterSubsystem::TriggerEncounter(
	ASovereignBaseCharacter* PlayerLeader,
	AActor* OverlappedMob,
	const TArray<ASovereignBaseCharacter*>& EnemyUnits,
	bool bCanFlee,
	ASovereignBattleManager* CustomBattleManager)
{
	if (!PlayerLeader)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PlayerLeader)
	{
		World = PlayerLeader->GetWorld();
	}
	if (!World && CustomBattleManager)
	{
		World = CustomBattleManager->GetWorld();
	}

	// 1. Locate or Spawn Battle Manager
	ASovereignBattleManager* BattleMgr = CustomBattleManager;
	if (!BattleMgr && World)
	{
		for (TActorIterator<ASovereignBattleManager> It(World); It; ++It)
		{
			BattleMgr = *It;
			break;
		}

		if (!BattleMgr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// Spawn 5000 units above player as default arena platform if none exists in level
			FVector ArenaPos = PlayerLeader->GetActorLocation() + FVector(0.0f, 0.0f, 5000.0f);
			BattleMgr = World->SpawnActor<ASovereignBattleManager>(ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams);
		}
	}

	if (!BattleMgr)
	{
		return false;
	}

	ActiveBattleManager = BattleMgr;
	if (AASovereignBattleArena* Arena = ResolveBattleArena(World, PreferredBattleArena))
	{
		BattleMgr->SetActiveArena(Arena, true);
	}

	InteractedDungeonMob = OverlappedMob;

	// 2. Cache Dungeon Return Transform
	DungeonReturnTransform = PlayerLeader->GetActorTransform();

	// 3. Assemble Player Party
	ActivePlayerParty.Empty();
	TArray<ASovereignBaseCharacter*> DeployedParty;

	if (USovereignPartyComponent* PartyComp = PlayerLeader->GetPartyComponent())
	{
		DeployedParty = PartyComp->GetAllActivePartyMembers();
	}
	if (DeployedParty.Num() == 0)
	{
		DeployedParty.Add(PlayerLeader);
	}

	for (auto* Member : DeployedParty)
	{
		ActivePlayerParty.Add(Member);
	}

	// 4. Hide/Disable Collision on Roaming Dungeon Mob
	if (OverlappedMob)
	{
		OverlappedMob->SetActorHiddenInGame(true);
		OverlappedMob->SetActorEnableCollision(false);
	}

	// 5. Bind Battle Ended delegate
	BattleMgr->OnBattleEnded.RemoveDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);
	BattleMgr->OnBattleEnded.AddDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);

	// 6. Initialize Encounter on Battle Manager
	bIsEncounterActive = true;

	const FVector ReturnLoc = DungeonReturnTransform.GetLocation();
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("⚔️  [SOVEREIGN ENCOUNTER] DUNGEON -> BATTLE ARENA TRANSITION"));
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("  Player Leader: %s | Party Members: %d | Enemies: %d | Escapable: %s"),
		*PlayerLeader->GetName(), DeployedParty.Num(), EnemyUnits.Num(), bCanFlee ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Display, TEXT("  Cached Dungeon Return: X=%.0f Y=%.0f Z=%.0f"),
		ReturnLoc.X, ReturnLoc.Y, ReturnLoc.Z);
	UE_LOG(LogTemp, Display, TEXT("  Interacted Mob: %s | Battle Manager: %s"),
		OverlappedMob ? *OverlappedMob->GetName() : TEXT("None"),
		*BattleMgr->GetName());
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));

	BattleMgr->InitializeEncounter(DeployedParty, EnemyUnits, bCanFlee);

	// 7. Activate Battle Camera & disable exploration input
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			BattleMgr->ActivateBattleCamera(PC);
			PlayerLeader->DisableInput(PC);
		}
	}

	OnEncounterTransitionStarted.Broadcast(BattleMgr);

	return true;
}

bool USovereignEncounterSubsystem::TriggerEncounterWithDefinition(
	ASovereignBaseCharacter* PlayerLeader,
	AActor* OverlappedMob,
	USovereignEncounterDefinition* EncounterDef,
	ASovereignBattleManager* CustomBattleManager)
{
	if (!PlayerLeader || !EncounterDef)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PlayerLeader)
	{
		World = PlayerLeader->GetWorld();
	}
	if (!World && CustomBattleManager)
	{
		World = CustomBattleManager->GetWorld();
	}

	// 1. Locate or Spawn Battle Manager
	ASovereignBattleManager* BattleMgr = CustomBattleManager;
	if (!BattleMgr && World)
	{
		for (TActorIterator<ASovereignBattleManager> It(World); It; ++It)
		{
			BattleMgr = *It;
			break;
		}

		if (!BattleMgr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector ArenaPos = PlayerLeader->GetActorLocation() + FVector(0.0f, 0.0f, 5000.0f);
			BattleMgr = World->SpawnActor<ASovereignBattleManager>(ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams);
		}
	}

	if (!BattleMgr)
	{
		return false;
	}

	ActiveBattleManager = BattleMgr;
	if (AASovereignBattleArena* Arena = ResolveBattleArena(World, PreferredBattleArena))
	{
		BattleMgr->SetActiveArena(Arena, true);
	}
	InteractedDungeonMob = OverlappedMob;

	// 2. Cache Dungeon Return Transform
	DungeonReturnTransform = PlayerLeader->GetActorTransform();

	// 3. Assemble Player Party
	ActivePlayerParty.Empty();
	TArray<ASovereignBaseCharacter*> DeployedParty;

	if (USovereignPartyComponent* PartyComp = PlayerLeader->GetPartyComponent())
	{
		DeployedParty = PartyComp->GetAllActivePartyMembers();
	}
	if (DeployedParty.Num() == 0)
	{
		DeployedParty.Add(PlayerLeader);
	}

	for (auto* Member : DeployedParty)
	{
		ActivePlayerParty.Add(Member);
	}

	// 4. Hide/Disable Collision on Roaming Dungeon Mob
	if (OverlappedMob)
	{
		OverlappedMob->SetActorHiddenInGame(true);
		OverlappedMob->SetActorEnableCollision(false);
	}

	// 5. Bind Battle Ended delegate
	BattleMgr->OnBattleEnded.RemoveDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);
	BattleMgr->OnBattleEnded.AddDynamic(this, &USovereignEncounterSubsystem::HandleBattleEnded);

	// 6. Initialize Encounter on Battle Manager from definition
	bIsEncounterActive = true;

	const FVector ReturnLoc = DungeonReturnTransform.GetLocation();
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("⚔️  [SOVEREIGN ENCOUNTER] AUTHORED DEFINITION -> BATTLE ARENA TRANSITION"));
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("  Encounter Def: %s (%s) | Escapable: %s"),
		*EncounterDef->GetName(), *EncounterDef->EncounterDisplayName.ToString(),
		EncounterDef->bCanFlee ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Display, TEXT("  Player Leader: %s | Party Members: %d | Configured Slots: %d"),
		*PlayerLeader->GetName(), DeployedParty.Num(), EncounterDef->EnemySlots.Num());
	UE_LOG(LogTemp, Display, TEXT("  Cached Dungeon Return: X=%.0f Y=%.0f Z=%.0f"),
		ReturnLoc.X, ReturnLoc.Y, ReturnLoc.Z);
	UE_LOG(LogTemp, Display, TEXT("  Interacted Mob: %s | Battle Manager: %s"),
		OverlappedMob ? *OverlappedMob->GetName() : TEXT("None"),
		*BattleMgr->GetName());
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));

	BattleMgr->InitializeEncounterFromDefinition(DeployedParty, EncounterDef);

	// 7. Activate Battle Camera & disable exploration input
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			BattleMgr->ActivateBattleCamera(PC);
			PlayerLeader->DisableInput(PC);
		}
	}

	OnEncounterTransitionStarted.Broadcast(BattleMgr);
	return true;
}

void USovereignEncounterSubsystem::HandleBattleEnded(EBattleState Outcome)
{
	ReturnPartyToDungeon(Outcome);
}

void USovereignEncounterSubsystem::ReturnPartyToDungeon(EBattleState Outcome)
{
	if (!bIsEncounterActive)
	{
		return;
	}

	// 0. Restore Camera & Re-enable Input
	UWorld* World = GetWorld();
	if (ActiveBattleManager.IsValid() && ActivePlayerParty.Num() > 0 && ActivePlayerParty[0])
	{
		if (World)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				ActiveBattleManager->RestorePlayerCamera(PC, ActivePlayerParty[0]);
				ActivePlayerParty[0]->EnableInput(PC);
			}
		}
	}

	// 1. Teleport Living Party Members Back to Dungeon Floor
	for (int32 i = 0; i < ActivePlayerParty.Num(); ++i)
	{
		if (ActivePlayerParty[i])
		{
			// Offset party members slightly around the leader return transform
			FVector Offset = (i == 0) ? FVector::ZeroVector : FVector(0.0f, (i % 2 == 0 ? 1.0f : -1.0f) * ((i + 1) / 2) * 100.0f, 0.0f);
			FTransform TargetTransform = DungeonReturnTransform;
			TargetTransform.AddToTranslation(Offset);

			ActivePlayerParty[i]->SetActorTransform(TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// 2. Mob Cleanup vs Restoration
	if (InteractedDungeonMob.IsValid())
	{
		if (Outcome == EBattleState::Victory)
		{
			// Mob defeated: destroy from world
			InteractedDungeonMob->Destroy();
		}
		else if (Outcome == EBattleState::Fled)
		{
			// Fled: unhide mob so player can re-encounter or navigate around it
			InteractedDungeonMob->SetActorHiddenInGame(false);
			InteractedDungeonMob->SetActorEnableCollision(true);
		}
	}

	bIsEncounterActive = false;

	const TCHAR* OutcomeStr = TEXT("UNKNOWN");
	switch (Outcome)
	{
	case EBattleState::Victory: OutcomeStr = TEXT("VICTORY"); break;
	case EBattleState::Defeat:  OutcomeStr = TEXT("DEFEAT");  break;
	case EBattleState::Fled:    OutcomeStr = TEXT("FLED");    break;
	default: break;
	}

	const FVector ReturnPos = DungeonReturnTransform.GetLocation();
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("🏛️  [SOVEREIGN ENCOUNTER] BATTLE ARENA -> DUNGEON RETURN (%s)"), OutcomeStr);
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));
	UE_LOG(LogTemp, Display, TEXT("  Restored %d party member(s) to Dungeon Position: X=%.0f Y=%.0f Z=%.0f"),
		ActivePlayerParty.Num(), ReturnPos.X, ReturnPos.Y, ReturnPos.Z);
	if (InteractedDungeonMob.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("  Interacted Mob [%s]: %s"),
			*InteractedDungeonMob->GetName(),
			(Outcome == EBattleState::Victory) ? TEXT("DESTROYED (Encounter Won)") : TEXT("RESTORED VISIBILITY (Encounter Evaded)"));
	}
	UE_LOG(LogTemp, Display, TEXT("================================================================================"));

	OnEncounterTransitionEnded.Broadcast(Outcome);
}