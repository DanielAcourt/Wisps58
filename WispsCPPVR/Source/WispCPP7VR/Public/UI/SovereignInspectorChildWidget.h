// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"
#include "SovereignInspectorChildWidget.generated.h"

/**
 * USovereignInspectorChildWidget: Polymorphic base UMG UserWidget for specialized component inspector child panels.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Sovereign))
class WISPCPP7VR_API USovereignInspectorChildWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USovereignInspectorChildWidget(const FObjectInitializer& ObjectInitializer);

	/** Currently bound component being inspected by this child widget */
	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|UI Inspector")
	UActorComponent* BoundComponent;

	/** Binds target component and triggers child UI update */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspector")
	void UpdateInspectorData(UActorComponent* TargetComponent);
	virtual void UpdateInspectorData_Implementation(UActorComponent* TargetComponent);
};
