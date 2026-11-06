// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SovereignWidgetSanitizer.generated.h"

class UUserWidget;

/**
 * USovereignWidgetSanitizer
 * Utility library and C++ safety guard for UMG Widget Property validation (Ticket B-114).
 * Detects case-insensitive duplicate property names in UWidgetBlueprintGeneratedClasses
 * before instantiation to prevent fatal engine ensure failures (e.g. duplicate 'WBP_Chatwindow').
 */
UCLASS()
class WISPCPP7VR_API USovereignWidgetSanitizer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Audits a Widget Blueprint Generated Class for duplicate property names (case-insensitive).
	 * Returns true if the class properties are clean and safe for instantiation, false if duplicates exist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI|Sanitizer")
	static bool ValidateWidgetClassProperties(UClass* InWidgetClass, TArray<FString>& OutDuplicateNames);

	/**
	 * Safe helper to instantiate a UserWidget with pre-flight property collision checks.
	 * If property collisions are detected, logs a descriptive warning and safely handles the error.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|UI|Sanitizer", meta = (WorldContext = "WorldContextObject"))
	static UUserWidget* SafeCreateWidget(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, FName WidgetName = NAME_None);
};
