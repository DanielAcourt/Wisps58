// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignDungeonGenerator.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ASovereignDungeonGenerator::ASovereignDungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	TargetRoomCount = 8;
	GenerationSeed = 1337;
	EnemyBuffMultiplier = 1.0f;
	RewardMultiplier = 1.0f;
	FarOffArenaOrigin = FVector(50000.0f, 50000.0f, 5000.0f);
	StepThresholdDistance = 1500.0f;
	RandomEncounterChance = 25.0f;
	AccumulatedStepDistance = 0.0f;
	BossNodeIndex = INDEX_NONE;

	bAutoSpawnDefaultLighting = true;
	bSpawnDirectionalLight = true;
	bSpawnPostProcessVolume = true;
	bSpawnPhysicalGeometry = true;
	AmbientLightIntensity = 1000.0f;
	AmbientLightColor = FColor(180, 205, 235);

	// Setup instanced mesh components for draw call efficiency
	FloorInstancedComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstancedComponent"));
	if (!RootComponent)
	{
		SetRootComponent(FloorInstancedComponent);
	}
	else
	{
		FloorInstancedComponent->SetupAttachment(RootComponent);
	}

	WallInstancedComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstancedComponent"));
	WallInstancedComponent->SetupAttachment(FloorInstancedComponent);

	// Default fallback mesh finder for basic cube/floor shapes if custom modular assets aren't yet loaded
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeObj(TEXT("/Engine/BasicShapes/Cube"));
	if (DefaultCubeObj.Succeeded())
	{
		FloorMesh = DefaultCubeObj.Object;
		WallMesh = DefaultCubeObj.Object;
	}
}

void ASovereignDungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Ingest expedition parameters from GameInstance Subsystem if arriving from gateway transition
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSubsystem = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				if (DungeonSubsystem->bHasActiveExpedition)
				{
					GenerationSeed = DungeonSubsystem->ActiveExpeditionSeed;
					EnemyBuffMultiplier = DungeonSubsystem->ActiveEnemyBuffMultiplier;
					RewardMultiplier = DungeonSubsystem->ActiveRewardMultiplier;

					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Expedition parameters ingested: Seed=%d, EnemyBuff=x%f, RewardMult=x%f"),
						GenerationSeed, EnemyBuffMultiplier, RewardMultiplier);
				}
			}
		}
	}

	GenerateDungeon();
}

void ASovereignDungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ASovereignDungeonGenerator::GenerateDungeon()
{
	DungeonNodes.Empty();
	FRandomStream Stream(GenerationSeed != 0 ? GenerationSeed : FMath::Rand());

	const FVector Origin = GetActorLocation();

	// 1. Create Entrance Node at (0, 0)
	FSovereignDungeonNode EntranceNode;
	EntranceNode.GridCoord = FIntPoint(0, 0);
	EntranceNode.TileType = EDungeonTileType::EntranceRoom;
	EntranceNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin);
	EntranceNode.bHasFixedEliteMob = false;
	DungeonNodes.Add(EntranceNode);

	TSet<FIntPoint> OccupiedCoords;
	OccupiedCoords.Add(EntranceNode.GridCoord);

	FIntPoint CurrentCoord = EntranceNode.GridCoord;

	// 2. Random Walk Graph Generation (Connecting Corridors and Chambers)
	for (int32 i = 1; i < TargetRoomCount; ++i)
	{
		// Pick random cardinal direction (North, South, East, West)
		TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
		FIntPoint Step = Directions[Stream.RandRange(0, Directions.Num() - 1)];
		CurrentCoord += Step;

		// Prevent infinite overlap
		int32 Retries = 0;
		while (OccupiedCoords.Contains(CurrentCoord) && Retries < 10)
		{
			Step = Directions[Stream.RandRange(0, Directions.Num() - 1)];
			CurrentCoord += Step;
			Retries++;
		}
		OccupiedCoords.Add(CurrentCoord);

		FSovereignDungeonNode Node;
		Node.GridCoord = CurrentCoord;
		Node.TileType = (i % 2 == 0) ? EDungeonTileType::Chamber : EDungeonTileType::Corridor;
		Node.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(CurrentCoord.X * TILE_SIZE, CurrentCoord.Y * TILE_SIZE, 0.0f));

		// 20% chance of a fixed elite guard on chambers
		Node.bHasFixedEliteMob = (Node.TileType == EDungeonTileType::Chamber && Stream.FRand() < 0.25f);
		DungeonNodes.Add(Node);
	}

	// 3. Place Terminal Boss Chamber at the furthest point
	int32 MaxDistSq = -1;
	int32 BestBossIdx = INDEX_NONE;

	for (int32 i = 0; i < DungeonNodes.Num(); ++i)
	{
		const int32 DistSq = (DungeonNodes[i].GridCoord.X * DungeonNodes[i].GridCoord.X) +
			                 (DungeonNodes[i].GridCoord.Y * DungeonNodes[i].GridCoord.Y);
		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			BestBossIdx = i;
		}
	}

	if (BestBossIdx != INDEX_NONE)
	{
		DungeonNodes[BestBossIdx].TileType = EDungeonTileType::BossChamber;
		DungeonNodes[BestBossIdx].bHasFixedEliteMob = true; // Floor Boss lives here!
		BossNodeIndex = BestBossIdx;
	}

	// 4. Spawn/Configure Far-Off Battle Arenas
	SpawnFarOffBattleArenas();

	// 5. Spawn 3D Physical Geometry (Ticket B-089)
	if (bSpawnPhysicalGeometry)
	{
		SpawnDungeonGeometry();
	}

	// 6. Spawn Default Lighting Rig if world is blank/unlit
	if (bAutoSpawnDefaultLighting)
	{
		SpawnDefaultLightingRig();
	}

	OnDungeonGenerated.Broadcast(DungeonNodes.Num());
	return true;
}

