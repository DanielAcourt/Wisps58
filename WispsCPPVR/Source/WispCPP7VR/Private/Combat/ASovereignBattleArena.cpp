// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/ASovereignBattleArena.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
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
	constexpr float CenterOffset = 400.0f;

	for (int32 Slot = 0; Slot < Count; ++Slot)
	{
		const int32 Rank = Slot / 3;       // 0 = Front, 1 = Mid, 2 = Rear
		const int32 Col = (Slot % 3) - 1;  // -1 = Left, 0 = Center, +1 = Right

		// Player party sits on -X facing +X; enemies sit on +X facing -X
		const FVector PlayerLocalPos(-(CenterOffset + (Rank * SlotSpacing.X)), Col * SlotSpacing.Y, 0.0f);
		const FVector EnemyLocalPos(CenterOffset + (Rank * SlotSpacing.X), Col * SlotSpacing.Y, 0.0f);

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


void AASovereignBattleArena::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (!World || World->IsGameWorld())
	{
		return;
	}

	FlushPersistentDebugLines(World);

	if (!bShowEditorSlotPreview)
	{
		return;
	}

	const TArray<FTransform> PlayerSlots = GetPlayerSlotTransforms();
	const TArray<FTransform> EnemySlots = GetEnemySpawnTransforms();

	// Draw Player Slots (Cyan/Blue)
	for (int32 i = 0; i < PlayerSlots.Num(); ++i)
	{
		const FVector Loc = PlayerSlots[i].GetLocation();
		const FRotator Rot = PlayerSlots[i].GetRotation().Rotator();
		const FVector Fwd = PlayerSlots[i].GetRotation().GetForwardVector();

		DrawDebugCapsule(World, Loc + FVector(0.f, 0.f, 90.f), 90.f, 35.f, Rot.Quaternion(), FColor::Cyan, true, -1.f, 0, 2.f);
		DrawDebugDirectionalArrow(World, Loc + FVector(0.f, 0.f, 90.f), Loc + FVector(0.f, 0.f, 90.f) + Fwd * 120.f, 25.f, FColor::Cyan, true, -1.f, 0, 3.f);
		DrawDebugString(World, Loc + FVector(0.f, 0.f, 195.f), FString::Printf(TEXT("P%d"), i), nullptr, FColor::Cyan, -1.f, true);
	}

	// Draw Enemy Slots (Red/Orange)
	for (int32 i = 0; i < EnemySlots.Num(); ++i)
	{
		const FVector Loc = EnemySlots[i].GetLocation();
		const FRotator Rot = EnemySlots[i].GetRotation().Rotator();
		const FVector Fwd = EnemySlots[i].GetRotation().GetForwardVector();

		DrawDebugCapsule(World, Loc + FVector(0.f, 0.f, 90.f), 90.f, 35.f, Rot.Quaternion(), FColor::Red, true, -1.f, 0, 2.f);
		DrawDebugDirectionalArrow(World, Loc + FVector(0.f, 0.f, 90.f), Loc + FVector(0.f, 0.f, 90.f) + Fwd * 120.f, 25.f, FColor::Red, true, -1.f, 0, 3.f);
		DrawDebugString(World, Loc + FVector(0.f, 0.f, 195.f), FString::Printf(TEXT("E%d"), i), nullptr, FColor::Red, -1.f, true);
	}
#endif
}


void AASovereignBattleArena::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}