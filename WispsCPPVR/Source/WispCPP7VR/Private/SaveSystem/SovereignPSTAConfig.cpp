// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "SaveSystem/SovereignPSTAConfig.h"

USovereignPSTAConfig::USovereignPSTAConfig()
{
    // Default weights
    DimensionWeights.Add(EPSTADimension::Psychological, 0.25f);
    DimensionWeights.Add(EPSTADimension::Social, 0.25f);
    DimensionWeights.Add(EPSTADimension::Technical, 0.25f);
    DimensionWeights.Add(EPSTADimension::Administrative, 0.25f);

    // Default Failure Thresholds (tau_fail,i = 0.3f)
    DimensionFailureThresholds.Add(EPSTADimension::Psychological, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Social, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Technical, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Administrative, 0.3f);

    // Default Caution Thresholds (tau_caut,i = 0.7f)
    DimensionCautionThresholds.Add(EPSTADimension::Psychological, 0.7f);
    DimensionCautionThresholds.Add(EPSTADimension::Social, 0.7f);
    DimensionCautionThresholds.Add(EPSTADimension::Technical, 0.7f);
    DimensionCautionThresholds.Add(EPSTADimension::Administrative, 0.7f);

    // Default Max Risk Velocities (V_max,i = 0.5f health points / sec)
    DimensionMaxVelocities.Add(EPSTADimension::Psychological, 0.5f);
    DimensionMaxVelocities.Add(EPSTADimension::Social, 0.5f);
    DimensionMaxVelocities.Add(EPSTADimension::Technical, 0.5f);
    DimensionMaxVelocities.Add(EPSTADimension::Administrative, 0.5f);
}

float USovereignPSTAConfig::NormalizeValue(const FPSTATagMapping& Mapping, float RawValue) const
{
    float Range = Mapping.RangeMax - Mapping.RangeMin;
    if (FMath::IsNearlyZero(Range))
    {
        return RawValue >= Mapping.RangeMax ? 1.0f : 0.0f;
    }

    float Normalized = (RawValue - Mapping.RangeMin) / Range;
    return FMath::Clamp(Normalized, 0.0f, 1.0f);
}

const FPSTATagMapping* USovereignPSTAConfig::GetMappingForTag(const FString& TagKey) const
{
    // Lazy build if empty (or if called in-game)
    if (CachedMappings.Num() == 0 && TagMappings.Num() > 0)
    {
        const_cast<USovereignPSTAConfig*>(this)->RebuildCache();
    }
    return CachedMappings.Find(TagKey);
}

EPSTABaseBit USovereignPSTAConfig::EvaluateBaseBit(EPSTADimension Dimension, float Di, float RiskVelocity, float SymmetryDelta, bool bIsVetted) const
{
    // Void Safety Rule: Untracked or unvetted reality collapses deterministically to Void (\perp)
    if (!bIsVetted)
    {
        return EPSTABaseBit::Void;
    }

    float TauFail = DimensionFailureThresholds.Contains(Dimension) ? DimensionFailureThresholds[Dimension] : 0.3f;
    float TauCaut = DimensionCautionThresholds.Contains(Dimension) ? DimensionCautionThresholds[Dimension] : 0.7f;
    float VMax = DimensionMaxVelocities.Contains(Dimension) ? DimensionMaxVelocities[Dimension] : 0.5f;

    // Hard Breached Check: Di below tau_fail OR Symmetry Delta exceeds Kernel Severance Threshold
    if (Di < TauFail || SymmetryDelta > KernelSymmetryThreshold)
    {
        return EPSTABaseBit::Unsafe;
    }

    // Nominal Safe Check: Di >= tau_caut AND Risk Velocity within Permissible Rate of Decay
    if (Di >= TauCaut && FMath::Abs(RiskVelocity) <= VMax)
    {
        return EPSTABaseBit::Nominal;
    }

    // Unsafe default if caution or rate-of-decay bounds are violated
    return EPSTABaseBit::Unsafe;
}

float USovereignPSTAConfig::EvaluateLeontiefGuard(EPSTADimension Dimension, float Di, EPSTABaseBit BaseBit) const
{
    // Non-Compensatory Leontief Guard: \theta_i = step(D_i - \tau_{fail,i}) * I(B_i != \perp)
    if (BaseBit == EPSTABaseBit::Void)
    {
        return 0.0f; // Indicator function I(B_i != \perp) = 0
    }

    float TauFail = DimensionFailureThresholds.Contains(Dimension) ? DimensionFailureThresholds[Dimension] : 0.3f;
    return (Di >= TauFail) ? 1.0f : 0.0f;
}

#if WITH_EDITOR
void USovereignPSTAConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    RebuildCache();
}
#endif

void USovereignPSTAConfig::RebuildCache()
{
    CachedMappings.Empty();
    for (const FPSTATagMapping& Mapping : TagMappings)
    {
        CachedMappings.Add(Mapping.TagKey, Mapping);
    }
}
