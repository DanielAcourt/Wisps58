// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/SovereignItemDefinition.h"
#include "SovereignInventoryComponent.generated.h"

class ASovereignBaseCharacter;

/**
 * FSovereignInventoryItemInstance
 * Lightweight runtime representation of an item in a character vessel's pack.
 * Value-based struct to avoid GC clutter and enable trivial JSON serialization.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignInventoryItemInstance
{
	GENERATED_BODY()

	/** Unique identifier for this runtime instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory", meta = (IgnoreForMemberInitializationTest))
	FGuid InstanceId;

	/** Soft object pointer to the immutable archetype definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
	TSoftObjectPtr<USovereignItemDefinition> ItemDefinition;

	/** Number of units in this slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
	int32 Quantity = 1;

	/** Dynamic runtime state tags (e.g. Item.State.Equipped, Item.State.Infused) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
	FGameplayTagContainer DynamicTags;

	/** Custom dynamic properties (e.g. Durability, Quality, Crafter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
	TMap<FString, FString> DynamicProperties;

	FSovereignInventoryItemInstance()
		: InstanceId(FGuid::NewGuid())
		, Quantity(1)
	{
	}

	FSovereignInventoryItemInstance(USovereignItemDefinition* InDef, int32 InQuantity = 1)
		: InstanceId(FGuid::NewGuid())
		, ItemDefinition(InDef)
		, Quantity(InQuantity)
	{
	}
};

/** Dynamic Multicast Delegates for inventory events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemAdded, const FSovereignInventoryItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemRemoved, const FGuid&, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**
 * USovereignInventoryComponent
 * Manages slots, item addition/removal, consumption, and equipment for character vessels.
 * Attached strictly to ASovereignBaseCharacter.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignInventoryComponent : public USovereignBaseComponent
{
	GENERATED_BODY()

public:
	USovereignInventoryComponent();

	/** Maximum number of distinct item slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
	int32 MaxSlotCount = 20;

	/** Active item instances in this inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Inventory")
	TArray<FSovereignInventoryItemInstance> InventorySlots;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Inventory|Events")
	FOnInventoryItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Inventory|Events")
	FOnInventoryItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Inventory|Events")
	FOnInventoryUpdated OnInventoryUpdated;

	// --- CORE OPERATIONS ---

	/**
	 * Adds an item archetype to the inventory. Stacks with existing slot if matched, or creates a new slot.
	 * @param ItemDef Primary DataAsset defining the item archetype.
	 * @param Quantity Number of units to add.
	 * @return True if item was added, false if inventory was full or definition was null.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool AddItem(USovereignItemDefinition* ItemDef, int32 Quantity = 1);

	/**
	 * Removes units of an item by runtime InstanceId. Removes slot completely if quantity reaches zero.
	 * @param InstanceId The GUID identifying the specific slot instance.
	 * @param Quantity Number of units to decrement.
	 * @return True if item was found and removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool RemoveItemByInstanceId(const FGuid& InstanceId, int32 Quantity = 1);

	/**
	 * Consumes an item, invoking OnConsumed on all attached fragments.
	 * If consumption succeeds on any fragment, decrements the item quantity.
	 * @param InstanceId The GUID identifying the item instance.
	 * @param Quantity Number of units to consume.
	 * @return True if consumption succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool ConsumeItem(const FGuid& InstanceId, int32 Quantity = 1);

	/**
	 * Equips an item to a named socket / slot on the owning character vessel.
	 * Invokes OnEquipped on all attached fragments and adds Item.State.Equipped tag.
	 * @param InstanceId The GUID identifying the item instance.
	 * @param SlotName Identifier of the equipment socket/slot.
	 * @return True if equipment succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool EquipItem(const FGuid& InstanceId, const FName& SlotName);

	/**
	 * Unequips an item from the character vessel.
	 * Invokes OnUnequipped on all attached fragments and removes Item.State.Equipped tag.
	 * @param InstanceId The GUID identifying the item instance.
	 * @param SlotName Identifier of the equipment socket/slot.
	 * @return True if unequipping succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool UnequipItem(const FGuid& InstanceId, const FName& SlotName);

	// --- QUERIES ---

	/** Checks whether the inventory has reached MaxSlotCount */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
	bool IsFull() const;

	/** Returns the total count of all items across all slots */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
	int32 GetTotalItemCount() const;

	/** Returns total quantity of items matching a specific ItemTag */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
	int32 GetItemCountByTag(const FGameplayTag& ItemTag) const;

	/** Finds an item instance by its InstanceId */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
	bool FindSlotByInstanceId(const FGuid& InstanceId, FSovereignInventoryItemInstance& OutInstance) const;

	/** Returns a mutable pointer to a slot by instance ID, or nullptr */
	FSovereignInventoryItemInstance* GetSlotMutable(const FGuid& InstanceId);

	/** Helper to get owner character vessel */
	ASovereignBaseCharacter* GetOwnerCharacter() const;
};
