// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Templates/SubclassOf.h"
#include "SovereignUIInspectable.generated.h"

class UUserWidget;

// 1. The U-Class
UINTERFACE(MinimalAPI, Blueprintable)
class USovereignUIInspectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISovereignUIInspectable: Dynamic component/actor UI provider interface.
 */
class WISPCPP7VR_API ISovereignUIInspectable
{
	GENERATED_BODY()

public:
	/** Returns the user-friendly display name for the inspectable target */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspection")
	FText GetInspectorDisplayName() const;

	/** Returns the inspector category (e.g. "Bio", "Attribute", "Element", "Qi", "General") */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspection")
	FString GetInspectorCategory() const;

	/** Returns key-value property dictionary for dynamic UI inspection */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspection")
	TMap<FString, FString> GetInspectorProperties() const;

	/** Returns JSON serialized string representation of inspectable state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspection")
	FString GetInspectorDataJson() const;

	/** Returns optional custom widget class recommendation for custom UI binding */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|UI Inspection")
	TSubclassOf<UUserWidget> GetInspectorWidgetClass() const;
};
