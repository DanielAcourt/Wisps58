// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Components/SovereignATBComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignClassComponent.h"
#include "Components/SovereignStatusComponent.h"

USovereignATBComponent::USovereignATBComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Enabled during combat arena encounters
	CurrentATB = 0.0f;
	ATBState = EATBState::Charging;
	bIsWaitModeActive = false;
	bIsDefending = false;
	DefendDamageMultiplier = 0.5f;
	BaseATBRate = 20.0f;
}

void USovereignATBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// In battle arena, Tick advances ATB if charging or charging telegraphed attack
	if ((ATBState == EATBState::Charging || ATBState == EATBState::ChargingAttack) && !bIsWaitModeActive)
	{
		AdvanceATB(DeltaTime);
	}
}

float USovereignATBComponent::CalculateEffectiveFillRate() const
{
	float DexterityBonus = 0.0f;
	float ClassSpeedMultiplier = 1.0f;

	if (const ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(GetOwner()))
	{
		// 1. Dexterity contribution from AttributeComponent
		if (const USovereignAttributeComponent* AttrComp = Character->GetAttributeComponent())
		{
			// DEX scaling: +2% rate per point of Dexterity (DEX 10 = +20%, DEX 50 = +100%)
			DexterityBonus = FMath::Max(0.0f, static_cast<float>(AttrComp->Dexterity) / 50.0f);
		}

		// 2. Class Speed Multiplier from ClassComponent
		if (const USovereignClassComponent* ClassComp = Character->GetClassComponent())
		{
			ClassSpeedMultiplier = FMath::Max(0.1f, ClassComp->GetTotalDexterityMultiplier());
		}

		// 3. Status Multiplier (Haste 1.5x, Slow 0.5x) - Ticket CS-005
		if (const USovereignStatusComponent* StatusComp = Character->GetStatusComponent())
		{
			ClassSpeedMultiplier *= StatusComp->GetATBSpeedMultiplier();
		}
	}

	return BaseATBRate * (1.0f + DexterityBonus) * ClassSpeedMultiplier;
}

void USovereignATBComponent::AdvanceATB(float DeltaTime)
{
	if (DeltaTime <= 0.0f || (ATBState != EATBState::Charging && ATBState != EATBState::ChargingAttack) || bIsWaitModeActive)
	{
		return;
	}

	const float FillRate = CalculateEffectiveFillRate();
	CurrentATB = FMath::Clamp(CurrentATB + (FillRate * DeltaTime), 0.0f, MAX_ATB);
	OnATBProgressChanged.Broadcast(CurrentATB, MAX_ATB);

	if (CurrentATB >= MAX_ATB)
	{
		SetATBState(EATBState::Ready);
		OnATBReady.Broadcast(Cast<ASovereignBaseCharacter>(GetOwner()));
	}
}

void USovereignATBComponent::ResetATB()
{
	CurrentATB = 0.0f;
	SetATBState(EATBState::Charging);
	OnATBProgressChanged.Broadcast(CurrentATB, MAX_ATB);
}

void USovereignATBComponent::BeginActionSelection()
{
	SetATBState(EATBState::SelectingAction);
}

void USovereignATBComponent::BeginActionExecution()
{
	// Once an action starts executing, any prior Defend stance is consumed/cleared
	bIsDefending = false;
	SetATBState(EATBState::ExecutingAction);
}

void USovereignATBComponent::CompleteActionExecution()
{
	ResetATB();
}

void USovereignATBComponent::EnterDefendStance()
{
	// Defending sets the defensive mitigation flag and ends turn
	bIsDefending = true;
	ResetATB();
}

void USovereignATBComponent::BeginTelegraphedCharge(const FSovereignSkillInstance& InSkillInstance, const FText& WarningText)
{
	ChargingSkillInstance = InSkillInstance;
	CurrentATB = 0.0f;
	SetATBState(EATBState::ChargingAttack);
	OnATBProgressChanged.Broadcast(CurrentATB, MAX_ATB);

	FGameplayTag SkillTag;
	if (USovereignSkillDefinition* Def = InSkillInstance.SkillDefinition.Get())
	{
		SkillTag = Def->SkillTag;
	}
	OnTelegraphedAttackStarted.Broadcast(Cast<ASovereignBaseCharacter>(GetOwner()), SkillTag, WarningText);
}

void USovereignATBComponent::CancelTelegraphedCharge()
{
	if (ATBState == EATBState::ChargingAttack)
	{
		FGameplayTag SkillTag;
		if (USovereignSkillDefinition* Def = ChargingSkillInstance.SkillDefinition.Get())
		{
			SkillTag = Def->SkillTag;
		}
		ChargingSkillInstance = FSovereignSkillInstance();
		ResetATB();
		OnTelegraphedAttackInterrupted.Broadcast(Cast<ASovereignBaseCharacter>(GetOwner()), SkillTag);
	}
}

void USovereignATBComponent::SetATBState(EATBState NewState)
{
	if (ATBState != NewState)
	{
		const EATBState OldState = ATBState;
		ATBState = NewState;
		OnATBStateChanged.Broadcast(OldState, NewState);
	}
}

void USovereignATBComponent::SetWaitModeActive(bool bActive)
{
	bIsWaitModeActive = bActive;
}
