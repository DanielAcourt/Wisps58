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
	BossChamber     UMETA(DisplayName = "Boss Chamber")
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
 * FSovereignDungeonNode
 * Represents an individual uniform 1000cm x 1000cm tile node in the procedural layout.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	int32 NodeIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Corridor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FTransform WorldTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	bool bHasFixedEliteMob = false;

	/** Transient bitmask cache derived strictly from DungeonEdges (Bit 0=N, 1=E, 2=S, 3=W) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon")
	uint8 ConnectedPassageMask = 0;
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

	/** Minimum required path distance from Entrance to Boss Chamber */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 MinSpineLength = 4;

	/** Max controlled loop count (default 0 enforces spanning-tree invariant |E| = |V| - 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Config")
	int32 MaxLoopCount = 0;

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

	/** Static Mesh for standard 1000cm x 1000cm floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> FloorMesh;

	/** Static Mesh for perimeter walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Geometry")
	TObjectPtr<UStaticMesh> WallMesh;

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

	/** Coordinates for the far-off dedicated Battle Arena cluster */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Arena")
	FVector FarOffArenaOrigin = FVector(50000.0f, 50000.0f, 5000.0f);

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

	/**
	 * Spawns the dedicated battle arena platform array far off-map.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void SpawnFarOffBattleArenas();

	/**
	 * Physically instantiates 3D floors, boundary walls, and boss altar meshes (Ticket B-089).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Geometry")
	void SpawnDungeonGeometry();

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

	/** Returns world transform of the terminal Boss Chamber */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	FTransform GetBossChamberTransform() const;

	/** Selects an active battle manager from the biome arenas array */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Dungeon")
	ASovereignBattleManager* GetBattleArenaForBiome(int32 BiomeIndex = 0) const;

	/**
	 * Returns party back to the saved Meditation Garden and restores world state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	bool ReturnToGarden();
};
