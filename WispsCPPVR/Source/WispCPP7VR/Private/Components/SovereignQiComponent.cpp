// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28

#include "Components/SovereignQiComponent.h"
#include "Components/SovereignLevelComponent.h"
#include "Entities/SovereignBaseEntity.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Dom/JsonObject.h"

USovereignQiComponent::USovereignQiComponent()
	: AlignmentInfluence(100.0f)
	, MagicInfluence(0.0f)
	, CurrentQi(0.0f)
	, MaxQiCapacity(100.0f)
	, QiPurity(1.0f)
{
	AlignmentSocket = ESovereignElement::Grey;
	MagicSocket = ESovereignElement::None;
	PrimaryComponentTick.TickInterval = 0.1f;

	static ConstructorHelpers::FClassFinder<UUserWidget> QiWidgetFinder(TEXT("/Game/Assets/UI/WBP_QI.WBP_QI_C"));
	if (QiWidgetFinder.Succeeded())
	{
		InspectorWidgetClass = QiWidgetFinder.Class;
	}
}

void USovereignQiComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->RegisterBroker(this);
		}

		SyncQiAuraDefaultsFromEntity();
	}
}

void USovereignQiComponent::SyncQiAuraDefaultsFromEntity()
{
	USovereignSpeciesData* SpeciesData = nullptr;

	if (ASovereignBaseEntity* OwnerEntity = Cast<ASovereignBaseEntity>(GetOwner()))
	{
		SpeciesData = OwnerEntity->GetSpeciesData();
	}
	else if (ASovereignBaseCharacter* OwnerChar = Cast<ASovereignBaseCharacter>(GetOwner()))
	{
		SpeciesData = OwnerChar->GetSpeciesData();
	}

	if (SpeciesData)
	{
		QiGive = SpeciesData->QiGive;
		QiTake = SpeciesData->QiTake;
		QiAuraRadius = SpeciesData->QiAuraRadius;

		UE_LOG(LogTemp, Log, TEXT("Sovereign: %s synced Qi Aura defaults from Species %s -> Give: %.1f | Take: %.1f | Radius: %.1f"),
			*GetOwner()->GetName(), *SpeciesData->SpeciesName.ToString(), QiGive, QiTake, QiAuraRadius);
	}
}

void USovereignQiComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		if (USovereignSaveableEntityComponent* SoulHub = Owner->FindComponentByClass<USovereignSaveableEntityComponent>())
		{
			SoulHub->UnregisterBroker(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void USovereignQiComponent::AbsorbQi(float Amount, float SourcePurity)
{
	if (Amount <= 0.0f) return;
	float NewTotalQi = CurrentQi + Amount;
	if (NewTotalQi > 0.0f) QiPurity = ((CurrentQi * QiPurity) + (Amount * SourcePurity)) / NewTotalQi;
	CurrentQi = FMath::Min(NewTotalQi, MaxQiCapacity);
	TotalQiAccumulated += Amount;

	if (TotalQiAccumulated >= 1000.0f)
	{
		if (ASovereignBaseEntity* Owner = Cast<ASovereignBaseEntity>(GetOwner())) Owner->Evolve();
	}
}

bool USovereignQiComponent::SpendQi(float Amount)
{
	float AdjustedCost = Amount * (2.0f - QiPurity);
	if (CurrentQi >= AdjustedCost)
	{
		CurrentQi -= AdjustedCost;
		return true;
	}
	return false;
}

float USovereignQiComponent::GetTotalQiRegenRate(int32 WisdomStat) const
{
	float BaseRegen = (static_cast<float>(WisdomStat) * 0.1f) * QiPurity;
	return BaseRegen + EnvironmentalQiRegenBonus;
}

void USovereignQiComponent::ProcessQiFlow(float DeltaTime, int32 WisdomStat)
{
	if (QiPurity < 1.0f) QiPurity += 0.00001f * DeltaTime;
	float TotalRegenRate = GetTotalQiRegenRate(WisdomStat);
	if (CurrentQi < MaxQiCapacity) CurrentQi = FMath::Min(CurrentQi + (TotalRegenRate * DeltaTime), MaxQiCapacity);
}

void USovereignQiComponent::ProcessEnvironmentalQiAura(float HeartbeatInterval)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || QiAuraRadius <= 0.0f || QiTake <= 0.0f)
	{
		EnvironmentalQiRegenBonus = 0.0f;
		return;
	}

	// Gate spatial aura sweeps strictly to entities with a USovereignLevelComponent actively training in Qi, Wisdom, or Intelligence stances
	USovereignLevelComponent* LevelComp = OwnerActor->FindComponentByClass<USovereignLevelComponent>();
	if (!LevelComp)
	{
		EnvironmentalQiRegenBonus = 0.0f;
		return;
	}

	ESovereignTrainingFocus Focus = LevelComp->ActiveTrainingFocus;
	if (Focus != ESovereignTrainingFocus::Qi &&
		Focus != ESovereignTrainingFocus::Wisdom &&
		Focus != ESovereignTrainingFocus::Intelligence)
	{
		EnvironmentalQiRegenBonus = 0.0f;
		return;
	}

	FVector Origin = OwnerActor->GetActorLocation();
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FOverlapResult> Overlaps;
	bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(QiAuraRadius),
		QueryParams
	);

	if (!bHit)
	{
		EnvironmentalQiRegenBonus = 0.0f;
		return;
	}

	float TotalRatePerSecond = 0.0f;
	TSet<AActor*> ProcessedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OtherActor = Overlap.GetActor();
		if (!OtherActor || ProcessedActors.Contains(OtherActor))
		{
			continue;
		}

		ProcessedActors.Add(OtherActor);

		USovereignQiComponent* OtherQiComp = OtherActor->FindComponentByClass<USovereignQiComponent>();
		if (OtherQiComp && OtherQiComp->bIsQiSource && OtherQiComp->QiGive > 0.0f)
		{
			float Distance = FVector::Dist(Origin, OtherActor->GetActorLocation());
			float DistanceFactor = FMath::Clamp(1.0f - (Distance / QiAuraRadius), 0.0f, 1.0f);
			float RateFromSource = FMath::Min(OtherQiComp->QiGive, QiTake) * DistanceFactor;
			TotalRatePerSecond += RateFromSource;
		}
	}

	EnvironmentalQiRegenBonus = TotalRatePerSecond;
}

void USovereignQiComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	TSharedPtr<FJsonObject> QiObj = MakeShareable(new FJsonObject());

	QiObj->SetStringField(TEXT("AlignmentSocket"), UEnum::GetValueAsString(AlignmentSocket));
	QiObj->SetNumberField(TEXT("AlignmentInfluence"), AlignmentInfluence);
	QiObj->SetStringField(TEXT("MagicSocket"), UEnum::GetValueAsString(MagicSocket));
	QiObj->SetNumberField(TEXT("MagicInfluence"), MagicInfluence);

	QiObj->SetNumberField(TEXT("CurrentQi"), CurrentQi);
	QiObj->SetNumberField(TEXT("MaxQiCapacity"), MaxQiCapacity);
	QiObj->SetNumberField(TEXT("QiPurity"), QiPurity);
	QiObj->SetNumberField(TEXT("TotalQiAccumulated"), TotalQiAccumulated);

	// Universal Domain Rune and Territory parameters [B-036]
	QiObj->SetBoolField(TEXT("bIsDomainOwner"), bIsDomainOwner);
	QiObj->SetStringField(TEXT("DomainRuneTier"), DomainRuneTier);
	QiObj->SetNumberField(TEXT("DomainResonanceRadius"), DomainResonanceRadius);
	QiObj->SetNumberField(TEXT("DomainNetworkSaturation"), DomainNetworkSaturation);

	// Environmental Qi Aura parameters [B-059]
	QiObj->SetNumberField(TEXT("QiAuraRadius"), QiAuraRadius);
	QiObj->SetBoolField(TEXT("bIsQiSource"), bIsQiSource);
	QiObj->SetNumberField(TEXT("QiGive"), QiGive);
	QiObj->SetNumberField(TEXT("QiTake"), QiTake);

	TSharedPtr<FJsonObject> ResObj = MakeShareable(new FJsonObject());
	for (const auto& Elem : ElementalResonance)
	{
		ResObj->SetNumberField(Elem.Key.ToString(), Elem.Value);
	}
	QiObj->SetObjectField(TEXT("Resonance"), ResObj);

	OutJson->SetObjectField(TEXT("Qi"), QiObj);
}

void USovereignQiComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	const TSharedPtr<FJsonObject>* QiObj;
	if (InJson->TryGetObjectField(TEXT("Qi"), QiObj))
	{
		FString AlignStr;
		if ((*QiObj)->TryGetStringField(TEXT("AlignmentSocket"), AlignStr))
		{
			UEnum* EnumPtr = StaticEnum<ESovereignElement>();
			if (EnumPtr)
			{
				int64 Val = EnumPtr->GetValueByNameString(AlignStr);
				if (Val != INDEX_NONE) AlignmentSocket = static_cast<ESovereignElement>(Val);
			}
		}

		double TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("AlignmentInfluence"), TempVal)) AlignmentInfluence = (float)TempVal;

		FString MagicStr;
		if ((*QiObj)->TryGetStringField(TEXT("MagicSocket"), MagicStr))
		{
			UEnum* EnumPtr = StaticEnum<ESovereignElement>();
			if (EnumPtr)
			{
				int64 Val = EnumPtr->GetValueByNameString(MagicStr);
				if (Val != INDEX_NONE) MagicSocket = static_cast<ESovereignElement>(Val);
			}
		}
		if ((*QiObj)->TryGetNumberField(TEXT("MagicInfluence"), TempVal)) MagicInfluence = (float)TempVal;

		if ((*QiObj)->TryGetNumberField(TEXT("CurrentQi"), TempVal)) CurrentQi = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("MaxQiCapacity"), TempVal)) MaxQiCapacity = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("QiPurity"), TempVal)) QiPurity = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("TotalQiAccumulated"), TempVal)) TotalQiAccumulated = (float)TempVal;

		// Universal Domain Rune and Territory parameters [B-036]
		(*QiObj)->TryGetBoolField(TEXT("bIsDomainOwner"), bIsDomainOwner);
		(*QiObj)->TryGetStringField(TEXT("DomainRuneTier"), DomainRuneTier);
		if ((*QiObj)->TryGetNumberField(TEXT("DomainResonanceRadius"), TempVal)) DomainResonanceRadius = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("DomainNetworkSaturation"), TempVal)) DomainNetworkSaturation = (float)TempVal;

		// Environmental Qi Aura parameters [B-059]
		if ((*QiObj)->TryGetNumberField(TEXT("QiAuraRadius"), TempVal)) QiAuraRadius = (float)TempVal;
		(*QiObj)->TryGetBoolField(TEXT("bIsQiSource"), bIsQiSource);
		if ((*QiObj)->TryGetNumberField(TEXT("QiGive"), TempVal)) QiGive = (float)TempVal;
		if ((*QiObj)->TryGetNumberField(TEXT("QiTake"), TempVal)) QiTake = (float)TempVal;

		const TSharedPtr<FJsonObject>* ResObj;
		if ((*QiObj)->TryGetObjectField(TEXT("Resonance"), ResObj))
		{
			ElementalResonance.Empty();
			for (auto& Elem : (*ResObj)->Values)
			{
				// Explicitly convert Elem.Key to FString for Unreal 5.8 compatibility
				FString KeyStr = FString(Elem.Key);
				ElementalResonance.Add(FName(*KeyStr), (float)Elem.Value->AsNumber());
			}
		}
	}
}

void USovereignQiComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// Process Qi data
}
