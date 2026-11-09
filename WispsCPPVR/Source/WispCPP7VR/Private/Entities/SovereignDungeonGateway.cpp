// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Entities/SovereignDungeonGateway.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "SaveSystem/SovereignSaveManager.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Inventory/SovereignItemDomainFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ASovereignDungeonGateway::ASovereignDungeonGateway()
{
	bIsUnlocked = false;
	bAutoUnlockOnHatch = true;
	InfusedQi = 0.0f;

	GatewayDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatewayDoorMesh"));
	if (RootComponent)
	{
		GatewayDoorMesh->SetupAttachment(RootComponent);
	}
	else
	{
		RootComponent = GatewayDoorMesh;
	}

	SacrificeInventory = CreateDefaultSubobject<USovereignInventoryComponent>(TEXT("SacrificeInventory"));
	SacrificeInventory->MaxSlotCount = 10;
}

void ASovereignDungeonGateway::SetGatewayUnlocked(bool bNewUnlocked)
{
	if (bIsUnlocked != bNewUnlocked)
	{
		bIsUnlocked = bNewUnlocked;
		OnGatewayUnlocked.Broadcast(bIsUnlocked);

		if (GatewayDoorMesh)
		{
			// When unlocked, door collision opens
			GatewayDoorMesh->SetCollisionEnabled(bIsUnlocked ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
		}
	}
}

bool ASovereignDungeonGateway::ChannelQiFromCharacter(ASovereignBaseCharacter* Character, float Amount)
{
	if (!Character || Amount <= 0.0f)
	{
		return false;
	}

	USovereignQiComponent* QiComp = Character->GetQiComponent();
	if (!QiComp || QiComp->CurrentQi < Amount)
	{
		return false;
	}

	QiComp->CurrentQi -= Amount;
	InfusedQi += Amount;

	OnGatewayEmpowered.Broadcast(CalculateEnemyBuffMultiplier(), CalculateRewardMultiplier());
	return true;
}

float ASovereignDungeonGateway::GetTotalSacrificedMass() const
{
	if (!SacrificeInventory)
	{
		return 0.0f;
	}

	float TotalMass = 0.0f;
	for (const FSovereignInventoryItemInstance& Item : SacrificeInventory->InventorySlots)
	{
		const USovereignItemDefinition* Def = Item.ItemDefinition.Get();
		if (!Def)
		{
			Def = Item.ItemDefinition.LoadSynchronous();
		}

		if (Def)
		{
			if (const USovereignItemFragment_Weight* WeightFrag = Def->FindFragment<USovereignItemFragment_Weight>())
			{
				TotalMass += WeightFrag->GetStackWeight(Item.Quantity);
			}
			else
			{
				TotalMass += 1.0f * Item.Quantity;
			}
		}
	}
	return TotalMass;
}

float ASovereignDungeonGateway::CalculateEnemyBuffMultiplier() const
{
	const float Mass = GetTotalSacrificedMass();
	// 1.0 + (SacrificeMass * 0.05) + (InfusedQi * 0.002)
	return 1.0f + (Mass * 0.05f) + (InfusedQi * 0.002f);
}

float ASovereignDungeonGateway::CalculateRewardMultiplier() const
{
	const float Mass = GetTotalSacrificedMass();
	// 1.0 + (SacrificeMass * 0.10) + (InfusedQi * 0.005)
	return 1.0f + (Mass * 0.10f) + (InfusedQi * 0.005f);
}

bool ASovereignDungeonGateway::EmbarkExpedition(ASovereignBaseCharacter* PartyLeader)
{
	if (!bIsUnlocked || !PartyLeader)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGateway] Cannot embark: Gateway is sealed or Leader is invalid."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] Expedition launched! Enemy Buff: x%f, Reward Mult: x%f"),
		CalculateEnemyBuffMultiplier(), CalculateRewardMultiplier());

	return true;
}

void ASovereignDungeonGateway::ConsumeSacrifices()
{
	if (SacrificeInventory)
	{
		SacrificeInventory->InventorySlots.Empty();
		SacrificeInventory->OnInventoryUpdated.Broadcast();
	}
	InfusedQi = 0.0f;
	OnGatewayEmpowered.Broadcast(1.0f, 1.0f);
}

