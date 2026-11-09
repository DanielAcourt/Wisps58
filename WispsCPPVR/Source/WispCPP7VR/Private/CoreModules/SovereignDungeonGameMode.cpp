// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "CoreModules/SovereignDungeonGameMode.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Combat/SovereignDungeonGenerator.h"
#include "Combat/SovereignBattleManager.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Components/SovereignPartyComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

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
					if (!ClassPath.EndsWith(TEXT("_C")))
					{
						ClassPath += TEXT("_C");
					}

					if (UClass* LoadedClass = LoadClass<APawn>(nullptr, *ClassPath))
					{
						UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Dynamic vessel pawn class resolved for expedition: %s"), *LoadedClass->GetName());
						return LoadedClass;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[DungeonGameMode] Failed to load party leader vessel class [%s], using default."), *ClassPath);
					}
				}
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ASovereignDungeonGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. Locate the dungeon generator in the world (or auto-spawn if none exists)
	TArray<AActor*> FoundGenerators;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASovereignDungeonGenerator::StaticClass(), FoundGenerators);
	if (FoundGenerators.Num() > 0)
	{
		ActiveDungeonGenerator = Cast<ASovereignDungeonGenerator>(FoundGenerators[0]);
	}
	else if (GetWorld())
	{
		UClass* GenClassToSpawn = DungeonGeneratorClass ? DungeonGeneratorClass.Get() : ASovereignDungeonGenerator::StaticClass();

		// Check if DungeonSubsystem specifies a custom Blueprint generator class from the gateway
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
					if (!GenPath.EndsWith(TEXT("_C")))
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
		ActiveDungeonGenerator = GetWorld()->SpawnActor<ASovereignDungeonGenerator>(GenClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] No generator found in scene. Auto-spawned %s (Class: %s) at runtime."),
			ActiveDungeonGenerator ? *ActiveDungeonGenerator->GetName() : TEXT("None"),
			GenClassToSpawn ? *GenClassToSpawn->GetName() : TEXT("None"));
	}

	// 2. Align local player controller if already logged in
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			SetupPartyLeaderAtEntrance(PC);
		}
	}
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

	// If player spawned as a Wisp, ensure it is possessing or transitioning into the party leader vessel
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

	// Restore cached vessel stats, attributes, Qi, inventory, and bio vitals from DungeonSubsystem
	if (DungeonPartyLeader)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				if (!DungeonSub->PartyLeaderStateJson.IsEmpty())
				{
					TSharedPtr<FJsonObject> StateJsonObj;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DungeonSub->PartyLeaderStateJson);
					if (FJsonSerializer::Deserialize(Reader, StateJsonObj) && StateJsonObj.IsValid())
					{
						if (USovereignSaveableEntityComponent* SaveComp = DungeonPartyLeader->FindComponentByClass<USovereignSaveableEntityComponent>())
						{
							SaveComp->ApplyStateFromJsonObject(StateJsonObj);
							UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Restored runtime stats and inventory to party leader vessel [%s] from subsystem state."),
								*DungeonPartyLeader->GetName());
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
	if (ActiveDungeonGenerator)
	{
		const FTransform EntranceTransform = ActiveDungeonGenerator->GetEntranceTransform();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorTransform(EntranceTransform, false, nullptr, ETeleportType::TeleportPhysics);
			PC->SetControlRotation(EntranceTransform.Rotator());
			UE_LOG(LogTemp, Log, TEXT("[DungeonGameMode] Teleported party leader to Entrance Room (%s)"),
				*EntranceTransform.GetLocation().ToString());
		}
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
