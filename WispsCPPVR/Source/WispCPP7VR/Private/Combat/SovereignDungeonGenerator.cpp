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
	MinSpineLength = 4;
	MaxLoopCount = 0;
	MaxGenerationRetries = 20;

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

bool ASovereignDungeonGenerator::ValidateTopologicalReachability(int32 EntranceIdx, int32 BossIdx) const
{
	if (!DungeonNodes.IsValidIndex(EntranceIdx) || !DungeonNodes.IsValidIndex(BossIdx))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Invalid Entrance (%d) or Boss (%d) index."), EntranceIdx, BossIdx);
		return false;
	}

	// 1. Edge Adjacency & Direction Integrity Invariant
	for (const FSovereignDungeonEdge& Edge : DungeonEdges)
	{
		if (Edge.NodeIndexA == Edge.NodeIndexB)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Edge %d self-connects Node %d."), Edge.EdgeId, Edge.NodeIndexA);
			return false;
		}
		if (!DungeonNodes.IsValidIndex(Edge.NodeIndexA) || !DungeonNodes.IsValidIndex(Edge.NodeIndexB))
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Edge %d references invalid nodes (%d, %d)."),
				Edge.EdgeId, Edge.NodeIndexA, Edge.NodeIndexB);
			return false;
		}

		const FIntPoint CoordA = DungeonNodes[Edge.NodeIndexA].GridCoord;
		const FIntPoint CoordB = DungeonNodes[Edge.NodeIndexB].GridCoord;
		const int32 ManhattanDist = FMath::Abs(CoordA.X - CoordB.X) + FMath::Abs(CoordA.Y - CoordB.Y);
		if (ManhattanDist != 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Edge %d connects non-adjacent coordinates (%d,%d) and (%d,%d) (Dist: %d)."),
				Edge.EdgeId, CoordA.X, CoordA.Y, CoordB.X, CoordB.Y, ManhattanDist);
			return false;
		}

		// Verify declared cardinal direction
		FIntPoint ExpectedOffset(0, 0);
		switch (Edge.DirectionAtoB)
		{
		case EDungeonDirection::North: ExpectedOffset = FIntPoint(1, 0); break;
		case EDungeonDirection::East:  ExpectedOffset = FIntPoint(0, 1); break;
		case EDungeonDirection::South: ExpectedOffset = FIntPoint(-1, 0); break;
		case EDungeonDirection::West:  ExpectedOffset = FIntPoint(0, -1); break;
		default: break;
		}

		if (CoordB != CoordA + ExpectedOffset)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Edge %d direction does not match coordinate delta."), Edge.EdgeId);
			return false;
		}
	}

	// 2. Pure-Data BFS Reachability from Entrance
	TSet<int32> Visited;
	TQueue<int32> Queue;
	TMap<int32, int32> Distances;

	Visited.Add(EntranceIdx);
	Queue.Enqueue(EntranceIdx);
	Distances.Add(EntranceIdx, 0);

	while (!Queue.IsEmpty())
	{
		int32 Current = INDEX_NONE;
		Queue.Dequeue(Current);

		const int32 CurrentDist = Distances[Current];

		for (const FSovereignDungeonEdge& Edge : DungeonEdges)
		{
			if (Edge.Connects(Current))
			{
				const int32 Neighbor = Edge.GetOtherNode(Current);
				if (!Visited.Contains(Neighbor))
				{
					Visited.Add(Neighbor);
					Distances.Add(Neighbor, CurrentDist + 1);
					Queue.Enqueue(Neighbor);
				}
			}
		}
	}

	// Invariant: Boss must be reachable
	if (!Visited.Contains(BossIdx))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Boss Chamber (%d) unreachable from Entrance (%d)."), BossIdx, EntranceIdx);
		return false;
	}

	// Invariant: Zero orphan nodes (every node must be reachable)
	if (Visited.Num() != DungeonNodes.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Graph contains %d unreachable orphan nodes (%d visited of %d total)."),
			DungeonNodes.Num() - Visited.Num(), Visited.Num(), DungeonNodes.Num());
		return false;
	}

	// Invariant: Path length to Boss must meet minimum spine threshold
	const int32 PathLengthToBoss = Distances[BossIdx];
	if (PathLengthToBoss < MinSpineLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Path length to Boss (%d) is less than MinSpineLength (%d)."),
			PathLengthToBoss, MinSpineLength);
		return false;
	}

	// Invariant: Exactly 1 entrance edge into Boss Chamber
	int32 BossEdgeCount = 0;
	int32 BossPredecessorIdx = INDEX_NONE;
	const FSovereignDungeonEdge* BossEdgePtr = nullptr;

	for (const FSovereignDungeonEdge& Edge : DungeonEdges)
	{
		if (Edge.Connects(BossIdx))
		{
			BossEdgeCount++;
			BossPredecessorIdx = Edge.GetOtherNode(BossIdx);
			BossEdgePtr = &Edge;
		}
	}

	if (BossEdgeCount != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Boss Chamber has %d connecting edges (must be strictly 1)."), BossEdgeCount);
		return false;
	}

	// Invariant: Node preceding Boss must be an EliteChamber with SealedEliteGate
	if (BossPredecessorIdx != INDEX_NONE && DungeonNodes.IsValidIndex(BossPredecessorIdx))
	{
		if (DungeonNodes[BossPredecessorIdx].TileType != EDungeonTileType::EliteChamber)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Node %d preceding Boss must be EliteChamber, but is %d."),
				BossPredecessorIdx, (int32)DungeonNodes[BossPredecessorIdx].TileType);
			return false;
		}
		if (BossEdgePtr && BossEdgePtr->DoorState != EDoorState::SealedEliteGate)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Boss doorway must have SealedEliteGate state."));
			return false;
		}
	}

	return true;
}

