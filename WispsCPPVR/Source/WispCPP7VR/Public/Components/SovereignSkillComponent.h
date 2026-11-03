// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "Combat/SovereignSkillAugment.h"
#include "SovereignSkillComponent.generated.h"

class ASovereignBaseCharacter;
class FJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillAugmentInfused, const FGuid&, SkillId, const FGameplayTag&, AugmentTag);

/**
 * USovereignSkillComponent
 * Manages a character vessel's active equipped combat skills and executes Qi infusions
 * to customize ability damage multipliers, target shapes, and elemental affinities.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignSkillComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignSkillComponent();

	/** Maximum active combat skills that can be equipped simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Skill")
	int32 MaxEquippedSkills = 4;

	/** Active equipped skills on this vessel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Skill")
	TArray<FSovereignSkillInstance> EquippedSkills;

	/** Broadcast when a skill is infused with a new Qi augment */
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Skill|Events")
	FOnSkillAugmentInfused OnAugmentInfused;

	/** --- 1. SKILL MANAGEMENT --- */

	/** Equips a skill archetype to an available slot */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Skill")
	bool EquipSkill(USovereignSkillDefinition* SkillDef, FGuid& OutInstanceId);

	/** Unequips a skill by its runtime instance ID */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Skill")
	bool UnequipSkill(const FGuid& SkillInstanceId);

	/** Finds a skill instance by its runtime instance ID */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Skill")
	bool FindSkillInstance(const FGuid& SkillInstanceId, FSovereignSkillInstance& OutInstance) const;

	/** --- 2. QI INFUSION PIPELINE --- */

	/**
	 * Spends Qi from the owner's USovereignQiComponent to attach an augment to a skill.
	 * @param SkillInstanceId Target skill instance to customize.
	 * @param Augment The augment subobject to attach.
	 * @return True if Qi was successfully deducted and augment attached.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Skill|Infusion")
	bool InfuseSkillWithAugment(const FGuid& SkillInstanceId, USovereignSkillAugment* Augment);

	/** Helper to get owner character vessel */
	ASovereignBaseCharacter* GetOwnerCharacter() const;

	/** --- 3. ISovereignBrokerInterface Implementation --- */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;
};
