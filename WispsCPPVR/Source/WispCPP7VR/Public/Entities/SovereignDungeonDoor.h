// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "GameplayTagContainer.h"
#include "SovereignDungeonDoor.generated.h"

class UStaticMeshComponent;
class ASovereignBaseCharacter;

UENUM(BlueprintType)
enum class EBossGateUnlockCondition : uint8
{
	None                  UMETA(DisplayName = "None (Unlocked)"),
	DefeatEliteGuardian   UMETA(DisplayName = "Defeat Elite Guardian"),
	RequiresBossKey       UMETA(DisplayName = "Requires Boss Key"),
	RequiresQiEmpowerment UMETA(DisplayName = "Requires Qi Empowerment")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonDoorStateChanged, bool, bIsOpen);

/**
 * ASovereignDungeonDoor
 * Interactive Blueprint-assignable swing door and sealed Boss Gate actor (Ticket B-105).
 * Features a static door frame, swinging door leaf mesh, key/guardian unlock conditions,
 * and smooth hinge rotation.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignDungeonDoor : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignDungeonDoor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Visual Components ---

	/** Static outer door frame snapped into the 400x500cm doorway opening */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Door")
	TObjectPtr<UStaticMeshComponent> DoorFrameMeshComponent;

	/** Movable swinging door leaf mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Door")
	TObjectPtr<UStaticMeshComponent> DoorLeafMeshComponent;

	// --- State & Settings ---

	/** Whether this door is currently open */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	bool bIsOpen = false;

	/** Whether this door is locked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	bool bIsLocked = false;

	/** Unlock condition governing this door / gate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	EBossGateUnlockCondition UnlockCondition = EBossGateUnlockCondition::None;

	/** GameplayTag required in interactor's inventory if UnlockCondition == RequiresBossKey */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door", meta = (EditCondition = "UnlockCondition == EBossGateUnlockCondition::RequiresBossKey"))
	FGameplayTag RequiredKeyTag;

	/** If true, consuming the key upon unlocking removes 1 unit of the key item from the interactor's inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door", meta = (EditCondition = "UnlockCondition == EBossGateUnlockCondition::RequiresBossKey"))
	bool bConsumeKeyOnUnlock = true;

	/** Required Qi amount if UnlockCondition == RequiresQiEmpowerment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door", meta = (EditCondition = "UnlockCondition == EBossGateUnlockCondition::RequiresQiEmpowerment"))
	float RequiredQiToUnlock = 50.0f;

	/** Current channeled Qi towards unlocking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Dungeon|Door")
	float ChanneledQi = 0.0f;

	/** Rotation angle in degrees when fully opened (default 90 deg Yaw) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	float OpenAngle = 90.0f;

	/** Swing animation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	float SwingSpeed = 4.0f;

	/** If true, the door stays open once opened and cannot be closed again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Dungeon|Door")
	bool bStayOpenOnceOpened = true;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Dungeon|Door")
	FOnDungeonDoorStateChanged OnDoorStateChanged;

	// --- Interaction Overrides ---

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;

	// --- Door Actions ---

	/** Opens or closes the door smoothly */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sovereign|Dungeon|Door")
	void ToggleDoor();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Door")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Door")
	void UnlockDoor();

	/** Channels Qi from cultivator character towards unlocking */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Dungeon|Door")
	bool ChannelQiToUnlock(ASovereignBaseCharacter* Cultivator, float Amount);

protected:
	FRotator ClosedRotation = FRotator::ZeroRotator;
	FRotator TargetRotation = FRotator::ZeroRotator;
};