void ASovereignDungeonGenerator::RebuildNodeAdjacencyCaches()
{
	for (int32 i = 0; i < DungeonNodes.Num(); ++i)
	{
		DungeonNodes[i].ConnectedPassageMask = 0;
		DungeonNodes[i].NodeIndex = i;
	}

	for (const FSovereignDungeonEdge& Edge : DungeonEdges)
	{
		if (DungeonNodes.IsValidIndex(Edge.NodeIndexA) && DungeonNodes.IsValidIndex(Edge.NodeIndexB))
		{
			// Set bit for DirectionAtoB on Node A
			if (Edge.DirectionAtoB != EDungeonDirection::None)
			{
				const uint8 DirBitA = (1 << (uint8)Edge.DirectionAtoB);
				DungeonNodes[Edge.NodeIndexA].ConnectedPassageMask |= DirBitA;

				// Opposite direction on Node B
				uint8 OppDir = 0;
				switch (Edge.DirectionAtoB)
				{
				case EDungeonDirection::North: OppDir = (uint8)EDungeonDirection::South; break;
				case EDungeonDirection::East:  OppDir = (uint8)EDungeonDirection::West; break;
				case EDungeonDirection::South: OppDir = (uint8)EDungeonDirection::North; break;
				case EDungeonDirection::West:  OppDir = (uint8)EDungeonDirection::East; break;
				default: break;
				}
				const uint8 DirBitB = (1 << OppDir);
				DungeonNodes[Edge.NodeIndexB].ConnectedPassageMask |= DirBitB;
			}
		}
	}
}

