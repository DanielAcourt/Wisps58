// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Combat/SovereignDungeonGenerator.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Combat/SovereignEncounterSubsystem.h"
#include "Combat/SovereignDungeonSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "LevelEditorViewport.h"
#endif

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

	DoorwayInstancedComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DoorwayInstancedComponent"));
	DoorwayInstancedComponent->SetupAttachment(FloorInstancedComponent);

	// Default fallback mesh finder for basic cube/floor shapes if custom modular assets aren't yet loaded
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeObj(TEXT("/Engine/BasicShapes/Cube"));
	if (DefaultCubeObj.Succeeded())
	{
		FloorMesh = DefaultCubeObj.Object;
		WallMesh = DefaultCubeObj.Object;
		BossGateMesh = DefaultCubeObj.Object;
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

bool ASovereignDungeonGenerator::CanFitRoomFootprint(const FIntPoint& Anchor, const FIntPoint& Dims) const
{
	const int32 W = FMath::Max(1, Dims.X);
	const int32 H = FMath::Max(1, Dims.Y);

	for (int32 X = 0; X < W; ++X)
	{
		for (int32 Y = 0; Y < H; ++Y)
		{
			const FIntPoint TestCoord = Anchor + FIntPoint(X, Y);
			if (MasterGridOccupancy.Contains(TestCoord))
			{
				return false;
			}
		}
	}
	return true;
}

int32 ASovereignDungeonGenerator::CalculateGraphDistance(int32 NodeA, int32 NodeB) const
{
	if (NodeA == NodeB)
	{
		return 0;
	}
	if (!DungeonNodes.IsValidIndex(NodeA) || !DungeonNodes.IsValidIndex(NodeB))
	{
		return INDEX_NONE;
	}

	TSet<int32> Visited;
	TQueue<int32> Queue;
	TMap<int32, int32> Distances;

	Visited.Add(NodeA);
	Queue.Enqueue(NodeA);
	Distances.Add(NodeA, 0);

	while (!Queue.IsEmpty())
	{
		int32 Current = INDEX_NONE;
		Queue.Dequeue(Current);

		const int32 CurrentDist = Distances[Current];
		if (Current == NodeB)
		{
			return CurrentDist;
		}

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

	return INDEX_NONE; // Not reachable
}

int32 ASovereignDungeonGenerator::GenerateControlledLoops(FRandomStream& Stream)
{
	if (MaxLoopCount <= 0 || DungeonNodes.Num() < 4)
	{
		return 0;
	}

	int32 LoopsAdded = 0;

	// Collect candidate pairs of nodes
	// Invariants:
	// 1. Neither node can be BossChamber or EntranceRoom (strict protection)
	// 2. Neither node can be TreasureRoom (dead-end leaf protection)
	// 3. Both nodes must have CanAcceptConnection() == true
	// 4. TreeDistance(u, v) >= 3 (meaningful shortcut)
	// 5. Must have matching adjacent unconnected sockets facing each other

	struct FLoopCandidate
	{
		int32 NodeU;
		int32 NodeV;
		int32 SocketIndexU;
		int32 SocketIndexV;
		EDungeonDirection DirUtoV;
		int32 GraphDist;
	};

	TArray<FLoopCandidate> Candidates;

	for (int32 u = 0; u < DungeonNodes.Num(); ++u)
	{
		const FSovereignDungeonNode& NodeU = DungeonNodes[u];
		if (NodeU.TileType == EDungeonTileType::EntranceRoom ||
			NodeU.TileType == EDungeonTileType::BossChamber ||
			NodeU.TileType == EDungeonTileType::TreasureRoom ||
			!NodeU.CanAcceptConnection())
		{
			continue;
		}

		for (int32 v = u + 1; v < DungeonNodes.Num(); ++v)
		{
			const FSovereignDungeonNode& NodeV = DungeonNodes[v];
			if (NodeV.TileType == EDungeonTileType::EntranceRoom ||
				NodeV.TileType == EDungeonTileType::BossChamber ||
				NodeV.TileType == EDungeonTileType::TreasureRoom ||
				!NodeV.CanAcceptConnection())
			{
				continue;
			}

			// Check if already directly connected by an existing edge
			bool bAlreadyConnected = false;
			for (const FSovereignDungeonEdge& Edge : DungeonEdges)
			{
				if (Edge.Connects(u) && Edge.Connects(v))
				{
					bAlreadyConnected = true;
					break;
				}
			}
			if (bAlreadyConnected)
			{
				continue;
			}

			// Check topological distance >= 3
			const int32 Dist = CalculateGraphDistance(u, v);
			if (Dist < 3)
			{
				continue;
			}

			// Check if there are matching adjacent unconnected sockets
			for (int32 sU = 0; sU < NodeU.PerimeterSockets.Num(); ++sU)
			{
				const FSovereignDoorwaySocket& SockU = NodeU.PerimeterSockets[sU];
				if (SockU.bIsConnected)
				{
					continue;
				}

				FIntPoint OffsetU(0, 0);
				EDungeonDirection RequiredFacingV = EDungeonDirection::None;
				switch (SockU.FacingDirection)
				{
				case EDungeonDirection::North: OffsetU = FIntPoint(1, 0);  RequiredFacingV = EDungeonDirection::South; break;
				case EDungeonDirection::East:  OffsetU = FIntPoint(0, 1);  RequiredFacingV = EDungeonDirection::West; break;
				case EDungeonDirection::South: OffsetU = FIntPoint(-1, 0); RequiredFacingV = EDungeonDirection::North; break;
				case EDungeonDirection::West:  OffsetU = FIntPoint(0, -1); RequiredFacingV = EDungeonDirection::East; break;
				default: break;
				}

				const FIntPoint TargetCoordV = SockU.CellCoord + OffsetU;

				for (int32 sV = 0; sV < NodeV.PerimeterSockets.Num(); ++sV)
				{
					const FSovereignDoorwaySocket& SockV = NodeV.PerimeterSockets[sV];
					if (!SockV.bIsConnected && SockV.CellCoord == TargetCoordV && SockV.FacingDirection == RequiredFacingV)
					{
						FLoopCandidate Cand;
						Cand.NodeU = u;
						Cand.NodeV = v;
						Cand.SocketIndexU = sU;
						Cand.SocketIndexV = sV;
						Cand.DirUtoV = SockU.FacingDirection;
						Cand.GraphDist = Dist;
						Candidates.Add(Cand);
						break;
					}
				}
			}
		}
	}

	// Shuffle candidates for procedural variety
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		const int32 SwapIdx = Stream.RandRange(0, Candidates.Num() - 1);
		Candidates.Swap(i, SwapIdx);
	}

	// Add up to MaxLoopCount shortcut loops
	for (const FLoopCandidate& Cand : Candidates)
	{
		if (LoopsAdded >= MaxLoopCount)
		{
			break;
		}

		// Re-verify both nodes can still accept a connection
		if (!DungeonNodes[Cand.NodeU].CanAcceptConnection() || !DungeonNodes[Cand.NodeV].CanAcceptConnection())
		{
			continue;
		}

		// Create the shortcut loop edge
		FSovereignDungeonEdge LoopEdge;
		LoopEdge.EdgeId = DungeonEdges.Num();
		LoopEdge.NodeIndexA = Cand.NodeU;
		LoopEdge.NodeIndexB = Cand.NodeV;
		LoopEdge.DirectionAtoB = Cand.DirUtoV;
		LoopEdge.PassageType = EPassageType::StandardDoorway;
		LoopEdge.DoorState = EDoorState::Open;

		DungeonEdges.Add(LoopEdge);
		LoopsAdded++;

		UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Created Shortcut Loop Edge %d between Node %d and Node %d (Topological Dist: %d)."),
			LoopEdge.EdgeId, Cand.NodeU, Cand.NodeV, Cand.GraphDist);
	}

	return LoopsAdded;
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

		// Ensure perimeter sockets are initialized
		if (DungeonNodes[i].PerimeterSockets.Num() == 0)
		{
			DungeonNodes[i].GeneratePerimeterSockets();
		}

		// Reset connection states on sockets prior to sync
		for (FSovereignDoorwaySocket& Socket : DungeonNodes[i].PerimeterSockets)
		{
			Socket.bIsConnected = false;
			Socket.ConnectedEdgeId = INDEX_NONE;
			Socket.ConnectedNodeIndex = INDEX_NONE;
		}
	}

	for (const FSovereignDungeonEdge& Edge : DungeonEdges)
	{
		if (DungeonNodes.IsValidIndex(Edge.NodeIndexA) && DungeonNodes.IsValidIndex(Edge.NodeIndexB))
		{
			const EDungeonDirection DirA = Edge.DirectionAtoB;
			if (DirA != EDungeonDirection::None)
			{
				// 1. Bitmask update on Node A
				const uint8 DirBitA = (1 << (uint8)DirA);
				DungeonNodes[Edge.NodeIndexA].ConnectedPassageMask |= DirBitA;

				// 2. Bitmask update on Node B (Opposite direction)
				uint8 OppDir = 0;
				switch (DirA)
				{
				case EDungeonDirection::North: OppDir = (uint8)EDungeonDirection::South; break;
				case EDungeonDirection::East:  OppDir = (uint8)EDungeonDirection::West; break;
				case EDungeonDirection::South: OppDir = (uint8)EDungeonDirection::North; break;
				case EDungeonDirection::West:  OppDir = (uint8)EDungeonDirection::East; break;
				default: break;
				}
				const uint8 DirBitB = (1 << OppDir);
				DungeonNodes[Edge.NodeIndexB].ConnectedPassageMask |= DirBitB;

				// Cardinal offset matching DirA
				FIntPoint OffsetAtoB(0, 0);
				switch (DirA)
				{
				case EDungeonDirection::North: OffsetAtoB = FIntPoint(1, 0); break;
				case EDungeonDirection::East:  OffsetAtoB = FIntPoint(0, 1); break;
				case EDungeonDirection::South: OffsetAtoB = FIntPoint(-1, 0); break;
				case EDungeonDirection::West:  OffsetAtoB = FIntPoint(0, -1); break;
				default: break;
				}

				const TArray<FIntPoint> OccupiedCellsA = DungeonNodes[Edge.NodeIndexA].GetOccupiedCells();
				const TArray<FIntPoint> OccupiedCellsB = DungeonNodes[Edge.NodeIndexB].GetOccupiedCells();

				// 3. Update Socket state on Node A (Ticket B-095 coordinate-exact binding)
				for (FSovereignDoorwaySocket& SockA : DungeonNodes[Edge.NodeIndexA].PerimeterSockets)
				{
					if (!SockA.bIsConnected && SockA.FacingDirection == DirA)
					{
						const FIntPoint TargetNeighborCoord = SockA.CellCoord + OffsetAtoB;
						if (OccupiedCellsB.Contains(TargetNeighborCoord))
						{
							SockA.bIsConnected = true;
							SockA.ConnectedEdgeId = Edge.EdgeId;
							SockA.ConnectedNodeIndex = Edge.NodeIndexB;
							break;
						}
					}
				}

				// 4. Update Socket state on Node B (Ticket B-095 coordinate-exact binding)
				const FIntPoint OffsetBtoA(-OffsetAtoB.X, -OffsetAtoB.Y);
				for (FSovereignDoorwaySocket& SockB : DungeonNodes[Edge.NodeIndexB].PerimeterSockets)
				{
					if (!SockB.bIsConnected && SockB.FacingDirection == (EDungeonDirection)OppDir)
					{
						const FIntPoint TargetNeighborCoord = SockB.CellCoord + OffsetBtoA;
						if (OccupiedCellsA.Contains(TargetNeighborCoord))
						{
							SockB.bIsConnected = true;
							SockB.ConnectedEdgeId = Edge.EdgeId;
							SockB.ConnectedNodeIndex = Edge.NodeIndexA;
							break;
						}
					}
				}
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
	if (DoorwayInstancedComponent)
	{
		DoorwayInstancedComponent->ClearInstances();
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
	MasterGridOccupancy.Empty();
	MainSpineNodeIndices.Empty();
	BossNodeIndex = INDEX_NONE;

	UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] CleanupDungeon complete: All generator-owned actors and instances cleared."));
}

bool ASovereignDungeonGenerator::GenerateDungeon()
{
	const int32 MaxRetries = FMath::Max(1, MaxGenerationRetries);
	const FVector Origin = GetActorLocation();

	const TArray<TPair<EDungeonDirection, FIntPoint>> CardinalOptions = {
		{ EDungeonDirection::North, FIntPoint(1, 0) },
		{ EDungeonDirection::East,  FIntPoint(0, 1) },
		{ EDungeonDirection::South, FIntPoint(-1, 0) },
		{ EDungeonDirection::West,  FIntPoint(0, -1) }
	};

	for (int32 Attempt = 0; Attempt < MaxRetries; ++Attempt)
	{
		CleanupDungeon();

		const int32 SeedToUse = (GenerationSeed != 0) ? (GenerationSeed + Attempt) : FMath::Rand();
		FRandomStream Stream(SeedToUse);

		// 1. Create Entrance Node at (0, 0) (Footprint 1x1)
		FSovereignDungeonNode EntranceNode;
		EntranceNode.NodeIndex = 0;
		EntranceNode.GridCoord = FIntPoint(0, 0);
		EntranceNode.FootprintDimensions = GetDefaultFootprintForTileType(EDungeonTileType::EntranceRoom);
		EntranceNode.MaxConnections = GetDefaultMaxConnectionsForTileType(EDungeonTileType::EntranceRoom);
		EntranceNode.TileType = EDungeonTileType::EntranceRoom;
		EntranceNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin);
		EntranceNode.bHasFixedEliteMob = false;
		EntranceNode.GeneratePerimeterSockets();

		DungeonNodes.Add(EntranceNode);
		MainSpineNodeIndices.Add(0);

		for (const FIntPoint& Cell : EntranceNode.GetOccupiedCells())
		{
			MasterGridOccupancy.Add(Cell, 0);
		}

		FIntPoint CurrentCoord = EntranceNode.GridCoord;
		int32 CurrentIndex = 0;

		// 2. Generate Critical Main Spine (Option A: Bounded to 8-25 rooms to prevent self-trapping on mega-dungeons)
		const int32 MaxSpineBound = FMath::Clamp(TargetRoomCount - 2, MinSpineLength, 25);
		const int32 DesiredSpineCount = FMath::FloorToInt(TargetRoomCount * 0.65f);
		const int32 SpineTargetCount = FMath::Clamp(DesiredSpineCount, MinSpineLength, MaxSpineBound);

		for (int32 i = 1; i < SpineTargetCount; ++i)
		{
			const EDungeonTileType NextType = (i == SpineTargetCount - 1) ? EDungeonTileType::EliteChamber : EDungeonTileType::Corridor;
			const FIntPoint NextDims = GetDefaultFootprintForTileType(NextType);

			// Filter for open directions capable of accommodating the room's footprint
			TArray<TPair<EDungeonDirection, FIntPoint>> OpenOptions;
			for (const auto& Opt : CardinalOptions)
			{
				const FIntPoint CandidateAnchor = CurrentCoord + Opt.Value;
				if (CanFitRoomFootprint(CandidateAnchor, NextDims))
				{
					OpenOptions.Add(Opt);
				}
			}

			// Fallback to 1x1 footprint if multi-cell space is constrained
			FIntPoint DimsToUse = NextDims;
			if (OpenOptions.Num() == 0 && (NextDims.X > 1 || NextDims.Y > 1))
			{
				DimsToUse = FIntPoint(1, 1);
				for (const auto& Opt : CardinalOptions)
				{
					const FIntPoint CandidateAnchor = CurrentCoord + Opt.Value;
					if (CanFitRoomFootprint(CandidateAnchor, DimsToUse))
					{
						OpenOptions.Add(Opt);
					}
				}
			}

			if (OpenOptions.Num() == 0)
			{
				break;
			}

			const auto Chosen = OpenOptions[Stream.RandRange(0, OpenOptions.Num() - 1)];
			CurrentCoord += Chosen.Value;

			const int32 NextIndex = DungeonNodes.Num();
			FSovereignDungeonNode Node;
			Node.NodeIndex = NextIndex;
			Node.GridCoord = CurrentCoord;
			Node.FootprintDimensions = DimsToUse;
			Node.MaxConnections = GetDefaultMaxConnectionsForTileType(NextType);
			Node.TileType = NextType;
			Node.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(CurrentCoord.X * TILE_SIZE, CurrentCoord.Y * TILE_SIZE, 0.0f));
			Node.bHasFixedEliteMob = (Node.TileType == EDungeonTileType::EliteChamber);
			Node.GeneratePerimeterSockets();

			DungeonNodes.Add(Node);
			MainSpineNodeIndices.Add(NextIndex);

			for (const FIntPoint& Cell : Node.GetOccupiedCells())
			{
				MasterGridOccupancy.Add(Cell, NextIndex);
			}

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

		// 3. Guaranteed Boss Placement (Ticket B-099)
		// Try placing adjacent to CurrentIndex first; if trapped, search backwards along deepest spine nodes
		const FIntPoint BossTargetDims = GetDefaultFootprintForTileType(EDungeonTileType::BossChamber);

		int32 ChosenEliteParentIdx = INDEX_NONE;
		FIntPoint ChosenBossCoord = FIntPoint::ZeroValue;
		FIntPoint ChosenBossDims = BossTargetDims;
		EDungeonDirection ChosenBossDir = EDungeonDirection::None;

		// Candidate parent nodes ordered from deepest spine tip backward to Entrance
		TArray<int32> CandidateParents;
		for (int32 SpineIdx = MainSpineNodeIndices.Num() - 1; SpineIdx >= 1; --SpineIdx)
		{
			CandidateParents.Add(MainSpineNodeIndices[SpineIdx]);
		}

		for (int32 ParentCandidate : CandidateParents)
		{
			const FIntPoint ParentAnchor = DungeonNodes[ParentCandidate].GridCoord;

			// Try 4x4 first
			for (const auto& Opt : CardinalOptions)
			{
				const FIntPoint CandidateBossAnchor = ParentAnchor + Opt.Value;
				if (CanFitRoomFootprint(CandidateBossAnchor, BossTargetDims))
				{
					ChosenEliteParentIdx = ParentCandidate;
					ChosenBossCoord = CandidateBossAnchor;
					ChosenBossDims = BossTargetDims;
					ChosenBossDir = Opt.Key;
					break;
				}
			}
			if (ChosenEliteParentIdx != INDEX_NONE)
			{
				break;
			}

			// Try 1x1 fallback if 4x4 couldn't fit
			for (const auto& Opt : CardinalOptions)
			{
				const FIntPoint CandidateBossAnchor = ParentAnchor + Opt.Value;
				if (CanFitRoomFootprint(CandidateBossAnchor, FIntPoint(1, 1)))
				{
					ChosenEliteParentIdx = ParentCandidate;
					ChosenBossCoord = CandidateBossAnchor;
					ChosenBossDims = FIntPoint(1, 1);
					ChosenBossDir = Opt.Key;
					break;
				}
			}
			if (ChosenEliteParentIdx != INDEX_NONE)
			{
				break;
			}
		}

		if (ChosenEliteParentIdx != INDEX_NONE)
		{
			// Ensure parent node is converted to EliteChamber
			DungeonNodes[ChosenEliteParentIdx].TileType = EDungeonTileType::EliteChamber;
			DungeonNodes[ChosenEliteParentIdx].bHasFixedEliteMob = true;

			const int32 BossIndex = DungeonNodes.Num();
			FSovereignDungeonNode BossNode;
			BossNode.NodeIndex = BossIndex;
			BossNode.GridCoord = ChosenBossCoord;
			BossNode.FootprintDimensions = ChosenBossDims;
			BossNode.MaxConnections = GetDefaultMaxConnectionsForTileType(EDungeonTileType::BossChamber); // Strict single entrance
			BossNode.TileType = EDungeonTileType::BossChamber;
			BossNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(ChosenBossCoord.X * TILE_SIZE, ChosenBossCoord.Y * TILE_SIZE, 0.0f));
			BossNode.bHasFixedEliteMob = true; // Boss entity
			BossNode.GeneratePerimeterSockets();

			DungeonNodes.Add(BossNode);
			BossNodeIndex = BossIndex;

			for (const FIntPoint& Cell : BossNode.GetOccupiedCells())
			{
				MasterGridOccupancy.Add(Cell, BossIndex);
			}

			FSovereignDungeonEdge BossEdge;
			BossEdge.EdgeId = DungeonEdges.Num();
			BossEdge.NodeIndexA = ChosenEliteParentIdx;
			BossEdge.NodeIndexB = BossIndex;
			BossEdge.DirectionAtoB = ChosenBossDir;
			BossEdge.PassageType = EPassageType::StandardDoorway;
			BossEdge.DoorState = EDoorState::SealedEliteGate;
			DungeonEdges.Add(BossEdge);
		}
		else
		{
			// If even fallback placement failed, retry with next seed
			continue;
		}

		// 4. Build Deep Branching Network for Remaining Room Budget (Ticket B-099 Option A)
		int32 RoomsRemaining = TargetRoomCount - DungeonNodes.Num();
		TArray<int32> EligibleBranchParents;
		for (int32 i = 1; i < DungeonNodes.Num(); ++i)
		{
			if (i != BossNodeIndex && DungeonNodes[i].TileType != EDungeonTileType::BossChamber)
			{
				EligibleBranchParents.Add(i);
			}
		}

		while (RoomsRemaining > 0 && EligibleBranchParents.Num() > 0)
		{
			const int32 RandPick = Stream.RandRange(0, EligibleBranchParents.Num() - 1);
			const int32 ParentIdx = EligibleBranchParents[RandPick];
			const FIntPoint ParentCoord = DungeonNodes[ParentIdx].GridCoord;

			if (!DungeonNodes[ParentIdx].CanAcceptConnection())
			{
				EligibleBranchParents.RemoveAt(RandPick);
				continue;
			}

			const EDungeonTileType BranchType = (RoomsRemaining == 1) ? EDungeonTileType::TreasureRoom : EDungeonTileType::Chamber;
			const FIntPoint BranchDims = GetDefaultFootprintForTileType(BranchType);

			TArray<TPair<EDungeonDirection, FIntPoint>> BranchOpen;
			for (const auto& Opt : CardinalOptions)
			{
				const FIntPoint CandidateBranchAnchor = ParentCoord + Opt.Value;
				if (CanFitRoomFootprint(CandidateBranchAnchor, BranchDims))
				{
					BranchOpen.Add(Opt);
				}
			}

			if (BranchOpen.Num() == 0)
			{
				EligibleBranchParents.RemoveAt(RandPick);
				continue;
			}

			const auto ChosenBranch = BranchOpen[Stream.RandRange(0, BranchOpen.Num() - 1)];
			const FIntPoint BranchCoord = ParentCoord + ChosenBranch.Value;

			const int32 BranchIdx = DungeonNodes.Num();
			FSovereignDungeonNode BranchNode;
			BranchNode.NodeIndex = BranchIdx;
			BranchNode.GridCoord = BranchCoord;
			BranchNode.FootprintDimensions = BranchDims;
			BranchNode.MaxConnections = GetDefaultMaxConnectionsForTileType(BranchType);
			BranchNode.TileType = BranchType;
			BranchNode.WorldTransform = FTransform(FRotator::ZeroRotator, Origin + FVector(BranchCoord.X * TILE_SIZE, BranchCoord.Y * TILE_SIZE, 0.0f));
			BranchNode.bHasFixedEliteMob = false;
			BranchNode.GeneratePerimeterSockets();

			DungeonNodes.Add(BranchNode);

			for (const FIntPoint& Cell : BranchNode.GetOccupiedCells())
			{
				MasterGridOccupancy.Add(Cell, BranchIdx);
			}

			FSovereignDungeonEdge BranchEdge;
			BranchEdge.EdgeId = DungeonEdges.Num();
			BranchEdge.NodeIndexA = ParentIdx;
			BranchEdge.NodeIndexB = BranchIdx;
			BranchEdge.DirectionAtoB = ChosenBranch.Key;
			BranchEdge.PassageType = EPassageType::OpenArchway;
			BranchEdge.DoorState = EDoorState::Open;
			DungeonEdges.Add(BranchEdge);

			// Add new branch room to eligible parents if it's a chamber
			if (BranchType == EDungeonTileType::Chamber)
			{
				EligibleBranchParents.Add(BranchIdx);
			}

			RoomsRemaining--;
		}

		// 4.5. Generate Controlled Shortcut Loops (Ticket B-096)
		if (MaxLoopCount > 0)
		{
			GenerateControlledLoops(Stream);
		}

		// 5. Derive Node Adjacency Caches
		RebuildNodeAdjacencyCaches();

		// 6. Validate Reachability & Invariants
		if (BossNodeIndex != INDEX_NONE && ValidateTopologicalReachability(0, BossNodeIndex))
		{
			// Successful generation!
			SpawnFarOffBattleArenas();

			if (bSpawnPhysicalGeometry)
			{
				SpawnDungeonGeometry();
			}

			if (bAutoSpawnDefaultLighting)
			{
				SpawnDefaultLightingRig();
			}

			UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Successfully generated dungeon with %d nodes, %d edges on attempt %d."),
				DungeonNodes.Num(), DungeonEdges.Num(), Attempt + 1);

			OnDungeonGenerated.Broadcast(DungeonNodes.Num());
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[DungeonGenerator] Failed to generate valid dungeon after %d attempts!"), MaxRetries);
	return false;
}

void ASovereignDungeonGenerator::SpawnDungeonGeometry()
{
	// 1. Prepare Instanced Static Mesh Components
	if (!FloorInstancedComponent)
	{
		FloorInstancedComponent = NewObject<UInstancedStaticMeshComponent>(this, TEXT("RuntimeFloorInstancedComponent"));
		if (GetWorld())
		{
			FloorInstancedComponent->RegisterComponent();
		}
	}
	if (!WallInstancedComponent)
	{
		WallInstancedComponent = NewObject<UInstancedStaticMeshComponent>(this, TEXT("RuntimeWallInstancedComponent"));
		if (GetWorld())
		{
			WallInstancedComponent->RegisterComponent();
		}
	}
	if (!DoorwayInstancedComponent)
	{
		DoorwayInstancedComponent = NewObject<UInstancedStaticMeshComponent>(this, TEXT("RuntimeDoorwayInstancedComponent"));
		if (GetWorld())
		{
			DoorwayInstancedComponent->RegisterComponent();
		}
	}

	FloorInstancedComponent->ClearInstances();
	WallInstancedComponent->ClearInstances();
	DoorwayInstancedComponent->ClearInstances();

	if (FloorMesh)
	{
		FloorInstancedComponent->SetStaticMesh(FloorMesh);
	}
	if (WallMesh)
	{
		WallInstancedComponent->SetStaticMesh(WallMesh);
	}
	if (DoorwayMesh)
	{
		DoorwayInstancedComponent->SetStaticMesh(DoorwayMesh);
	}

	const FVector Origin = GetActorLocation();

	// 2. Spawn Floor Tiles for Each Room (Multi-cell aware)
	for (int32 NodeIdx = 0; NodeIdx < DungeonNodes.Num(); ++NodeIdx)
	{
		const FSovereignDungeonNode& Node = DungeonNodes[NodeIdx];
		const TArray<FIntPoint> OccupiedCells = Node.GetOccupiedCells();

		for (const FIntPoint& Cell : OccupiedCells)
		{
			const FVector CellPos = Origin + FVector(Cell.X * TILE_SIZE, Cell.Y * TILE_SIZE, 0.0f);
			FTransform FloorTransform;
			FloorTransform.SetLocation(CellPos - FVector(0.0f, 0.0f, 10.0f));
			FloorTransform.SetRotation(FQuat::Identity);
			FloorTransform.SetScale3D(FVector(TILE_SIZE / 100.0f, TILE_SIZE / 100.0f, 0.2f));

			FloorInstancedComponent->AddInstance(FloorTransform);
		}
	}

	// 3. Multi-Cell Perimeter Boundary & Edge-Aware Doorway Snapping (Ticket B-095)
	// Iterate strictly over Node.PerimeterSockets - internal cells have zero sockets, guaranteeing zero interior walls!
	TSet<int32> ProcessedSealedGates;

	for (int32 NodeIdx = 0; NodeIdx < DungeonNodes.Num(); ++NodeIdx)
	{
		const FSovereignDungeonNode& Node = DungeonNodes[NodeIdx];
		const FVector AnchorPos = Node.WorldTransform.GetLocation();

		for (const FSovereignDoorwaySocket& Socket : Node.PerimeterSockets)
		{
			// World coordinate of this specific perimeter cell
			const FVector CellPos = Origin + FVector(Socket.CellCoord.X * TILE_SIZE, Socket.CellCoord.Y * TILE_SIZE, 0.0f);

			// Cardinal offset and rotation based on socket facing direction
			// Standard Modular convention: Wall faces outward along Local +X; Width is along Local Y; Height is along Local Z.
			// Rotating by Yaw orients the wall outward cleanly for all 4 cardinal directions.
			const float WallHeight = 600.0f;
			const float VerticalOffset = bWallMeshCenterPivot ? (WallHeight * 0.5f) : 0.0f;

			FVector DirectionOffset = FVector::ZeroVector;
			FRotator FaceRotation = FRotator::ZeroRotator;

			switch (Socket.FacingDirection)
			{
			case EDungeonDirection::North:
				DirectionOffset = FVector(TILE_SIZE * 0.5f, 0.0f, VerticalOffset);
				FaceRotation = FRotator(0.0f, 0.0f, 0.0f); // Faces +X
				break;
			case EDungeonDirection::East:
				DirectionOffset = FVector(0.0f, TILE_SIZE * 0.5f, VerticalOffset);
				FaceRotation = FRotator(0.0f, 90.0f, 0.0f); // Faces +Y
				break;
			case EDungeonDirection::South:
				DirectionOffset = FVector(-TILE_SIZE * 0.5f, 0.0f, VerticalOffset);
				FaceRotation = FRotator(0.0f, 180.0f, 0.0f); // Faces -X
				break;
			case EDungeonDirection::West:
				DirectionOffset = FVector(0.0f, -TILE_SIZE * 0.5f, VerticalOffset);
				FaceRotation = FRotator(0.0f, 270.0f, 0.0f); // Faces -Y
				break;
			default:
				break;
			}

			FTransform FaceTransform;
			FaceTransform.SetLocation(CellPos + DirectionOffset);
			FaceTransform.SetRotation(FaceRotation.Quaternion());

			// Scale standard modular wall (Thickness = 30cm in local X, Width = 1000cm in local Y, Height = 600cm in local Z)
			// Uniform scale across all directions prevents perpendicular slicing into rooms!
			const FVector StandardWallScale(0.3f, TILE_SIZE / 100.0f, WallHeight / 100.0f);
			FaceTransform.SetScale3D(StandardWallScale);

			if (!Socket.bIsConnected)
			{
				// Unconnected perimeter socket: Spawn solid outer boundary wall
				WallInstancedComponent->AddInstance(FaceTransform);
			}
			else
			{
				// Connected perimeter socket: Check edge semantics
				const int32 EdgeId = Socket.ConnectedEdgeId;
				const FSovereignDungeonEdge* EdgePtr = DungeonEdges.IsValidIndex(EdgeId) ? &DungeonEdges[EdgeId] : nullptr;

				// Spawn Boss Gate if sealed elite gate
				if (EdgePtr && EdgePtr->DoorState == EDoorState::SealedEliteGate)
				{
					if (!ProcessedSealedGates.Contains(EdgeId) && GetWorld())
					{
						ProcessedSealedGates.Add(EdgeId);

						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

						if (AStaticMeshActor* GateActor = GetWorld()->SpawnActor<AStaticMeshActor>(
							AStaticMeshActor::StaticClass(), FaceTransform.GetLocation(), FaceTransform.GetRotation().Rotator(), SpawnParams))
						{
							if (GateActor->GetStaticMeshComponent())
							{
								if (BossGateMesh)
								{
									GateActor->GetStaticMeshComponent()->SetStaticMesh(BossGateMesh);
								}
								GateActor->SetActorScale3D(FVector(0.5f, (TILE_SIZE * 0.5f) / 100.0f, 5.0f));
							}
							GeneratorSpawnedActors.Add(GateActor);
							UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned Sealed Boss Gate Actor at Edge %d."), EdgeId);
						}
					}
				}

				// If DoorwayMesh is assigned, spawn it in DoorwayInstancedComponent
				if (DoorwayMesh)
				{
					DoorwayInstancedComponent->AddInstance(FaceTransform);
				}
				else if (bProceduralArchwayFallback && WallInstancedComponent)
				{
					// Option 2 Archway Fallback: Construct a 400w x 500h opening using left post, right post, and top lintel
					// Left Post (Width 300cm, centered at local Y = -350cm)
					const FVector LeftPostLocal(-350.0f * FaceRotation.RotateVector(FVector::RightVector));
					FTransform LeftTransform = FaceTransform;
					LeftTransform.SetLocation(FaceTransform.GetLocation() + LeftPostLocal);
					LeftTransform.SetScale3D(FVector(0.3f, 3.0f, WallHeight / 100.0f));
					WallInstancedComponent->AddInstance(LeftTransform);

					// Right Post (Width 300cm, centered at local Y = +350cm)
					const FVector RightPostLocal(350.0f * FaceRotation.RotateVector(FVector::RightVector));
					FTransform RightTransform = FaceTransform;
					RightTransform.SetLocation(FaceTransform.GetLocation() + RightPostLocal);
					RightTransform.SetScale3D(FVector(0.3f, 3.0f, WallHeight / 100.0f));
					WallInstancedComponent->AddInstance(RightTransform);

					// Top Lintel (Width 400cm, Height 100cm, centered at Z = 550cm)
					const float LintelZ = bWallMeshCenterPivot ? 250.0f : 500.0f;
					FTransform LintelTransform = FaceTransform;
					LintelTransform.SetLocation(FaceTransform.GetLocation() + FVector(0.0f, 0.0f, LintelZ));
					LintelTransform.SetScale3D(FVector(0.3f, 4.0f, 1.0f));
					WallInstancedComponent->AddInstance(LintelTransform);
				}
			}
		}

		// 4. For Terminal Boss Chamber: Spawn Boss Altar / Egg Prop
		if (Node.TileType == EDungeonTileType::BossChamber)
		{
			if (BossAltarMesh && GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Place altar in the center of the multi-cell Boss Chamber
				const FVector BossCenterPos = AnchorPos + FVector(
					(Node.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
					(Node.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f,
					50.0f
				);

				if (AStaticMeshActor* AltarActor = GetWorld()->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(), BossCenterPos, FRotator::ZeroRotator, SpawnParams))
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

	UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned 3D Dungeon Geometry: %d Floors, %d Walls, %d Doorways."),
		FloorInstancedComponent->GetInstanceCount(), WallInstancedComponent->GetInstanceCount(), DoorwayInstancedComponent->GetInstanceCount());
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
	const TArray<FString> BiomeNames = { TEXT("Earth"), TEXT("Lava"), TEXT("Crystal"), TEXT("Forest") };

	const float Spacing = FMath::Max(2000.0f, ArenaSpacing);
	TArray<FVector> BiomeOffsets = {
		FVector(0.0f, 0.0f, 0.0f),           // Earth
		FVector(Spacing, 0.0f, 0.0f),        // Lava
		FVector(0.0f, Spacing, 0.0f),        // Crystal
		FVector(Spacing, Spacing, 0.0f)      // Forest
	};

	UStaticMesh* PlatformMeshToUse = ArenaPlatformMesh ? ArenaPlatformMesh.Get() : FloorMesh.Get();
	UStaticMesh* WallMeshToUse = ArenaWallMesh ? ArenaWallMesh.Get() : WallMesh.Get();

	for (int32 i = 0; i < BiomeOffsets.Num(); ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FVector ArenaPos = FarOffArenaOrigin + BiomeOffsets[i];

		ASovereignBattleManager* Arena = World->SpawnActor<ASovereignBattleManager>(
			ASovereignBattleManager::StaticClass(), ArenaPos, FRotator::ZeroRotator, SpawnParams
		);

		if (Arena)
		{
#if WITH_EDITOR
			Arena->SetActorLabel(FString::Printf(TEXT("BattleArena_%s"), *BiomeNames[i]));
#endif
			Arena->GenerateDefaultGridTransforms();
			BiomeBattleArenas.Add(Arena);

			// Physically build 3D theater stage (Ticket B-100)
			if (bSpawnPhysicalArenaGeometry)
			{
				// 1. Stage Floor Platform (3x3 grid of 1000cm tiles = 3000cm x 3000cm)
				// Center of stage is ArenaPos; tiles span X from -1 to +1, Y from -1 to +1
				for (int32 X = -1; X <= 1; ++X)
				{
					for (int32 Y = -1; Y <= 1; ++Y)
					{
						const FVector TilePos = ArenaPos + FVector(X * TILE_SIZE, Y * TILE_SIZE, 0.0f);
						if (AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(
							AStaticMeshActor::StaticClass(), TilePos - FVector(0.0f, 0.0f, 10.0f), FRotator::ZeroRotator, SpawnParams))
						{
							if (FloorActor->GetStaticMeshComponent() && PlatformMeshToUse)
							{
								FloorActor->GetStaticMeshComponent()->SetStaticMesh(PlatformMeshToUse);
								FloorActor->SetActorScale3D(FVector(TILE_SIZE / 100.0f, TILE_SIZE / 100.0f, 0.2f));
							}
#if WITH_EDITOR
							FloorActor->SetActorLabel(FString::Printf(TEXT("Arena_%s_Floor_%d_%d"), *BiomeNames[i], X, Y));
#endif
							GeneratorSpawnedActors.Add(FloorActor);
						}
					}
				}

				// 2. 3-Sided Theater Perimeter Walls: Back (+X), Left (+Y), Right (-Y), Open Front (-X)
				const float WallZ = bWallMeshCenterPivot ? 300.0f : 0.0f;

				// Back Wall: along +X face (X = +1.5 * TILE_SIZE = +1500cm), spanning Y = -1, 0, +1
				for (int32 Y = -1; Y <= 1; ++Y)
				{
					const FVector WallPos = ArenaPos + FVector(TILE_SIZE * 1.5f, Y * TILE_SIZE, WallZ);
					if (AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(
						AStaticMeshActor::StaticClass(), WallPos, FRotator::ZeroRotator, SpawnParams))
					{
						if (WallActor->GetStaticMeshComponent() && WallMeshToUse)
						{
							WallActor->GetStaticMeshComponent()->SetStaticMesh(WallMeshToUse);
							WallActor->SetActorScale3D(FVector(0.3f, TILE_SIZE / 100.0f, 6.0f));
						}
#if WITH_EDITOR
						WallActor->SetActorLabel(FString::Printf(TEXT("Arena_%s_BackWall_%d"), *BiomeNames[i], Y));
#endif
						GeneratorSpawnedActors.Add(WallActor);
					}
				}

				// Left Wall: along +Y face (Y = +1.5 * TILE_SIZE = +1500cm), spanning X = -1, 0, +1 (Yaw = 90)
				for (int32 X = -1; X <= 1; ++X)
				{
					const FVector WallPos = ArenaPos + FVector(X * TILE_SIZE, TILE_SIZE * 1.5f, WallZ);
					if (AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(
						AStaticMeshActor::StaticClass(), WallPos, FRotator(0.0f, 90.0f, 0.0f), SpawnParams))
					{
						if (WallActor->GetStaticMeshComponent() && WallMeshToUse)
						{
							WallActor->GetStaticMeshComponent()->SetStaticMesh(WallMeshToUse);
							WallActor->SetActorScale3D(FVector(0.3f, TILE_SIZE / 100.0f, 6.0f));
						}
#if WITH_EDITOR
						WallActor->SetActorLabel(FString::Printf(TEXT("Arena_%s_LeftWall_%d"), *BiomeNames[i], X));
#endif
						GeneratorSpawnedActors.Add(WallActor);
					}
				}

				// Right Wall: along -Y face (Y = -1.5 * TILE_SIZE = -1500cm), spanning X = -1, 0, +1 (Yaw = 270)
				for (int32 X = -1; X <= 1; ++X)
				{
					const FVector WallPos = ArenaPos + FVector(X * TILE_SIZE, -TILE_SIZE * 1.5f, WallZ);
					if (AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(
						AStaticMeshActor::StaticClass(), WallPos, FRotator(0.0f, 270.0f, 0.0f), SpawnParams))
					{
						if (WallActor->GetStaticMeshComponent() && WallMeshToUse)
						{
							WallActor->GetStaticMeshComponent()->SetStaticMesh(WallMeshToUse);
							WallActor->SetActorScale3D(FVector(0.3f, TILE_SIZE / 100.0f, 6.0f));
						}
#if WITH_EDITOR
						WallActor->SetActorLabel(FString::Printf(TEXT("Arena_%s_RightWall_%d"), *BiomeNames[i], X));
#endif
						GeneratorSpawnedActors.Add(WallActor);
					}
				}
				// Front (-X face) remains 100% open like a stage!

				// 3. Local Stage Illumination Light
				if (APointLight* StageLight = World->SpawnActor<APointLight>(
					APointLight::StaticClass(), ArenaPos + FVector(0.0f, 0.0f, 800.0f), FRotator::ZeroRotator, SpawnParams))
				{
					if (UPointLightComponent* LightComp = Cast<UPointLightComponent>(StageLight->GetLightComponent()))
					{
						LightComp->SetIntensity(5000.0f);
						LightComp->SetAttenuationRadius(3500.0f);
						LightComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));
					}
#if WITH_EDITOR
					StageLight->SetActorLabel(FString::Printf(TEXT("Arena_%s_StageLight"), *BiomeNames[i]));
#endif
					GeneratorSpawnedActors.Add(StageLight);
				}
			}
		}
	}
}

FTransform ASovereignDungeonGenerator::GetCombatCameraTransform(int32 BiomeIndex) const
{
	FVector ArenaCenter = FarOffArenaOrigin;
	if (BiomeBattleArenas.IsValidIndex(BiomeIndex) && BiomeBattleArenas[BiomeIndex])
	{
		ArenaCenter = BiomeBattleArenas[BiomeIndex]->GetActorLocation();
	}

	// Camera stands in front of the open stage looking toward +X
	const FVector CamLocation = ArenaCenter + FVector(-2500.0f, 0.0f, 650.0f);
	const FRotator CamRotation = FRotator(-12.0f, 0.0f, 0.0f); // Angled slightly down looking across the stage

	return FTransform(CamRotation, CamLocation);
}

void ASovereignDungeonGenerator::InspectBattleArena()
{
#if WITH_EDITOR
	if (GEditor && GEditor->GetActiveViewport())
	{
		const FTransform CamTransform = GetCombatCameraTransform(0);
		for (FLevelEditorViewportClient* ViewportClient : GEditor->GetLevelViewportClients())
		{
			if (ViewportClient)
			{
				ViewportClient->SetViewLocation(CamTransform.GetLocation());
				ViewportClient->SetViewRotation(CamTransform.GetRotation().Rotator());
				ViewportClient->Invalidate();
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Active editor viewport focused on Battle Arena Stage."));
	}
#endif
}

void ASovereignDungeonGenerator::InspectEntrance()
{
#if WITH_EDITOR
	if (GEditor && GEditor->GetActiveViewport())
	{
		const FTransform EntranceTransform = GetEntranceTransform();
		const FVector CamPos = EntranceTransform.GetLocation() + FVector(-1200.0f, 0.0f, 600.0f);
		const FRotator CamRot = FRotator(-20.0f, 0.0f, 0.0f);

		for (FLevelEditorViewportClient* ViewportClient : GEditor->GetLevelViewportClients())
		{
			if (ViewportClient)
			{
				ViewportClient->SetViewLocation(CamPos);
				ViewportClient->SetViewRotation(CamRot);
				ViewportClient->Invalidate();
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Active editor viewport focused on Dungeon Entrance."));
	}
#endif
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
