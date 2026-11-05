// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Inventory/SovereignInventoryComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Inventory/SovereignItemFragment.h"

USovereignInventoryComponent::USovereignInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxSlotCount = 20;
}

ASovereignBaseCharacter* USovereignInventoryComponent::GetOwnerCharacter() const
{
	return Cast<ASovereignBaseCharacter>(GetOwner());
}

bool USovereignInventoryComponent::AddItem(USovereignItemDefinition* ItemDef, int32 Quantity)
{
	if (!ItemDef || Quantity <= 0)
	{
		return false;
	}

	// 1. Search for existing slot with matching ItemDefinition to stack
	for (FSovereignInventoryItemInstance& Slot : InventorySlots)
	{
		if (Slot.ItemDefinition.Get() == ItemDef || Slot.ItemDefinition.ToSoftObjectPath().ToString() == ItemDef->GetPathName())
		{
			Slot.Quantity += Quantity;
			OnItemAdded.Broadcast(Slot);
			OnInventoryUpdated.Broadcast();
			return true;
		}
	}

	// 2. If no existing slot, check if inventory is full
	if (IsFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignInventoryComponent] Cannot add item %s: inventory full (%d/%d slots)."),
			*ItemDef->GetName(), InventorySlots.Num(), MaxSlotCount);
		return false;
	}

	// 3. Create a new slot
	FSovereignInventoryItemInstance NewSlot(ItemDef, Quantity);
	InventorySlots.Add(NewSlot);

	OnItemAdded.Broadcast(NewSlot);
	OnInventoryUpdated.Broadcast();
	return true;
}

bool USovereignInventoryComponent::RemoveItemByInstanceId(const FGuid& InstanceId, int32 Quantity)
{
	if (!InstanceId.IsValid() || Quantity <= 0)
	{
		return false;
	}

	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		FSovereignInventoryItemInstance& Slot = InventorySlots[Index];
		if (Slot.InstanceId == InstanceId)
		{
			Slot.Quantity -= Quantity;
			if (Slot.Quantity <= 0)
			{
				InventorySlots.RemoveAt(Index);
			}

			OnItemRemoved.Broadcast(InstanceId);
			OnInventoryUpdated.Broadcast();
			return true;
		}
	}

	return false;
}

bool USovereignInventoryComponent::RemoveItemByTag(const FGameplayTag& ItemTag, int32 Quantity)
{
	if (!ItemTag.IsValid() || Quantity <= 0)
	{
		return false;
	}

	int32 RemainingToRemove = Quantity;
	bool bAnyRemoved = false;

	for (int32 Index = InventorySlots.Num() - 1; Index >= 0 && RemainingToRemove > 0; --Index)
	{
		FSovereignInventoryItemInstance& Slot = InventorySlots[Index];
		if (const USovereignItemDefinition* Def = Slot.ItemDefinition.Get())
		{
			if (Def->ItemTag.MatchesTag(ItemTag))
			{
				const int32 DeductAmount = FMath::Min(RemainingToRemove, Slot.Quantity);
				Slot.Quantity -= DeductAmount;
				RemainingToRemove -= DeductAmount;
				bAnyRemoved = true;

				if (Slot.Quantity <= 0)
				{
					const FGuid RemovedGuid = Slot.InstanceId;
					InventorySlots.RemoveAt(Index);
					OnItemRemoved.Broadcast(RemovedGuid);
				}
			}
		}
	}

	if (bAnyRemoved)
	{
		OnInventoryUpdated.Broadcast();
	}

	return bAnyRemoved;
}

