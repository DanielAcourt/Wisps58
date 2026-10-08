// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Components/SovereignTelemetryComponent.h"

USovereignTelemetryComponent::USovereignTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TemperatureCelsius = 0.0f;
	PhValue = 7.0f;
	WaterDepthMM = 0.0f;
}

void USovereignTelemetryComponent::SetTemperatureCelsius(float InTemperature)
{
	TemperatureCelsius = InTemperature;
}

void USovereignTelemetryComponent::SetPhValue(float InPhValue)
{
	PhValue = InPhValue;
}

void USovereignTelemetryComponent::SetWaterDepthMM(float InWaterDepth)
{
	WaterDepthMM = InWaterDepth;
}

void USovereignTelemetryComponent::UpdateTelemetry(float InTemperature, float InPhValue, float InWaterDepth)
{
	TemperatureCelsius = InTemperature;
	PhValue = InPhValue;
	WaterDepthMM = InWaterDepth;
}

TMap<FString, FString> USovereignTelemetryComponent::GetSaveData()
{
	TMap<FString, FString> Data;
	Data.Add(TEXT("Telemetry.temp_c"), FString::SanitizeFloat(TemperatureCelsius));
	Data.Add(TEXT("Telemetry.ph_val"), FString::SanitizeFloat(PhValue));
	Data.Add(TEXT("Telemetry.water_depth_mm"), FString::SanitizeFloat(WaterDepthMM));
	return Data;
}

void USovereignTelemetryComponent::RestoreSaveData(const TMap<FString, FString>& Data)
{
	if (const FString* Val = Data.Find(TEXT("Telemetry.temp_c")))
	{
		TemperatureCelsius = FCString::Atof(**Val);
	}

	if (const FString* Val = Data.Find(TEXT("Telemetry.ph_val")))
	{
		PhValue = FCString::Atof(**Val);
	}

	if (const FString* Val = Data.Find(TEXT("Telemetry.water_depth_mm")))
	{
		WaterDepthMM = FCString::Atof(**Val);
	}
}

FText USovereignTelemetryComponent::GetInspectorDisplayName_Implementation() const
{
	return FText::FromString(TEXT("IoT Telemetry Component"));
}

FString USovereignTelemetryComponent::GetInspectorCategory_Implementation() const
{
	return TEXT("Telemetry");
}
