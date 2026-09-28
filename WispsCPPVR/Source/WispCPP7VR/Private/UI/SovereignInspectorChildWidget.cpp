// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#include "UI/SovereignInspectorChildWidget.h"

USovereignInspectorChildWidget::USovereignInspectorChildWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoundComponent = nullptr;
}

void USovereignInspectorChildWidget::UpdateInspectorData_Implementation(UActorComponent* TargetComponent)
{
	BoundComponent = TargetComponent;
}