void ASovereignDungeonGenerator::CleanupDungeon()
{
	// 1. Clear Instanced Static Mesh Components
	if (FloorInstancedComponent)
	{
		FloorInstancedComponent->ClearInstances();
	}
	if (WallInstancedComponent)
	{
		WallInstancedComponent->ClearInstances();
	}

	// 2. Destroy strictly generator-spawned discrete actors (boss altar, props)
	for (TObjectPtr<AActor>& SpawnedActor : GeneratorSpawnedActors)
	{
		if (IsValid(SpawnedActor.Get()))
		{
			SpawnedActor->Destroy();
		}
	}
	GeneratorSpawnedActors.Empty();

	// 3. Destroy strictly generator-spawned battle arenas
	for (TObjectPtr<ASovereignBattleManager>& Arena : BiomeBattleArenas)
	{
		if (IsValid(Arena.Get()))
		{
			Arena->Destroy();
		}
	}
	BiomeBattleArenas.Empty();

	// 4. Destroy fallback lighting rig if created by this generator
	if (IsValid(GeneratorSpawnedLight.Get()))
	{
		GeneratorSpawnedLight->Destroy();
		GeneratorSpawnedLight = nullptr;
	}
	if (IsValid(GeneratorSpawnedPostProcess.Get()))
	{
		GeneratorSpawnedPostProcess->Destroy();
		GeneratorSpawnedPostProcess = nullptr;
	}

	// 5. Reset graph data
	DungeonNodes.Empty();
	DungeonEdges.Empty();
	MainSpineNodeIndices.Empty();
	BossNodeIndex = INDEX_NONE;

	UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] CleanupDungeon complete: All generator-owned actors and instances cleared."));
}

