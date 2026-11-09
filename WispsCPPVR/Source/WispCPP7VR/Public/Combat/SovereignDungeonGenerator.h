// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/SovereignBattleManager.h"
#include "SovereignDungeonGenerator.generated.h"

class ASovereignBaseCharacter;
class UStaticMesh;

// --- Direction, Passage, and Door Enums (Ticket B-090) ---

UENUM(BlueprintType)
enum class EDungeonDirection : uint8
{
	North = 0 UMETA(DisplayName = "North (+X)"),
	East  = 1 UMETA(DisplayName = "East (+Y)"),
	South = 2 UMETA(DisplayName = "South (-X)"),
	West  = 3 UMETA(DisplayName = "West (-Y)"),
	None  = 4 UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EPassageType : uint8
{
	OpenArchway     UMETA(DisplayName = "Open Archway"),
	SolidWall       UMETA(DisplayName = "Solid Wall"),
	StandardDoorway UMETA(DisplayName = "Standard Doorway")
};

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Open            UMETA(DisplayName = "Open"),
	ClosedUnlocked  UMETA(DisplayName = "Closed Unlocked"),
	SealedEliteGate UMETA(DisplayName = "Sealed Elite Gate")
};

/**
 * EDungeonTileType
 * Modular room and corridor types assembled by the procedural generator.
 */
UENUM(BlueprintType)
enum class EDungeonTileType : uint8
{
	EntranceRoom    UMETA(DisplayName = "Entrance Room"),
	Corridor        UMETA(DisplayName = "Corridor"),
	Chamber         UMETA(DisplayName = "Chamber"),
	EliteChamber    UMETA(DisplayName = "Elite Chamber"),
	TreasureRoom    UMETA(DisplayName = "Treasure Room"),
	RestSanctuary   UMETA(DisplayName = "Rest Sanctuary"),
	GrandChamber    UMETA(DisplayName = "Grand Chamber"),
	Colosseum       UMETA(DisplayName = "Trial Colosseum"),
	BossChamber     UMETA(DisplayName = "Boss Chamber"),
	StairRoom       UMETA(DisplayName = "Stair Room")
};

