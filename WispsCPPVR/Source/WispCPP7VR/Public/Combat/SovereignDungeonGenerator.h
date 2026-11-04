// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/SovereignBattleManager.h"
#include "SovereignDungeonGenerator.generated.h"

class ASovereignBaseCharacter;
class UStaticMesh;

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
	BossChamber     UMETA(DisplayName = "Boss Chamber")
};

/**
 * FSovereignDungeonNode
 * Represents an individual 1000cm x 1000cm tile node in the procedural layout.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignDungeonNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Corridor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	FTransform WorldTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon")
	bool bHasFixedEliteMob = false;
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

	/** Active layout nodes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	TArray<FSovereignDungeonNode> DungeonNodes;

	/** Index of the terminal Boss Chamber node */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Layout")
	int32 BossNodeIndex = INDEX_NONE;

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	bool GenerateDungeon();

	/**
	 * Spawns the dedicated battle arena platform array far off-map.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon")
	void SpawnFarOffBattleArenas();

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
