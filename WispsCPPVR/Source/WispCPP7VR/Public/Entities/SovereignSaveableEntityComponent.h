// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.10. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-06

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/SovereignEntityInterface.h"
#include "SovereignSaveableEntityComponent.generated.h"

// Forward declarations
class FJsonObject;
class ISovereignBrokerInterface;

/** Delegate for UI widgets to listen for state updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSovereignStateChanged, USovereignSaveableEntityComponent*, SoulHub);

/**
 * USovereignSaveableEntityComponent: The "Central Hub" or "Soul" of a Sovereign Actor.
 * It acts as a Mediator between specialized modules (Bio, Qi, Elements) and the Bridge.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignSaveableEntityComponent : public UActorComponent, public ISovereignEntityInterface
{
    GENERATED_BODY()

public:
    USovereignSaveableEntityComponent();

    /** --- 1. CORE IDENTITY --- */

    /** Unique Identifier for this Soul across all simulations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity")
    FGuid EntityID;

    /** The exact moment this Soul was manifested */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Identity")
    FDateTime BirthTimestamp;

    /** ANCESTRAL OVERRIDE: Use a custom date instead of the current time? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity")
    bool bUseManualBirthDate = false;

    /** Custom Date string (e.g. "2017.03.23-16.00.00") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Identity", meta = (EditCondition = "bUseManualBirthDate"))
    FString ManualBirthDateStr = "2017.03.23-16.00.00";

    /** Is this entity currently being controlled by the Lead/Spirit? */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Soul")
    bool bIsBeingPossessed = false;


    /** --- 2. MODULAR ARCHITECTURE (The Mediator) --- */

    /** Registers a specialized module with the Hub */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Architecture")
    void RegisterBroker(TScriptInterface<ISovereignBrokerInterface> Broker);

    /** Unregisters a module */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Architecture")
    void UnregisterBroker(TScriptInterface<ISovereignBrokerInterface> Broker);

    /** Captures the entire categorized state of the entity for the Bridge/SaveSystem */
    TSharedPtr<FJsonObject> CaptureFullEntityState();

    /** Restores state from a categorized JSON object */
    void ApplyStateFromJsonObject(const TSharedPtr<FJsonObject>& JsonData);

    /** Triggers the 07 Check-In handshake with the Bridge */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Core")
    void InitializeSoul();


    /** --- 3. UI & EVENT SYSTEM --- */

    /** Called whenever a registered module updates its state significantly */
    UPROPERTY(BlueprintAssignable, Category = "Sovereign|Events")
    FOnSovereignStateChanged OnStateChanged;

    /** Helper for UI widgets to get a raw string representation of a specific category (Bio, Qi, etc.) */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|UI")
    FString GetCategoryStateJson(FString CategoryName);

    /** Configurable rate throttle for UI state updates (default: 0.05 seconds for 20Hz refresh rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|UI")
    float UIUpdateThrottleInterval = 0.05f;

    /** Forces invalidation of the state ingestion cache (useful when an event or mutation occurs) */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|UI")
    void InvalidateStateCache();


    /** --- 4. REALITY & TRUTH --- */

    /** The density of logical paradoxes within this entity (0.0 = Pure Truth, 1.0 = Total Paradox) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Reality")
    float ParadoxDensity = 0.0f;

    /** Adds a tag that the system doesn't yet recognize, potentially increasing Paradox Density */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Reality")
    void AddUnknownTag(FString Key, FString Value);

    /** LEGACY DNA SUPPORT: Returns the flat map of unknown tags */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Legacy")
    TMap<FString, FString> GetUnknownMetaTags() const { return UnknownMetaTags; }

    /** LEGACY DNA SUPPORT: Ingests a flat map of tags and distributes them */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Legacy")
    void ApplyMetaTags(TMap<FString, FString> LoadedTags);

    /** ISovereignEntityInterface Implementation */
    virtual USovereignSaveableEntityComponent* GetSovereignSoul_Implementation() const override { return const_cast<USovereignSaveableEntityComponent*>(this); }
    virtual float GetSystemConfidence_Implementation() const override;


protected:
    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** All modules currently registered with this Hub */
    UPROPERTY()
    TArray<TScriptInterface<ISovereignBrokerInterface>> RegisteredBrokers;

public:
    /** Diagnostic Broker instance (Truth) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Brokers")
    class UDiagnosticBroker* DiagnosticBroker;

    /** Cultivation Broker instance (Magic) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Brokers")
    class UCultivationBroker* CultivationBroker;

protected:
    /** Internal storage for unhandled meta-tags */
    UPROPERTY(VisibleAnywhere, Category = "Sovereign|Reality")
    TMap<FString, FString> UnknownMetaTags;

#if WITH_EDITOR
    virtual void PostEditImport() override;
    virtual void PostDuplicate(bool bDuplicateForPIE) override;
#endif

private:
    /** Helper to serialize a JSON object to string for BP consumption */
    FString SerializeJsonToString(TSharedPtr<FJsonObject> JsonObj);

    /** Invalidation helper for caching */
    uint64 LastCachedFrame = 0;
    double LastCachedTime = -1.0;
    TSharedPtr<FJsonObject> CachedFullState;
    TMap<FString, FString> CachedCategoryStrings;
};
