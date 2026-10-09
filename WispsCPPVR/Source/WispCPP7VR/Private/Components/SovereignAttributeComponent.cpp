// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SovereignAttributeComponent.h"
#include "Entities/SovereignBaseEntity.h"
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Components/SovereignControllerComponent.h"
#include "Dom/JsonObject.h"
#include "GameFramework/Actor.h"

USovereignAttributeComponent::USovereignAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default internal values to prevent crashes before sync
	Strength = 10;
	Dexterity = 10;

	Constitution = 10;
	Intelligence = 10;

	Wisdom = 10;
	Charisma = 10;

	Luck = 10;
	ArmourClass = 10;

	CurrentHealth = 1.0f;
	
	//moved to bio component
	//CurrentStamina = 1.0f;

	//DefaultDifficalty = 1.0f
}

void USovereignAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// Automatically sync when the game starts
	SyncStatsFromEntity();

	// Start the vessel at full health/stamina based on its new stats
	CurrentHealth = GetMaxHealth();
	MaxHealth = GetMaxHealth();
}

void USovereignAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void USovereignAttributeComponent::SyncStatsFromEntity()
{
	USovereignSpeciesData* SpeciesData = nullptr;
	int32 StageIdx = 0;

	if (ASovereignBaseEntity* OwnerEntity = Cast<ASovereignBaseEntity>(GetOwner()))
	{
		SpeciesData = OwnerEntity->GetSpeciesData();
		StageIdx = OwnerEntity->GetCurrentGrowthStage();
	}

	if (SpeciesData && SpeciesData->GrowthStages.IsValidIndex(StageIdx))
	{
		const FSovereignGrowthStage& Stage = SpeciesData->GrowthStages[StageIdx];
		Strength = Stage.BaseStrength;
		Dexterity = Stage.BaseDexterity;
		Constitution = Stage.BaseConstitution;
		Intelligence = Stage.BaseIntelligence;
		Wisdom = Stage.BaseWisdom;
		Charisma = Stage.BaseCharisma;
		Luck = Stage.BaseLuck;

		UE_LOG(LogTemp, Log, TEXT("Sovereign: %s synced D&D attributes from Species %s (Stage %d) -> STR:%d DEX:%d CON:%d INT:%d WIS:%d CHA:%d LCK:%d"),
			*GetOwner()->GetName(), *SpeciesData->SpeciesName.ToString(), StageIdx,
			Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma, Luck);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Sovereign: %s synced default attributes."), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

float USovereignAttributeComponent::GetMaxHealth() const
{
	// D&D Formula: Constitution acts as the multiplier.
	// We use 10.0f as a base, so 10 Con = 100 HP.
	return static_cast<float>(Constitution) * 10.0f;


	// Implementation of your "Version 3.0" Idea:
		// We use the floor of Constitution (the Level) plus the fractional Experience.
	//float BaseValue = 100.0f; // Every Sovereign entity starts with a 100 HP vessel
	//float GrowthScaling = 15.0f; // HP gained per point of Constitution

	//return BaseValue + (static_cast<float>(Constitution) * GrowthScaling);
}

/*
float USovereignAttributeComponent::GetMaxStamina() const
{
	// Stamina uses both Agility (Dex) and Toughness (Con)
	//float BaseValue = 50.0f;
	//return BaseValue;
	//+ (static_cast<float>(Constitution + Dexterity) * 5.0f);

	return 1;
}
*/
void USovereignAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1. GET POSSESSION MULTIPLIER
	float PossessionBonus = 1.0f;
	ASovereignBaseEntity* OwnerEntity = Cast<ASovereignBaseEntity>(GetOwner());

	if (OwnerEntity)
	{
		// Check the Brain component we built earlier
		auto ControlComp = OwnerEntity->FindComponentByClass<USovereignControllerComponent>();
		if (ControlComp && ControlComp->bIsPlayerPossessed)
		{
			PossessionBonus = 1.5f; // 50% faster regen and slower decay while player is inside
		}
	}
}


//Moving to biocomponent
/*
	// 2. DECAY VITALS (Modified by Possession and Constitution)
	// High Constitution makes you hardy; Possession focus makes you "forget" hunger briefly
	float DecayMult = 1.0f / (PossessionBonus * (1.0f + (Constitution * 0.01f)));

	Hunger = FMath::Max(0.0f, Hunger - (0.5f * DecayMult * DeltaTime));
	Hydration = FMath::Max(0.0f, Hydration - (0.8f * DecayMult * DeltaTime));

	// Fatigue builds up faster if you are low on hydration
	float FatigueRate = (Hydration < 20.0f) ? 2.0f : 1.0f;
	Tiredness = FMath::Min(100.0f, Tiredness + (0.2f * FatigueRate * DeltaTime));

	// 3. PASSIVE REGENERATION LOGIC
	// We only regenerate if we aren't starving or dehydrated!
	bool bCanRegen = (Hunger > 10.0f && Hydration > 10.0f);

	// Wisdom is the base, Possession is the spark
	float RegenModifier = (static_cast<float>(Wisdom) * 0.05f) * PossessionBonus;

	if (bCanRegen)
	{
		// Health Regen
		if (CurrentHealth < GetMaxHealth())
		{
			CurrentHealth = FMath::Min(CurrentHealth + (RegenModifier * DeltaTime), GetMaxHealth());
		}

		// Stamina Regen (Faster than health)
		if (CurrentStamina < GetMaxStamina())
		{
			float StaminaMult = (Tiredness > 80.0f) ? 0.2f : 1.0f; // Exhaustion penalty
			CurrentStamina = FMath::Min(CurrentStamina + (RegenModifier * 2.5f * StaminaMult * DeltaTime), GetMaxStamina());
		}
	}

	// 4. APPLY CRITICAL CONSEQUENCES
	if (Hunger <= 0.0f || Hydration <= 0.0f)
	{
		// Starvation damage ignores Possession armor—it's a physical reality
		CurrentHealth -= (1.0f * DeltaTime);

		if (CurrentHealth <= 0.0f)
		{
			// Handle Death logic here or via delegate
		}
	}
}
*/


void USovereignAttributeComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	TSharedPtr<FJsonObject> AttrObj = MakeShareable(new FJsonObject());

	AttrObj->SetNumberField(TEXT("Strength"), Strength);
	AttrObj->SetNumberField(TEXT("StrengthExperience"), StrengthExperience);

	AttrObj->SetNumberField(TEXT("Dexterity"), Dexterity);
	AttrObj->SetNumberField(TEXT("DexterityExperience"), DexterityExperience);

	AttrObj->SetNumberField(TEXT("Constitution"), Constitution);
	AttrObj->SetNumberField(TEXT("ConstitutionExperience"), ConstitutionExperience);

	AttrObj->SetNumberField(TEXT("Intelligence"), Intelligence);
	AttrObj->SetNumberField(TEXT("IntelligenceExperience"), IntelligenceExperience);

	AttrObj->SetNumberField(TEXT("Wisdom"), Wisdom);
	AttrObj->SetNumberField(TEXT("WisdomExperience"), WisdomExperience);

	AttrObj->SetNumberField(TEXT("Charisma"), Charisma);
	AttrObj->SetNumberField(TEXT("CharismaExperience"), CharismaExperience);

	AttrObj->SetNumberField(TEXT("Luck"), Luck);
	AttrObj->SetNumberField(TEXT("LuckExperience"), LuckExperience);

	AttrObj->SetNumberField(TEXT("ArmourClass"), ArmourClass);

	AttrObj->SetNumberField(TEXT("CurrentHealth"), CurrentHealth);
	AttrObj->SetNumberField(TEXT("MaxHealth"), MaxHealth);
	AttrObj->SetNumberField(TEXT("CurrentStamina"), CurrentStamina);
	AttrObj->SetNumberField(TEXT("MaxStamina"), MaxStamina);

	AttrObj->SetNumberField(TEXT("PhysicalResistance"), PhysicalResistance);
	AttrObj->SetNumberField(TEXT("MagicalResistance"), MagicalResistance);
	AttrObj->SetNumberField(TEXT("MentalResistance"), MentalResistance);
	AttrObj->SetNumberField(TEXT("PoisonResistance"), PoisonResistance);
	AttrObj->SetNumberField(TEXT("SlowResistance"), SlowResistance);

	OutJson->SetObjectField(TEXT("Sovereign.Attributes"), AttrObj);
}

void USovereignAttributeComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	const TSharedPtr<FJsonObject>* AttrObj;
	if (InJson->TryGetObjectField(TEXT("Sovereign.Attributes"), AttrObj))
	{
		double Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("Strength"), Val)) Strength = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("StrengthExperience"), Val)) StrengthExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Dexterity"), Val)) Dexterity = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("DexterityExperience"), Val)) DexterityExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Constitution"), Val)) Constitution = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("ConstitutionExperience"), Val)) ConstitutionExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Intelligence"), Val)) Intelligence = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("IntelligenceExperience"), Val)) IntelligenceExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Wisdom"), Val)) Wisdom = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("WisdomExperience"), Val)) WisdomExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Charisma"), Val)) Charisma = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("CharismaExperience"), Val)) CharismaExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("Luck"), Val)) Luck = (int32)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("LuckExperience"), Val)) LuckExperience = Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("ArmourClass"), Val)) ArmourClass = (int32)Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("CurrentHealth"), Val)) CurrentHealth = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("MaxHealth"), Val)) MaxHealth = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("CurrentStamina"), Val)) CurrentStamina = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("MaxStamina"), Val)) MaxStamina = (float)Val;

		if ((*AttrObj)->TryGetNumberField(TEXT("PhysicalResistance"), Val)) PhysicalResistance = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("MagicalResistance"), Val)) MagicalResistance = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("MentalResistance"), Val)) MentalResistance = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("PoisonResistance"), Val)) PoisonResistance = (float)Val;
		if ((*AttrObj)->TryGetNumberField(TEXT("SlowResistance"), Val)) SlowResistance = (float)Val;
	}
}

void USovereignAttributeComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// Process attribute data
}