// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-15

#include "Components/SovereignLevelComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Dom/JsonObject.h"

USovereignLevelComponent::USovereignLevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ActiveTrainingFocus = ESovereignTrainingFocus::None;
	TrainingRateMultiplier = 1.0f;

	StrengthXP = 0.0;
	DexterityXP = 0.0;
	ConstitutionXP = 0.0;
	IntelligenceXP = 0.0;
	WisdomXP = 0.0;
	CharismaXP = 0.0;
	LuckXP = 0.0;
}

void USovereignLevelComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->RegisterBroker(this);
		}
	}

	DetermineDefaultTrainingFocus();
}

void USovereignLevelComponent::SetTrainingFocus(ESovereignTrainingFocus NewFocus)
{
	ActiveTrainingFocus = NewFocus;
}

void USovereignLevelComponent::DetermineDefaultTrainingFocus()
{
	if (ActiveTrainingFocus != ESovereignTrainingFocus::None)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	USovereignAttributeComponent* AttrComp = OwnerActor->FindComponentByClass<USovereignAttributeComponent>();
	if (!AttrComp)
	{
		return;
	}

	struct FStatFocusPair
	{
		ESovereignTrainingFocus Focus;
		int32 Value;
	};

	TArray<FStatFocusPair> StatPairs = {
		{ ESovereignTrainingFocus::Strength, AttrComp->Strength },
		{ ESovereignTrainingFocus::Dexterity, AttrComp->Dexterity },
		{ ESovereignTrainingFocus::Constitution, AttrComp->Constitution },
		{ ESovereignTrainingFocus::Intelligence, AttrComp->Intelligence },
		{ ESovereignTrainingFocus::Wisdom, AttrComp->Wisdom },
		{ ESovereignTrainingFocus::Charisma, AttrComp->Charisma },
		{ ESovereignTrainingFocus::Luck, AttrComp->Luck }
	};

	int32 MaxStatVal = -9999;
	for (const FStatFocusPair& Pair : StatPairs)
	{
		if (Pair.Value > MaxStatVal)
		{
			MaxStatVal = Pair.Value;
		}
	}

	TArray<ESovereignTrainingFocus> HighestCandidates;
	for (const FStatFocusPair& Pair : StatPairs)
	{
		if (Pair.Value == MaxStatVal)
		{
			HighestCandidates.Add(Pair.Focus);
		}
	}

	if (HighestCandidates.Num() > 0)
	{
		int32 RandomIdx = FMath::RandRange(0, HighestCandidates.Num() - 1);
		ActiveTrainingFocus = HighestCandidates[RandomIdx];
		UE_LOG(LogTemp, Log, TEXT("Sovereign: %s determined default training focus %d (Highest Base Stat Value: %d, Candidates: %d)"),
			*OwnerActor->GetName(), static_cast<uint8>(ActiveTrainingFocus), MaxStatVal, HighestCandidates.Num());
	}
}

void USovereignLevelComponent::AddAttributeXP(ESovereignTrainingFocus AttributeFocus, double Amount)
{
	if (Amount <= 0.0)
	{
		return;
	}

	double AdjustedAmount = Amount * TrainingRateMultiplier;
	USovereignAttributeComponent* AttrComp = GetOwner() ? GetOwner()->FindComponentByClass<USovereignAttributeComponent>() : nullptr;

	switch (AttributeFocus)
	{
	case ESovereignTrainingFocus::Strength:
		StrengthXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(StrengthXP, AttrComp->Strength);
		break;
	case ESovereignTrainingFocus::Dexterity:
		DexterityXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(DexterityXP, AttrComp->Dexterity);
		break;
	case ESovereignTrainingFocus::Constitution:
		ConstitutionXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(ConstitutionXP, AttrComp->Constitution);
		break;
	case ESovereignTrainingFocus::Intelligence:
		IntelligenceXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(IntelligenceXP, AttrComp->Intelligence);
		break;
	case ESovereignTrainingFocus::Wisdom:
		WisdomXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(WisdomXP, AttrComp->Wisdom);
		break;
	case ESovereignTrainingFocus::Charisma:
		CharismaXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(CharismaXP, AttrComp->Charisma);
		break;
	case ESovereignTrainingFocus::Luck:
		LuckXP += AdjustedAmount;
		if (AttrComp) EvaluateXPRollover(LuckXP, AttrComp->Luck);
		break;
	case ESovereignTrainingFocus::Qi:
		// Qi experience feeds Wisdom and Constitution
		WisdomXP += AdjustedAmount * 0.5;
		ConstitutionXP += AdjustedAmount * 0.5;
		if (AttrComp)
		{
			EvaluateXPRollover(WisdomXP, AttrComp->Wisdom);
			EvaluateXPRollover(ConstitutionXP, AttrComp->Constitution);
		}
		break;
	default:
		break;
	}
}

