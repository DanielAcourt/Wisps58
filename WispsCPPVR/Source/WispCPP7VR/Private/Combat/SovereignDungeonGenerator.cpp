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
#include "Entities/SovereignRestTerminal.h"
#include "Entities/SovereignDungeonDoor.h"
#include "Entities/SovereignTreasureChest.h"
#include "Entities/SovereignKeyPickup.h"
#include "Entities/SovereignDungeonEntrance.h"
#include "Entities/SovereignBossTeleporter.h"
#include "Entities/SovereignDungeonStairs.h"
#include "Entities/SovereignUpwardStairs.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Inventory/SovereignItemDefinition.h"
#include "GameplayTagsManager.h"
#include "EngineUtils.h"
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
	bRequireBossKey = true;
	bSpawnKeyAsWorldPickup = true;
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Item.Key.BossGate"));
	BossKeyItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
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

	bUseMetricModularMeshScale = false;

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
					TargetRoomCount = DungeonSubsystem->TargetRoomCount;
					SpecialRoomPercentage = DungeonSubsystem->SpecialRoomPercentage;
					TreasureRoomRatio = DungeonSubsystem->TreasureRoomRatio;
					RestSanctuaryRatio = DungeonSubsystem->RestSanctuaryRatio;
					GrandChamberRatio = DungeonSubsystem->GrandChamberRatio;
					ColosseumRatio = DungeonSubsystem->ColosseumRatio;
					EnemyBuffMultiplier = DungeonSubsystem->ActiveEnemyBuffMultiplier;
					RewardMultiplier = DungeonSubsystem->ActiveRewardMultiplier;

					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Expedition parameters ingested: Seed=%d, TargetRooms=%d, SpecialRoomPct=%f, EnemyBuff=x%f, RewardMult=x%f"),
						GenerationSeed, TargetRoomCount, SpecialRoomPercentage, EnemyBuffMultiplier, RewardMultiplier);
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

		// Multi-cell edge contact invariant: verify that at least one occupied cell in NodeA touches an occupied cell in NodeB in DirectionAtoB
		FIntPoint ExpectedOffset(0, 0);
		switch (Edge.DirectionAtoB)
		{
		case EDungeonDirection::North: ExpectedOffset = FIntPoint(1, 0); break;
		case EDungeonDirection::East:  ExpectedOffset = FIntPoint(0, 1); break;
		case EDungeonDirection::South: ExpectedOffset = FIntPoint(-1, 0); break;
		case EDungeonDirection::West:  ExpectedOffset = FIntPoint(0, -1); break;
		default: break;
		}

		const TArray<FIntPoint> CellsA = DungeonNodes[Edge.NodeIndexA].GetOccupiedCells();
		const TArray<FIntPoint> CellsB = DungeonNodes[Edge.NodeIndexB].GetOccupiedCells();

		bool bHasAdjacencyContact = false;
		for (const FIntPoint& CellA : CellsA)
		{
			if (CellsB.Contains(CellA + ExpectedOffset))
			{
				bHasAdjacencyContact = true;
				break;
			}
		}

		if (!bHasAdjacencyContact)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Validation Failed: Edge %d does not have adjacent occupied cell contact between Node %d and Node %d in direction %d."),
				Edge.EdgeId, Edge.NodeIndexA, Edge.NodeIndexB, (int32)Edge.DirectionAtoB);
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
	SpecialRooms.Empty();
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

		// 2. Generate Critical Main Spine (Option A: Bounded to prevent self-trapping on mega-dungeons, scaling dynamically with TargetRoomCount)
		const int32 DynamicMaxSpine = FMath::Max(25, FMath::FloorToInt(TargetRoomCount * 0.40f));
		const int32 MaxSpineBound = FMath::Clamp(TargetRoomCount - 2, MinSpineLength, DynamicMaxSpine);
		const int32 DesiredSpineCount = FMath::FloorToInt(TargetRoomCount * 0.50f);
		const int32 SpineTargetCount = FMath::Clamp(DesiredSpineCount, MinSpineLength, MaxSpineBound);

		// Place a Rest Sanctuary along the critical spine at roughly the midway point (Ticket B-102 & B-103)
		const int32 MidwaySpineIdx = (SpineTargetCount >= 6) ? (SpineTargetCount / 2) : INDEX_NONE;

		for (int32 i = 1; i < SpineTargetCount; ++i)
		{
			EDungeonTileType NextType = EDungeonTileType::Corridor;
			if (i == SpineTargetCount - 1)
			{
				NextType = EDungeonTileType::EliteChamber;
			}
			else if (i == MidwaySpineIdx)
			{
				NextType = EDungeonTileType::RestSanctuary;
			}
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

		// 3. Guaranteed 4x4 Boss Placement with Sliding Socket Clearance Alignment (Ticket B-101)
		// Strict 4x4 enforcement: Searches all candidate parents across all 4 sliding socket offsets along each face.
		const FIntPoint BossDims = FIntPoint(4, 4); // Strict 4x4 grand hall

		int32 ChosenEliteParentIdx = INDEX_NONE;
		FIntPoint ChosenBossCoord = FIntPoint::ZeroValue;
		EDungeonDirection ChosenBossDir = EDungeonDirection::None;

		// Candidate parent nodes ordered from deepest spine tip backward to Entrance
		TArray<int32> CandidateParents;
		for (int32 SpineIdx = MainSpineNodeIndices.Num() - 1; SpineIdx >= 1; --SpineIdx)
		{
			CandidateParents.Add(MainSpineNodeIndices[SpineIdx]);
		}

		for (int32 ParentCandidate : CandidateParents)
		{
			const FSovereignDungeonNode& ParentNode = DungeonNodes[ParentCandidate];
			const FIntPoint ParentCoord = ParentNode.GridCoord;

			// Check all 4 cardinal directions leaving the parent
			for (const auto& Opt : CardinalOptions)
			{
				const EDungeonDirection OutgoingDir = Opt.Key;
				const FIntPoint TargetContactCell = ParentCoord + Opt.Value;

				// For a 4x4 Boss Chamber, the doorway can connect to any of the 4 perimeter sockets on the opposing face:
				// If OutgoingDir is North (+X), Boss South perimeter faces South (-X). Sockets are at relative (X_offset, 0) where X_offset in [0, 3].
				// Therefore Boss Anchor X = TargetContactCell.X, Boss Anchor Y = TargetContactCell.Y - SlideOffset.
				// If OutgoingDir is South (-X), Boss North perimeter faces North (+X). Relative coords are (X_offset, 3).
				// Therefore Boss Anchor X = TargetContactCell.X, Boss Anchor Y = TargetContactCell.Y - SlideOffset.
				// If OutgoingDir is East (+Y), Boss West perimeter faces West (-Y). Relative coords are (0, Y_offset).
				// Therefore Boss Anchor X = TargetContactCell.X - SlideOffset, Boss Anchor Y = TargetContactCell.Y.
				// If OutgoingDir is West (-Y), Boss East perimeter faces East (+Y). Relative coords are (3, Y_offset).
				// Therefore Boss Anchor X = TargetContactCell.X - SlideOffset, Boss Anchor Y = TargetContactCell.Y.

				for (int32 Slide = 0; Slide < 4; ++Slide)
				{
					FIntPoint TestBossAnchor = FIntPoint::ZeroValue;
					switch (OutgoingDir)
					{
					case EDungeonDirection::North:
						TestBossAnchor = FIntPoint(TargetContactCell.X, TargetContactCell.Y - Slide);
						break;
					case EDungeonDirection::South:
						TestBossAnchor = FIntPoint(TargetContactCell.X - 3, TargetContactCell.Y - Slide);
						break;
					case EDungeonDirection::East:
						TestBossAnchor = FIntPoint(TargetContactCell.X - Slide, TargetContactCell.Y);
						break;
					case EDungeonDirection::West:
						TestBossAnchor = FIntPoint(TargetContactCell.X - Slide, TargetContactCell.Y - 3);
						break;
					default:
						break;
					}

					if (CanFitRoomFootprint(TestBossAnchor, BossDims))
					{
						ChosenEliteParentIdx = ParentCandidate;
						ChosenBossCoord = TestBossAnchor;
						ChosenBossDir = OutgoingDir;
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
			BossNode.FootprintDimensions = BossDims; // Guaranteed 4x4!
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

			UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Guaranteed 4x4 Boss Chamber Placed at Anchor (%d, %d) connected to Elite Chamber %d facing %d."),
				ChosenBossCoord.X, ChosenBossCoord.Y, ChosenEliteParentIdx, (int32)ChosenBossDir);
		}
		else
		{
			// Strict 4x4 requirement: If 4x4 could not fit, retry generation with next seed
			UE_LOG(LogTemp, Warning, TEXT("[DungeonGenerator] Attempt %d: Could not place 4x4 Boss Chamber. Retrying with next seed..."), Attempt + 1);
			continue;
		}

		// 4. Build Deep Branching Network with Proportional Special Room Quotas (Tickets B-102, B-103, B-104)
		int32 RoomsRemaining = TargetRoomCount - DungeonNodes.Num();

		// Calculate capped special room budget: strictly capped at MaxSpecialRoomCapPercentage (default 50%)
		const float EffectiveSpecialPct = FMath::Clamp(SpecialRoomPercentage, 0.0f, FMath::Min(50.0f, MaxSpecialRoomCapPercentage));
		const int32 TotalSpecialBudget = FMath::Clamp(FMath::RoundToInt(TargetRoomCount * (EffectiveSpecialPct / 100.0f)), 1, FMath::Max(1, TargetRoomCount / 2));

		int32 TreasureCount = FMath::Max(1, FMath::RoundToInt(TotalSpecialBudget * TreasureRoomRatio));
		int32 RestSanctuaryCount = FMath::RoundToInt(TotalSpecialBudget * RestSanctuaryRatio);
		int32 GrandChamberCount = FMath::RoundToInt(TotalSpecialBudget * GrandChamberRatio);
		int32 ColosseumCount = FMath::RoundToInt(TotalSpecialBudget * ColosseumRatio);

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

			// Select special room type based on budget quotas.
			// Prioritize guaranteed TreasureRoom (1x1 dead-end vault) so it isn't starved by large rooms.
			EDungeonTileType BranchType = EDungeonTileType::Chamber;
			if (TreasureCount > 0)
			{
				BranchType = EDungeonTileType::TreasureRoom;
			}
			else if (ColosseumCount > 0 && TargetRoomCount >= 30)
			{
				BranchType = EDungeonTileType::Colosseum;
			}
			else if (GrandChamberCount > 0 && TargetRoomCount >= 15)
			{
				BranchType = EDungeonTileType::GrandChamber;
			}
			else if (RestSanctuaryCount > 0)
			{
				BranchType = EDungeonTileType::RestSanctuary;
			}
			else
			{
				BranchType = EDungeonTileType::Chamber;
			}

			FIntPoint BranchDims = GetDefaultFootprintForTileType(BranchType);

			TArray<TPair<EDungeonDirection, FIntPoint>> BranchOpen;
			for (const auto& Opt : CardinalOptions)
			{
				const FIntPoint CandidateBranchAnchor = ParentCoord + Opt.Value;
				if (CanFitRoomFootprint(CandidateBranchAnchor, BranchDims))
				{
					BranchOpen.Add(Opt);
				}
			}

			// Fallback: If large special multi-cell room (Colosseum 8x8, Grand Chamber 4x6, Rest Sanctuary 2x2) cannot fit,
			// fallback to a 1x1 Standard Chamber rather than popping and discarding the parent node!
			if (BranchOpen.Num() == 0 && (BranchDims.X > 1 || BranchDims.Y > 1))
			{
				BranchType = EDungeonTileType::Chamber;
				BranchDims = FIntPoint(1, 1);
				for (const auto& Opt : CardinalOptions)
				{
					const FIntPoint CandidateBranchAnchor = ParentCoord + Opt.Value;
					if (CanFitRoomFootprint(CandidateBranchAnchor, BranchDims))
					{
						BranchOpen.Add(Opt);
					}
				}
			}

			if (BranchOpen.Num() == 0)
			{
				// Truly blocked in all 4 cardinal directions even for 1x1 tile
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

			// Decrement budget counters
			if (BranchType == EDungeonTileType::Colosseum) ColosseumCount--;
			else if (BranchType == EDungeonTileType::GrandChamber) GrandChamberCount--;
			else if (BranchType == EDungeonTileType::RestSanctuary) RestSanctuaryCount--;
			else if (BranchType == EDungeonTileType::TreasureRoom) TreasureCount--;

			// Add new branch room to eligible parents if it can accept further branches
			if (BranchType == EDungeonTileType::Chamber || BranchType == EDungeonTileType::GrandChamber || BranchType == EDungeonTileType::Colosseum)
			{
				EligibleBranchParents.Add(BranchIdx);
			}

			RoomsRemaining--;
		}

		// 4.2. Dedicated 4x4 Stair Room Placement (Ticket B-116: DU-Add stairs)
		// If bToggleBossOnlyStairs is false, and this floor has previously been cleared, roll StairRoomSpawnChance
		int32 ActiveExpeditionDepth = 0;
		int32 PlayerDeepestCompleted = 0;
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
			{
				ActiveExpeditionDepth = DungeonSub->GetCurrentDepth();
			}
		}

		if (UWorld* CurrentWorld = GetWorld())
		{
			for (TActorIterator<ASovereignPlayerWisp> WispIt(CurrentWorld); WispIt; ++WispIt)
			{
				if (ASovereignPlayerWisp* Wisp = *WispIt)
				{
					PlayerDeepestCompleted = FMath::Max(PlayerDeepestCompleted, Wisp->DeepestDungeonCompleted);
				}
			}
		}

		const bool bEligibleForStairRoom = !bToggleBossOnlyStairs && (ActiveExpeditionDepth < PlayerDeepestCompleted);
		if (bEligibleForStairRoom && (Stream.FRand() <= StairRoomSpawnChance) && EligibleBranchParents.Num() > 0)
		{
			const FIntPoint StairDims = GetDefaultFootprintForTileType(EDungeonTileType::StairRoom);
			for (int32 AttemptIdx = 0; AttemptIdx < EligibleBranchParents.Num(); ++AttemptIdx)
			{
				const int32 CandidateParent = EligibleBranchParents[AttemptIdx];
				FSovereignDungeonNode& PNode = DungeonNodes[CandidateParent];
				if (!PNode.CanAcceptConnection()) continue;

				bool bPlacedStairRoom = false;
				for (const FSovereignDoorwaySocket& ParentSocket : PNode.PerimeterSockets)
				{
					if (ParentSocket.bIsConnected) continue;

					FIntPoint StepDir = FIntPoint(1, 0);
					switch (ParentSocket.FacingDirection)
					{
					case EDungeonDirection::North: StepDir = FIntPoint(1, 0); break;
					case EDungeonDirection::East:  StepDir = FIntPoint(0, 1); break;
					case EDungeonDirection::South: StepDir = FIntPoint(-1, 0); break;
					case EDungeonDirection::West:  StepDir = FIntPoint(0, -1); break;
					default: break;
					}
					const FIntPoint CandidateAnchor = ParentSocket.CellCoord + StepDir;

					if (CanFitRoomFootprint(CandidateAnchor, StairDims))
					{
						FSovereignDungeonNode StairNode;
						StairNode.NodeIndex = DungeonNodes.Num();
						StairNode.TileType = EDungeonTileType::StairRoom;
						StairNode.GridCoord = CandidateAnchor;
						StairNode.FootprintDimensions = StairDims;
						StairNode.MaxConnections = GetDefaultMaxConnectionsForTileType(EDungeonTileType::StairRoom);
						StairNode.GeneratePerimeterSockets();

						const FVector StairWorldLoc = GetActorLocation() + FVector(CandidateAnchor.X * TILE_SIZE, CandidateAnchor.Y * TILE_SIZE, 0.0f);
						StairNode.WorldTransform = FTransform(FRotator::ZeroRotator, StairWorldLoc, FVector::OneVector);

						for (const FIntPoint& Cell : StairNode.GetOccupiedCells())
						{
							MasterGridOccupancy.Add(Cell, StairNode.NodeIndex);
						}

						const int32 StairIdx = DungeonNodes.Add(StairNode);

						FSovereignDungeonEdge StairEdge;
						StairEdge.EdgeId = DungeonEdges.Num();
						StairEdge.NodeIndexA = CandidateParent;
						StairEdge.NodeIndexB = StairIdx;
						StairEdge.DirectionAtoB = ParentSocket.FacingDirection;
						StairEdge.PassageType = EPassageType::OpenArchway;
						StairEdge.DoorState = EDoorState::Open;
						DungeonEdges.Add(StairEdge);

						UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Successfully placed dedicated 4x4 Stair Room %d off Parent %d!"),
							StairIdx, CandidateParent);

						bPlacedStairRoom = true;
						break;
					}
				}
				if (bPlacedStairRoom) break;
			}
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

			// Build and update SpecialRooms registry from generated nodes (Ticket B-120)
			SpecialRooms.Empty();
			const FVector GenOrigin = GetActorLocation();

			for (int32 NodeIdx = 0; NodeIdx < DungeonNodes.Num(); ++NodeIdx)
			{
				const FSovereignDungeonNode& Node = DungeonNodes[NodeIdx];
				// Register all non-standard rooms (Entrance, Treasure, Rest, Stair, Grand, Colosseum, Elite, Boss)
				if (Node.TileType != EDungeonTileType::Corridor && Node.TileType != EDungeonTileType::Chamber)
				{
					FSovereignSpecialRoomInfo Info;
					Info.NodeIndex = NodeIdx;
					Info.TileType = Node.TileType;
					Info.GridCoord = Node.GridCoord;
					Info.FootprintDimensions = Node.FootprintDimensions;
					Info.WorldCenterLocation = GenOrigin + FVector(
						(Node.GridCoord.X + (Node.FootprintDimensions.X - 1) * 0.5f) * TILE_SIZE,
						(Node.GridCoord.Y + (Node.FootprintDimensions.Y - 1) * 0.5f) * TILE_SIZE,
						0.0f
					);

					// Associate spawned actor if one exists for this node
					for (AActor* SpawnedAct : GeneratorSpawnedActors)
					{
						if (IsValid(SpawnedAct))
						{
							const float Dist2D = FVector::Dist2D(SpawnedAct->GetActorLocation(), Info.WorldCenterLocation);
							if (Dist2D <= (FMath::Max(Node.FootprintDimensions.X, Node.FootprintDimensions.Y) * TILE_SIZE * 0.75f))
							{
								Info.SpawnedActor = SpawnedAct;
								break;
							}
						}
					}

					FString TypeName;
					switch (Node.TileType)
					{
					case EDungeonTileType::EntranceRoom:  TypeName = TEXT("Entrance Room"); break;
					case EDungeonTileType::TreasureRoom:  TypeName = TEXT("Treasure Room"); break;
					case EDungeonTileType::RestSanctuary: TypeName = TEXT("Rest Sanctuary"); break;
					case EDungeonTileType::GrandChamber:  TypeName = TEXT("Grand Chamber"); break;
					case EDungeonTileType::Colosseum:     TypeName = TEXT("Trial Colosseum"); break;
					case EDungeonTileType::EliteChamber:  TypeName = TEXT("Elite Chamber"); break;
					case EDungeonTileType::BossChamber:   TypeName = TEXT("Boss Chamber"); break;
					case EDungeonTileType::StairRoom:     TypeName = TEXT("Stair Room"); break;
					default:                              TypeName = TEXT("Special Room"); break;
					}
					Info.DisplayName = FString::Printf(TEXT("%s (Node %d)"), *TypeName, NodeIdx);
					SpecialRooms.Add(Info);
				}
			}

			// Prominent ASCII Summary Diagnostic Logging (Ticket B-120)
			int32 ChamberCount = 0;
			int32 CorridorCount = 0;
			int32 TreasureCountTotal = 0;
			int32 RestCountTotal = 0;
			int32 GrandCountTotal = 0;
			int32 ColosseumCountTotal = 0;
			int32 EliteCountTotal = 0;
			int32 BossCountTotal = 0;
			int32 StairCountTotal = 0;

			for (const FSovereignDungeonNode& Node : DungeonNodes)
			{
				switch (Node.TileType)
				{
				case EDungeonTileType::EntranceRoom:  break;
				case EDungeonTileType::Corridor:      CorridorCount++; break;
				case EDungeonTileType::Chamber:       ChamberCount++; break;
				case EDungeonTileType::TreasureRoom:  TreasureCountTotal++; break;
				case EDungeonTileType::RestSanctuary: RestCountTotal++; break;
				case EDungeonTileType::GrandChamber:  GrandCountTotal++; break;
				case EDungeonTileType::Colosseum:     ColosseumCountTotal++; break;
				case EDungeonTileType::EliteChamber:  EliteCountTotal++; break;
				case EDungeonTileType::BossChamber:   BossCountTotal++; break;
				case EDungeonTileType::StairRoom:     StairCountTotal++; break;
				}
			}

			UE_LOG(LogTemp, Display, TEXT("================================================================================"));
			UE_LOG(LogTemp, Display, TEXT("🏛️ [SOVEREIGN DUNGEON GENERATOR] LAYOUT GENERATION COMPLETE (Attempt %d, Seed %d)"), Attempt + 1, GenerationSeed);
			UE_LOG(LogTemp, Display, TEXT("================================================================================"));
			UE_LOG(LogTemp, Display, TEXT(" Total Rooms: %d | Edges: %d | Spine Length: %d"), DungeonNodes.Num(), DungeonEdges.Num(), MainSpineNodeIndices.Num());
			UE_LOG(LogTemp, Display, TEXT(" Standard Rooms -> Corridors: %d | Chambers: %d"), CorridorCount, ChamberCount);
			UE_LOG(LogTemp, Display, TEXT(" Special Rooms  -> Entrance: 1 | Treasure: %d | Rest: %d | Boss: %d | Stairs: %d | Grand: %d | Colosseum: %d | Elite: %d"),
				TreasureCountTotal, RestCountTotal, BossCountTotal, StairCountTotal, GrandCountTotal, ColosseumCountTotal, EliteCountTotal);
			UE_LOG(LogTemp, Display, TEXT("--------------------------------------------------------------------------------"));
			UE_LOG(LogTemp, Display, TEXT(" 📦 SPECIAL ROOMS & CHEST LOCATIONS REGISTRY (%d Total):"), SpecialRooms.Num());
			for (const FSovereignSpecialRoomInfo& RoomInfo : SpecialRooms)
			{
				const FString ActorStr = RoomInfo.SpawnedActor ? RoomInfo.SpawnedActor->GetClass()->GetName() : TEXT("None");
				UE_LOG(LogTemp, Display, TEXT("   • [%s] Coord: (%d, %d) | Footprint: %dx%d | Center: X=%.0f Y=%.0f Z=%.0f | Actor: %s"),
					*RoomInfo.DisplayName, RoomInfo.GridCoord.X, RoomInfo.GridCoord.Y,
					RoomInfo.FootprintDimensions.X, RoomInfo.FootprintDimensions.Y,
					RoomInfo.WorldCenterLocation.X, RoomInfo.WorldCenterLocation.Y, RoomInfo.WorldCenterLocation.Z,
					*ActorStr);
			}
			UE_LOG(LogTemp, Display, TEXT("================================================================================"));

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

	const FVector Origin = GetActorLocation();

	// 2. Spawn Floor Tiles for Each Room (Multi-cell aware, Ticket B-109 metric scale toggle)
	const FVector FloorScale = bUseMetricModularMeshScale ? FVector(1.0f, 1.0f, 1.0f) : FVector(TILE_SIZE / 100.0f, TILE_SIZE / 100.0f, 0.2f);

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
			FloorTransform.SetScale3D(FloorScale);

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

			// Scale standard modular wall (1:1 scale for metric artist mesh, or primitive cube scaling, Ticket B-109)
			const FVector StandardWallScale = bUseMetricModularMeshScale ? FVector(1.0f, 1.0f, 1.0f) : FVector(0.3f, TILE_SIZE / 100.0f, WallHeight / 100.0f);
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

				// Spawn Interactive Door / Boss Gate Actor (Ticket B-105)
				if (EdgePtr && !ProcessedSealedGates.Contains(EdgeId) && GetWorld())
				{
					ProcessedSealedGates.Add(EdgeId);

					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					if (EdgePtr->DoorState == EDoorState::SealedEliteGate)
					{
						// Boss Gate spawning: Use BossGateBlueprintClass if assigned, else fallback
						UClass* GateClass = BossGateBlueprintClass ? BossGateBlueprintClass.Get() : ASovereignDungeonDoor::StaticClass();
						if (ASovereignDungeonDoor* GateDoor = GetWorld()->SpawnActor<ASovereignDungeonDoor>(
							GateClass, FaceTransform.GetLocation(), FaceTransform.GetRotation().Rotator(), SpawnParams))
						{
							GateDoor->bIsLocked = true;
							if (bRequireBossKey)
							{
								GateDoor->UnlockCondition = EBossGateUnlockCondition::RequiresBossKey;
								GateDoor->RequiredKeyTag = BossKeyItemTag.IsValid() ? BossKeyItemTag : FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
							}
							else
							{
								GateDoor->UnlockCondition = EBossGateUnlockCondition::DefeatEliteGuardian;
							}
							GeneratorSpawnedActors.Add(GateDoor);
							UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned Interactive Sealed Boss Gate Actor at Edge %d (Condition: %d)."),
								EdgeId, (int32)GateDoor->UnlockCondition);
						}
					}
					else if (DoorBlueprintClass)
					{
						// Standard Doorway Blueprint Actor
						if (ASovereignDungeonDoor* StandardDoor = GetWorld()->SpawnActor<ASovereignDungeonDoor>(
							DoorBlueprintClass, FaceTransform.GetLocation(), FaceTransform.GetRotation().Rotator(), SpawnParams))
						{
							StandardDoor->bIsLocked = false;
							GeneratorSpawnedActors.Add(StandardDoor);
							UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned Interactive Door Actor at Edge %d."), EdgeId);
						}
					}
				}

				// If no DoorBlueprintClass was assigned and bProceduralArchwayFallback is enabled, construct primitive archway opening
				if (!DoorBlueprintClass && bProceduralArchwayFallback && WallInstancedComponent)
				{
					// Construct a 400w x 500h opening using left post, right post, and top lintel
					const FVector LeftPostLocal(-350.0f * FaceRotation.RotateVector(FVector::RightVector));
					FTransform LeftTransform = FaceTransform;
					LeftTransform.SetLocation(FaceTransform.GetLocation() + LeftPostLocal);
					LeftTransform.SetScale3D(FVector(0.3f, 3.0f, WallHeight / 100.0f));
					WallInstancedComponent->AddInstance(LeftTransform);

					const FVector RightPostLocal(350.0f * FaceRotation.RotateVector(FVector::RightVector));
					FTransform RightTransform = FaceTransform;
					RightTransform.SetLocation(FaceTransform.GetLocation() + RightPostLocal);
					RightTransform.SetScale3D(FVector(0.3f, 3.0f, WallHeight / 100.0f));
					WallInstancedComponent->AddInstance(RightTransform);

					const float LintelZ = bWallMeshCenterPivot ? 250.0f : 500.0f;
					FTransform LintelTransform = FaceTransform;
					LintelTransform.SetLocation(FaceTransform.GetLocation() + FVector(0.0f, 0.0f, LintelZ));
					LintelTransform.SetScale3D(FVector(0.3f, 4.0f, 1.0f));
					WallInstancedComponent->AddInstance(LintelTransform);
				}
			}
		}

		// 3.5 For Entrance Room: Spawn Interactive Entrance Gateway Actor (Ticket B-111)
		if (Node.TileType == EDungeonTileType::EntranceRoom && GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Find an unconnected boundary perimeter socket (closed wall) opposite outgoing passages
			FVector EntrancePortalPos = Node.WorldTransform.GetLocation();
			FRotator EntrancePortalRot = Node.WorldTransform.GetRotation().Rotator();
			bool bFoundClosedWall = false;

			for (const FSovereignDoorwaySocket& Socket : Node.PerimeterSockets)
			{
				const uint8 DirMask = 1 << static_cast<uint8>(Socket.FacingDirection);
				const bool bHasEdge = (Node.ConnectedPassageMask & DirMask) != 0;
				if (!bHasEdge)
				{
					// Found an unconnected perimeter boundary
					FVector DirVector = FVector::ZeroVector;
					switch (Socket.FacingDirection)
					{
					case EDungeonDirection::North: DirVector = FVector(1.0f, 0.0f, 0.0f); break;
					case EDungeonDirection::South: DirVector = FVector(-1.0f, 0.0f, 0.0f); break;
					case EDungeonDirection::East:  DirVector = FVector(0.0f, 1.0f, 0.0f); break;
					case EDungeonDirection::West:  DirVector = FVector(0.0f, -1.0f, 0.0f); break;
					default: break;
					}

					EntrancePortalPos = Node.WorldTransform.GetLocation() + (DirVector * (TILE_SIZE * 0.45f));
					EntrancePortalRot = DirVector.Rotation();
					bFoundClosedWall = true;
					break;
				}
			}

			int32 ActiveDepth = 0;
			if (UGameInstance* GI = GetWorld()->GetGameInstance())
			{
				if (USovereignDungeonSubsystem* DungeonSub = GI->GetSubsystem<USovereignDungeonSubsystem>())
				{
					ActiveDepth = DungeonSub->GetCurrentDepth();
				}
			}

			UClass* ActorClassToSpawn = nullptr;
			if (ActiveDepth > 0)
			{
				ActorClassToSpawn = UpwardStairsClass ? UpwardStairsClass.Get() : ASovereignUpwardStairs::StaticClass();
			}
			else
			{
				ActorClassToSpawn = EntranceActorClass ? EntranceActorClass.Get() : ASovereignDungeonEntrance::StaticClass();
			}

			if (ASovereignBaseInteractable* EntranceActor = GetWorld()->SpawnActor<ASovereignBaseInteractable>(
				ActorClassToSpawn, EntrancePortalPos, EntrancePortalRot, SpawnParams))
			{
				GeneratorSpawnedActors.Add(EntranceActor);
				UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned %s in Node %d at %s (Depth: %d)."),
					*ActorClassToSpawn->GetName(), NodeIdx, *EntrancePortalPos.ToString(), ActiveDepth);
			}
		}

		// 4. For Terminal Boss Chamber: Spawn Boss Altar / Egg Prop & Extraction Teleporter (Ticket B-112)
		if (Node.TileType == EDungeonTileType::BossChamber)
		{
			const FVector BossCenterPos = AnchorPos + FVector(
				(Node.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
				(Node.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f,
				50.0f
			);

			if (BossAltarMesh && GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

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

			// Spawn ASovereignBossTeleporter placed behind the boss opposite the entry door
			if (GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Find direction of the boss entrance edge
				EDungeonDirection EntryDirection = EDungeonDirection::South;
				for (const FSovereignDungeonEdge& Edge : DungeonEdges)
				{
					if (Edge.Connects(NodeIdx))
					{
						// If NodeIdx is NodeB, the edge entered from NodeA via DirectionAtoB
						if (Edge.NodeIndexB == NodeIdx)
						{
							EntryDirection = Edge.DirectionAtoB;
						}
						else
						{
							switch (Edge.DirectionAtoB)
							{
							case EDungeonDirection::North: EntryDirection = EDungeonDirection::South; break;
							case EDungeonDirection::South: EntryDirection = EDungeonDirection::North; break;
							case EDungeonDirection::East:  EntryDirection = EDungeonDirection::West; break;
							case EDungeonDirection::West:  EntryDirection = EDungeonDirection::East; break;
							default: break;
							}
						}
						break;
					}
				}

				// The back wall is in the forward direction of EntryDirection
				FVector BackWallOffset = FVector::ZeroVector;
				switch (EntryDirection)
				{
				case EDungeonDirection::North: BackWallOffset = FVector(1.0f, 0.0f, 0.0f); break;
				case EDungeonDirection::South: BackWallOffset = FVector(-1.0f, 0.0f, 0.0f); break;
				case EDungeonDirection::East:  BackWallOffset = FVector(0.0f, 1.0f, 0.0f); break;
				case EDungeonDirection::West:  BackWallOffset = FVector(0.0f, -1.0f, 0.0f); break;
				default: BackWallOffset = FVector(1.0f, 0.0f, 0.0f); break;
				}

				// Offset teleporter behind boss relative to boss entrance
				const float HalfChamberExtent = (Node.FootprintDimensions.X * TILE_SIZE) * 0.40f;
				const FVector TeleporterPos = BossCenterPos + (BackWallOffset * HalfChamberExtent);
				const FRotator TeleporterRot = (-BackWallOffset).Rotation(); // Face inwards toward center

				UClass* TeleporterClassToSpawn = BossTeleporterClass ? BossTeleporterClass.Get() : ASovereignBossTeleporter::StaticClass();
				if (ASovereignBossTeleporter* Teleporter = GetWorld()->SpawnActor<ASovereignBossTeleporter>(
					TeleporterClassToSpawn, TeleporterPos, TeleporterRot, SpawnParams))
				{
					GeneratorSpawnedActors.Add(Teleporter);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned ASovereignBossTeleporter in BossChamber Node %d at %s."),
						NodeIdx, *TeleporterPos.ToString());
				}

				// Spawn descent stairs in Boss Chamber (Ticket B-116)
				if (bSpawnDescentStairs)
				{
					FVector OrthoOffset = FVector::CrossProduct(BackWallOffset, FVector::UpVector).GetSafeNormal();
					const FVector StairsPos = TeleporterPos + (OrthoOffset * (TILE_SIZE * 0.35f));

					UClass* StairsClassToSpawn = DescentStairsClass ? DescentStairsClass.Get() : ASovereignDungeonStairs::StaticClass();
					if (ASovereignDungeonStairs* Stairs = GetWorld()->SpawnActor<ASovereignDungeonStairs>(
						StairsClassToSpawn, StairsPos, TeleporterRot, SpawnParams))
					{
						if (DescentStairsMesh && Stairs->GatewayDoorMesh)
						{
							Stairs->GatewayDoorMesh->SetStaticMesh(DescentStairsMesh);
						}

						GeneratorSpawnedActors.Add(Stairs);
						UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned ASovereignDungeonStairs in BossChamber Node %d at %s."),
							NodeIdx, *StairsPos.ToString());
					}
				}
			}
		}
		// 5. For Rest Sanctuary: Spawn ASovereignRestTerminal (Ticket B-103 & B-106)
		else if (Node.TileType == EDungeonTileType::RestSanctuary)
		{
			if (GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FVector RestCenterPos = AnchorPos + FVector(
					(Node.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
					(Node.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f,
					30.0f
				);

				UClass* TerminalClassToSpawn = RestTerminalClass ? RestTerminalClass.Get() : ASovereignRestTerminal::StaticClass();
				if (ASovereignRestTerminal* Terminal = GetWorld()->SpawnActor<ASovereignRestTerminal>(TerminalClassToSpawn, RestCenterPos, FRotator::ZeroRotator, SpawnParams))
				{
					Terminal->MaxRestCharges = DefaultRestCharges;
					Terminal->RemainingRestCharges = DefaultRestCharges;
					Terminal->bRandomizeCharges = bRandomizeRestCharges;
					GeneratorSpawnedActors.Add(Terminal);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned ASovereignRestTerminal in RestSanctuary Node %d."), NodeIdx);
				}
			}
		}
		// 6. For Treasure Room: Spawn ASovereignTreasureChest (Ticket B-107)
		else if (Node.TileType == EDungeonTileType::TreasureRoom)
		{
			if (GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FVector ChestCenterPos = AnchorPos + FVector(
					(Node.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
					(Node.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f,
					30.0f
				);

				UClass* ChestClassToSpawn = TreasureChestBlueprintClass ? TreasureChestBlueprintClass.Get() : ASovereignTreasureChest::StaticClass();
				if (ASovereignTreasureChest* Chest = GetWorld()->SpawnActor<ASovereignTreasureChest>(ChestClassToSpawn, ChestCenterPos, FRotator::ZeroRotator, SpawnParams))
				{
					// If Boss Key required and Option C routes key into chest, add key item
					if (bRequireBossKey && !bSpawnKeyAsWorldPickup && Chest->LootInventory)
					{
						USovereignItemDefinition* KeyDefToUse = BossKeyItemDefinition ? BossKeyItemDefinition.Get() : NewObject<USovereignItemDefinition>(this);
						if (!BossKeyItemDefinition)
						{
							KeyDefToUse->ItemTag = BossKeyItemTag.IsValid() ? BossKeyItemTag : FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
							KeyDefToUse->DisplayName = NSLOCTEXT("Sovereign", "BossKeyItemName", "Vault Guardian Key");
						}
						Chest->LootInventory->AddItem(KeyDefToUse, 1);
						UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Placed Boss Key inside Treasure Chest in Node %d."), NodeIdx);
					}

					GeneratorSpawnedActors.Add(Chest);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned ASovereignTreasureChest in TreasureRoom Node %d."), NodeIdx);
				}
			}
		}
		// 7. For Dedicated 4x4 Stair Room: Spawn Descent Stairs & Optional Rest Sanctuary (Ticket B-116)
		else if (Node.TileType == EDungeonTileType::StairRoom)
		{
			if (GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FVector StairCenterPos = AnchorPos + FVector(
					(Node.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
					(Node.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f,
					30.0f
				);

				UClass* StairsClassToSpawn = DescentStairsClass ? DescentStairsClass.Get() : ASovereignDungeonStairs::StaticClass();
				if (ASovereignDungeonStairs* Stairs = GetWorld()->SpawnActor<ASovereignDungeonStairs>(
					StairsClassToSpawn, StairCenterPos, FRotator::ZeroRotator, SpawnParams))
				{
					// Dedicated Stair Room bypasses the boss; stairs start unlocked without requiring boss kill
					Stairs->bRequireBossDefeat = false;
					Stairs->SetGatewayUnlocked(true);

					if (DescentStairsMesh && Stairs->GatewayDoorMesh)
					{
						Stairs->GatewayDoorMesh->SetStaticMesh(DescentStairsMesh);
					}

					GeneratorSpawnedActors.Add(Stairs);
					UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned Unlocked ASovereignDungeonStairs in StairRoom Node %d."), NodeIdx);
				}

				// Optional Rest Terminal spawn chance inside Stair Room (Ticket B-116)
				if (FMath::FRand() <= StairRoomRestTerminalChance)
				{
					const FVector RestPos = StairCenterPos + FVector(TILE_SIZE * 0.35f, TILE_SIZE * 0.35f, 0.0f);
					UClass* TerminalClassToSpawn = RestTerminalClass ? RestTerminalClass.Get() : ASovereignRestTerminal::StaticClass();
					if (ASovereignRestTerminal* Terminal = GetWorld()->SpawnActor<ASovereignRestTerminal>(
						TerminalClassToSpawn, RestPos, FRotator::ZeroRotator, SpawnParams))
					{
						Terminal->MaxRestCharges = DefaultRestCharges;
						Terminal->RemainingRestCharges = DefaultRestCharges;
						Terminal->bRandomizeCharges = bRandomizeRestCharges;
						GeneratorSpawnedActors.Add(Terminal);
						UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned bonus ASovereignRestTerminal in StairRoom Node %d."), NodeIdx);
					}
				}
			}
		}
	}

	// 5. Boss Key Physical World Pickup Routing (Option C: bSpawnKeyAsWorldPickup == true)
	if (bRequireBossKey && bSpawnKeyAsWorldPickup && GetWorld())
	{
		// Find a suitable pre-boss room along the spine or branch (RestSanctuary or middle Corridor/Chamber)
		int32 KeyRoomIdx = INDEX_NONE;

		// Prioritize RestSanctuary
		for (int32 i = 0; i < DungeonNodes.Num(); ++i)
		{
			if (DungeonNodes[i].TileType == EDungeonTileType::RestSanctuary)
			{
				KeyRoomIdx = i;
				break;
			}
		}

		// Fallback to middle spine room
		if (KeyRoomIdx == INDEX_NONE && MainSpineNodeIndices.Num() > 1)
		{
			KeyRoomIdx = MainSpineNodeIndices[MainSpineNodeIndices.Num() / 2];
		}

		if (KeyRoomIdx != INDEX_NONE && DungeonNodes.IsValidIndex(KeyRoomIdx))
		{
			const FSovereignDungeonNode& KeyRoom = DungeonNodes[KeyRoomIdx];
			const FVector KeyPos = KeyRoom.WorldTransform.GetLocation() + FVector(
				(KeyRoom.FootprintDimensions.X - 1) * TILE_SIZE * 0.5f,
				(KeyRoom.FootprintDimensions.Y - 1) * TILE_SIZE * 0.5f + 250.0f,
				80.0f
			);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UClass* KeyClassToSpawn = KeyPickupBlueprintClass ? KeyPickupBlueprintClass.Get() : ASovereignKeyPickup::StaticClass();
			if (ASovereignKeyPickup* KeyPickup = GetWorld()->SpawnActor<ASovereignKeyPickup>(KeyClassToSpawn, KeyPos, FRotator::ZeroRotator, SpawnParams))
			{
				KeyPickup->KeyTag = BossKeyItemTag.IsValid() ? BossKeyItemTag : FGameplayTag::RequestGameplayTag(FName("Item.Key.BossGate"), false);
				KeyPickup->KeyItemDefinition = BossKeyItemDefinition;
				GeneratorSpawnedActors.Add(KeyPickup);
				UE_LOG(LogTemp, Log, TEXT("[DungeonGenerator] Spawned physical ASovereignKeyPickup on pedestal in Node %d."), KeyRoomIdx);
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

ASovereignBossTeleporter* ASovereignDungeonGenerator::GetBossTeleporterActor() const
{
	for (AActor* SpawnedActor : GeneratorSpawnedActors)
	{
		if (SpawnedActor && (SpawnedActor->IsA<ASovereignBossTeleporter>() || (BossTeleporterClass && SpawnedActor->IsA(BossTeleporterClass))))
		{
			return Cast<ASovereignBossTeleporter>(SpawnedActor);
		}
	}
	return nullptr;
}

ASovereignBaseInteractable* ASovereignDungeonGenerator::GetEntranceGatewayActor() const
{
	for (AActor* SpawnedActor : GeneratorSpawnedActors)
	{
		if (SpawnedActor && (SpawnedActor->IsA<ASovereignDungeonEntrance>() || (EntranceActorClass && SpawnedActor->IsA(EntranceActorClass))))
		{
			return Cast<ASovereignBaseInteractable>(SpawnedActor);
		}
	}
	return nullptr;
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
