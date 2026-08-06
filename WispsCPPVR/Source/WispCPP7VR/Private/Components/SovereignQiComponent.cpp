// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#include "Components/SovereignQiComponent.h"
#include "Entities/SovereignBaseEntity.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Dom/JsonObject.h"

USovereignQiComponent::USovereignQiComponent()
	: AlignmentInfluence(100.0f)
	, MagicInfluence(0.0f)
	, CurrentQi(0.0f)
	, MaxQiCapacity(100.0f)
	, QiPurity(1.0f)
{
	AlignmentSocket = ESovereignElement::Grey;
	MagicSocket = ESovereignElement::None;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void USovereignQiComponent::BeginPlay()
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

void USovereignQiComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void USovereignQiComponent::AbsorbQi(float Amount, float SourcePurity)
{
	if (Amount <= 0.0f) return;
	float NewTotalQi = CurrentQi + Amount;
	if (NewTotalQi > 0.0f) QiPurity = ((CurrentQi * QiPurity) + (Amount * SourcePurity)) / NewTotalQi;
	CurrentQi = FMath::Min(NewTotalQi, MaxQiCapacity);
	TotalQiAccumulated += Amount;

	if (TotalQiAccumulated >= 1000.0f)
	{
		if (ASovereignBaseEntity* Owner = Cast<ASovereignBaseEntity>(GetOwner())) Owner->Evolve();
	}
}

bool USovereignQiComponent::SpendQi(float Amount)
{
	float AdjustedCost = Amount * (2.0f - QiPurity);
	if (CurrentQi >= AdjustedCost)
	{
		CurrentQi -= AdjustedCost;
		return true;
	}
	return false;
}

void USovereignQiComponent::ProcessQiFlow(float DeltaTime, int32 WisdomStat)
{
	if (QiPurity < 1.0f) QiPurity += 0.00001f * DeltaTime;
	float RegenRate = (static_cast<float>(WisdomStat) * 0.1f) * QiPurity;
	if (CurrentQi < MaxQiCapacity) CurrentQi = FMath::Min(CurrentQi + (RegenRate * DeltaTime), MaxQiCapacity);
}

void USovereignQiComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	TSharedPtr<FJsonObject> QiObj = MakeShareable(new FJsonObject());

	QiObj->SetStringField(TEXT("AlignmentSocket"), UEnum::GetValueAsString(AlignmentSocket));
	QiObj->SetNumberField(TEXT("AlignmentInfluence"), AlignmentInfluence);
	QiObj->SetStringField(TEXT("MagicSocket"), UEnum::GetValueAsString(MagicSocket));
	QiObj->SetNumberField(TEXT("MagicInfluence"), MagicInfluence);

	QiObj->SetNumberField(TEXT("CurrentQi"), CurrentQi);
	QiObj->SetNumberField(TEXT("MaxQiCapacity"), MaxQiCapacity);
	QiObj->SetNumberField(TEXT("QiPurity"), QiPurity);
	QiObj->SetNumberField(TEXT("TotalQiAccumulated"), TotalQiAccumulated);

	// Universal Domain Rune and Territory parameters [B-036]
	QiObj->SetBoolField(TEXT("bIsDomainOwner"), bIsDomainOwner);
	QiObj->SetStringField(TEXT("DomainRuneTier"), DomainRuneTier);
	QiObj->SetNumberField(TEXT("DomainResonanceRadius"), DomainResonanceRadius);
	QiObj->SetNumberField(TEXT("DomainNetworkSaturation"), DomainNetworkSaturation);

	TSharedPtr<FJsonObject> ResObj = MakeShareable(new FJsonObject());
	for (const auto& Elem : ElementalResonance)
	{
		ResObj->SetNumberField(Elem.Key.ToString(), Elem.Value);
	}
	QiObj->SetObjectField(TEXT("Resonance"), ResObj);

	OutJson->SetObjectField(TEXT("Qi"), QiObj);
}

void USovereignQiComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	const TSharedPtr<FJsonObject>* QiObj;
	if (InJson->TryGetObjectField(TEXT("Qi"), QiObj))
	{
		FString AlignStr;
		if ((*QiObj)->TryGetStringField(TEXT("AlignmentSocket"), AlignStr))
		{
			UEnum* EnumPtr = StaticEnum<ESovereignElement>();
			if (EnumPtr)
			{
				int64 Val = EnumPtr->GetValueByNameString(AlignStr);
				if (Val != INDEX_NONE) AlignmentSocket = static_cast<ESovereignElement>(Val);
			}
		}

		double TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("AlignmentInfluence"), TempVal)) AlignmentInfluence = (float)TempVal;

		FString MagicStr;
		if ((*QiObj)->TryGetStringField(TEXT("MagicSocket"), MagicStr))
		{
			UEnum* EnumPtr = StaticEnum<ESovereignElement>();
			if (EnumPtr)
			{
				int64 Val = EnumPtr->GetValueByNameString(MagicStr);
				if (Val != INDEX_NONE) MagicSocket = static_cast<ESovereignElement>(Val);
			}
		}
		if ((*QiObj)->TryGetNumberField(TEXT("MagicInfluence"), TempVal)) MagicInfluence = (float)TempVal;

		if ((*QiObj)->TryGetNumberField(TEXT("CurrentQi"), TempVal)) CurrentQi = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("MaxQiCapacity"), TempVal)) MaxQiCapacity = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("QiPurity"), TempVal)) QiPurity = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("TotalQiAccumulated"), TempVal)) TotalQiAccumulated = (float)TempVal;

		// Universal Domain Rune and Territory parameters [B-036]
		(*QiObj)->TryGetBoolField(TEXT("bIsDomainOwner"), bIsDomainOwner);
		(*QiObj)->TryGetStringField(TEXT("DomainRuneTier"), DomainRuneTier);
		if ((*QiObj)->TryGetNumberField(TEXT("DomainResonanceRadius"), TempVal)) DomainResonanceRadius = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("DomainNetworkSaturation"), TempVal)) DomainNetworkSaturation = (float)TempVal;

		const TSharedPtr<FJsonObject>* ResObj;
		if ((*QiObj)->TryGetObjectField(TEXT("Resonance"), ResObj))
		{
			ElementalResonance.Empty();
			for (auto& Elem : (*ResObj)->Values)
			{
				ElementalResonance.Add(FName(*Elem.Key), (float)Elem.Value->AsNumber());
			}
		}
	}
}

void USovereignQiComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// Process Qi data
}