/**
 * FSovereignDungeonEdge
 * Authoritative Single Source of Truth (SSoT) for connectivity between two rooms (Ticket B-090).
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonEdge
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 EdgeId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndexA = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndexB = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EDungeonDirection DirectionAtoB = EDungeonDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EPassageType PassageType = EPassageType::OpenArchway;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EDoorState DoorState = EDoorState::Open;

	bool Connects(int32 NodeIdx) const { return NodeIndexA == NodeIdx || NodeIndexB == NodeIdx; }
	int32 GetOtherNode(int32 NodeIdx) const { return (NodeIndexA == NodeIdx) ? NodeIndexB : (NodeIndexB == NodeIdx ? NodeIndexA : INDEX_NONE); }
};

/**
 * FSovereignDoorwaySocket
 * Represents an individual connection socket along the outer perimeter of a room footprint (Ticket B-094.2).
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDoorwaySocket
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 SocketId = INDEX_NONE;

	/** Specific 1000cm grid cell coordinate on the room perimeter where this socket resides */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint CellCoord = FIntPoint::ZeroValue;

	/** Outward facing cardinal direction of this doorway opening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	EDungeonDirection FacingDirection = EDungeonDirection::None;

	/** Whether this socket is actively connected to an edge leading to another room */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	bool bIsConnected = false;

	/** ID of the connecting edge in DungeonEdges */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 ConnectedEdgeId = INDEX_NONE;

	/** Index of the adjacent room node connected via this socket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 ConnectedNodeIndex = INDEX_NONE;
};

/**
 * FSovereignDungeonNode
 * Represents an individual room node with footprint dimensions and perimeter connection sockets (Ticket B-094 & B-094.2).
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndex = INDEX_NONE;

	/** Bottom-Left anchor cell coordinate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	/** Footprint dimensions in 1000cm grid cells (e.g., 1x1, 2x2, 4x4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint FootprintDimensions = FIntPoint(1, 1);

	/** Maximum allowed concurrent connections to other rooms (Treasure=1, Boss=1, Corridor=2, Chamber=4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	int32 MaxConnections = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Corridor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FTransform WorldTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	bool bHasFixedEliteMob = false;

	/** Transient bitmask cache derived strictly from DungeonEdges (Bit 0=N, 1=E, 2=S, 3=W) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	uint8 ConnectedPassageMask = 0;

	/** Available perimeter sockets along the outer boundary of this room (Ticket B-094.2) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	TArray<FSovereignDoorwaySocket> PerimeterSockets;

	/** Returns all individual 1000cm grid cell coordinates occupied by this room footprint (Ticket B-094) */
	TArray<FIntPoint> GetOccupiedCells() const
	{
		TArray<FIntPoint> Cells;
		const int32 W = FMath::Max(1, FootprintDimensions.X);
		const int32 H = FMath::Max(1, FootprintDimensions.Y);
		Cells.Reserve(W * H);
		for (int32 X = 0; X < W; ++X)
		{
			for (int32 Y = 0; Y < H; ++Y)
			{
				Cells.Add(GridCoord + FIntPoint(X, Y));
			}
		}
		return Cells;
	}

	/** Returns how many perimeter sockets are currently connected */
	int32 GetConnectedSocketCount() const
	{
		int32 Count = 0;
		for (const FSovereignDoorwaySocket& Socket : PerimeterSockets)
		{
			if (Socket.bIsConnected)
			{
				Count++;
			}
		}
		return Count;
	}

	/** Whether this room has capacity to accept more connections */
	bool CanAcceptConnection() const
	{
		return GetConnectedSocketCount() < MaxConnections;
	}

	/**
	 * Automatically populates all 2*(W+H) outer boundary perimeter sockets based on footprint dimensions.
	 */
	void GeneratePerimeterSockets()
	{
		PerimeterSockets.Empty();
		const int32 W = FMath::Max(1, FootprintDimensions.X);
		const int32 H = FMath::Max(1, FootprintDimensions.Y);

		int32 CurrentId = 0;

		// In WispsGame coordinate system:
		// +X is North, -X is South
		// +Y is East,  -Y is West
		// FootprintDimensions: X spans [0, W-1] (North-South depth), Y spans [0, H-1] (East-West width)

		// 1. South perimeter (-X boundary): X = 0, for all Y in [0, H-1], facing South (-X)
		for (int32 Y = 0; Y < H; ++Y)
		{
			FSovereignDoorwaySocket Sock;
			Sock.SocketId = CurrentId++;
			Sock.CellCoord = GridCoord + FIntPoint(0, Y);
			Sock.FacingDirection = EDungeonDirection::South;
			PerimeterSockets.Add(Sock);
		}

		// 2. North perimeter (+X boundary): X = W - 1, for all Y in [0, H-1], facing North (+X)
		for (int32 Y = 0; Y < H; ++Y)
		{
			FSovereignDoorwaySocket Sock;
			Sock.SocketId = CurrentId++;
			Sock.CellCoord = GridCoord + FIntPoint(W - 1, Y);
			Sock.FacingDirection = EDungeonDirection::North;
			PerimeterSockets.Add(Sock);
		}

		// 3. West perimeter (-Y boundary): Y = 0, for all X in [0, W-1], facing West (-Y)
		for (int32 X = 0; X < W; ++X)
		{
			FSovereignDoorwaySocket Sock;
			Sock.SocketId = CurrentId++;
			Sock.CellCoord = GridCoord + FIntPoint(X, 0);
			Sock.FacingDirection = EDungeonDirection::West;
			PerimeterSockets.Add(Sock);
		}

		// 4. East perimeter (+Y boundary): Y = H - 1, for all X in [0, W-1], facing East (+Y)
		for (int32 X = 0; X < W; ++X)
		{
			FSovereignDoorwaySocket Sock;
			Sock.SocketId = CurrentId++;
			Sock.CellCoord = GridCoord + FIntPoint(X, H - 1);
			Sock.FacingDirection = EDungeonDirection::East;
			PerimeterSockets.Add(Sock);
		}
	}
};