bool USovereignInventoryComponent::ConsumeItem(const FGuid& InstanceId, int32 Quantity)
{
	if (!InstanceId.IsValid() || Quantity <= 0)
	{
		return false;
	}

	FSovereignInventoryItemInstance* Slot = GetSlotMutable(InstanceId);
	if (!Slot || Quantity > Slot->Quantity)
	{
		return false;
	}

	USovereignItemDefinition* ItemDef = Slot->ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return false;
	}

	ASovereignBaseCharacter* OwnerChar = GetOwnerCharacter();

	// Invoke OnConsumed on all attached fragments
	bool bAnyFragmentConsumed = false;
	for (const TObjectPtr<USovereignItemFragment>& Fragment : ItemDef->Fragments)
	{
		if (Fragment)
		{
			if (Fragment->OnConsumed(OwnerChar, Quantity))
			{
				bAnyFragmentConsumed = true;
			}
		}
	}

	// If at least one fragment processed consumption, decrement the item count
	if (bAnyFragmentConsumed)
	{
		return RemoveItemByInstanceId(InstanceId, Quantity);
	}

	return false;
}

bool USovereignInventoryComponent::EquipItem(const FGuid& InstanceId, const FName& SlotName)
{
	if (!InstanceId.IsValid())
	{
		return false;
	}

	FSovereignInventoryItemInstance* Slot = GetSlotMutable(InstanceId);
	if (!Slot)
	{
		return false;
	}

	USovereignItemDefinition* ItemDef = Slot->ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return false;
	}

	ASovereignBaseCharacter* OwnerChar = GetOwnerCharacter();
	bool bEquipped = false;

	for (const TObjectPtr<USovereignItemFragment>& Fragment : ItemDef->Fragments)
	{
		if (Fragment)
		{
			if (Fragment->OnEquipped(OwnerChar, SlotName))
			{
				bEquipped = true;
			}
		}
	}

	if (bEquipped)
	{
		Slot->DynamicTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.State.Equipped"), false));
		OnInventoryUpdated.Broadcast();
	}

	return bEquipped;
}

bool USovereignInventoryComponent::UnequipItem(const FGuid& InstanceId, const FName& SlotName)
{
	if (!InstanceId.IsValid())
	{
		return false;
	}

	FSovereignInventoryItemInstance* Slot = GetSlotMutable(InstanceId);
	if (!Slot)
	{
		return false;
	}

	USovereignItemDefinition* ItemDef = Slot->ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return false;
	}

	ASovereignBaseCharacter* OwnerChar = GetOwnerCharacter();
	bool bUnequipped = false;

	for (const TObjectPtr<USovereignItemFragment>& Fragment : ItemDef->Fragments)
	{
		if (Fragment)
		{
			if (Fragment->OnUnequipped(OwnerChar, SlotName))
			{
				bUnequipped = true;
			}
		}
	}

	if (bUnequipped)
	{
		Slot->DynamicTags.RemoveTag(FGameplayTag::RequestGameplayTag(TEXT("Item.State.Equipped"), false));
		OnInventoryUpdated.Broadcast();
	}

	return bUnequipped;
}

bool USovereignInventoryComponent::IsFull() const
{
	return InventorySlots.Num() >= MaxSlotCount;
}

int32 USovereignInventoryComponent::GetTotalItemCount() const
{
	int32 Total = 0;
	for (const FSovereignInventoryItemInstance& Slot : InventorySlots)
	{
		Total += Slot.Quantity;
	}
	return Total;
}

int32 USovereignInventoryComponent::GetItemCountByTag(const FGameplayTag& ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return 0;
	}

	int32 Count = 0;
	for (const FSovereignInventoryItemInstance& Slot : InventorySlots)
	{
		if (const USovereignItemDefinition* Def = Slot.ItemDefinition.Get())
		{
			if (Def->ItemTag.MatchesTag(ItemTag))
			{
				Count += Slot.Quantity;
			}
		}
	}
	return Count;
}

bool USovereignInventoryComponent::FindSlotByInstanceId(const FGuid& InstanceId, FSovereignInventoryItemInstance& OutInstance) const
{
	for (const FSovereignInventoryItemInstance& Slot : InventorySlots)
	{
		if (Slot.InstanceId == InstanceId)
		{
			OutInstance = Slot;
			return true;
		}
	}
	return false;
}

FSovereignInventoryItemInstance* USovereignInventoryComponent::GetSlotMutable(const FGuid& InstanceId)
{
	for (FSovereignInventoryItemInstance& Slot : InventorySlots)
	{
		if (Slot.InstanceId == InstanceId)
		{
			return &Slot;
		}
	}
	return nullptr;
}
