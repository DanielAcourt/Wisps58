// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseInteractable.h"
#include "Combat/SovereignEncounterDefinition.h"
#include "Components/SphereComponent.h"
#include "SovereignRoamingMob.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoamingMobEncounterTriggered, ASovereignRoamingMob*, Mob);

/**
 * ASovereignRoamingMob
 * Visible roaming or stationary dungeon mob patrolling corridors or guarding rooms.
 * Features an encounter definition, detection/overlap sphere, and triggers combat
 * through USovereignEncounterSubsystem when touched by the player party leader.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignRoamingMob : public ASovereignBaseInteractable
{
	GENERATED_BODY()

public:
	ASovereignRoamingMob();

	/** Proximity trigger sphere detecting player exploration collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Mob")
	TObjectPtr<USphereComponent> TriggerSphere;

	/** Encounter definition associated with this roaming mob */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Mob")
	TObjectPtr<USovereignEncounterDefinition> EncounterDefinition;

	/** Fallback enemy units if encounter definition is not assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Mob")
	TArray<TSubclassOf<ASovereignBaseCharacter>> FallbackEnemyClasses;

	/** Event broadcast when this mob triggers an encounter */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Mob|Events")
	FOnRoamingMobEncounterTriggered OnEncounterTriggered;

	// --- ASovereignBaseInteractable Overrides ---
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionHint_Implementation() override;

	/** Initiates combat with the given interactor */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Mob")
	bool InitiateBattle(AActor* Interactor);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
