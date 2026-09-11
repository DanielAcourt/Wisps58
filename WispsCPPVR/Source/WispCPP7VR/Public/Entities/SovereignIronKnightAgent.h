// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#pragma once

#include "CoreMinimal.h"
#include "Entities/SovereignBaseEntity.h"
#include "SovereignIronKnightAgent.generated.h"

class UWidgetComponent;
class USovereignBridgeSubsystem;

/**
 * ASovereignIronKnightAgent
 *
 * 3D Spatial Representation of the Iron Knight AI Agent inside Unreal Engine levels (AD-005b).
 * Inherits from ASovereignBaseEntity and operates as an autonomous spatial entity.
 *
 * Key Behaviors:
 * - bCanBePossessed set to false so Player Wisps cannot override or inhabit the Iron Knight.
 * - Automatically triggers USovereignBridgeSubsystem::ExecuteAASHandshake on BeginPlay.
 * - Exposes AAS Diligence / Score and Possession Target status for Blueprints, UI, and World Manifest.
 */
UCLASS()
class WISPCPP7VR_API ASovereignIronKnightAgent : public ASovereignBaseEntity
{
	GENERATED_BODY()

public:
	ASovereignIronKnightAgent();

	/** --- AAS & AGENT STATUS --- */

	/** Active AAS Score / Diligence level for this agent instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|IronKnight")
	float AASScore = 0.5f;

	/** True if the single-use AAS authority boost (+0.5 VSS) was successfully armed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|IronKnight")
	bool bAASHandshakeActive = false;

	/** Is Iron Knight currently possessing or controlling a target vessel actor? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|IronKnight")
	bool bIsPossessingTarget = false;

	/** Name or EntityID of the vessel currently possessed by Iron Knight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|IronKnight")
	FString PossessedTargetName = TEXT("None");

	/** Pointer to the target actor currently possessed by Iron Knight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|IronKnight")
	TObjectPtr<AActor> PossessedTargetActor;

	/** Optional 3D Status Widget Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|UI")
	UWidgetComponent* StatusWidgetComponent;

	/** --- POSSESSION LOGIC --- */

	/** Initiates Iron Knight possession of a target vessel actor */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|IronKnight")
	bool PerformAgentPossession(AActor* TargetVessel);

	/** Releases Iron Knight possession of the current target vessel actor */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|IronKnight")
	void EjectAgentPossession();

	/** Refreshes AAS authority status and requests handshake boost from Bridge */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|IronKnight")
	void RefreshAASHandshake();

protected:
	virtual void BeginPlay() override;
};
