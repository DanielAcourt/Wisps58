// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#include "UI/SovereignInspectorWidget.h"
#include "Entities/SovereignSaveableEntityComponent.h"

USovereignInspectorWidget::USovereignInspectorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InspectedActor = nullptr;
	InspectedComponent = nullptr;
}

void USovereignInspectorWidget::SetInspectedActor(AActor* InActor)
{
	InspectedActor = InActor;
	TArray<UActorComponent*> Inspectables = GetInspectableComponents();
	InspectedComponent = Inspectables.Num() > 0 ? Inspectables[0] : nullptr;
	RefreshInspector();
}

void USovereignInspectorWidget::SetInspectedComponent(UActorComponent* InComponent)
{
	InspectedComponent = InComponent;
	if (InComponent)
	{
		InspectedActor = InComponent->GetOwner();
	}
	RefreshInspector();
}

TArray<UActorComponent*> USovereignInspectorWidget::GetInspectableComponents() const
{
	return USovereignSaveableEntityComponent::GetInspectableComponents(InspectedActor);
}

FString USovereignInspectorWidget::GetAggregatedInspectionJson() const
{
	return USovereignSaveableEntityComponent::GetAggregatedInspectionJson(InspectedActor);
}

void USovereignInspectorWidget::RefreshInspector()
{
	OnInspectorDataRefreshed();
}
