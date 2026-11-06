// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-05

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "SovereignBossTeleporter.generated.h"

class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossTeleporterActivated);

/**
 * ASovereignBossTeleporter (Ticket B-112)
 * Extraction teleporter placed in the procedural 4x4 Boss Chamber behind the boss.
 * Starts in a dormant state with disabled illumination; activates and glows upon boss defeat,
 * allowing players to interact and return to the Meditation Garden.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignBossTeleporter : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignBossTeleporter();

	/** Point light component providing glowing visual feedback when active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Components")
	TObjectPtr<UPointLightComponent> PortalLightComponent;

	/** Whether this teleporter has been activated following boss defeat */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Teleporter")
	bool bIsActive = false;

	/** User-facing display title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Teleporter")
	FText DisplayName;

	/** Hint text displayed when teleporter is still dormant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Teleporter")
	FString DormantHint = TEXT("Dormant Portal - Defeat the Vault Boss to activate");

	/** Hint text displayed when teleporter is powered and ready */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Teleporter")
	FString ActiveHint = TEXT("Teleport to Meditation Garden");

	/** Multicast delegate broadcast when teleporter activates */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Teleporter")
	FOnBossTeleporterActivated OnBossTeleporterActivated;

	// --- ASovereignBaseInteractable Interface ---

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;
	virtual FText GetInteractableName_Implementation() override;

	// --- Teleporter Activation Workflow ---

	/** Powers on the teleporter, enabling light and interaction */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Teleporter")
	void ActivateTeleporter();

	/** Trigger hook called by game mode, encounter subsystem, or battle manager */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Teleporter")
	void NotifyBossDefeated();

	/** Executes return to garden flow via USovereignDungeonSubsystem */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Teleporter")
	bool ReturnToGarden();

protected:
	virtual void BeginPlay() override;
};