bool ASovereignDungeonGenerator::GenerateDungeon()
{
	CleanupDungeon();

	FRandomStream Stream(GenerationSeed != 0 ? GenerationSeed : FMath::Rand());
	const FVector Origin = GetActorLocation();

	// 1. Create Entrance Node at (0, 0)
	FSovereignDungeonNode EntranceNode;
	EntranceNode.NodeIndex = 0;
	EntranceNode.GridCoord = FIntPoint(0, 0);
	EntranceNode.TileType = EDungeonTileType::EntranceRoom;
	EntranceNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin);
	EntranceNode.bHasFixedEliteMob = false;
	DungeonNodes.Add(EntranceNode);
	MainSpineNodeIndices.Add(0);

	TSet<FIntPoint> OccupiedCoords;
	OccupiedCoords.Add(EntranceNode.GridCoord);

	FIntPoint CurrentCoord = EntranceNode.GridCoord;
	int32 CurrentIndex = 0;

	// 2. Generate Critical Main Spine
	const int32 SpineTargetCount = FMath::Clamp(FMath::FloorToInt(TargetRoomCount * 0.65f), MinSpineLength, TargetRoomCount - 2);

	for (int32 i = 1; i < SpineTargetCount; ++i)
	{
		TArray<TPair<EDungeonDirection, FIntPoint>> CardinalOptions = {
			{ EDungeonDirection::North, FIntPoint(1, 0) },
			{ EDungeonDirection::East,  FIntPoint(0, 1) },
			{ EDungeonDirection::South, FIntPoint(-1, 0) },
			{ EDungeonDirection::West,  FIntPoint(0, -1) }
		};

		// Filter for open directions
		TArray<TPair<EDungeonDirection, FIntPoint>> OpenOptions;
		for (const auto& Opt : CardinalOptions)
		{
			if (!OccupiedCoords.Contains(CurrentCoord + Opt.Value))
			{
				OpenOptions.Add(Opt);
			}
		}

		if (OpenOptions.Num() == 0)
		{
			break;
		}

		const auto Chosen = OpenOptions[Stream.RandRange(0, OpenOptions.Num() - 1)];
		CurrentCoord += Chosen.Value;
		OccupiedCoords.Add(CurrentCoord);

		const int32 NextIndex = DungeonNodes.Num();
		FSovereignDungeonNode Node;
		Node.NodeIndex = NextIndex;
		Node.GridCoord = CurrentCoord;
		Node.TileType = (i == SpineTargetCount - 1) ? EDungeonTileType::EliteChamber : EDungeonTileType::Corridor;
		Node.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(CurrentCoord.X * TILE_SIZE, CurrentCoord.Y * TILE_SIZE, 0.0f));
		Node.bHasFixedEliteMob = (Node.TileType == EDungeonTileType::EliteChamber);
		DungeonNodes.Add(Node);
		MainSpineNodeIndices.Add(NextIndex);

		// Record Edge
		FSovereignDungeonEdge Edge;
		Edge.EdgeId = DungeonEdges.Num();
		Edge.NodeIndexA = CurrentIndex;
		Edge.NodeIndexB = NextIndex;
		Edge.DirectionAtoB = Chosen.Key;
		Edge.PassageType = EPassageType::OpenArchway;
		Edge.DoorState = EDoorState::Open;
		DungeonEdges.Add(Edge);

		CurrentIndex = NextIndex;
	}

	// 3. Place Terminal Boss Chamber immediately attached to Elite Chamber
	TArray<TPair<EDungeonDirection, FIntPoint>> CardinalOptions = {
		{ EDungeonDirection::North, FIntPoint(1, 0) },
		{ EDungeonDirection::East,  FIntPoint(0, 1) },
		{ EDungeonDirection::South, FIntPoint(-1, 0) },
		{ EDungeonDirection::West,  FIntPoint(0, -1) }
	};
	TArray<TPair<EDungeonDirection, FIntPoint>> BossOpenOptions;
	for (const auto& Opt : CardinalOptions)
	{
		if (!OccupiedCoords.Contains(CurrentCoord + Opt.Value))
		{
			BossOpenOptions.Add(Opt);
		}
	}

	if (BossOpenOptions.Num() > 0)
	{
		const auto BossChosen = BossOpenOptions[Stream.RandRange(0, BossOpenOptions.Num() - 1)];
		CurrentCoord += BossChosen.Value;
		OccupiedCoords.Add(CurrentCoord);

		const int32 BossIndex = DungeonNodes.Num();
		FSovereignDungeonNode BossNode;
		BossNode.NodeIndex = BossIndex;
		BossNode.GridCoord = CurrentCoord;
		BossNode.TileType = EDungeonTileType::BossChamber;
		BossNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(CurrentCoord.X * TILE_SIZE, CurrentCoord.Y * TILE_SIZE, 0.0f));
		BossNode.bHasFixedEliteMob = true; // Boss entity
		DungeonNodes.Add(BossNode);
		MainSpineNodeIndices.Add(BossIndex);
		BossNodeIndex = BossIndex;

		FSovereignDungeonEdge BossEdge;
		BossEdge.EdgeId = DungeonEdges.Num();
		BossEdge.NodeIndexA = CurrentIndex;
		BossEdge.NodeIndexB = BossIndex;
		BossEdge.DirectionAtoB = BossChosen.Key;
		BossEdge.PassageType = EPassageType::StandardDoorway;
		BossEdge.DoorState = EDoorState::SealedEliteGate;
		DungeonEdges.Add(BossEdge);
	}

	// 4. Build Side Branches for Remaining Room Budget
	int32 RoomsRemaining = TargetRoomCount - DungeonNodes.Num();
	TArray<int32> EligibleSpineParents = MainSpineNodeIndices;
	// Remove Entrance and Boss from branch parents
	EligibleSpineParents.Remove(0);
	if (BossNodeIndex != INDEX_NONE)
	{
		EligibleSpineParents.Remove(BossNodeIndex);
	}

	while (RoomsRemaining > 0 && EligibleSpineParents.Num() > 0)
	{
		const int32 ParentIdx = EligibleSpineParents[Stream.RandRange(0, EligibleSpineParents.Num() - 1)];
		const FIntPoint ParentCoord = DungeonNodes[ParentIdx].GridCoord;

		TArray<TPair<EDungeonDirection, FIntPoint>> BranchOpen;
		for (const auto& Opt : CardinalOptions)
		{
			if (!OccupiedCoords.Contains(ParentCoord + Opt.Value))
			{
				BranchOpen.Add(Opt);
			}
		}

		if (BranchOpen.Num() == 0)
		{
			EligibleSpineParents.Remove(ParentIdx);
			continue;
		}

		const auto ChosenBranch = BranchOpen[Stream.RandRange(0, BranchOpen.Num() - 1)];
		const FIntPoint BranchCoord = ParentCoord + ChosenBranch.Value;
		OccupiedCoords.Add(BranchCoord);

		const int32 BranchIdx = DungeonNodes.Num();
		FSovereignDungeonNode BranchNode;
		BranchNode.NodeIndex = BranchIdx;
		BranchNode.GridCoord = BranchCoord;
		BranchNode.TileType = (RoomsRemaining == 1) ? EDungeonTileType::TreasureRoom : EDungeonTileType::Chamber;
		BranchNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(BranchCoord.X * TILE_SIZE, BranchCoord.Y * TILE_SIZE, 0.0f));
		BranchNode.bHasFixedEliteMob = false;
		DungeonNodes.Add(BranchNode);

		FSovereignDungeonEdge BranchEdge;
		BranchEdge.EdgeId = DungeonEdges.Num();
		BranchEdge.NodeIndexA = ParentIdx;
		BranchEdge.NodeIndexB = BranchIdx;
		BranchEdge.DirectionAtoB = ChosenBranch.Key;
		BranchEdge.PassageType = EPassageType::OpenArchway;
		BranchEdge.DoorState = EDoorState::Open;
		DungeonEdges.Add(BranchEdge);

		RoomsRemaining--;
	}

	// 5. Derive Node Adjacency Caches
	RebuildNodeAdjacencyCaches();

	// 6. Spawn/Configure Far-Off Battle Arenas
	SpawnFarOffBattleArenas();

	// 7. Spawn 3D Physical Geometry (Ticket B-089)
	if (bSpawnPhysicalGeometry)
	{
		SpawnDungeonGeometry();
	}

	// 8. Spawn Default Lighting Rig if world is blank/unlit
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

		// Check 4 cardinal directions matching EDungeonDirection order:
		// North = 0 (+X), East = 1 (+Y), South = 2 (-X), West = 3 (-Y)
		const TArray<TPair<FIntPoint, FVector>> CardinalOffsets = {
			TPair<FIntPoint, FVector>(FIntPoint(1, 0), FVector(TILE_SIZE * 0.5f, 0.0f, 300.0f)),   // Index 0: North (+X)
			TPair<FIntPoint, FVector>(FIntPoint(0, 1), FVector(0.0f, TILE_SIZE * 0.5f, 300.0f)),   // Index 1: East (+Y)
			TPair<FIntPoint, FVector>(FIntPoint(-1, 0), FVector(-TILE_SIZE * 0.5f, 0.0f, 300.0f)),  // Index 2: South (-X)
			TPair<FIntPoint, FVector>(FIntPoint(0, -1), FVector(0.0f, -TILE_SIZE * 0.5f, 300.0f))   // Index 3: West (-Y)
		};

		const TArray<FRotator> CardinalRotations = {
			FRotator(0.0f, 0.0f, 0.0f),   // North
			FRotator(0.0f, 90.0f, 0.0f),   // East
			FRotator(0.0f, 180.0f, 0.0f),  // South
			FRotator(0.0f, -90.0f, 0.0f)   // West
		};

		for (int32 DirIdx = 0; DirIdx < CardinalOffsets.Num(); ++DirIdx)
		{
			// Check if bit for this cardinal direction is set in ConnectedPassageMask
			const uint8 DirBit = (1 << DirIdx);
			const bool bHasEdgePassage = (Node.ConnectedPassageMask & DirBit) != 0;

			if (!bHasEdgePassage)
			{
				// No passage edge exists across this boundary face.
				// Place a solid boundary wall. This correctly encloses exterior perimeters
				// AND shared boundary walls between physically adjacent nodes that are disconnected!
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
				if (AltarActor)
				{
					if (AltarActor->GetStaticMeshComponent())
					{
						AltarActor->GetStaticMeshComponent()->SetStaticMesh(BossAltarMesh);
					}
					GeneratorSpawnedActors.Add(AltarActor);
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
			GeneratorSpawnedLight = DirLight;
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
			GeneratorSpawnedPostProcess = PPVolume;
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
