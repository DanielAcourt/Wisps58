// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "CoreModules/SovereignDungeonGameMode.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Components/SovereignPartyComponent.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Combat/SovereignEncounterDefinition.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
namespace
{
	UClass* ResolveSovereignPawnClass(const FString& RawClassPath)
	{
		if (RawClassPath.IsEmpty())
		{
			return nullptr;
		}

		FString ResolvedPath = RawClassPath;
		// Only append _C for Blueprint asset paths, never native /Script/ classes
		if (!ResolvedPath.StartsWith(TEXT("/Script/")) && !ResolvedPath.EndsWith(TEXT("_C")))
		{
			ResolvedPath += TEXT("_C");
		}

		return LoadClass<APawn>(nullptr, *ResolvedPath);
	}
}

void ApplyJsonStateDeferred(UWorld* World, TWeakObjectPtr<ASovereignBaseCharacter> TargetCharacter, const FString& StateJson, const FString& DebugLabel)
{
	if (!World || !TargetCharacter.IsValid() || StateJson.IsEmpty())
	{
		return;
	}

	if (ASovereignBaseCharacter* Char = TargetCharacter.Get())
	{
		Char->SetIsLoadedCharacter(true);
		if (USovereignSaveableEntityComponent* SaveComp = Char->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SaveComp->SetIsLoadedEntity(true);
		}
	}

	// Defer by 1 tick so the character's BeginPlay completes BEFORE saved state is applied
	World->GetTimerManager().SetTimerForNextTick([TargetCharacter, StateJson, DebugLabel]()
		{
			ASovereignBaseCharacter* Char = TargetCharacter.Get();
			if (!Char)
			{
				return;
			}

			TSharedPtr<FJsonObject> StateJsonObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(StateJson);
			if (FJsonSerializer::Deserialize(Reader, StateJsonObj) && StateJsonObj.IsValid())
			{
				if (USovereignSaveableEntityComponent* SaveComp = Char->FindComponentByClass<USovereignSaveableEntityComponent>())
				{
					SaveComp->ApplyStateFromJsonObject(StateJsonObj);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Deferred state restored to %s [%s] after BeginPlay species sync."),
						*DebugLabel, *Char->GetName());
				}
			}
		});
}

ASovereignDungeonGameMode::ASovereignDungeonGameMode()
{
	bLockPossessionToLeader = false;
	DefaultPawnClass = ASovereignBaseCharacter::StaticClass();
	DungeonGeneratorClass = ASovereignDungeonGenerator::StaticClass();
}

UClass* ASovereignDungeonGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				if (DungeonSub->HasPartyLeaderState())
				{
					FString ClassPath = DungeonSub->PartyLeaderClassPath;
					// Resolve Blueprint generated class if standard path
					if (UClass* LoadedClass = ResolveSovereignPawnClass(DungeonSub->PartyLeaderClassPath))
					{
						UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Dynamic vessel pawn class resolved for expedition: %s"), *LoadedClass->GetName());
						return LoadedClass;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[DungeonGameMode] Failed to load party leader vessel class [%s], using default."), *DungeonSub->PartyLeaderClassPath);
					}
				}
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ASovereignDungeonGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;

	// Check if this player pawn is reconstituting a loaded party leader vessel from expedition
	bool bIsLoadedVessel = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
		{
			if (DungeonSub->HasPartyLeaderState())
			{
				bIsLoadedVessel = true; // Wait, make sure variable name matches: bIsLoadedVessel
			}
		}
	}

	if (bIsLoadedVessel)
	{
		SpawnInfo.bDeferConstruction = true;
	}

	APawn* SpawnedPawn = World->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (SpawnedPawn)
	{
		if (bIsLoadedVessel)
		{
			if (ASovereignBaseCharacter* BaseChar = Cast<ASovereignBaseCharacter>(SpawnedPawn))
			{
				BaseChar->SetIsLoadedCharacter(true);
			}
			if (USovereignSaveableEntityComponent* SaveComp = SpawnedPawn->FindComponentByClass<USovereignSaveableEntityComponent>())
			{
				SaveComp->SetIsLoadedEntity(true);
			}

			// Finish spawning now that flags are set, which triggers BeginPlay() WITH the loaded flags already true
			SpawnedPawn->FinishSpawning(SpawnTransform);
		}
	}

	return SpawnedPawn;
}

