// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Inventory/SovereignItemFragment.h"
#include "Entities/SovereignBaseCharacter.h"

USovereignItemFragment::USovereignItemFragment()
{
}

bool USovereignItemFragment::OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const
{
	// Default: No-op
	return false;
}

bool USovereignItemFragment::OnEquipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const
{
	// Default: No-op
	return false;
}

bool USovereignItemFragment::OnUnequipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const
{
	// Default: No-op
	return false;
}
