// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignDungeonGenerator.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Engine/World.h"

ASovereignDungeonGenerator::ASovereignDungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	TargetRoomCount = 8;
	GenerationSeed = 1337;
	FarOffArenaOrigin = FVector(50000.0f, 50000.0f, 5000.0f);
	StepThresholdDistance = 1500.0f;
	RandomEncounterChance = 25.0f;
	AccumulatedStepDistance = 0.0f;
	BossNodeIndex = INDEX_NONE;
}

void ASovereignDungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
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

	OnDungeonGenerated.Broadcast(DungeonNodes.Num());
	return true;
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
