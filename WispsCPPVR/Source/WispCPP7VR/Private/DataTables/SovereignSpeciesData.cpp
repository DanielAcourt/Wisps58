// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTables/SovereignSpeciesData.h"

void USovereignSpeciesData::GenerateGrowthStages()
{
    // Clamp target stage count between 1 and 16
    GrowthStageCount = FMath::Clamp(GrowthStageCount, 1, 16);

    FSovereignGrowthStage Stage0 = (GrowthStages.Num() > 0) ? GrowthStages[0] : FSovereignGrowthStage();

    GrowthStages.SetNum(GrowthStageCount);
    GrowthStages[0] = Stage0;

    for (int32 StageIdx = 0; StageIdx < GrowthStages.Num(); ++StageIdx)
    {
        FSovereignGrowthStage& Stage = GrowthStages[StageIdx];

        float Multiplier = 1.0f;

        switch (GrowthCurveType)
        {
        case ESovereignGrowthCurve::Exponential:
            Multiplier = FMath::Pow(GrowthScaleFactor, static_cast<float>(StageIdx));
            break;

        case ESovereignGrowthCurve::Linear:
            Multiplier = 1.0f + (static_cast<float>(StageIdx) * (GrowthScaleFactor - 1.0f));
            break;

        case ESovereignGrowthCurve::FrontLoaded:
            // S-Curve logarithmic front-loading: rapid initial gains that taper off
            Multiplier = 1.0f + ((GrowthScaleFactor - 1.0f) * FMath::Log2(static_cast<float>(StageIdx) + 1.0f));
            break;

        case ESovereignGrowthCurve::Flat:
        default:
            Multiplier = 1.0f;
            break;
        }

        Stage.BaseStrength     = FMath::RoundToInt32(static_cast<float>(Stage0.BaseStrength) * Multiplier);
        Stage.BaseDexterity    = FMath::RoundToInt32(static_cast<float>(Stage0.BaseDexterity) * Multiplier);
        Stage.BaseConstitution = FMath::RoundToInt32(static_cast<float>(Stage0.BaseConstitution) * Multiplier);
        Stage.BaseIntelligence = FMath::RoundToInt32(static_cast<float>(Stage0.BaseIntelligence) * Multiplier);
        Stage.BaseWisdom       = FMath::RoundToInt32(static_cast<float>(Stage0.BaseWisdom) * Multiplier);
        Stage.BaseCharisma     = FMath::RoundToInt32(static_cast<float>(Stage0.BaseCharisma) * Multiplier);
        Stage.BaseLuck         = FMath::RoundToInt32(static_cast<float>(Stage0.BaseLuck) * Multiplier);
        Stage.MetabolicRate    = Stage0.MetabolicRate * Multiplier;
        Stage.VisualScale      = Stage0.VisualScale * (1.0f + (static_cast<float>(StageIdx) * 0.15f));
    }
}

#if WITH_EDITOR
void USovereignSpeciesData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(USovereignSpeciesData, GrowthCurveType) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(USovereignSpeciesData, GrowthScaleFactor) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(USovereignSpeciesData, GrowthStageCount))
    {
        GenerateGrowthStages();
    }
}
#endif
