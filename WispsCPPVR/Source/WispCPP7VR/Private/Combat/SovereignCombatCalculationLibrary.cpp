// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Combat/SovereignCombatCalculationLibrary.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignClassComponent.h"

ESovereignBattleRank FSovereignCombatEntitySnapshot::GetRank() const
{
	return USovereignCombatCalculationLibrary::GetRankForSlot(FormationSlot);
}

ESovereignBattleRank USovereignCombatCalculationLibrary::GetRankForSlot(int32 SlotIndex)
{
	// Slot layout:
	// Front Rank: 1, 2, 3
	// Mid Rank:   4, 5, 6
	// Rear Rank:  7, 8, 9
	if (SlotIndex <= 3)
	{
		return ESovereignBattleRank::Front;
	}
	if (SlotIndex <= 6)
	{
		return ESovereignBattleRank::Mid;
	}
	return ESovereignBattleRank::Rear;
}

int32 USovereignCombatCalculationLibrary::GetColumnForSlot(int32 SlotIndex)
{
	// Columns 0, 1, 2:
	// Slot 1, 4, 7 -> Column 0
	// Slot 2, 5, 8 -> Column 1
	// Slot 3, 6, 9 -> Column 2
	if (SlotIndex < 1 || SlotIndex > 9)
	{
		return 0;
	}
	return (SlotIndex - 1) % 3;
}

float USovereignCombatCalculationLibrary::GetAttackerRankModifier(ESovereignBattleRank AttackerRank, ESovereignSkillScalingAttribute ScalingAttr)
{
	// GDD Section 5.1:
	// Front Rank: +10% Melee Attack Power
	// Mid Rank:   +15% Ranged Physical Damage, +15% Spell & Qi Damage
	// Rear Rank:  +20% Healing Output (handled during heal resolution)
	switch (AttackerRank)
	{
	case ESovereignBattleRank::Front:
		if (ScalingAttr == ESovereignSkillScalingAttribute::Strength)
		{
			return 1.10f; // +10% Melee Attack
		}
		break;

	case ESovereignBattleRank::Mid:
		if (ScalingAttr == ESovereignSkillScalingAttribute::Dexterity ||
			ScalingAttr == ESovereignSkillScalingAttribute::Intelligence ||
			ScalingAttr == ESovereignSkillScalingAttribute::Wisdom)
		{
			return 1.15f; // +15% Ranged Physical / Spell / Qi
		}
		break;

	case ESovereignBattleRank::Rear:
	default:
		break;
	}

	return 1.0f;
}

float USovereignCombatCalculationLibrary::GetTargetRankDefenseModifier(ESovereignBattleRank TargetRank)
{
	// GDD Section 5.1: Front Rank grants +15% Physical Defense
	if (TargetRank == ESovereignBattleRank::Front)
	{
		return 1.15f;
	}
	return 1.0f;
}

int32 USovereignCombatCalculationLibrary::GetRelevantAttributeValue(const FSovereignCombatEntitySnapshot& Attacker, ESovereignSkillScalingAttribute ScalingAttr)
{
	switch (ScalingAttr)
	{
	case ESovereignSkillScalingAttribute::Strength:
		return Attacker.Strength;
	case ESovereignSkillScalingAttribute::Dexterity:
		return Attacker.Dexterity;
	case ESovereignSkillScalingAttribute::Intelligence:
		return Attacker.Intelligence;
	case ESovereignSkillScalingAttribute::Wisdom:
		return Attacker.Wisdom;
	case ESovereignSkillScalingAttribute::Constitution:
		return Attacker.Constitution;
	default:
		return Attacker.Strength;
	}
}

float USovereignCombatCalculationLibrary::CalculateEffectiveDefense(const FSovereignCombatEntitySnapshot& Target)
{
	// Base defense derived from Constitution + Equipment Defense, boosted by Rank modifier
	const float BaseDef = static_cast<float>(Target.Constitution) + Target.EquipmentDefense;
	const float RankMod = GetTargetRankDefenseModifier(Target.GetRank());
	return FMath::Max(0.0f, BaseDef * RankMod);
}

bool USovereignCombatCalculationLibrary::EvaluateCriticalHit(
	const FSovereignCombatEntitySnapshot& Attacker,
	const FSovereignCombatEntitySnapshot& Target,
	float ForcedRoll)
{
	// Base Crit: 5% + (Attacker.Luck - Target.Luck) * 0.5%, clamped between 5% and 50%
	const float LuckDelta = static_cast<float>(Attacker.Luck - Target.Luck);
	const float CritChance = FMath::Clamp(0.05f + (LuckDelta * 0.005f), 0.05f, 0.50f);

	const float Roll = (ForcedRoll >= 0.0f) ? ForcedRoll : FMath::FRand();
	return Roll <= CritChance;
}

