// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Inventory/SovereignInventoryBroker.h"
#include "Inventory/SovereignInventoryComponent.h"
#include "Inventory/SovereignItemDefinition.h"
#include "Dom/JsonObject.h"

USovereignInventoryBroker::USovereignInventoryBroker()
{
}

void USovereignInventoryBroker::Initialize(USovereignInventoryComponent* InComponent)
{
	TargetInventory = InComponent;
}

void USovereignInventoryBroker::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid() || !TargetInventory.IsValid())
	{
		return;
	}

	USovereignInventoryComponent* Inv = TargetInventory.Get();

	TSharedPtr<FJsonObject> InvObj = MakeShared<FJsonObject>();
	InvObj->SetNumberField(TEXT("MaxSlotCount"), Inv->MaxSlotCount);

	TArray<TSharedPtr<FJsonValue>> SlotsArray;
	for (const FSovereignInventoryItemInstance& Slot : Inv->InventorySlots)
	{
		TSharedPtr<FJsonObject> SlotObj = MakeShared<FJsonObject>();
		SlotObj->SetStringField(TEXT("InstanceId"), Slot.InstanceId.ToString());
		SlotObj->SetStringField(TEXT("ItemDef"), Slot.ItemDefinition.ToSoftObjectPath().ToString());
		SlotObj->SetNumberField(TEXT("Quantity"), Slot.Quantity);

		// Dynamic Tags
		TArray<TSharedPtr<FJsonValue>> TagsArray;
		for (const FGameplayTag& Tag : Slot.DynamicTags)
		{
			TagsArray.Add(MakeShared<FJsonValueString>(Tag.ToString()));
		}
		SlotObj->SetArrayField(TEXT("DynamicTags"), TagsArray);

		// Dynamic Properties
		TSharedPtr<FJsonObject> PropsObj = MakeShared<FJsonObject>();
		for (const auto& PropPair : Slot.DynamicProperties)
		{
			PropsObj->SetStringField(PropPair.Key, PropPair.Value);
		}
		SlotObj->SetObjectField(TEXT("DynamicProperties"), PropsObj);

		SlotsArray.Add(MakeShared<FJsonValueObject>(SlotObj));
	}

	InvObj->SetArrayField(TEXT("Slots"), SlotsArray);
	OutJson->SetObjectField(TEXT("Sovereign.Inventory"), InvObj);
}

void USovereignInventoryBroker::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid() || !TargetInventory.IsValid() || !InJson->HasField(TEXT("Sovereign.Inventory")))
	{
		return;
	}

	USovereignInventoryComponent* Inv = TargetInventory.Get();
	TSharedPtr<FJsonObject> InvObj = InJson->GetObjectField(TEXT("Sovereign.Inventory"));

	if (InvObj->HasField(TEXT("MaxSlotCount")))
	{
		Inv->MaxSlotCount = InvObj->GetIntegerField(TEXT("MaxSlotCount"));
	}

	Inv->InventorySlots.Empty();

	const TArray<TSharedPtr<FJsonValue>>* SlotsArrayPtr = nullptr;
	if (InvObj->TryGetArrayField(TEXT("Slots"), SlotsArrayPtr) && SlotsArrayPtr)
	{
		for (const TSharedPtr<FJsonValue>& SlotVal : *SlotsArrayPtr)
		{
			TSharedPtr<FJsonObject> SlotObj = SlotVal->AsObject();
			if (!SlotObj.IsValid())
			{
				continue;
			}

			FSovereignInventoryItemInstance LoadedSlot;

			if (SlotObj->HasField(TEXT("InstanceId")))
			{
				FGuid::Parse(SlotObj->GetStringField(TEXT("InstanceId")), LoadedSlot.InstanceId);
			}

			if (SlotObj->HasField(TEXT("ItemDef")))
			{
				FString PathStr = SlotObj->GetStringField(TEXT("ItemDef"));
				LoadedSlot.ItemDefinition = TSoftObjectPtr<USovereignItemDefinition>(FSoftObjectPath(PathStr));
			}

			if (SlotObj->HasField(TEXT("Quantity")))
			{
				LoadedSlot.Quantity = SlotObj->GetIntegerField(TEXT("Quantity"));
			}

			// Dynamic Tags
			const TArray<TSharedPtr<FJsonValue>>* TagsArrayPtr = nullptr;
			if (SlotObj->TryGetArrayField(TEXT("DynamicTags"), TagsArrayPtr) && TagsArrayPtr)
			{
				for (const TSharedPtr<FJsonValue>& TagVal : *TagsArrayPtr)
				{
					FString TagStr = TagVal->AsString();
					LoadedSlot.DynamicTags.AddTag(FGameplayTag::RequestGameplayTag(FName(*TagStr), false));
				}
			}

			// Dynamic Properties
			const TSharedPtr<FJsonObject>* PropsObjPtr = nullptr;
			if (SlotObj->TryGetObjectField(TEXT("DynamicProperties"), PropsObjPtr) && PropsObjPtr && (*PropsObjPtr).IsValid())
			{
				for (const auto& PropPair : (*PropsObjPtr)->Values)
				{
					LoadedSlot.DynamicProperties.Add(FString(PropPair.Key), PropPair.Value->AsString());
				}
			}

			Inv->InventorySlots.Add(LoadedSlot);
		}
	}

	Inv->OnInventoryUpdated.Broadcast();
}

void USovereignInventoryBroker::OnProcessData(const TMap<FString, FString>& Data)
{
	// External telemetry injection hook if needed
}
