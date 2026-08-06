// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h" // <--- CRITICAL: Fixes C2061
#include "WispsGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class WISPCPP7VR_API AWispsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWispsGameModeBase();

protected:
	/** Global reference to your Species/Item Data Table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sovereign|Data")
	class UDataTable* ItemDataTable;

	/** Helper to find species data during spawning */
	class USovereignSpeciesData* GetSpeciesDataByTag(FGameplayTag SpeciesTag);
};
