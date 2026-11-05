// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Inventory/SovereignItemDefinition.h"
#include "DataTables/SovereignSpeciesData.h"

USovereignItemDefinition::USovereignItemDefinition()
{
}

FPrimaryAssetId USovereignItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("SovereignItem")), GetFName());
}

const USovereignItemFragment* USovereignItemDefinition::GetFragmentByClass(TSubclassOf<USovereignItemFragment> FragmentClass) const
{
	if (!FragmentClass)
	{
		return nullptr;
	}

	for (const TObjectPtr<USovereignItemFragment>& Fragment : Fragments)
	{
		if (Fragment && Fragment->IsA(FragmentClass))
		{
			return Fragment.Get();
		}
	}

	return nullptr;
}
