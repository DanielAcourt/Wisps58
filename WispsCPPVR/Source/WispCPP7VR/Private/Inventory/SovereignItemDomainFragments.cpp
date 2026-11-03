// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Inventory/SovereignItemDomainFragments.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignLevelComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

// --- CONSUMABLE FRAGMENT IMPLEMENTATION ---

bool USovereignItemFragment_Consumable::OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const
{
	if (!TargetCharacter || Quantity <= 0)
	{
		return false;
	}

	bool bHandled = false;

	// 1. Replenish Bio Vitals
	if (USovereignBioComponent* Bio = TargetCharacter->GetBioComponent())
	{
		Bio->Hunger = FMath::Clamp(Bio->Hunger + (HungerRestore * Quantity), 0.0f, 100.0f);
		Bio->Hydration = FMath::Clamp(Bio->Hydration + (HydrationRestore * Quantity), 0.0f, 100.0f);
		Bio->Fatigue = FMath::Clamp(Bio->Fatigue - (FatigueRelief * Quantity), 0.0f, 100.0f);
		Bio->Toxicity = FMath::Clamp(Bio->Toxicity + (ToxicityIncrease * Quantity), 0.0f, 100.0f);
		bHandled = true;
	}

	// 2. Award Item Experience Gains to LevelComponent
	if (USovereignLevelComponent* Level = TargetCharacter->GetLevelComponent())
	{
		double Mult = static_cast<double>(Quantity);
		if (StrengthXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Strength, StrengthXP * Mult);
		if (DexterityXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Dexterity, DexterityXP * Mult);
		if (ConstitutionXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Constitution, ConstitutionXP * Mult);
		if (IntelligenceXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Intelligence, IntelligenceXP * Mult);
		if (WisdomXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Wisdom, WisdomXP * Mult);
		if (CharismaXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Charisma, CharismaXP * Mult);
		if (LuckXP > 0.0) Level->AddAttributeXP(ESovereignTrainingFocus::Luck, LuckXP * Mult);
		bHandled = true;
	}

	return bHandled;
}

// --- SPIRITUAL FRAGMENT IMPLEMENTATION ---

bool USovereignItemFragment_Spiritual::OnConsumed_Implementation(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const
{
	if (!TargetCharacter || Quantity <= 0)
	{
		return false;
	}

	// Query QiComponent directly or via character vessel
	USovereignQiComponent* QiComp = TargetCharacter->FindComponentByClass<USovereignQiComponent>();
	if (!QiComp)
	{
		return false;
	}

	if (QiGive > 0.0f)
	{
		float TotalQi = QiGive * Quantity;
		QiComp->AbsorbQi(TotalQi, 100.0f);
		return true;
	}

	return false;
}

// --- EQUIPPABLE FRAGMENT IMPLEMENTATION ---

bool USovereignItemFragment_Equippable::OnEquipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const
{
	if (!TargetCharacter)
	{
		return false;
	}

	USkeletalMeshComponent* CharMesh = TargetCharacter->GetMesh();
	if (!CharMesh)
	{
		return false;
	}

	FName TargetSocket = !SocketName.IsNone() ? SocketName : SlotName;

	// 1. Attach Static Mesh if specified
	if (!StaticMeshAsset.IsNull())
	{
		UStaticMesh* LoadedStatic = StaticMeshAsset.LoadSynchronous();
		if (LoadedStatic)
		{
			FName CompName = MakeUniqueObjectName(TargetCharacter, UStaticMeshComponent::StaticClass(), FName(TEXT("EquippedStaticMesh")));
			UStaticMeshComponent* NewStaticComp = NewObject<UStaticMeshComponent>(TargetCharacter, CompName);
			if (NewStaticComp)
			{
				NewStaticComp->SetStaticMesh(LoadedStatic);
				if (!MaterialOverride.IsNull())
				{
					NewStaticComp->SetMaterial(0, MaterialOverride.LoadSynchronous());
				}
				NewStaticComp->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
				NewStaticComp->RegisterComponent();
				return true;
			}
		}
	}

	// 2. Attach Skeletal Mesh if specified
	if (!SkeletalMeshAsset.IsNull())
	{
		USkeletalMesh* LoadedSkel = SkeletalMeshAsset.LoadSynchronous();
		if (LoadedSkel)
		{
			FName CompName = MakeUniqueObjectName(TargetCharacter, USkeletalMeshComponent::StaticClass(), FName(TEXT("EquippedSkeletalMesh")));
			USkeletalMeshComponent* NewSkelComp = NewObject<USkeletalMeshComponent>(TargetCharacter, CompName);
			if (NewSkelComp)
			{
				NewSkelComp->SetSkeletalMeshAsset(LoadedSkel);
				if (!MaterialOverride.IsNull())
				{
					NewSkelComp->SetMaterial(0, MaterialOverride.LoadSynchronous());
				}
				NewSkelComp->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
				NewSkelComp->RegisterComponent();
				return true;
			}
		}
	}

	return true;
}

bool USovereignItemFragment_Equippable::OnUnequipped_Implementation(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const
{
	if (!TargetCharacter)
	{
		return false;
	}

	USkeletalMeshComponent* CharMesh = TargetCharacter->GetMesh();
	if (!CharMesh)
	{
		return false;
	}

	FName TargetSocket = !SocketName.IsNone() ? SocketName : SlotName;

	// Detach child components attached to TargetSocket
	TArray<USceneComponent*> Children = CharMesh->GetAttachChildren();
	for (USceneComponent* Child : Children)
	{
		if (Child && Child->GetAttachSocketName() == TargetSocket)
		{
			Child->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			Child->DestroyComponent();
		}
	}

	return true;
}
