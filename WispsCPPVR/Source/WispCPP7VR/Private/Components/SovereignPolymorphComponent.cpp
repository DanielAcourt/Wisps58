// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Components/SovereignPolymorphComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Dom/JsonObject.h"

USovereignPolymorphComponent::USovereignPolymorphComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentForm = ESovereignVesselForm::TrueForm;
	RequiredGrowthStage = 7;
	HumanoidCapsuleRadius = 34.0f;
	HumanoidCapsuleHalfHeight = 88.0f;
	HumanoidWalkSpeed = 600.0f;
}

void USovereignPolymorphComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->RegisterBroker(this);
		}
	}

	CacheTrueFormDefaults();
}

ASovereignBaseCharacter* USovereignPolymorphComponent::GetOwnerCharacter() const
{
	return Cast<ASovereignBaseCharacter>(GetOwner());
}

void USovereignPolymorphComponent::CacheTrueFormDefaults()
{
	ASovereignBaseCharacter* Character = GetOwnerCharacter();
	if (!Character) return;

	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		CachedTrueMesh = TSoftObjectPtr<USkeletalMesh>(Mesh->GetSkeletalMeshAsset());
		if (Mesh->GetNumMaterials() > 0)
		{
			CachedTrueMaterial = TSoftObjectPtr<UMaterialInterface>(Mesh->GetMaterial(0));
		}
		CachedTrueAnimClass = Mesh->GetAnimClass();
	}

	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		CachedTrueCapsuleRadius = Capsule->GetUnscaledCapsuleRadius();
		CachedTrueCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	}

	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		CachedTrueMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		CachedTrueMaxFlySpeed = MoveComp->MaxFlySpeed;
		CachedTrueMovementMode = static_cast<uint8>(MoveComp->MovementMode);
	}

	bTrueDefaultsCached = true;
}

bool USovereignPolymorphComponent::CanPolymorph() const
{
	ASovereignBaseCharacter* Character = GetOwnerCharacter();
	if (!Character) return false;

	// Require adult growth stage threshold
	return Character->GetCurrentGrowthStage() >= RequiredGrowthStage;
}

bool USovereignPolymorphComponent::TogglePolymorph()
{
	ESovereignVesselForm TargetForm = (CurrentForm == ESovereignVesselForm::TrueForm)
		? ESovereignVesselForm::HumanoidVisage
		: ESovereignVesselForm::TrueForm;

	return SetVesselForm(TargetForm);
}

