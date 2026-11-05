// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Components/SovereignSkillComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignQiComponent.h"
#include "Dom/JsonObject.h"

USovereignSkillComponent::USovereignSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxEquippedSkills = 4;
}

ASovereignBaseCharacter* USovereignSkillComponent::GetOwnerCharacter() const
{
	return Cast<ASovereignBaseCharacter>(GetOwner());
}

bool USovereignSkillComponent::EquipSkill(USovereignSkillDefinition* SkillDef, FGuid& OutInstanceId)
{
	if (!SkillDef)
	{
		return false;
	}

	if (EquippedSkills.Num() >= MaxEquippedSkills)
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignSkillComponent] Cannot equip skill: slots full (%d/%d)."),
			EquippedSkills.Num(), MaxEquippedSkills);
		return false;
	}

	FSovereignSkillInstance NewInstance(SkillDef);
	EquippedSkills.Add(NewInstance);
	OutInstanceId = NewInstance.SkillInstanceId;

	return true;
}

bool USovereignSkillComponent::UnequipSkill(const FGuid& SkillInstanceId)
{
	for (int32 i = 0; i < EquippedSkills.Num(); ++i)
	{
		if (EquippedSkills[i].SkillInstanceId == SkillInstanceId)
		{
			EquippedSkills.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool USovereignSkillComponent::FindSkillInstance(const FGuid& SkillInstanceId, FSovereignSkillInstance& OutInstance) const
{
	for (const FSovereignSkillInstance& Instance : EquippedSkills)
	{
		if (Instance.SkillInstanceId == SkillInstanceId)
		{
			OutInstance = Instance;
			return true;
		}
	}
	return false;
}

bool USovereignSkillComponent::InfuseSkillWithAugment(const FGuid& SkillInstanceId, USovereignSkillAugment* Augment)
{
	if (!SkillInstanceId.IsValid() || !Augment)
	{
		return false;
	}

	// 1. Locate Skill Instance
	FSovereignSkillInstance* TargetSkill = nullptr;
	for (FSovereignSkillInstance& Instance : EquippedSkills)
	{
		if (Instance.SkillInstanceId == SkillInstanceId)
		{
			TargetSkill = &Instance;
			break;
		}
	}

	if (!TargetSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignSkillComponent] Infuse failed: skill instance not found."));
		return false;
	}

	// 2. Query Owner's USovereignQiComponent to Spend Qi
	ASovereignBaseCharacter* OwnerChar = GetOwnerCharacter();
	if (OwnerChar && Augment->QiCostToInfuse > 0.0f)
	{
		USovereignQiComponent* QiComp = OwnerChar->FindComponentByClass<USovereignQiComponent>();
		if (!QiComp || QiComp->CurrentQi < Augment->QiCostToInfuse)
		{
			UE_LOG(LogTemp, Warning, TEXT("[USovereignSkillComponent] Infuse failed: insufficient Qi (Req: %.1f)."),
				Augment->QiCostToInfuse);
			return false;
		}

		// Deduct Qi
		QiComp->SpendQi(Augment->QiCostToInfuse);
	}

	// 3. Attach Augment to Skill
	TargetSkill->ActiveAugments.Add(Augment);
	OnAugmentInfused.Broadcast(SkillInstanceId, Augment->AugmentTag);

	UE_LOG(LogTemp, Log, TEXT("[USovereignSkillComponent] Infused skill %s with augment %s."),
		*SkillInstanceId.ToString(), *Augment->AugmentName.ToString());

	return true;
}

void USovereignSkillComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid()) return;

	TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
	RootObj->SetNumberField(TEXT("MaxEquippedSkills"), MaxEquippedSkills);

	TArray<TSharedPtr<FJsonValue>> SkillsArray;
	for (const FSovereignSkillInstance& Skill : EquippedSkills)
	{
		TSharedPtr<FJsonObject> SkillObj = MakeShared<FJsonObject>();
		SkillObj->SetStringField(TEXT("SkillInstanceId"), Skill.SkillInstanceId.ToString());
		SkillObj->SetStringField(TEXT("SkillDef"), Skill.SkillDefinition.ToSoftObjectPath().ToString());
		SkillObj->SetNumberField(TEXT("SkillLevel"), Skill.SkillLevel);

		// Serialize Augments
		TArray<TSharedPtr<FJsonValue>> AugmentsArray;
		for (const TObjectPtr<USovereignSkillAugment>& Augment : Skill.ActiveAugments)
		{
			if (!Augment) continue;

			TSharedPtr<FJsonObject> AugObj = MakeShared<FJsonObject>();
			AugObj->SetStringField(TEXT("AugmentTag"), Augment->AugmentTag.ToString());
			AugObj->SetStringField(TEXT("AugmentName"), Augment->AugmentName.ToString());
			AugObj->SetNumberField(TEXT("AugmentType"), static_cast<uint8>(Augment->AugmentType));
			AugObj->SetNumberField(TEXT("AdditionalStatMultiplier"), Augment->AdditionalStatMultiplier);
			AugObj->SetNumberField(TEXT("AdditionalBasePower"), Augment->AdditionalBasePower);
			AugObj->SetBoolField(TEXT("bOverrideTargetPattern"), Augment->bOverrideTargetPattern);
			AugObj->SetNumberField(TEXT("NewTargetPattern"), static_cast<uint8>(Augment->NewTargetPattern));

			AugmentsArray.Add(MakeShared<FJsonValueObject>(AugObj));
		}
		SkillObj->SetArrayField(TEXT("ActiveAugments"), AugmentsArray);

		SkillsArray.Add(MakeShared<FJsonValueObject>(SkillObj));
	}
	RootObj->SetArrayField(TEXT("EquippedSkills"), SkillsArray);

	OutJson->SetObjectField(TEXT("Sovereign.Skills"), RootObj);
}

void USovereignSkillComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid() || !InJson->HasField(TEXT("Sovereign.Skills"))) return;

	TSharedPtr<FJsonObject> RootObj = InJson->GetObjectField(TEXT("Sovereign.Skills"));
	if (RootObj->HasField(TEXT("MaxEquippedSkills")))
	{
		MaxEquippedSkills = RootObj->GetIntegerField(TEXT("MaxEquippedSkills"));
	}

	EquippedSkills.Empty();

	const TArray<TSharedPtr<FJsonValue>>* SkillsArrayPtr = nullptr;
	if (RootObj->TryGetArrayField(TEXT("EquippedSkills"), SkillsArrayPtr) && SkillsArrayPtr)
	{
		for (const TSharedPtr<FJsonValue>& Val : *SkillsArrayPtr)
		{
			TSharedPtr<FJsonObject> SkillObj = Val->AsObject();
			if (!SkillObj.IsValid()) continue;

			FSovereignSkillInstance LoadedSkill;

			if (SkillObj->HasField(TEXT("SkillInstanceId")))
			{
				FGuid::Parse(SkillObj->GetStringField(TEXT("SkillInstanceId")), LoadedSkill.SkillInstanceId);
			}

			if (SkillObj->HasField(TEXT("SkillDef")))
			{
				FString PathStr = SkillObj->GetStringField(TEXT("SkillDef"));
				LoadedSkill.SkillDefinition = TSoftObjectPtr<USovereignSkillDefinition>(FSoftObjectPath(PathStr));
			}

			if (SkillObj->HasField(TEXT("SkillLevel")))
			{
				LoadedSkill.SkillLevel = SkillObj->GetIntegerField(TEXT("SkillLevel"));
			}

			// Deserialize Augments
			const TArray<TSharedPtr<FJsonValue>>* AugmentsArrayPtr = nullptr;
			if (SkillObj->TryGetArrayField(TEXT("ActiveAugments"), AugmentsArrayPtr) && AugmentsArrayPtr)
			{
				for (const TSharedPtr<FJsonValue>& AugVal : *AugmentsArrayPtr)
				{
					TSharedPtr<FJsonObject> AugObj = AugVal->AsObject();
					if (!AugObj.IsValid()) continue;

					USovereignSkillAugment* LoadedAug = NewObject<USovereignSkillAugment>(this);
					FString TagStr = AugObj->GetStringField(TEXT("AugmentTag"));
					LoadedAug->AugmentTag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
					LoadedAug->AugmentName = FText::FromString(AugObj->GetStringField(TEXT("AugmentName")));
					LoadedAug->AugmentType = static_cast<ESovereignAugmentType>(AugObj->GetIntegerField(TEXT("AugmentType")));
					LoadedAug->AdditionalStatMultiplier = AugObj->GetNumberField(TEXT("AdditionalStatMultiplier"));
					LoadedAug->AdditionalBasePower = AugObj->GetNumberField(TEXT("AdditionalBasePower"));
					LoadedAug->bOverrideTargetPattern = AugObj->GetBoolField(TEXT("bOverrideTargetPattern"));
					LoadedAug->NewTargetPattern = static_cast<ESovereignSkillTargetPattern>(AugObj->GetIntegerField(TEXT("NewTargetPattern")));

					LoadedSkill.ActiveAugments.Add(LoadedAug);
				}
			}

			EquippedSkills.Add(LoadedSkill);
		}
	}
}

void USovereignSkillComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// Telemetry hook if needed
}
