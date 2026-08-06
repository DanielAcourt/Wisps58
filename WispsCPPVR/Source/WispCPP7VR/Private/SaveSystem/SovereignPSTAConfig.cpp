// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "SaveSystem/SovereignPSTAConfig.h"

USovereignPSTAConfig::USovereignPSTAConfig()
{
    // Default weights
    DimensionWeights.Add(EPSTADimension::Psychological, 0.25f);
    DimensionWeights.Add(EPSTADimension::Social, 0.25f);
    DimensionWeights.Add(EPSTADimension::Technical, 0.25f);
    DimensionWeights.Add(EPSTADimension::Administrative, 0.25f);

    // Default Failure Thresholds (0.3 by default)
    DimensionFailureThresholds.Add(EPSTADimension::Psychological, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Social, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Technical, 0.3f);
    DimensionFailureThresholds.Add(EPSTADimension::Administrative, 0.3f);
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