/**
 * FSovereignSpecialRoomInfo
 * Diagnostic & gameplay registry entry for special rooms spawned within the procedural dungeon (Ticket B-120).
 * Tracks Entrance, Treasure, Rest, Stair, Grand, Colosseum, Elite, and Boss rooms.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignSpecialRoomInfo
{
	GENERATED_BODY()

	/** Logical node index in DungeonNodes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndex = INDEX_NONE;

	/** Semantic room type (EntranceRoom, TreasureRoom, RestSanctuary, BossChamber, StairRoom, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Chamber;

	/** Top-left anchor coordinate in 1000cm grid units */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	/** World location representing the spatial center of this room footprint */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	FVector WorldCenterLocation = FVector::ZeroVector;

	/** Width and height of this room in 1000cm grid cells */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	FIntPoint FootprintDimensions = FIntPoint(1, 1);

	/** Spawned interactive actor associated with this room (ASovereignTreasureChest, RestTerminal, Stairs, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	TObjectPtr<AActor> SpawnedActor = nullptr;

	/** Human-readable display label (e.g., "Treasure Room (Node 4)") */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	FString DisplayName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonGenerated, int32, TotalTilesGenerated);

/**
 * ASovereignDungeonGenerator
 * Procedural dungeon builder assembling modular 1000cm x 1000cm rooms, corridors,
 * and terminal Boss Chambers with guaranteed dragon egg rewards.
 * Places dedicated Battle Arena stages far off at extreme coordinates (50,000, 50,000, 5,000),
 * and drives random step encounter meters alongside fixed elite encounters.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASovereignDungeonGenerator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Generation Settings ---
	static constexpr float TILE_SIZE = 1000.0f; // 1000cm per tile

	/** Number of rooms/chambers to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 TargetRoomCount = 8;

	/** Random generation seed (0 = random) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 GenerationSeed = 1337;

	// --- Boss Key Configuration & Routing (Ticket B-108) ---

	/** Whether the Boss Gate requires a Key Item to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Keys")
	bool bRequireBossKey = true;

	/** Option C: If true, spawns a physical 3D key pickup actor on a pedestal. If false, places key inside a Treasure Chest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Keys", meta = (EditCondition = "bRequireBossKey"))
	bool bSpawnKeyAsWorldPickup = true;

	/** GameplayTag identifying the key item required to unlock the Boss Gate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Keys", meta = (EditCondition = "bRequireBossKey"))
	FGameplayTag BossKeyItemTag;

	/** Optional custom item definition DataAsset for the Boss Key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Keys", meta = (EditCondition = "bRequireBossKey"))
	TObjectPtr<class USovereignItemDefinition> BossKeyItemDefinition;

	/** Optional custom Blueprint class for the physical world key pickup actor (inherits ASovereignKeyPickup) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignKeyPickup> KeyPickupBlueprintClass;

	// --- Special Room Budgeting & Ratios (Tickets B-102, B-103, B-104) ---

	/** Percentage of Total Rooms allocated to Special Rooms (Default 20%, strictly capped by MaxSpecialRoomCapPercentage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float SpecialRoomPercentage = 20.0f;

	/** Hard maximum cap on special room allocation (Strict 50% cap prevents runaway special room bloat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	float MaxSpecialRoomCapPercentage = 50.0f;

	/** Ratio of special room budget allocated to Treasure Rooms (1x1 dead-end loot vault) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	float TreasureRoomRatio = 0.40f;

	/** Ratio of special room budget allocated to Rest Sanctuaries (2x2 healing camp) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	float RestSanctuaryRatio = 0.30f;

	/** Ratio of special room budget allocated to Grand Chambers (4x6 large pillar halls) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	float GrandChamberRatio = 0.20f;

	/** Ratio of special room budget allocated to Trial Colosseums (8x8 massive battle arenas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	float ColosseumRatio = 0.10f;

	/** Optional custom actor class spawned in Rest Sanctuaries (Defaults to ASovereignRestTerminal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	TSubclassOf<class ASovereignRestTerminal> RestTerminalClass;

	/** Default rest charges for spawned rest terminals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	int32 DefaultRestCharges = 3;

	/** If true, randomizes rest charges between 1 and 5 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Special Rooms")
	bool bRandomizeRestCharges = false;

	/** Minimum required path distance from Entrance to Boss Chamber */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 MinSpineLength = 4;

	/** Max controlled loop count (Ticket B-096: allows controlled shortcut loops between non-critical rooms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 MaxLoopCount = 1;

	/** Max deterministic generation retries if layout fails validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 MaxGenerationRetries = 20;

	/** Active combat enemy buff multiplier inherited from expedition gateway */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Config")
	float EnemyBuffMultiplier = 1.0f;

	/** Active reward multiplier inherited from expedition gateway */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Config")
	float RewardMultiplier = 1.0f;

	// --- Visuals & Lighting Rig Settings ---

	/** Master toggle: Whether to automatically spawn fallback ambient lighting and post-processing.
	 * Disable this if your dungeon level template already defines custom SkyLight, DirectionalLight, or environment lighting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Lighting")
	bool bAutoSpawnDefaultLighting = true;

	/** If true and bAutoSpawnDefaultLighting is enabled, spawns an ambient directional fill light if none exists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Lighting", meta = (EditCondition = "bAutoSpawnDefaultLighting"))
	bool bSpawnDirectionalLight = true;

	/** If true and bAutoSpawnDefaultLighting is enabled, spawns an unbounded post-process volume with auto-exposure bounds if none exists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Lighting", meta = (EditCondition = "bAutoSpawnDefaultLighting"))
	bool bSpawnPostProcessVolume = true;

	/** Intensity for ambient directional fill light in procedural dungeons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Lighting", meta = (EditCondition = "bAutoSpawnDefaultLighting && bSpawnDirectionalLight"))
	float AmbientLightIntensity = 1000.0f;

	/** Color tint for subterranean ambient lighting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Lighting", meta = (EditCondition = "bAutoSpawnDefaultLighting && bSpawnDirectionalLight"))
	FColor AmbientLightColor = FColor(180, 205, 235);

	// --- 3D Modular Mesh Assets (Ticket B-089) ---

	/** Whether to physically instantiate 3D geometry components and spawn world meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	bool bSpawnPhysicalGeometry = true;

	/** True if wall mesh pivot is in the center of the mesh (e.g. Engine/BasicShapes/Cube), false if bottom-center (standard modular kit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	bool bWallMeshCenterPivot = true;

	/** When true, mesh instances use 1:1 scale (1.0, 1.0, 1.0) for artist modular assets modeled to metric standard. When false, scales 100cm primitive cubes (Ticket B-109). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	bool bUseMetricModularMeshScale = false;

	/** When true and no DoorBlueprintClass is assigned, assembles a 3-part archway (two posts and top lintel) with a 400x500cm opening instead of leaving an empty void */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	bool bProceduralArchwayFallback = false;

	/** Static Mesh for standard 1000cm x 1000cm floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> FloorMesh;

	/** Static Mesh for perimeter walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> WallMesh;

	/** Optional custom Blueprint actor class for interactive swinging doors (Ticket B-105, inherits ASovereignDungeonDoor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignDungeonDoor> DoorBlueprintClass;

	/** Optional custom Blueprint actor class for sealed Boss Gates (Ticket B-105, inherits ASovereignDungeonDoor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignDungeonDoor> BossGateBlueprintClass;

	/** Optional custom Blueprint actor class for treasure chests (Ticket B-107, inherits ASovereignTreasureChest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignTreasureChest> TreasureChestBlueprintClass;

	/** Optional custom Blueprint actor class for dungeon entrance gateway (Ticket B-111, inherits ASovereignBaseInteractable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignBaseInteractable> EntranceActorClass;

	/** Optional custom Blueprint actor class for upward return stairs on Depth >= 1 (Ticket B-117, inherits ASovereignUpwardStairs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignUpwardStairs> UpwardStairsClass;

	/** Optional custom Blueprint actor class for post-boss exit teleporter (Ticket B-112, inherits ASovereignBossTeleporter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignBossTeleporter> BossTeleporterClass;

	/** Optional custom Blueprint actor class for procedural descent stairs (Ticket B-116, inherits ASovereignDungeonStairs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Blueprints")
	TSubclassOf<class ASovereignDungeonStairs> DescentStairsClass;

	/** Mode toggle: If true, stairs only spawn in Boss Chamber; if false, can also spawn in dedicated Stair Rooms (Ticket B-116) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Stairs")
	bool bToggleBossOnlyStairs = true;

	/** If true, spawns descent stairs in the Boss Chamber alongside/behind the boss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Stairs")
	bool bSpawnDescentStairs = true;

	/** Randomized probability of spawning a 4x4 Stair Room when floor is already conquered (Ticket B-116) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Stairs")
	float StairRoomSpawnChance = 0.5f;

	/** Chance to spawn an ASovereignRestTerminal inside the 4x4 Stair Room (Ticket B-116) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Stairs")
	float StairRoomRestTerminalChance = 0.5f;

	/** Optional mesh for descent stairs (wedge / ramp / modeled stairs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> DescentStairsMesh;

	/** Optional specialized mesh for sealed Boss Gate barrier actor (used when BossGateBlueprintClass is null) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> BossGateMesh;

	/** Optional specialized mesh for terminal Boss Chamber altar / centerpiece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> BossAltarMesh;

	/** Instanced Static Mesh Component for rendering all floor tiles in a single draw call */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<class UInstancedStaticMeshComponent> FloorInstancedComponent;

	/** Instanced Static Mesh Component for rendering all boundary wall tiles in a single draw call */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<class UInstancedStaticMeshComponent> WallInstancedComponent;

	/** Active layout nodes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TArray<FSovereignDungeonNode> DungeonNodes;

	/** Registry of all special rooms generated (Entrance, Treasure, Rest, Stair, Colosseum, Grand, Boss) (Ticket B-120) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TArray<FSovereignSpecialRoomInfo> SpecialRooms;

	/** Master spatial cell reservation map mapping each occupied 1000cm grid cell to its owning room index (Ticket B-094) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TMap<FIntPoint, int32> MasterGridOccupancy;

	/** Authoritative Single Source of Truth for dungeon connectivity (Ticket B-090) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TArray<FSovereignDungeonEdge> DungeonEdges;

	/** Ordered node indices along the primary critical spine */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TArray<int32> MainSpineNodeIndices;

	/** Index of the terminal Boss Chamber node */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	int32 BossNodeIndex = INDEX_NONE;

	// --- Explicit Lifecycle Ownership Tracking (Ticket B-092) ---

	/** Discrete actors spawned by the generator (altars, props, chests) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> GeneratorSpawnedActors;

	/** Fallback directional fill light spawned by the generator (nullptr if level has designer lighting) */
	UPROPERTY(Transient)
	TObjectPtr<class ADirectionalLight> GeneratorSpawnedLight;

	/** Fallback post process volume spawned by the generator (nullptr if level has designer volume) */
	UPROPERTY(Transient)
	TObjectPtr<class APostProcessVolume> GeneratorSpawnedPostProcess;

	// --- Dedicated Battle Arena Array Placement ---

	/** Master toggle: Whether to physically spawn 3D visual platform floors, 3-sided theater walls, and stage lighting for battle arenas (Ticket B-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	bool bSpawnPhysicalArenaGeometry = true;

	/** Configurable world location origin for dedicated Battle Arena stage clusters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	FVector FarOffArenaOrigin = FVector(20000.0f, 20000.0f, 3000.0f);

	/** Spacing distance between individual biome arenas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	float ArenaSpacing = 5000.0f;

	/** Optional custom static mesh for battle arena stage floor platform (fallback: FloorMesh) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	TObjectPtr<UStaticMesh> ArenaPlatformMesh;

	/** Optional custom static mesh for battle arena 3-sided theater perimeter walls (fallback: WallMesh) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	TObjectPtr<UStaticMesh> ArenaWallMesh;

	/** Spawned or configured battle managers across distinct biomes (Earth, Lava, Crystal, Forest) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Arena")
	TArray<TObjectPtr<ASovereignBattleManager>> BiomeBattleArenas;

	// --- Step-Based Random Encounters (Pokemon / FF7 style) ---

	/** Step accumulation distance in cm before evaluating random encounter roll (Default 1500cm ~ 15 steps) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Encounters")
	float StepThresholdDistance = 1500.0f;

	/** Base percentage chance of encounter when step threshold is reached (Default 25%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Encounters")
	float RandomEncounterChance = 25.0f;

	/** Accumulated movement distance since last encounter check */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Encounters")
	float AccumulatedStepDistance = 0.0f;

	/** Cached location of player leader for movement tracking */
	FVector LastLeaderLocation = FVector::ZeroVector;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Events")
	FOnDungeonGenerated OnDungeonGenerated;

	// --- Generator API ---

	/**
	 * Generates the complete procedural dungeon layout and spawns far-off battle arena platforms.
	 * Can be clicked directly in the Unreal Editor Details Panel via CallInEditor!
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Editor Actions")
	bool GenerateDungeon();

	/**
	 * Cleans up and clears all spawned dungeon geometry, floors, walls, and arenas.
	 * Can be clicked directly in the Unreal Editor Details Panel via CallInEditor!
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Editor Actions")
	void ClearDungeonGeometry() { CleanupDungeon(); }

	/** Teleports the active Level Editor viewport camera to look directly at the primary Battle Arena theater stage! (Ticket B-100) */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Editor Actions")
	void InspectBattleArena();

	/** Teleports the active Level Editor viewport camera to look directly at the Dungeon Entrance! */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Editor Actions")
	void InspectEntrance();

	/** Returns the list of all special rooms generated in this dungeon (Ticket B-120) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Layout")
	const TArray<FSovereignSpecialRoomInfo>& GetSpecialRooms() const { return SpecialRooms; }

	/**
	 * Spawns the dedicated battle arena platform array with physical 3D theater stages.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void SpawnFarOffBattleArenas();

	/** Returns ideal combat camera viewpoint transform facing the theater stage from the open front */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Arena")
	FTransform GetCombatCameraTransform(int32 BiomeIndex = 0) const;

	/**
	 * Physically instantiates 3D floors, boundary walls, and boss altar meshes (Ticket B-089).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Geometry")
	void SpawnDungeonGeometry();

	/** Returns default footprint dimensions for each room semantic type (Ticket B-094) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Footprints")
	static FIntPoint GetDefaultFootprintForTileType(EDungeonTileType Type)
	{
		switch (Type)
		{
		case EDungeonTileType::EntranceRoom:  return FIntPoint(1, 1);
		case EDungeonTileType::Corridor:      return FIntPoint(1, 1);
		case EDungeonTileType::Chamber:       return FIntPoint(1, 1);
		case EDungeonTileType::TreasureRoom:  return FIntPoint(1, 1);
		case EDungeonTileType::RestSanctuary: return FIntPoint(2, 2);
		case EDungeonTileType::GrandChamber:  return FIntPoint(4, 6);
		case EDungeonTileType::Colosseum:     return FIntPoint(8, 8);
		case EDungeonTileType::EliteChamber:  return FIntPoint(2, 2);
		case EDungeonTileType::BossChamber:   return FIntPoint(4, 4);
		case EDungeonTileType::StairRoom:     return FIntPoint(4, 4);
		default:                              return FIntPoint(1, 1);
		}
	}

	/** Returns default max connection capacity for each room type (Ticket B-094.2 & B-102) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Footprints")
	static int32 GetDefaultMaxConnectionsForTileType(EDungeonTileType Type)
	{
		switch (Type)
		{
		case EDungeonTileType::EntranceRoom:  return 2;
		case EDungeonTileType::Corridor:      return 2;
		case EDungeonTileType::Chamber:       return 4;
		case EDungeonTileType::TreasureRoom:  return 1; // Strict dead-end leaf
		case EDungeonTileType::RestSanctuary: return 2; // Safe retreat / midway
		case EDungeonTileType::GrandChamber:  return 4; // Multiple branch connections
		case EDungeonTileType::Colosseum:     return 4; // Major trial hub
		case EDungeonTileType::EliteChamber:  return 2; // Linear gatekeeper
		case EDungeonTileType::BossChamber:   return 1; // Single entrance strictly
		case EDungeonTileType::StairRoom:     return 2; // Descent hub
		default:                              return 2;
		}
	}

	/** Validates if a rectangular footprint can fit without overlapping already occupied cells (Ticket B-094) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Footprints")
	bool CanFitRoomFootprint(const FIntPoint& Anchor, const FIntPoint& Dims) const;

	/** Calculates the shortest topological graph distance (number of hops) between two nodes in DungeonEdges (Ticket B-096) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon|Validation")
	int32 CalculateGraphDistance(int32 NodeA, int32 NodeB) const;

	/** Generates controlled shortcut loops between candidate adjacent rooms with TreeDistance >= 3 (Ticket B-096) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Generation")
	int32 GenerateControlledLoops(FRandomStream& Stream);

	/** Validates structural reachability and gameplay pacing via pure-data BFS (Ticket B-090) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Validation")
	bool ValidateTopologicalReachability(int32 EntranceIdx, int32 BossIdx) const;

	/** Rebuilds fast-lookup ConnectedPassageMask on nodes strictly derived from DungeonEdges (Ticket B-090) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Validation")
	void RebuildNodeAdjacencyCaches();

	/** Idempotent cleanup of generator-owned actors, components, and arenas without touching level actors (Ticket B-092) */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Lifecycle")
	void CleanupDungeon();

	/**
	 * Automatically spawns default ambient directional light and post-process volume
	 * if the current level lacks them, preventing pitch black / blank world rendering.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Lighting")
	void SpawnDefaultLightingRig();

	/**
	 * Updates player movement distance and evaluates random step encounter rolls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void ProcessPlayerMovement(ASovereignBaseCharacter* PlayerLeader, float DeltaDistance);

	/** Returns world transform of the Entrance Room */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	FTransform GetEntranceTransform() const;

	/** Returns spawned entrance gateway actor if present (Ticket B-111) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	class ASovereignBaseInteractable* GetEntranceGatewayActor() const;

	/** Returns world transform of the terminal Boss Chamber */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	FTransform GetBossChamberTransform() const;

	/** Returns spawned boss teleporter actor if present (Ticket B-112) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	class ASovereignBossTeleporter* GetBossTeleporterActor() const;

	/** Selects an active battle manager from the biome arenas array */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	ASovereignBattleManager* GetBattleArenaForBiome(int32 BiomeIndex = 0) const;

	/**
	 * Returns party back to the saved Meditation Garden and restores world state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	bool ReturnToGarden();
};
