// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#include "Components/SovereignElementComponent.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Dom/JsonObject.h"

USovereignElementComponent::USovereignElementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	FireAffinity = 0.0f;
	EarthAffinity = 0.0f;
	WindAffinity = 0.0f;
	WaterAffinity = 0.0f;
}

void USovereignElementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->RegisterBroker(this);
		}
	}
}

void USovereignElementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->UnregisterBroker(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void USovereignElementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float USovereignElementComponent::GetElementalMultiplier(ESovereignElement IncomingType)
{
	float Multiplier = 1.0f;

	if (BodySocket == ESovereignElement::Nature) {
		if (IncomingType == ESovereignElement::Water) Multiplier = 2.0f;
		if (IncomingType == ESovereignElement::Fire)  Multiplier = 0.5f;
	}
	else if (BodySocket == ESovereignElement::Earth) {
		if (IncomingType == ESovereignElement::Nature) Multiplier = 0.5f;
		if (IncomingType == ESovereignElement::Water)  Multiplier = 2.0f;
	}
	else if (BodySocket == ESovereignElement::Water) {
		if (IncomingType == ESovereignElement::Fire)   Multiplier = 2.0f;
		if (IncomingType == ESovereignElement::Nature) Multiplier = 0.5f;
	}
	else if (BodySocket == ESovereignElement::Fire) {
		if (IncomingType == ESovereignElement::Nature) Multiplier = 2.0f;
		if (IncomingType == ESovereignElement::Water)  Multiplier = 0.5f;
	}
	else if (BodySocket == ESovereignElement::Air) {
		if (IncomingType == ESovereignElement::Fire)   Multiplier = 2.0f;
		if (IncomingType == ESovereignElement::Earth)  Multiplier = 0.5f;
	}

	return Multiplier;
}

void USovereignElementComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	TSharedPtr<FJsonObject> ElemObj = MakeShareable(new FJsonObject());

	ElemObj->SetStringField(TEXT("BodySocket"), UEnum::GetValueAsString(BodySocket));
	ElemObj->SetNumberField(TEXT("BodyInfluence"), BodyInfluence);

	ElemObj->SetNumberField(TEXT("FireAffinity"), FireAffinity);
	ElemObj->SetNumberField(TEXT("EarthAffinity"), EarthAffinity);
	ElemObj->SetNumberField(TEXT("WindAffinity"), WindAffinity);
	ElemObj->SetNumberField(TEXT("WaterAffinity"), WaterAffinity);

	OutJson->SetObjectField(TEXT("Elements"), ElemObj);
}

void USovereignElementComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	const TSharedPtr<FJsonObject>* ElemObj;
	if (InJson->TryGetObjectField(TEXT("Elements"), ElemObj))
	{
		FString SocketStr;
		if ((*ElemObj)->TryGetStringField(TEXT("BodySocket"), SocketStr))
		{
			UEnum* EnumPtr = StaticEnum<ESovereignElement>();
			if (EnumPtr)
			{
				int64 Val = EnumPtr->GetValueByNameString(SocketStr);
				if (Val != INDEX_NONE) BodySocket = static_cast<ESovereignElement>(Val);
			}
		}

		double TempVal;
		if ((*ElemObj)->TryGetNumberField(TEXT("BodyInfluence"), TempVal)) BodyInfluence = (float)TempVal;
		if ((*ElemObj)->TryGetNumberField(TEXT("FireAffinity"), TempVal)) FireAffinity = (float)TempVal;
		if ((*ElemObj)->TryGetNumberField(TEXT("EarthAffinity"), TempVal)) EarthAffinity = (float)TempVal;
		if ((*ElemObj)->TryGetNumberField(TEXT("WindAffinity"), TempVal)) WindAffinity = (float)TempVal;
		if ((*ElemObj)->TryGetNumberField(TEXT("WaterAffinity"), TempVal)) WaterAffinity = (float)TempVal;
	}
}

void USovereignElementComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// Elemental processing
}