void ASovereignDungeonGameMode::BeginPlay()
{
	// 1. Locate or spawn the dungeon generator BEFORE Super::BeginPlay() so EntranceTransform is ready if PostLogin triggers during Super
	UWorld* World = GetWorld();
	if (World && !ActiveDungeonGenerator)
	{
		TArray<AActor*> FoundGenerators;
		UGameplayStatics::GetAllActorsOfClass(World, ASovereignDungeonGenerator::StaticClass(), FoundGenerators);
		if (FoundGenerators.Num() > 0)
		{
			ActiveDungeonGenerator = Cast<ASovereignDungeonGenerator>(FoundGenerators[0]);
		}
		else
		{
			UClass* GenClassToSpawn = DungeonGeneratorClass ? DungeonGeneratorClass.Get() : ASovereignDungeonGenerator::StaticClass();

			if (UGameInstance* GI = GetGameInstance())
			{
				if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
				{
					if (DungeonSub->DungeonGeneratorClassOverride)
					{
						GenClassToSpawn = DungeonSub->DungeonGeneratorClassOverride;
					}
					else if (!DungeonSub->DungeonGeneratorClassPath.IsEmpty())
					{
						FString GenPath = DungeonSub->DungeonGeneratorClassPath;
						if (!GenPath.StartsWith(TEXT("/Script/")) && !GenPath.EndsWith(TEXT("_C")))
						{
							GenPath += TEXT("_C");
						}
						if (UClass* LoadedGenClass = LoadClass<ASovereignDungeonGenerator>(nullptr, *GenPath))
						{
							GenClassToSpawn = LoadedGenClass;
						}
					}
				}
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ActiveDungeonGenerator = World->SpawnActor<ASovereignDungeonGenerator>(GenClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] No generator found in scene. Auto-spawned %s (Class: %s) at runtime."),
				ActiveDungeonGenerator ? *ActiveDungeonGenerator->GetName() : TEXT("None"),
				GenClassToSpawn ? *GenClassToSpawn->GetName() : TEXT("None"));
		}
	}

	Super::BeginPlay();

	InitializeDungeonSystems();

	// 2. Align local player controller if already logged in and not yet configured by PostLogin
	if (World && !DungeonPartyLeader)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			SetupPartyLeaderAtEntrance(PC);
		}
	}
}

ASovereignBattleManager* ASovereignDungeonGameMode::GetBattleManager(int32 BiomeIndex) const
{
	// 1. Prefer the dedicated Battle Arena spawned by the Dungeon Generator for this biome
	if (ActiveDungeonGenerator)
	{
		if (ASovereignBattleManager* BiomeArena = ActiveDungeonGenerator->GetBattleArenaForBiome(BiomeIndex))
		{
			return BiomeArena;
		}
	}

	// 2. Fallback to cached or world-iterated BattleManager
	if (CachedFallbackBattleManager)
	{
		return CachedFallbackBattleManager;
	}

	if (UWorld* World = GetWorld())
	{
		if (AActor* Found = UGameplayStatics::GetActorOfClass(World, ASovereignBattleManager::StaticClass()))
		{
			return Cast<ASovereignBattleManager>(Found);
		}
	}

	return nullptr;
}

USovereignEncounterSubsystem* ASovereignDungeonGameMode::GetEncounterSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<USovereignEncounterSubsystem>();
	}
	return nullptr;
}

void ASovereignDungeonGameMode::InitializeDungeonSystems()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Verify EncounterSubsystem is online
	USovereignEncounterSubsystem* EncounterSub = GetEncounterSubsystem();
	if (!EncounterSub)
	{
		UE_LOG(LogTemp, Error, TEXT("[DungeonGameMode] USovereignEncounterSubsystem failed to initialize!"));
	}

	// 2. Ensure at least one BattleManager arena exists (uses Generator's arena first, spawns fallback only if missing)
	ASovereignBattleManager* Arena = GetBattleManager(0);
	if (!Arena)
	{
		UClass* ManagerClass = FallbackBattleManagerClass ? FallbackBattleManagerClass.Get() : ASovereignBattleManager::StaticClass();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn far off-map so it never collides with dungeon corridors
		CachedFallbackBattleManager = World->SpawnActor<ASovereignBattleManager>(
			ManagerClass,
			FVector(20000.0f, 20000.0f, 3000.0f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		Arena = CachedFallbackBattleManager;
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGameMode] No biome arena found on generator; spawned fallback BattleManager [%s]."),
			Arena ? *Arena->GetName() : TEXT("None"));
	}

	if (Arena && EncounterSub)
	{
		EncounterSub->ActiveBattleManager = Arena;
		UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Combat systems online -> BattleManager: [%s], EncounterSubsystem: Ready."), *Arena->GetName());
	}
}

