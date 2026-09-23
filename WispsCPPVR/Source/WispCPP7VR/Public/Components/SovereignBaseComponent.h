// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/SovereignSaveInterface.h"
#include "Interaction/SovereignUIInspectable.h"
#include "SovereignBaseComponent.generated.h"

UCLASS( ClassGroup=(Sovereign), meta=(BlueprintSpawnableComponent) )
class WISPCPP7VR_API USovereignBaseComponent : public UActorComponent, public ISovereignSaveInterface, public ISovereignUIInspectable
{
	GENERATED_BODY()

public:	
	USovereignBaseComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// ISovereignSaveInterface Implementation (LEGACY)
	virtual TMap<FString, FString> GetSaveData() override { return TMap<FString, FString>(); }
	virtual void RestoreSaveData(const TMap<FString, FString>& Data) override { /* Base: Do nothing */ }

	// ISovereignUIInspectable Interface Implementation
	virtual FText GetInspectorDisplayName_Implementation() const override;
	virtual FString GetInspectorCategory_Implementation() const override;
	virtual TMap<FString, FString> GetInspectorProperties_Implementation() const override;
	virtual FString GetInspectorDataJson_Implementation() const override;
	virtual TSubclassOf<UUserWidget> GetInspectorWidgetClass_Implementation() const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