bool USovereignPolymorphComponent::SetVesselForm(ESovereignVesselForm NewForm)
{
	if (NewForm == CurrentForm)
	{
		return true;
	}

	// Gate transformation to valid conditions when switching to Humanoid
	if (NewForm == ESovereignVesselForm::HumanoidVisage && !CanPolymorph())
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignPolymorphComponent] Cannot polymorph: entity has not reached adult maturity stage %d."), RequiredGrowthStage);
		return false;
	}

	ASovereignBaseCharacter* Character = GetOwnerCharacter();
	if (!Character) return false;

	if (!bTrueDefaultsCached)
	{
		CacheTrueFormDefaults();
	}

	USkeletalMeshComponent* CharMesh = Character->GetMesh();
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();

	if (NewForm == ESovereignVesselForm::HumanoidVisage)
	{
		// --- 1. APPLY HUMANOID VISAGE ---
		if (CharMesh)
		{
			if (!HumanoidSkeletalMesh.IsNull())
			{
				USkeletalMesh* LoadedSkel = HumanoidSkeletalMesh.LoadSynchronous();
				if (LoadedSkel)
				{
					CharMesh->SetSkeletalMeshAsset(LoadedSkel);
				}
			}

			if (!HumanoidMaterial.IsNull())
			{
				UMaterialInterface* LoadedMat = HumanoidMaterial.LoadSynchronous();
				if (LoadedMat)
				{
					CharMesh->SetMaterial(0, LoadedMat);
				}
			}

			if (HumanoidAnimClass)
			{
				CharMesh->SetAnimInstanceClass(HumanoidAnimClass);
			}

			// Reset visual scale to 1.0 for standard human proportions
			CharMesh->SetRelativeScale3D(FVector(1.0f));
		}

		// --- 2. RESIZE CAPSULE TO HUMAN DIMENSIONS ---
		if (Capsule)
		{
			const bool bUpdateOverlaps = (Character->GetWorld() != nullptr);
			Capsule->SetCapsuleSize(HumanoidCapsuleRadius, HumanoidCapsuleHalfHeight, bUpdateOverlaps);
		}

		// --- 3. ADJUST MOVEMENT TO BIPEDAL WALKING ---
		if (MoveComp)
		{
			MoveComp->SetMovementMode(MOVE_Walking);
			MoveComp->MaxWalkSpeed = HumanoidWalkSpeed;
		}

		CurrentForm = ESovereignVesselForm::HumanoidVisage;
		UE_LOG(LogTemp, Log, TEXT("[%s] Polymorphed into Humanoid Visage."), *Character->GetName());
	}
	else
	{
		// --- 1. RESTORE TRUE FORM (DRAGON / BEAST) ---
		if (CharMesh)
		{
			if (!CachedTrueMesh.IsNull())
			{
				USkeletalMesh* LoadedSkel = CachedTrueMesh.LoadSynchronous();
				if (LoadedSkel)
				{
					CharMesh->SetSkeletalMeshAsset(LoadedSkel);
				}
			}

			if (!CachedTrueMaterial.IsNull())
			{
				UMaterialInterface* LoadedMat = CachedTrueMaterial.LoadSynchronous();
				if (LoadedMat)
				{
					CharMesh->SetMaterial(0, LoadedMat);
				}
			}

			if (CachedTrueAnimClass)
			{
				CharMesh->SetAnimInstanceClass(CachedTrueAnimClass);
			}

			// Restore draconic growth stage visuals and scale
			Character->RefreshVisuals();
		}

		// --- 2. RESTORE DRACONIC CAPSULE ---
		if (Capsule)
		{
			const bool bUpdateOverlaps = (Character->GetWorld() != nullptr);
			Capsule->SetCapsuleSize(CachedTrueCapsuleRadius, CachedTrueCapsuleHalfHeight, bUpdateOverlaps);
		}

		// --- 3. RESTORE MOVEMENT (FLYING/SPEED) ---
		if (MoveComp)
		{
			MoveComp->SetMovementMode(static_cast<EMovementMode>(CachedTrueMovementMode));
			MoveComp->MaxWalkSpeed = CachedTrueMaxWalkSpeed;
			MoveComp->MaxFlySpeed = CachedTrueMaxFlySpeed;
		}

		CurrentForm = ESovereignVesselForm::TrueForm;
		UE_LOG(LogTemp, Log, TEXT("[%s] Reverted to True Form."), *Character->GetName());
	}

	OnVesselFormChanged.Broadcast(CurrentForm);
	return true;
}

void USovereignPolymorphComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid()) return;

	TSharedPtr<FJsonObject> PolymorphObj = MakeShared<FJsonObject>();
	PolymorphObj->SetNumberField(TEXT("CurrentForm"), static_cast<int32>(CurrentForm));
	PolymorphObj->SetNumberField(TEXT("RequiredGrowthStage"), RequiredGrowthStage);

	OutJson->SetObjectField(TEXT("Sovereign.Polymorph"), PolymorphObj);
}

void USovereignPolymorphComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid() || !InJson->HasField(TEXT("Sovereign.Polymorph"))) return;

	TSharedPtr<FJsonObject> PolymorphObj = InJson->GetObjectField(TEXT("Sovereign.Polymorph"));
	if (!PolymorphObj.IsValid()) return;

	double FormVal;
	if (PolymorphObj->TryGetNumberField(TEXT("CurrentForm"), FormVal))
	{
		ESovereignVesselForm LoadedForm = static_cast<ESovereignVesselForm>((uint8)FormVal);
		SetVesselForm(LoadedForm);
	}
}

void USovereignPolymorphComponent::OnProcessData(const TMap<FString, FString>& Data)
{
}
