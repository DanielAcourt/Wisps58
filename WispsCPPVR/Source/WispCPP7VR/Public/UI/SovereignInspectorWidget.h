// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/SovereignUIInspectable.h"
#include "SovereignInspectorWidget.generated.h"

/**
 * USovereignInspectorWidget: Base UMG UserWidget for dynamic component and actor inspection.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Sovereign))
class WISPCPP7VR_API USovereignInspectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USovereignInspectorWidget(const FObjectInitializer& ObjectInitializer);

	/** Currently bound target actor for inspection */
	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|UI Inspector")
	AActor* InspectedActor;

	/** Currently active inspectable component */
	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|UI Inspector")
	UActorComponent* InspectedComponent;

	/** Binds a target actor for dynamic UI inspection */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI Inspector")
	void SetInspectedActor(AActor* InActor);

	/** Binds a specific target component for dynamic UI inspection */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI Inspector")
	void SetInspectedComponent(UActorComponent* InComponent);

	/** Retrieves all inspectable components on the currently bound actor */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI Inspector")
	TArray<UActorComponent*> GetInspectableComponents() const;

	/** Retrieves aggregated JSON string of all inspectable components on bound actor */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI Inspector")
	FString GetAggregatedInspectionJson() const;

	/** Triggers UI refresh and broadcasts event to Blueprint widgets */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI Inspector")
	void RefreshInspector();

	/** Event triggered when inspection target or property state is refreshed */
	UFUNCTION(BlueprintImplementableEvent, Category = "Sovereign|UI Inspector")
	void OnInspectorDataRefreshed();
};
