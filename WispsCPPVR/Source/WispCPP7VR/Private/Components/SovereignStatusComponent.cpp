// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Components/SovereignStatusComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"

USovereignStatusComponent::USovereignStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USovereignStatusComponent::ApplyStatusEffect(ESovereignStatusType Type, int32 DurationTurns, float Potency, AActor* Source)
{
	if (Type == ESovereignStatusType::None || DurationTurns <= 0)
	{
		return false;
	}

	for (FSovereignStatusInstance& Existing : ActiveStatuses)
	{
		if (Existing.StatusType == Type)
		{
			// Refresh duration to max of existing or new
			Existing.RemainingTurns = FMath::Max(Existing.RemainingTurns, DurationTurns);
			Existing.Potency = FMath::Max(Existing.Potency, Potency);
			Existing.SourceActor = Source;
			OnStatusApplied.Broadcast(Type, Existing.RemainingTurns);
			return true;
		}
	}

	FSovereignStatusInstance NewInst(Type, DurationTurns, Potency, Source);
	ActiveStatuses.Add(NewInst);
	OnStatusApplied.Broadcast(Type, DurationTurns);
	return true;
}

bool USovereignStatusComponent::RemoveStatusEffect(ESovereignStatusType Type)
{
	for (int32 i = ActiveStatuses.Num() - 1; i >= 0; --i)
	{
		if (ActiveStatuses[i].StatusType == Type)
		{
			ActiveStatuses.RemoveAt(i);
			OnStatusExpired.Broadcast(Type);
			return true;
		}
	}
	return false;
}

void USovereignStatusComponent::ClearAllStatuses()
{
	for (const FSovereignStatusInstance& Status : ActiveStatuses)
	{
		OnStatusExpired.Broadcast(Status.StatusType);
	}
	ActiveStatuses.Empty();
}

bool USovereignStatusComponent::HasStatusEffect(ESovereignStatusType Type) const
{
	for (const FSovereignStatusInstance& Status : ActiveStatuses)
	{
		if (Status.StatusType == Type && Status.RemainingTurns > 0)
		{
			return true;
		}
	}
	return false;
}

bool USovereignStatusComponent::GetStatusInstance(ESovereignStatusType Type, FSovereignStatusInstance& OutInstance) const
{
	for (const FSovereignStatusInstance& Status : ActiveStatuses)
	{
		if (Status.StatusType == Type)
		{
			OutInstance = Status;
			return true;
		}
	}
	return false;
}

float USovereignStatusComponent::GetATBSpeedMultiplier() const
{
	float Mult = 1.0f;
	if (HasStatusEffect(ESovereignStatusType::Haste))
	{
		Mult *= 1.5f;
	}
	if (HasStatusEffect(ESovereignStatusType::Slow))
	{
		Mult *= 0.5f;
	}
	return Mult;
}

void USovereignStatusComponent::ProcessTurnEnd()
{
	ASovereignBaseCharacter* OwnerChar = Cast<ASovereignBaseCharacter>(GetOwner());
	USovereignAttributeComponent* AttrComp = OwnerChar ? OwnerChar->GetAttributeComponent() : nullptr;

	for (int32 i = ActiveStatuses.Num() - 1; i >= 0; --i)
	{
		FSovereignStatusInstance& Status = ActiveStatuses[i];

		// 1. Apply Turn-End Effects (Poison / Regen)
		if (Status.StatusType == ESovereignStatusType::Poison && AttrComp)
		{
			// Deals flat potency or % max HP damage floored at 1.0
			const float PoisonDmg = FMath::Max(1.0f, Status.Potency);
			AttrComp->CurrentHealth = FMath::Max(0.0f, AttrComp->CurrentHealth - PoisonDmg);
			OnStatusTicked.Broadcast(ESovereignStatusType::Poison, PoisonDmg, false);
		}
		else if (Status.StatusType == ESovereignStatusType::Regen && AttrComp)
		{
			const float RegenHeal = FMath::Max(1.0f, Status.Potency);
			AttrComp->CurrentHealth = FMath::Clamp(AttrComp->CurrentHealth + RegenHeal, 0.0f, AttrComp->MaxHealth);
			OnStatusTicked.Broadcast(ESovereignStatusType::Regen, RegenHeal, true);
		}

		// 2. Decrement remaining turns
		Status.RemainingTurns--;
		if (Status.RemainingTurns <= 0)
		{
			const ESovereignStatusType ExpiredType = Status.StatusType;
			ActiveStatuses.RemoveAt(i);
			OnStatusExpired.Broadcast(ExpiredType);
		}
	}
}

void USovereignStatusComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid()) return;

	TSharedPtr<FJsonObject> StatusJson = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> StatusList;

	for (const FSovereignStatusInstance& Status : ActiveStatuses)
	{
		TSharedPtr<FJsonObject> ItemObj = MakeShared<FJsonObject>();
		ItemObj->SetNumberField(TEXT("Type"), static_cast<int32>(Status.StatusType));
		ItemObj->SetNumberField(TEXT("Turns"), Status.RemainingTurns);
		ItemObj->SetNumberField(TEXT("Potency"), Status.Potency);
		StatusList.Add(MakeShared<FJsonValueObject>(ItemObj));
	}

	StatusJson->SetArrayField(TEXT("Statuses"), StatusList);
	OutJson->SetObjectField(TEXT("Sovereign.Status"), StatusJson);
}

void USovereignStatusComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid()) return;

	const TSharedPtr<FJsonObject>* StatusJson = nullptr;
	if (InJson->TryGetObjectField(TEXT("Sovereign.Status"), StatusJson) && StatusJson && StatusJson->IsValid())
	{
		ActiveStatuses.Empty();
		const TArray<TSharedPtr<FJsonValue>>* StatusList = nullptr;
		if ((*StatusJson)->TryGetArrayField(TEXT("Statuses"), StatusList) && StatusList)
		{
			for (const auto& Val : *StatusList)
			{
				TSharedPtr<FJsonObject> ItemObj = Val->AsObject();
				if (ItemObj.IsValid())
				{
					FSovereignStatusInstance Inst;
					Inst.StatusType = static_cast<ESovereignStatusType>(ItemObj->GetIntegerField(TEXT("Type")));
					Inst.RemainingTurns = ItemObj->GetIntegerField(TEXT("Turns"));
					Inst.Potency = ItemObj->GetNumberField(TEXT("Potency"));
					ActiveStatuses.Add(Inst);
				}
			}
		}
	}
}

void USovereignStatusComponent::OnProcessData(const TMap<FString, FString>& Data)
{
}
