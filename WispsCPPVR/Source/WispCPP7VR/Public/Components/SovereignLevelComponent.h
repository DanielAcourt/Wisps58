// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-15

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "DataTables/SovereignDataTypes.h"
#include "SovereignLevelComponent.generated.h"

class USovereignAttributeComponent;

/**
 * USovereignLevelComponent
 * Handles active entity training focus, stat experience accumulators,
 * and level-up rollover updates in sync with USovereignAttributeComponent.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignLevelComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignLevelComponent();

	/** Custom UI Inspector UserWidget for Leveling/Training inspection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling")
	TSubclassOf<UUserWidget> InspectorWidgetClass;

	/** --- 1. TRAINING FOCUS & RATES --- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling")
	ESovereignTrainingFocus ActiveTrainingFocus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling")
	float TrainingRateMultiplier;

	/** --- 2. EXPERIENCE ACCUMULATORS --- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double StrengthXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double DexterityXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double ConstitutionXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double IntelligenceXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double WisdomXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double CharismaXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Leveling|XP")
	double LuckXP;

	/** --- 3. PUBLIC API --- */

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Leveling")
	void SetTrainingFocus(ESovereignTrainingFocus NewFocus);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Leveling")
	void DetermineDefaultTrainingFocus();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Leveling")
	void AddAttributeXP(ESovereignTrainingFocus AttributeFocus, double Amount);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Leveling")
	void ConsumeItemXP(const FSovereignItemRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Leveling")
	void ProcessTrainingProgress(float DeltaTime);

	/** ISovereignBrokerInterface Implementation */
	virtual void OnSave(TSharedPtr<FJsonObject>& OutJson) override;
	virtual void OnLoad(const TSharedPtr<FJsonObject>& InJson) override;
	virtual void OnProcessData(const TMap<FString, FString>& Data) override;

protected:
	virtual void BeginPlay() override;

private:
	void EvaluateXPRollover(double& XPAccumulator, int32& TargetStat);
};
