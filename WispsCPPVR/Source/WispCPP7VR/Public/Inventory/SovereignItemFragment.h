// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SovereignItemFragment.generated.h"

class ASovereignBaseCharacter;

/**
 * USovereignItemFragment
 * Abstract base class for all composable, modular item archetype fragments.
 * Fragments are instanced subobjects attached to a USovereignItemDefinition.
 * Provides BlueprintNativeEvent lifecycle hooks allowing C++ or Blueprint extension.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType, Blueprintable)
class WISPCPP7VR_API USovereignItemFragment : public UObject
{
	GENERATED_BODY()

public:
	USovereignItemFragment();

	/**
	 * Called when an item containing this fragment is consumed by a character vessel.
	 * @param TargetCharacter The physical character vessel consuming the item.
	 * @param Quantity Number of units consumed.
	 * @return True if consumption logic succeeded.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Inventory|Fragment")
	bool OnConsumed(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const;
	virtual bool OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const;

	/**
	 * Called when an item containing this fragment is equipped by a character vessel.
	 * @param TargetCharacter The physical character vessel equipping the item.
	 * @param SlotName Identifier of the equipment slot / socket.
	 * @return True if equipping logic succeeded.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Inventory|Fragment")
	bool OnEquipped(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const;
	virtual bool OnEquipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const;

	/**
	 * Called when an item containing this fragment is unequipped from a character vessel.
	 * @param TargetCharacter The physical character vessel unequipping the item.
	 * @param SlotName Identifier of the equipment slot / socket.
	 * @return True if unequipping logic succeeded.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sovereign|Inventory|Fragment")
	bool OnUnequipped(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const;
	virtual bool OnUnequipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const;
};