bool ASovereignDungeonGameMode::TriggerCombatEncounter(USovereignEncounterDefinition* EncounterDefinition, AActor* SourceActor, int32 BiomeIndex)
{
	if (!EncounterDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGameMode] TriggerCombatEncounter aborted: EncounterDefinition is null."));
		return false;
	}

	if (!DungeonPartyLeader)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGameMode] TriggerCombatEncounter aborted: No active DungeonPartyLeader."));
		return false;
	}

	USovereignEncounterSubsystem* EncounterSub = GetEncounterSubsystem();
	if (!EncounterSub)
	{
		UE_LOG(LogTemp, Error, TEXT("[DungeonGameMode] TriggerCombatEncounter aborted: USovereignEncounterSubsystem unavailable."));
		return false;
	}

	if (EncounterSub->bIsEncounterActive)
	{
		return false;
	}

	ASovereignBattleManager* TargetArena = GetBattleManager(BiomeIndex);
	return EncounterSub->TriggerEncounterWithDefinition(DungeonPartyLeader, SourceActor, EncounterDefinition, TargetArena);
}

void ASovereignDungeonGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	SetupPartyLeaderAtEntrance(NewPlayer);
}

void ASovereignDungeonGameMode::SetupPartyLeaderAtEntrance(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	APawn* ControlledPawn = PC->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Guard against duplicate execution if both PostLogin and BeginPlay fire for the same pawn
	if (DungeonPartyLeader && (DungeonPartyLeader == ControlledPawn))
	{
		return;
	}

	// If player spawned as a Wisp, check for its possessed host vessel
	if (ASovereignPlayerWisp* Wisp = Cast<ASovereignPlayerWisp>(ControlledPawn))
	{
		if (AActor* Host = Wisp->GetCurrentHost())
		{
			DungeonPartyLeader = Cast<ASovereignBaseCharacter>(Host);
		}
	}
	else if (ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(ControlledPawn))
	{
		DungeonPartyLeader = Character;
	}

	UWorld* World = GetWorld();

	// Restore cached vessel stats, attributes, Qi, inventory, and bio vitals from DungeonSubsystem
	if (DungeonPartyLeader)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				if (!DungeonSub->PartyLeaderStateJson.IsEmpty())
				{
					DungeonPartyLeader->SetIsLoadedCharacter(true);
					if (USovereignSaveableEntityComponent* SaveComp = DungeonPartyLeader->FindComponentByClass<USovereignSaveableEntityComponent>())
					{
						SaveComp->SetIsLoadedEntity(true);
					}
					ApplyJsonStateDeferred(World, DungeonPartyLeader, DungeonSub->PartyLeaderStateJson, TEXT("party leader vessel"));
				}

				// Ticket B-122: Reconstitute Non-Leader Party Members on the dedicated Battle Arena
				USovereignPartyComponent* PartyComp = DungeonPartyLeader->GetPartyComponent();

				if (PartyComp && World && DungeonSub->ActivePartyMembersData.Num() > 0)
				{
					for (const FSovereignPartyMemberData& MemberData : DungeonSub->ActivePartyMembersData)
					{
						// Leader is already spawned and possessed
						if (MemberData.bIsLeader)
						{
							if (PartyComp->GridSlots.IsValidIndex(MemberData.SlotIndex))
							{
								PartyComp->AssignPartyMember(MemberData.SlotIndex, DungeonPartyLeader, true);
							}
							continue;
						}

						if (MemberData.ClassPath.IsEmpty())
						{
							continue;
						}

						UClass* MemberClass = ResolveSovereignPawnClass(MemberData.ClassPath);
						if (!MemberClass || !MemberClass->IsChildOf(ASovereignBaseCharacter::StaticClass()))
						{
							MemberClass = ASovereignBaseCharacter::StaticClass();
						}

						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						ASovereignBaseCharacter* SpawnedMember = World->SpawnActor<ASovereignBaseCharacter>(MemberClass, FVector(0, 0, -50000.0f), FRotator::ZeroRotator, SpawnParams);

						if (SpawnedMember)
						{
							SpawnedMember->SetIsLoadedCharacter(true);
							if (USovereignSaveableEntityComponent* MemberSaveComp = SpawnedMember->FindComponentByClass<USovereignSaveableEntityComponent>())
							{
								MemberSaveComp->SetIsLoadedEntity(true);
							}
							if (!MemberData.StateJson.IsEmpty())
							{
								ApplyJsonStateDeferred(World, SpawnedMember, MemberData.StateJson, FString::Printf(TEXT("party member slot %d"), MemberData.SlotIndex));
							}

							SpawnedMember->SetActorHiddenInGame(true);
							SpawnedMember->SetActorEnableCollision(false);

							if (PartyComp->GridSlots.IsValidIndex(MemberData.SlotIndex))
							{
								PartyComp->AssignPartyMember(MemberData.SlotIndex, SpawnedMember, false);
								UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Reconstituted party member [%s] in slot %d."),
									*SpawnedMember->GetName(), MemberData.SlotIndex);
							}
						}
					}
				}
			}
		}

		// Ensure the dragon vessel's Input Mapping Context is active on the local player controller
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (UInputMappingContext* VesselIMC = DungeonPartyLeader->GetDefaultMappingContext())
				{
					Subsystem->RemoveMappingContext(VesselIMC);
					Subsystem->AddMappingContext(VesselIMC, 0);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Activated vessel DefaultMappingContext [%s] for %s"),
						*VesselIMC->GetName(), *DungeonPartyLeader->GetName());
				}
			}
		}
	}

	// Teleport the party leader to the Entrance Room WorldTransform
	if (ActiveDungeonGenerator && ControlledPawn)
	{
		FTransform EntranceTransform = ActiveDungeonGenerator->GetEntranceTransform();
		EntranceTransform.AddToTranslation(FVector(0.0, 0.0, 1000.0));

		ControlledPawn->SetActorTransform(EntranceTransform, false, nullptr, ETeleportType::TeleportPhysics);
		PC->SetControlRotation(EntranceTransform.Rotator());
		UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Teleported party leader to Entrance Room (%s)"),
			*EntranceTransform.GetLocation().ToString());
	}

	ConfigurePartyForExploration();
}

