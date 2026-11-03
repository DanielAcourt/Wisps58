// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Components/SovereignClassComponent.h"
#include "Dom/JsonObject.h"

USovereignClassComponent::USovereignClassComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxCP = 5;
}

void USovereignClassComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureDefaultStarterClass();
}

void USovereignClassComponent::EnsureDefaultStarterClass()
{
	FGameplayTag AdventurerTag = FGameplayTag::RequestGameplayTag(TEXT("Class.Combat.Adventurer"), false);
	if (AdventurerTag.IsValid() && !UnlockedClasses.Contains(AdventurerTag))
	{
		FSovereignClassData AdventurerData;
		AdventurerData.ClassTag = AdventurerTag;
		AdventurerData.ClassDisplayName = FText::FromString(TEXT("Adventurer"));
		AdventurerData.ClassLevel = 1;
		AdventurerData.CPCost = 1;
		AdventurerData.StrengthMultiplier = 1.0f;
		AdventurerData.DexterityMultiplier = 1.0f;
		AdventurerData.ConstitutionMultiplier = 1.0f;
		AdventurerData.IntelligenceMultiplier = 1.0f;
		AdventurerData.WisdomMultiplier = 1.0f;

		UnlockClass(AdventurerData);
		EquipClass(AdventurerTag);
	}
}

int32 USovereignClassComponent::GetUsedCP() const
{
	int32 Used = 0;
	for (const FGameplayTag& Tag : EquippedClasses)
	{
		if (const FSovereignClassData* ClassData = UnlockedClasses.Find(Tag))
		{
			Used += ClassData->CPCost;
		}
	}
	return Used;
}

int32 USovereignClassComponent::GetAvailableCP() const
{
	return FMath::Max(0, MaxCP - GetUsedCP());
}

bool USovereignClassComponent::UnlockClass(const FSovereignClassData& NewClassData)
{
	if (!NewClassData.ClassTag.IsValid())
	{
		return false;
	}

	if (!UnlockedClasses.Contains(NewClassData.ClassTag))
	{
		UnlockedClasses.Add(NewClassData.ClassTag, NewClassData);
		OnClassUnlocked.Broadcast(NewClassData.ClassTag);
		return true;
	}

	return false;
}

bool USovereignClassComponent::EquipClass(const FGameplayTag& ClassTag)
{
	if (!ClassTag.IsValid() || IsClassEquipped(ClassTag))
	{
		return false;
	}

	const FSovereignClassData* ClassData = UnlockedClasses.Find(ClassTag);
	if (!ClassData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignClassComponent] Cannot equip locked class %s."), *ClassTag.ToString());
		return false;
	}

	if (GetUsedCP() + ClassData->CPCost > MaxCP)
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignClassComponent] Insufficient CP to equip %s (Cost: %d, Avail: %d)."),
			*ClassTag.ToString(), ClassData->CPCost, GetAvailableCP());
		return false;
	}

	EquippedClasses.AddTag(ClassTag);
	OnClassEquipped.Broadcast(ClassTag);
	return true;
}

bool USovereignClassComponent::UnequipClass(const FGameplayTag& ClassTag)
{
	if (!ClassTag.IsValid() || !IsClassEquipped(ClassTag))
	{
		return false;
	}

	EquippedClasses.RemoveTag(ClassTag);
	OnClassUnequipped.Broadcast(ClassTag);
	return true;
}

bool USovereignClassComponent::IsClassEquipped(const FGameplayTag& ClassTag) const
{
	return EquippedClasses.HasTag(ClassTag);
}

bool USovereignClassComponent::AddClassLevel(const FGameplayTag& ClassTag, int32 LevelsToAdd)
{
	if (FSovereignClassData* ClassData = UnlockedClasses.Find(ClassTag))
	{
		ClassData->ClassLevel += FMath::Max(1, LevelsToAdd);
		return true;
	}
	return false;
}

float USovereignClassComponent::GetTotalStrengthMultiplier() const
{
	float Mult = 1.0f;
	for (const FGameplayTag& Tag : EquippedClasses)
	{
		if (const FSovereignClassData* ClassData = UnlockedClasses.Find(Tag))
		{
			Mult *= ClassData->StrengthMultiplier;
		}
	}
	return Mult;
}

float USovereignClassComponent::GetTotalDexterityMultiplier() const
{
	float Mult = 1.0f;
	for (const FGameplayTag& Tag : EquippedClasses)
	{
		if (const FSovereignClassData* ClassData = UnlockedClasses.Find(Tag))
		{
			Mult *= ClassData->DexterityMultiplier;
		}
	}
	return Mult;
}

float USovereignClassComponent::GetTotalIntelligenceMultiplier() const
{
	float Mult = 1.0f;
	for (const FGameplayTag& Tag : EquippedClasses)
	{
		if (const FSovereignClassData* ClassData = UnlockedClasses.Find(Tag))
		{
			Mult *= ClassData->IntelligenceMultiplier;
		}
	}
	return Mult;
}

void USovereignClassComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid()) return;

	TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
	RootObj->SetNumberField(TEXT("MaxCP"), MaxCP);

	// Serialize Unlocked Classes
	TArray<TSharedPtr<FJsonValue>> UnlockedArray;
	for (const auto& Pair : UnlockedClasses)
	{
		TSharedPtr<FJsonObject> ItemObj = MakeShared<FJsonObject>();
		ItemObj->SetStringField(TEXT("ClassTag"), Pair.Key.ToString());
		ItemObj->SetStringField(TEXT("DisplayName"), Pair.Value.ClassDisplayName.ToString());
		ItemObj->SetNumberField(TEXT("ClassLevel"), Pair.Value.ClassLevel);
		ItemObj->SetNumberField(TEXT("CPCost"), Pair.Value.CPCost);
		ItemObj->SetNumberField(TEXT("StrengthMultiplier"), Pair.Value.StrengthMultiplier);
		ItemObj->SetNumberField(TEXT("DexterityMultiplier"), Pair.Value.DexterityMultiplier);
		ItemObj->SetNumberField(TEXT("ConstitutionMultiplier"), Pair.Value.ConstitutionMultiplier);
		ItemObj->SetNumberField(TEXT("IntelligenceMultiplier"), Pair.Value.IntelligenceMultiplier);
		ItemObj->SetNumberField(TEXT("WisdomMultiplier"), Pair.Value.WisdomMultiplier);
		UnlockedArray.Add(MakeShared<FJsonValueObject>(ItemObj));
	}
	RootObj->SetArrayField(TEXT("UnlockedClasses"), UnlockedArray);

	// Serialize Equipped Tags
	TArray<TSharedPtr<FJsonValue>> EquippedArray;
	for (const FGameplayTag& Tag : EquippedClasses)
	{
		EquippedArray.Add(MakeShared<FJsonValueString>(Tag.ToString()));
	}
	RootObj->SetArrayField(TEXT("EquippedClasses"), EquippedArray);

	OutJson->SetObjectField(TEXT("Sovereign.Classes"), RootObj);
}

void USovereignClassComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid() || !InJson->HasField(TEXT("Sovereign.Classes"))) return;

	TSharedPtr<FJsonObject> RootObj = InJson->GetObjectField(TEXT("Sovereign.Classes"));
	if (RootObj->HasField(TEXT("MaxCP")))
	{
		MaxCP = RootObj->GetIntegerField(TEXT("MaxCP"));
	}

	UnlockedClasses.Empty();
	EquippedClasses.Reset();

	// Deserialize Unlocked Classes
	const TArray<TSharedPtr<FJsonValue>>* UnlockedArrayPtr = nullptr;
	if (RootObj->TryGetArrayField(TEXT("UnlockedClasses"), UnlockedArrayPtr) && UnlockedArrayPtr)
	{
		for (const TSharedPtr<FJsonValue>& Val : *UnlockedArrayPtr)
		{
			TSharedPtr<FJsonObject> ItemObj = Val->AsObject();
			if (!ItemObj.IsValid()) continue;

			FSovereignClassData Data;
			FString TagStr = ItemObj->GetStringField(TEXT("ClassTag"));
			Data.ClassTag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
			Data.ClassDisplayName = FText::FromString(ItemObj->GetStringField(TEXT("DisplayName")));
			Data.ClassLevel = ItemObj->GetIntegerField(TEXT("ClassLevel"));
			Data.CPCost = ItemObj->GetIntegerField(TEXT("CPCost"));
			Data.StrengthMultiplier = ItemObj->GetNumberField(TEXT("StrengthMultiplier"));
			Data.DexterityMultiplier = ItemObj->GetNumberField(TEXT("DexterityMultiplier"));
			Data.ConstitutionMultiplier = ItemObj->GetNumberField(TEXT("ConstitutionMultiplier"));
			Data.IntelligenceMultiplier = ItemObj->GetNumberField(TEXT("IntelligenceMultiplier"));
			Data.WisdomMultiplier = ItemObj->GetNumberField(TEXT("WisdomMultiplier"));

			if (Data.ClassTag.IsValid())
			{
				UnlockedClasses.Add(Data.ClassTag, Data);
			}
		}
	}

	// Deserialize Equipped Classes
	const TArray<TSharedPtr<FJsonValue>>* EquippedArrayPtr = nullptr;
	if (RootObj->TryGetArrayField(TEXT("EquippedClasses"), EquippedArrayPtr) && EquippedArrayPtr)
	{
		for (const TSharedPtr<FJsonValue>& Val : *EquippedArrayPtr)
		{
			FString TagStr = Val->AsString();
			FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
			if (Tag.IsValid())
			{
				EquippedClasses.AddTag(Tag);
			}
		}
	}
}

void USovereignClassComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// External telemetry injection hook if needed
}