void ASovereignDungeonGenerator::SpawnDungeonGeometry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Prepare Instanced Static Mesh Components
	if (!FloorInstancedComponent)
	{
		FloorInstancedComponent = NewObject<UInstancedStaticMeshComponent>(this, TEXT("RuntimeFloorInstancedComponent"));
		FloorInstancedComponent->RegisterComponent();
	}
	if (!WallInstancedComponent)
	{
		WallInstancedComponent = NewObject<UInstancedStaticMeshComponent>(this, TEXT("RuntimeWallInstancedComponent"));
		WallInstancedComponent->RegisterComponent();
	}

	FloorInstancedComponent->ClearInstances();
	WallInstancedComponent->ClearInstances();

	if (FloorMesh)
	{
		FloorInstancedComponent->SetStaticMesh(FloorMesh);
	}
	if (WallMesh)
	{
		WallInstancedComponent->SetStaticMesh(WallMesh);
	}

	TSet<FIntPoint> OccupiedCoords;
	for (const FSovereignDungeonNode& Node : DungeonNodes)
	{
		OccupiedCoords.Add(Node.GridCoord);
	}

	const FVector Origin = GetActorLocation();

	// 2. Spawn Floor Tiles and Boundary Walls for Each Node
	for (int32 NodeIdx = 0; NodeIdx < DungeonNodes.Num(); ++NodeIdx)
	{
		const FSovereignDungeonNode& Node = DungeonNodes[NodeIdx];
		const FVector NodePos = Node.WorldTransform.GetLocation();

		// Add Floor Instance: Scale standard 100cm cube to 1000cm x 1000cm x 20cm
		FTransform FloorTransform;
		FloorTransform.SetLocation(NodePos - FVector(0.0f, 0.0f, 10.0f));
		FloorTransform.SetRotation(FQuat::Identity);
		FloorTransform.SetScale3D(FVector(TILE_SIZE / 100.0f, TILE_SIZE / 100.0f, 0.2f));

		FloorInstancedComponent->AddInstance(FloorTransform);

		// Check 4 cardinal directions (North, South, East, West) for boundary edges
		const TArray<TPair<FIntPoint, FVector>> CardinalOffsets = {
			TPair<FIntPoint, FVector>(FIntPoint(1, 0), FVector(TILE_SIZE * 0.5f, 0.0f, 300.0f)),   // North Wall
			TPair<FIntPoint, FVector>(FIntPoint(-1, 0), FVector(-TILE_SIZE * 0.5f, 0.0f, 300.0f)),  // South Wall
			TPair<FIntPoint, FVector>(FIntPoint(0, 1), FVector(0.0f, TILE_SIZE * 0.5f, 300.0f)),   // East Wall
			TPair<FIntPoint, FVector>(FIntPoint(0, -1), FVector(0.0f, -TILE_SIZE * 0.5f, 300.0f))   // West Wall
		};

		const TArray<FRotator> CardinalRotations = {
			FRotator(0.0f, 0.0f, 0.0f),
			FRotator(0.0f, 180.0f, 0.0f),
			FRotator(0.0f, 90.0f, 0.0f),
			FRotator(0.0f, -90.0f, 0.0f)
		};

		for (int32 DirIdx = 0; DirIdx < CardinalOffsets.Num(); ++DirIdx)
		{
			const FIntPoint NeighborCoord = Node.GridCoord + CardinalOffsets[DirIdx].Key;
			if (!OccupiedCoords.Contains(NeighborCoord))
			{
				// Unlinked exterior edge: Place a boundary wall instance
				FTransform WallTransform;
				WallTransform.SetLocation(NodePos + CardinalOffsets[DirIdx].Value);
				WallTransform.SetRotation(CardinalRotations[DirIdx].Quaternion());
				// Scale standard 100cm cube to 0.4m thick, 10m long, 6m high
				if (CardinalOffsets[DirIdx].Key.X != 0)
				{
					WallTransform.SetScale3D(FVector(0.4f, TILE_SIZE / 100.0f, 6.0f));
				}
				else
				{
					WallTransform.SetScale3D(FVector(TILE_SIZE / 100.0f, 0.4f, 6.0f));
				}

				WallInstancedComponent->AddInstance(WallTransform);
			}
		}

		// 3. For Terminal Boss Chamber: Spawn Boss Altar / Egg Prop
		if (Node.TileType == EDungeonTileType::BossChamber)
		{
			if (BossAltarMesh)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AStaticMeshActor* AltarActor = World->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(), NodePos + FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, SpawnParams
				);
				if (AltarActor && AltarActor->GetStaticMeshComponent())
				{
					AltarActor->GetStaticMeshComponent()->SetStaticMesh(BossAltarMesh);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned 3D Dungeon Geometry: %d Floors, %d Walls."),
		FloorInstancedComponent->GetInstanceCount(), WallInstancedComponent->GetInstanceCount());
}

void ASovereignDungeonGenerator::SpawnDefaultLightingRig()
{
	if (!bAutoSpawnDefaultLighting)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Directional Fill Light (optional sub-toggle, skips if level already defines lights)
	if (bSpawnDirectionalLight)
	{
		TArray<AActor*> ExistingLights;
		UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), ExistingLights);

		if (ExistingLights.Num() == 0)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const FVector LightPos = GetActorLocation() + FVector(0.0f, 0.0f, 3000.0f);
			const FRotator LightRot = FRotator(-50.0f, -45.0f, 0.0f); // Angled down

			if (ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), LightPos, LightRot, SpawnParams))
			{
				if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent()))
				{
					LightComp->SetIntensity(AmbientLightIntensity);
					LightComp->SetLightColor(FLinearColor(AmbientLightColor));
					LightComp->bAtmosphereSunLight = false;
					LightComp->bCastVolumetricShadow = false;
				}
				UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Blank level detected: Auto-spawned fallback DirectionalLight."));
			}
		}
	}

	// 2. PostProcessVolume with bounded exposure (optional sub-toggle, skips if level already defines one)
	if (bSpawnPostProcessVolume)
	{
		TArray<AActor*> ExistingPostProcess;
		UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), ExistingPostProcess);

		if (ExistingPostProcess.Num() == 0)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams))
			{
				PPVolume->bUnbound = true; // Applies globally across the whole dungeon
				PPVolume->Settings.bOverride_AutoExposureMinBrightness = true;
				PPVolume->Settings.bOverride_AutoExposureMaxBrightness = true;
				PPVolume->Settings.AutoExposureMinBrightness = 0.5f;
				PPVolume->Settings.AutoExposureMaxBrightness = 2.0f;
				UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Blank level detected: Auto-spawned fallback PostProcessVolume with exposure bounds."));
			}
		}
	}
}

