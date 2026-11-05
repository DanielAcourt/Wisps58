// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Combat/SovereignSkillAugment.h"

USovereignSkillAugment::USovereignSkillAugment()
{
	QiCostToInfuse = 25.0f;
	AugmentType = ESovereignAugmentType::Potency;
	AdditionalStatMultiplier = 0.30f;
	AdditionalBasePower = 10.0f;
	bOverrideTargetPattern = false;
	NewTargetPattern = ESovereignSkillTargetPattern::FrontRowSweep;
	InfusedElement = ESovereignElement::None;
}

ESovereignSkillTargetPattern FSovereignSkillInstance::GetEffectiveTargetPattern() const
{
	// Check if any attached augment overrides the target shape (e.g. Cleave/AOE)
	for (const TObjectPtr<USovereignSkillAugment>& Augment : ActiveAugments)
	{
		if (Augment && Augment->bOverrideTargetPattern)
		{
			return Augment->NewTargetPattern;
		}
	}

	if (const USovereignSkillDefinition* Def = SkillDefinition.Get())
	{
		return Def->TargetPattern;
	}

	return ESovereignSkillTargetPattern::SingleTarget;
}

float FSovereignSkillInstance::GetEffectiveStatMultiplier() const
{
	float TotalMult = 1.0f;
	if (const USovereignSkillDefinition* Def = SkillDefinition.Get())
	{
		TotalMult = Def->StatMultiplier;
	}

	for (const TObjectPtr<USovereignSkillAugment>& Augment : ActiveAugments)
	{
		if (Augment)
		{
			TotalMult += Augment->AdditionalStatMultiplier;
		}
	}

	return TotalMult;
}

float FSovereignSkillInstance::GetEffectiveBasePower() const
{
	float Base = 20.0f;
	if (const USovereignSkillDefinition* Def = SkillDefinition.Get())
	{
		Base = Def->BasePower;
	}

	for (const TObjectPtr<USovereignSkillAugment>& Augment : ActiveAugments)
	{
		if (Augment)
		{
			Base += Augment->AdditionalBasePower;
		}
	}

	return Base;
}

float FSovereignSkillInstance::CalculateFinalPotency(int32 StatValue, float ClassMultiplier) const
{
	float Base = GetEffectiveBasePower();
	float Mult = GetEffectiveStatMultiplier();
	float StatContribution = static_cast<float>(StatValue) * Mult;

	float FinalPower = (Base + StatContribution) * FMath::Max(0.01f, ClassMultiplier);
	return FinalPower;
}