FSovereignDamageCalculationResult USovereignCombatCalculationLibrary::CalculateCombatDamage(
	const FSovereignCombatEntitySnapshot& Attacker,
	const FSovereignCombatEntitySnapshot& Target,
	const FSovereignSkillInstance& SkillInstance,
	float ForcedCritRoll)
{
	FSovereignDamageCalculationResult Result;

	// 1. Identify scaling attribute
	ESovereignSkillScalingAttribute ScalingAttr = ESovereignSkillScalingAttribute::Strength;
	if (USovereignSkillDefinition* Def = SkillInstance.SkillDefinition.Get())
	{
		ScalingAttr = Def->ScalingAttribute;
	}

	// 2. Extract attacker attribute and calculate raw power with skill augments and job multipliers
	const int32 StatValue = GetRelevantAttributeValue(Attacker, ScalingAttr);
	Result.RawPower = SkillInstance.CalculateFinalPotency(StatValue, Attacker.ClassDamageMultiplier);

	// 3. Attacker Rank offensive modifier
	Result.AttackerRankModifier = GetAttackerRankModifier(Attacker.GetRank(), ScalingAttr);
	const float ScaledOffense = Result.RawPower * Result.AttackerRankModifier;

	// 4. Target Rank defensive modifier & Effective Defense
	Result.TargetDefenseModifier = GetTargetRankDefenseModifier(Target.GetRank());
	Result.EffectiveDefense = CalculateEffectiveDefense(Target);

	// 5. Diminishing returns mitigation curve: 100 / (100 + EffectiveDefense)
	Result.MitigationFactor = 100.0f / (100.0f + Result.EffectiveDefense);

	// 6. Base mitigated damage
	float ComputedDamage = ScaledOffense * Result.MitigationFactor;

	// 7. Elemental Weakness / Resistance Evaluation (Ticket CS-004)
	ESovereignElement AttackElement = ESovereignElement::None;
	if (SkillInstance.Augment && SkillInstance.Augment->InfusedElement != ESovereignElement::None)
	{
		AttackElement = SkillInstance.Augment->InfusedElement;
	}
	else if (USovereignSkillDefinition* Def = SkillInstance.SkillDefinition.Get())
	{
		AttackElement = Def->ElementType;
	}

	Result.ElementalMultiplier = 1.0f;
	if (AttackElement != ESovereignElement::None)
	{
		if (Target.WeaknessElement != ESovereignElement::None && Target.WeaknessElement == AttackElement)
		{
			Result.ElementalMultiplier = 1.5f; // +50% weakness bonus
		}
		else if (Target.ResistanceElement != ESovereignElement::None && Target.ResistanceElement == AttackElement)
		{
			Result.ElementalMultiplier = 0.5f; // 50% resistance mitigation
		}
	}
	ComputedDamage *= Result.ElementalMultiplier;

	// 8. Critical hit evaluation (1.5x multiplier)
	Result.bIsCriticalHit = EvaluateCriticalHit(Attacker, Target, ForcedCritRoll);
	if (Result.bIsCriticalHit)
	{
		ComputedDamage *= 1.5f;
	}

	// 9. Floor final damage at minimum 1.0f on hit
	Result.FinalDamage = FMath::Max(1.0f, FMath::RoundToFloat(ComputedDamage));

	return Result;
}

bool USovereignCombatCalculationLibrary::IsTargetSlotLegal(
	int32 TargetSlot,
	const TArray<FSovereignCombatEntitySnapshot>& TargetFormation,
	ESovereignSkillTargetPattern SkillPattern,
	ESovereignSkillScalingAttribute ScalingAttr)
{
	if (TargetSlot < 1 || TargetSlot > 9)
	{
		return false;
	}

	// Find the snapshot for the targeted slot
	const FSovereignCombatEntitySnapshot* TargetSnap = nullptr;
	for (const FSovereignCombatEntitySnapshot& Snap : TargetFormation)
	{
		if (Snap.FormationSlot == TargetSlot)
		{
			TargetSnap = &Snap;
			break;
		}
	}

	// Slot must exist and be occupied by an active, living unit
	if (!TargetSnap || !TargetSnap->bIsAlive)
	{
		return false;
	}

	// Spells (INT/WIS) and Ranged attacks (DEX) bypass front-line melee obstruction
	const bool bIsRangedOrMagic = (ScalingAttr == ESovereignSkillScalingAttribute::Dexterity ||
								   ScalingAttr == ESovereignSkillScalingAttribute::Intelligence ||
								   ScalingAttr == ESovereignSkillScalingAttribute::Wisdom);

	// Full AOE, Column Pierce, and Front Row Sweep handle obstruction in pattern resolution
	if (SkillPattern == ESovereignSkillTargetPattern::FullMatrixAOE ||
		SkillPattern == ESovereignSkillTargetPattern::ColumnPierce ||
		SkillPattern == ESovereignSkillTargetPattern::FrontRowSweep ||
		bIsRangedOrMagic)
	{
		return true;
	}

	// For SingleTarget melee attacks (Strength/Constitution), check rank protection:
	const ESovereignBattleRank TargetRank = GetRankForSlot(TargetSlot);

	// Mid Rank is protected if any living unit is in Front Rank
	if (TargetRank == ESovereignBattleRank::Mid)
	{
		for (const FSovereignCombatEntitySnapshot& Snap : TargetFormation)
		{
			if (Snap.bIsAlive && GetRankForSlot(Snap.FormationSlot) == ESovereignBattleRank::Front)
			{
				return false; // Front rank still has living defenders protecting mid rank!
			}
		}
	}
	// Rear Rank is protected if any living unit is in Front or Mid Rank
	else if (TargetRank == ESovereignBattleRank::Rear)
	{
		for (const FSovereignCombatEntitySnapshot& Snap : TargetFormation)
		{
			if (Snap.bIsAlive)
			{
				const ESovereignBattleRank R = GetRankForSlot(Snap.FormationSlot);
				if (R == ESovereignBattleRank::Front || R == ESovereignBattleRank::Mid)
				{
					return false; // Front or Mid rank still has living defenders protecting rear rank!
				}
			}
		}
	}

	return true;
}