void ASovereignDungeonGenerator::SpawnFarOffBattleArenas()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	BiomeBattleArenas.Empty();

	// 4 Biomes: Earth Cave, Lava Vault, Crystal Sanctum, Spirit Forest
	// Offset each arena platform by 5,000 units so they never touch or bleed visual effects
	TArray<FVector> BiomeOffsets = {
		FVector(0.0f, 0.0f, 0.0f),       // Earth
		FVector(5000.0f, 0.0f, 0.0f),    // Lava
		FVector(0.0f, 5000.0f, 0.0f),    // Crystal
		FVector(5000.0f, 5000.0f, 0.0f)  // Forest
	};

	for (int32 i = 0; i < BiomeOffsets.Num(); ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FVector ArenaPos = FarOffArenaOrigin + BiomeOffsets[i];

		ASovereignBattleManager* Arena = World->SpawnActor<ASovereignBattleManager>(
			ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams
		);

		if (Arena)
		{
			Arena->GenerateDefaultGridTransforms();
			BiomeBattleArenas.Add(Arena);
		}
	}
}

void ASovereignDungeonGenerator::ProcessPlayerMovement(ASovereignBaseCharacter* PlayerLeader, float DeltaDistance)
{
	if (!PlayerLeader || DeltaDistance <= 0.0f)
	{
		return;
	}

	AccumulatedStepDistance += DeltaDistance;

	// Check if step threshold reached
	if (AccumulatedStepDistance >= StepThresholdDistance)
	{
		AccumulatedStepDistance = 0.0f;

		// Roll for Pokemon/FF7 random encounter
		const float Roll = FMath::FRandRange(0.0f, 100.0f);
		if (Roll <= RandomEncounterChance)
		{
			UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Random step encounter triggered! (Roll: %f <= Chance: %f)"), Roll, RandomEncounterChance);

			if (UWorld* World = GetWorld())
			{
				if (USovereignEncounterSubsystem* Subsystem = World->GetSubsystem<USovereignEncounterSubsystem>())
				{
					// Pick an active biome arena
					ASovereignBattleManager* ActiveArena = GetBattleArenaForBiome(0);

					// Spawn temporary random enemies or trigger battle manager
					TArray<ASovereignBaseCharacter*> EnemyUnits;
					Subsystem->TriggerEncounter(PlayerLeader, nullptr, EnemyUnits, true, ActiveArena);
				}
			}
		}
	}
}

FTransform ASovereignDungeonGenerator::GetEntranceTransform() const
{
	if (DungeonNodes.Num() > 0)
	{
		return DungeonNodes[0].WorldTransform;
	}
	return GetActorTransform();
}

FTransform ASovereignDungeonGenerator::GetBossChamberTransform() const
{
	if (BossNodeIndex >= 0 && BossNodeIndex < DungeonNodes.Num())
	{
		return DungeonNodes[BossNodeIndex].WorldTransform;
	}
	return GetActorTransform();
}

ASovereignBattleManager* ASovereignDungeonGenerator::GetBattleArenaForBiome(int32 BiomeIndex) const
{
	if (BiomeBattleArenas.Num() > 0)
	{
		int32 SafeIdx = FMath::Clamp(BiomeIndex, 0, BiomeBattleArenas.Num() - 1);
		return BiomeBattleArenas[SafeIdx];
	}
	return nullptr;
}

bool ASovereignDungeonGenerator::ReturnToGarden()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USovereignDungeonSubsystem* DungeonSubsystem = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				return DungeonSubsystem->ReturnToGarden(this);
			}
		}
	}
	return false;
}
