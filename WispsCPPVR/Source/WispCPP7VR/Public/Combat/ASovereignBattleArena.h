// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASovereignBattleArena.generated.h"

class USceneComponent;
class UCameraComponent;
class UStaticMeshComponent;

UCLASS()
class WISPCPP7VR_API AASovereignBattleArena : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASovereignBattleArena();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Components")
	TObjectPtr<USceneComponent> ArenaRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Components")
	TObjectPtr<UStaticMeshComponent> ArenaFloorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Components")
	TObjectPtr<UCameraComponent> ArenaCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Placement")
	float DefaultZDepth = -20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Slots")
	int32 SlotCountPerSide = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Slots")
	FVector2D SlotSpacing = FVector2D(260.0f, 220.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Slots")
	TArray<FTransform> PlayerSlotTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Slots")
	TArray<FTransform> EnemySlotTransforms;

	UFUNCTION(BlueprintCallable, Category = "Arena|Slots")
	void RebuildDefaultSlots();

	UFUNCTION(BlueprintCallable, Category = "Arena|Slots")
	TArray<FTransform> GetEnemySpawnTransforms() const;

	UFUNCTION(BlueprintCallable, Category = "Arena|Slots")
	TArray<FTransform> GetPlayerSlotTransforms() const;

	UFUNCTION(BlueprintCallable, Category = "Arena|Camera")
	FTransform GetBattleCameraWorldTransform() const;

	/** Draw editor-only 3x3 slot capsules, facing arrows, and labels in the Blueprint viewport & level editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Arena|Preview")
	bool bShowEditorSlotPreview = true;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
