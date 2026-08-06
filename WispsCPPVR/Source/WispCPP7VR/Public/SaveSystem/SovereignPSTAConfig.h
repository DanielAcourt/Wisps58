// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SovereignPSTAConfig.generated.h"

/**
 * @enum EPSTADimension
 * @brief The four pillars of the Sovereign Framework.
 */
UENUM(BlueprintType)
enum class EPSTADimension : uint8
{
    Psychological UMETA(DisplayName = "P - Psychological"),
    Social        UMETA(DisplayName = "S - Social"),
    Technical     UMETA(DisplayName = "T - Technical"),
    Administrative UMETA(DisplayName = "A - Administrative")
};

/**
 * @struct FPSTATagMapping
 * @brief Maps a Meta-Tag to a dimension and defines its mathematical impact.
 */
USTRUCT(BlueprintType)
struct FPSTATagMapping
{
    GENERATED_BODY()

    /** The Meta-Tag key (e.g., "Telemetry.TemperatureCelsius"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TagKey;

    /** Which dimension this tag contributes to. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPSTADimension Dimension = EPSTADimension::Technical;

    /** The weight of this tag within its dimension (relative to other tags). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Weight = 1.0f;

    /** If true, a 0.0 value for this tag will force its entire Dimension Health (Di) to 0.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAnchorTag = false;

    /**
     * Normalization Range:
     * Values within [Min, Max] are mapped to [0, 1].
     * Example: Temp 20-30C. 20C = 0.0, 30C = 1.0.
     * Note: If Min > Max, the mapping is inverted (e.g., higher value = lower health).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RangeMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RangeMax = 1.0f;

    FPSTATagMapping() {}
};

/**
 * @class USovereignPSTAConfig
 * @brief Configuration asset for PSTA Dimension Health and Provable Trust math.
 */
UCLASS(BlueprintType)
class WISPCPP7VR_API USovereignPSTAConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** The master list of tag-to-dimension mappings. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSTA|Mappings")
    TArray<FPSTATagMapping> TagMappings;

    /** Weights for the final VSS calculation (alpha_i). Should ideally sum to 1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSTA|Weights")
    TMap<EPSTADimension, float> DimensionWeights;

    /** Dimension-specific failure thresholds (tau_fail,i). If Di falls below this, VSS collapses to 0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSTA|Thresholds")
    TMap<EPSTADimension, float> DimensionFailureThresholds;

    /** The legacy threshold for the Bottleneck Law (Deprecating in favor of VSS). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSTA|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CriticalInstabilityThreshold = 0.3f;

    USovereignPSTAConfig();

    /** Normalizes a raw value based on the mapping's range. */
    float NormalizeValue(const FPSTATagMapping& Mapping, float RawValue) const;

    /** Returns the mapping for a specific tag, or nullptr if not found. (O(1) lookup) */
    const FPSTATagMapping* GetMappingForTag(const FString& TagKey) const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Cached lookup map for performance. */
    TMap<FString, FPSTATagMapping> CachedMappings;

    void RebuildCache();
};