void ASovereignDungeonGameMode::ConfigurePartyForExploration()
{
	if (!DungeonPartyLeader)
	{
		return;
	}

	// Query party members: keep leader active, hide other party members and anchor them on the battle arena
	USovereignPartyComponent* PartyComp = DungeonPartyLeader->GetPartyComponent();
	if (!PartyComp)
	{
		return;
	}

	ASovereignBattleManager* BattleArena = ActiveDungeonGenerator ? ActiveDungeonGenerator->GetBattleArenaForBiome(0) : nullptr;

	for (int32 SlotIdx = 0; SlotIdx < PartyComp->GridSlots.Num(); ++SlotIdx)
	{
		const FSovereignPartySlot& Slot = PartyComp->GridSlots[SlotIdx];
		if (!Slot.IsOccupied() || !Slot.MemberCharacter.IsValid())
		{
			continue;
		}

		ASovereignBaseCharacter* Member = Slot.MemberCharacter.Get();
		if (Member == DungeonPartyLeader)
		{
			// Leader is visible and active exploring the dungeon corridors
			Member->SetActorHiddenInGame(false);
			Member->SetActorEnableCollision(true);
		}
		else
		{
			// Non-leader party members remain hidden during dungeon corridor traversal
			Member->SetActorHiddenInGame(true);
			Member->SetActorEnableCollision(false);

			// Pre-anchor to battle platform slot transforms so they are in position when combat triggers
			if (BattleArena && BattleArena->PlayerSlotTransforms.IsValidIndex(SlotIdx))
			{
				Member->SetActorTransform(BattleArena->PlayerSlotTransforms[SlotIdx], false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
}
