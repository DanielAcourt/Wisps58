// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/SovereignSaveInterface.h"
#include "SovereignBaseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WISPCPP7VR_API USovereignBaseComponent : public UActorComponent, public ISovereignSaveInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USovereignBaseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// ISovereignSaveInterface Implementation (LEGACY)
	virtual TMap<FString, FString> GetSaveData() override { return TMap<FString, FString>(); }
	virtual void RestoreSaveData(const TMap<FString, FString>& Data) override { /* Base: Do nothing */ }
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