void USovereignCombatCalculationLibrary::ResolveTargetSlots(
	int32 PrimaryTargetSlot,
	const TArray<FSovereignCombatEntitySnapshot>& DefendingFormation,
	ESovereignSkillTargetPattern SkillPattern,
	ESovereignSkillScalingAttribute ScalingAttr,
	TArray<int32>& OutTargetSlots)
{
	OutTargetSlots.Empty();

	switch (SkillPattern)
	{
	case ESovereignSkillTargetPattern::SingleTarget:
	{
		if (IsTargetSlotLegal(PrimaryTargetSlot, DefendingFormation, SkillPattern, ScalingAttr))
		{
			OutTargetSlots.Add(PrimaryTargetSlot);
		}
		break;
	}

	case ESovereignSkillTargetPattern::FrontRowSweep:
	{
		// Targets all living units in the foremost active rank of the enemy formation
		ESovereignBattleRank ForemostRank = ESovereignBattleRank::Rear;
		bool bFoundAny = false;

		// Check if Front rank has any living units
		for (const FSovereignCombatEntitySnapshot& Snap : DefendingFormation)
		{
			if (Snap.bIsAlive && GetRankForSlot(Snap.FormationSlot) == ESovereignBattleRank::Front)
			{
				ForemostRank = ESovereignBattleRank::Front;
				bFoundAny = true;
				break;
			}
		}

		// If no front units, check Mid rank
		if (!bFoundAny)
		{
			for (const FSovereignCombatEntitySnapshot& Snap : DefendingFormation)
			{
				if (Snap.bIsAlive && GetRankForSlot(Snap.FormationSlot) == ESovereignBattleRank::Mid)
				{
					ForemostRank = ESovereignBattleRank::Mid;
					bFoundAny = true;
					break;
				}
			}
		}

		// Collect all living units in the foremost active rank
		for (const FSovereignCombatEntitySnapshot& Snap : DefendingFormation)
		{
			if (Snap.bIsAlive && GetRankForSlot(Snap.FormationSlot) == ForemostRank)
			{
				OutTargetSlots.Add(Snap.FormationSlot);
			}
		}
		break;
	}

	case ESovereignSkillTargetPattern::ColumnPierce:
	{
		// Strikes through the target column hitting Front -> Mid -> Rear living units
		const int32 TargetColumn = GetColumnForSlot(PrimaryTargetSlot);
		for (const FSovereignCombatEntitySnapshot& Snap : DefendingFormation)
		{
			if (Snap.bIsAlive && GetColumnForSlot(Snap.FormationSlot) == TargetColumn)
			{
				OutTargetSlots.Add(Snap.FormationSlot);
			}
		}
		break;
	}

	case ESovereignSkillTargetPattern::FullMatrixAOE:
	{
		// Hits all living units in the entire matrix
		for (const FSovereignCombatEntitySnapshot& Snap : DefendingFormation)
		{
			if (Snap.bIsAlive)
			{
				OutTargetSlots.Add(Snap.FormationSlot);
			}
		}
		break;
	}

	case ESovereignSkillTargetPattern::SelfOnly:
	case ESovereignSkillTargetPattern::AllAllies:
	default:
		break;
	}
}

FSovereignCombatEntitySnapshot USovereignCombatCalculationLibrary::CreateSnapshotFromCharacter(
	ASovereignBaseCharacter* Character,
	int32 FormationSlot)
{
	FSovereignCombatEntitySnapshot Snapshot;
	Snapshot.FormationSlot = FormationSlot;

	if (!Character)
	{
		Snapshot.bIsAlive = false;
		return Snapshot;
	}

	USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent();
	if (AttrComp)
	{
		Snapshot.bIsAlive = (AttrComp->CurrentHealth > 0.0f);
		Snapshot.Strength = AttrComp->Strength;
		Snapshot.Dexterity = AttrComp->Dexterity;
		Snapshot.Constitution = AttrComp->Constitution;
		Snapshot.Intelligence = AttrComp->Intelligence;
		Snapshot.Wisdom = AttrComp->Wisdom;
		Snapshot.Luck = AttrComp->Luck;
	}
	else
	{
		Snapshot.bIsAlive = true;
	}

	if (USovereignClassComponent* ClassComp = Character->GetClassComponent())
	{
		Snapshot.ClassDamageMultiplier = ClassComp->GetTotalStrengthMultiplier();
	}

	return Snapshot;
}