void USovereignLevelComponent::ConsumeItemXP(const FSovereignItemRow& ItemRow)
{
	if (ItemRow.StrengthXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Strength, ItemRow.StrengthXP);
	if (ItemRow.DexterityXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Dexterity, ItemRow.DexterityXP);
	if (ItemRow.ConstitutionXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Constitution, ItemRow.ConstitutionXP);
	if (ItemRow.IntelligenceXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Intelligence, ItemRow.IntelligenceXP);
	if (ItemRow.WisdomXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Wisdom, ItemRow.WisdomXP);
	if (ItemRow.CharismaXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Charisma, ItemRow.CharismaXP);
	if (ItemRow.LuckXP > 0.0) AddAttributeXP(ESovereignTrainingFocus::Luck, ItemRow.LuckXP);
}

void USovereignLevelComponent::ProcessTrainingProgress(float DeltaTime)
{
	if (ActiveTrainingFocus == ESovereignTrainingFocus::None)
	{
		return;
	}

	double ProgressGain = static_cast<double>(DeltaTime) * 0.1;
	AddAttributeXP(ActiveTrainingFocus, ProgressGain);
}

void USovereignLevelComponent::EvaluateXPRollover(double& XPAccumulator, int32& TargetStat)
{
	if (XPAccumulator >= 1.0)
	{
		int32 EarnedLevels = FMath::FloorToInt(XPAccumulator);
		TargetStat += EarnedLevels;
		XPAccumulator -= static_cast<double>(EarnedLevels);
	}
}

void USovereignLevelComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	TSharedPtr<FJsonObject> LevelObj = MakeShareable(new FJsonObject());

	LevelObj->SetNumberField(TEXT("ActiveTrainingFocus"), static_cast<double>(ActiveTrainingFocus));
	LevelObj->SetNumberField(TEXT("TrainingRateMultiplier"), TrainingRateMultiplier);

	LevelObj->SetNumberField(TEXT("StrengthXP"), StrengthXP);
	LevelObj->SetNumberField(TEXT("DexterityXP"), DexterityXP);
	LevelObj->SetNumberField(TEXT("ConstitutionXP"), ConstitutionXP);
	LevelObj->SetNumberField(TEXT("IntelligenceXP"), IntelligenceXP);
	LevelObj->SetNumberField(TEXT("WisdomXP"), WisdomXP);
	LevelObj->SetNumberField(TEXT("CharismaXP"), CharismaXP);
	LevelObj->SetNumberField(TEXT("LuckXP"), LuckXP);

	OutJson->SetObjectField(TEXT("Sovereign.Level"), LevelObj);
}

void USovereignLevelComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	const TSharedPtr<FJsonObject>* LevelObj = nullptr;
	if (InJson->TryGetObjectField(TEXT("Sovereign.Level"), LevelObj) || InJson->TryGetObjectField(TEXT("Level"), LevelObj))
	{
		double TempVal;
		if ((*LevelObj)->TryGetNumberField(TEXT("ActiveTrainingFocus"), TempVal))
		{
			ActiveTrainingFocus = static_cast<ESovereignTrainingFocus>(static_cast<uint8>(TempVal));
		}
		if ((*LevelObj)->TryGetNumberField(TEXT("TrainingRateMultiplier"), TempVal))
		{
			TrainingRateMultiplier = static_cast<float>(TempVal);
		}

		(*LevelObj)->TryGetNumberField(TEXT("StrengthXP"), StrengthXP);
		(*LevelObj)->TryGetNumberField(TEXT("DexterityXP"), DexterityXP);
		(*LevelObj)->TryGetNumberField(TEXT("ConstitutionXP"), ConstitutionXP);
		(*LevelObj)->TryGetNumberField(TEXT("IntelligenceXP"), IntelligenceXP);
		(*LevelObj)->TryGetNumberField(TEXT("WisdomXP"), WisdomXP);
		(*LevelObj)->TryGetNumberField(TEXT("CharismaXP"), CharismaXP);
		(*LevelObj)->TryGetNumberField(TEXT("LuckXP"), LuckXP);
	}
}

void USovereignLevelComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	if (Data.Contains(TEXT("Level.Focus")))
	{
		uint8 FocusVal = static_cast<uint8>(FCString::Atoi(*Data[TEXT("Level.Focus")]));
		if (FocusVal < static_cast<uint8>(ESovereignTrainingFocus::MAX))
		{
			ActiveTrainingFocus = static_cast<ESovereignTrainingFocus>(FocusVal);
		}
	}
}