bool ASovereignDungeonGateway::EnterDungeon(ASovereignBaseCharacter* PartyLeader)
{
	if (!bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGateway] Cannot enter dungeon: Gateway is sealed/locked."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World && PartyLeader)
	{
		World = PartyLeader->GetWorld();
	}

	// 1. Calculate Multipliers prior to consuming sacrifices
	const float EnemyBuff = CalculateEnemyBuffMultiplier();
	const float RewardMult = CalculateRewardMultiplier();

	// 2. Generate or fetch expedition seed
	const int32 SeedToUse = (ExpeditionSeed != 0) ? ExpeditionSeed : FMath::RandRange(1, 999999);

	// Determine current level name to store as Garden return map
	FName CurrentMapName = NAME_None;
	if (World)
	{
		FString RawMapName = World->GetMapName();
		RawMapName.RemoveFromStart(World->StreamingLevelsPrefix);
		CurrentMapName = FName(*RawMapName);
	}

	// 3. Resolve the actual dragon / vessel entity if possessed by a wisp or directly controlled
	ASovereignBaseCharacter* EffectiveLeaderVessel = PartyLeader;
	if (ASovereignPlayerWisp* Wisp = Cast<ASovereignPlayerWisp>(PartyLeader))
	{
		if (AActor* Host = Wisp->GetCurrentHost())
		{
			if (ASovereignBaseCharacter* HostChar = Cast<ASovereignBaseCharacter>(Host))
			{
				EffectiveLeaderVessel = HostChar;
			}
		}
	}

	// 4. Save Garden World State
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (GI)
	{
		if (USaveManager* SaveManager = GI->GetSubsystem<USaveManager>())
		{
			UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] Auto-saving Garden state to slot [%s] (JSON)..."), *GardenSaveSlot);
			SaveManager->SaveWorldState(GardenSaveSlot, true);
		}

		// 5. Record Expedition parameters and Party Leader Vessel in GameInstance Subsystem
		if (USovereignDungeonSubsystem* DungeonSubsystem = GI->GetSubsystem<USovereignDungeonSubsystem>())
		{
			DungeonSubsystem->TargetRoomCount = TargetRoomCount;
			DungeonSubsystem->SpecialRoomPercentage = SpecialRoomPercentage;
			DungeonSubsystem->TreasureRoomRatio = TreasureRoomRatio;
			DungeonSubsystem->RestSanctuaryRatio = RestSanctuaryRatio;
			DungeonSubsystem->GrandChamberRatio = GrandChamberRatio;
			DungeonSubsystem->ColosseumRatio = ColosseumRatio;

			if (DungeonGeneratorBlueprintClass)
			{
				DungeonSubsystem->DungeonGeneratorClassOverride = DungeonGeneratorBlueprintClass;
				DungeonSubsystem->DungeonGeneratorClassPath = DungeonGeneratorBlueprintClass->GetPathName();
				UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] Configured custom Dungeon Generator class [%s] for expedition."),
					*DungeonSubsystem->DungeonGeneratorClassPath);
			}

			DungeonSubsystem->RecordExpedition(SeedToUse, EnemyBuff, RewardMult, CurrentMapName, GardenSaveSlot);

			if (EffectiveLeaderVessel)
			{
				const FString VesselClassPath = EffectiveLeaderVessel->GetClass()->GetPathName();
				FGuid VesselGuid;
				FString SerializedStateJson;

				if (USovereignSaveableEntityComponent* SaveComp = EffectiveLeaderVessel->FindComponentByClass<USovereignSaveableEntityComponent>())
				{
					VesselGuid = SaveComp->EntityID;
					TSharedPtr<FJsonObject> StateObj = SaveComp->CaptureFullEntityState();
					if (StateObj.IsValid())
					{
						TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedStateJson);
						FJsonSerializer::Serialize(StateObj.ToSharedRef(), Writer);
					}
				}

				DungeonSubsystem->RecordPartyLeaderState(VesselClassPath, VesselGuid, SerializedStateJson);
				UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] Cached party leader vessel [%s] (Class: %s) for dungeon expedition."),
					*EffectiveLeaderVessel->GetName(), *VesselClassPath);
			}
		}
	}

	// 5. Consume sacrifices
	ConsumeSacrifices();

	UE_LOG(LogTemp, Log, TEXT("[DungeonGateway] EnterDungeon: Transitioning to level [%s] with Seed %d (EnemyBuff: x%f, RewardMult: x%f)"),
		*DungeonLevelName.ToString(), SeedToUse, EnemyBuff, RewardMult);

	// 6. Transition level
	if (World && !DungeonLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(World, DungeonLevelName);
	}

	return true;
}

void ASovereignDungeonGateway::OnInteract_Implementation(AActor* Interactor)
{
	Super::OnInteract_Implementation(Interactor);

	if (!bIsUnlocked)
	{
		SetGatewayUnlocked(true);
	}
	else
	{
		ASovereignBaseCharacter* InteractingCharacter = Cast<ASovereignBaseCharacter>(Interactor);
		EnterDungeon(InteractingCharacter);
	}
}

FString ASovereignDungeonGateway::GetInteractionHint_Implementation()
{
	return bIsUnlocked ? TEXT("Enter Procedural Dungeon") : TEXT("Unseal Dungeon Gateway");
}
