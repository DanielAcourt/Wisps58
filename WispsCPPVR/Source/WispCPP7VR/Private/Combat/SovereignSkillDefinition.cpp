// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Combat/SovereignSkillDefinition.h"

USovereignSkillDefinition::USovereignSkillDefinition()
{
	BasePower = 20.0f;
	StatMultiplier = 1.20f;
	QiCost = 0.0f;
	CooldownTurns = 0;
	TargetPattern = ESovereignSkillTargetPattern::SingleTarget;
	ScalingAttribute = ESovereignSkillScalingAttribute::Strength;
}

FPrimaryAssetId USovereignSkillDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("SovereignSkill")), GetFName());
}

float USovereignSkillDefinition::CalculateRawPower(int32 StatValue, float ClassMultiplier) const
{
	float AttributeContribution = static_cast<float>(StatValue) * StatMultiplier;
	float TotalPower = (BasePower + AttributeContribution) * FMath::Max(0.01f, ClassMultiplier);
	return TotalPower;
}
