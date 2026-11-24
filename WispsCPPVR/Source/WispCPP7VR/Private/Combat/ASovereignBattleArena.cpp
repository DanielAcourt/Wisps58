// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/ASovereignBattleArena.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AASovereignBattleArena::AASovereignBattleArena()
{
	PrimaryActorTick.bCanEverTick = false;

	ArenaRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ArenaRoot"));
	SetRootComponent(ArenaRoot);

	ArenaFloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArenaFloorMesh"));
	ArenaFloorMesh->SetupAttachment(ArenaRoot);

	ArenaCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ArenaCamera"));
	ArenaCamera->SetupAttachment(ArenaRoot);
	ArenaCamera->SetRelativeLocation(FVector(-900.0f, 0.0f, 650.0f));
	ArenaCamera->SetRelativeRotation(FRotator(-22.0f, 0.0f, 0.0f));

	RebuildDefaultSlots();
}

void AASovereignBattleArena::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerSlotTransforms.Num() == 0 || EnemySlotTransforms.Num() == 0)
	{
		RebuildDefaultSlots();
	}
}

void AASovereignBattleArena::RebuildDefaultSlots()
{
	PlayerSlotTransforms.Reset();
	EnemySlotTransforms.Reset();

	const int32 Count = FMath::Max(1, SlotCountPerSide);
	const int32 Rows = 2;
	const int32 Cols = FMath::CeilToInt(static_cast<float>(Count) / Rows);

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Row = i % Rows;
		const int32 Col = i / Rows;
		const float YOffset = (Col - (Cols - 1) * 0.5f) * SlotSpacing.Y;

		// Player party faces +X on the left/back side; enemies face -X on the right/front side
		const FVector PlayerLocalPos(-350.0f - Row * SlotSpacing.X, YOffset, 0.0f);
		const FVector EnemyLocalPos(350.0f + Row * SlotSpacing.X, YOffset, 0.0f);

		PlayerSlotTransforms.Add(FTransform(FRotator(0.0f, 0.0f, 0.0f), PlayerLocalPos, FVector::OneVector));
		EnemySlotTransforms.Add(FTransform(FRotator(0.0f, 180.0f, 0.0f), EnemyLocalPos, FVector::OneVector));
	}
}

TArray<FTransform> AASovereignBattleArena::GetEnemySpawnTransforms() const
{
	TArray<FTransform> WorldTransforms;
	const FTransform ArenaXform = GetActorTransform();

	for (const FTransform& LocalSlot : EnemySlotTransforms)
	{
		WorldTransforms.Add(LocalSlot * ArenaXform);
	}

	return WorldTransforms;
}

TArray<FTransform> AASovereignBattleArena::GetPlayerSlotTransforms() const
{
	TArray<FTransform> WorldTransforms;
	const FTransform ArenaXform = GetActorTransform();

	for (const FTransform& LocalSlot : PlayerSlotTransforms)
	{
		WorldTransforms.Add(LocalSlot * ArenaXform);
	}

	return WorldTransforms;
}

FTransform AASovereignBattleArena::GetBattleCameraWorldTransform() const
{
	if (ArenaCamera)
	{
		return ArenaCamera->GetComponentTransform();
	}

	return GetActorTransform();
}

void AASovereignBattleArena::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}