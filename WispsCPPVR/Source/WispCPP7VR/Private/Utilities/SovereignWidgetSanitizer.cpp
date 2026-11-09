// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-11-06

#include "Utilities/SovereignWidgetSanitizer.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

bool USovereignWidgetSanitizer::ValidateWidgetClassProperties(UClass* InWidgetClass, TArray<FString>& OutDuplicateNames)
{
	OutDuplicateNames.Empty();
	if (!InWidgetClass)
	{
		return false;
	}

	TSet<FName> SeenNames;
	bool bHasDuplicates = false;

	for (TFieldIterator<FProperty> PropIt(InWidgetClass, EFieldIterationFlags::Default); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		if (!Prop) continue;

		FName PropName = Prop->GetFName();
		if (SeenNames.Contains(PropName))
		{
			OutDuplicateNames.Add(PropName.ToString());
			bHasDuplicates = true;
		}
		else
		{
			SeenNames.Add(PropName);
		}
	}

	return !bHasDuplicates;
}

UUserWidget* USovereignWidgetSanitizer::SafeCreateWidget(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, FName WidgetName)
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("USovereignWidgetSanitizer::SafeCreateWidget: WidgetClass is null."));
		return nullptr;
	}

	TArray<FString> Duplicates;
	if (!ValidateWidgetClassProperties(WidgetClass, Duplicates))
	{
		FString DuplicateList = FString::Join(Duplicates, TEXT(", "));
		UE_LOG(LogTemp, Error, TEXT("USovereignWidgetSanitizer::SafeCreateWidget: Detected duplicate property names in widget class %s: [%s]. Aborting creation to prevent engine crash."),
			*WidgetClass->GetName(), *DuplicateList);
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	return CreateWidget<UUserWidget>(World, WidgetClass, WidgetName);
}
