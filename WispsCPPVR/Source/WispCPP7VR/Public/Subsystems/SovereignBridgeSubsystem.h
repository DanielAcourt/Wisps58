// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).

#pragma once

// // [J] The 07 Bridge Subsystem: Connecting the simulation soul to the architectural engine. 2025-06-18

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "SovereignBridgeSubsystem.generated.h"

// Forward declaration for entity registration tracking
class USovereignSaveableEntityComponent;

/**
 * FSovereignChatLog: Represents a single tool execution log entry returned by the Iron Officer.
 */
USTRUCT(BlueprintType)
struct FSovereignChatLog
{
    GENERATED_BODY()

    /** The name of the tool executed (e.g., "get_system_telemetry") */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Chat")
    FString ToolName;

    /** A snippet of the tool's execution result for UI display */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Chat")
    FString ResultSnippet;
};

/**
 * FSovereignChatResponse: Data returned from the Iron Officer Bridge in response to a simulation chat.
 */
USTRUCT(BlueprintType)
struct FSovereignChatResponse
{
    GENERATED_BODY()

    /** The text response from the AI */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Chat")
    FString Content;

    /** Diagnostic logs for any tools the AI executed to fulfill the request */
    UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Chat")
    TArray<FSovereignChatLog> ToolLogs;
};

/**
 * FSovereignChatMessage: Represents a single message in the chat history.
 */
USTRUCT(BlueprintType)
struct FSovereignChatMessage
{
    GENERATED_BODY()

    /** Role of the messenger: "user" or "assistant" */
    UPROPERTY(BlueprintReadWrite, Category = "Sovereign|Chat")
    FString Role;

    /** Content of the message */
    UPROPERTY(BlueprintReadWrite, Category = "Sovereign|Chat")
    FString Content;

    /** Optional name/persona of the sender (e.g., SIM_MyActor) */
    UPROPERTY(BlueprintReadWrite, Category = "Sovereign|Chat")
    FString Name;
};

/** Delegate broadcasted when the bridge responds to a chat request */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSovereignChatResponse, const FSovereignChatResponse&, Response);

/** Delegate triggered when a proactive AI message is received from the mailbox */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSovereignAIChatPushed, const FString&, Message);

/**
 * USovereignBridgeSubsystem: Manages communication between Unreal and the Iron Officer Bridge.
 * Implements the 07 Protocol, Telemetry pipelines, and Simulation Chat.
 */
UCLASS()
class WISPCPP7VR_API USovereignBridgeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initiates the 07 Check-In protocol.
     * Performs a handshake and verifies bridge connectivity.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void Perform07CheckIn();

    /**
     * Pushes BlackBox data and a PSTA score to the bridge.
     * @param EntityID      The ID of the entity originating the data.
     * @param PSTAScore     The calculated PSTA viability score.
     * @param BlackBoxJson  JSON string containing metadata from the BlackBox.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void PushBlackBoxTelemetry(const FGuid& EntityID, float PSTAScore, const FString& BlackBoxJson);

    /**
     * Sends a chat message to the Iron Officer Bridge from a simulation actor.
     * @param ActorName     The name of the simulation actor sending the message.
     * @param Message       The text content of the message.
     * @param History       Optional history for stateless communication.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void SendSimulationChat(const FString& ActorName, const FString& Message, const TArray<FSovereignChatMessage>& History);

    /**
     * Executes the AAS Handshake to request a temporary authority boost (+0.5 VSS).
     * // [J] Exposing the handshake directly to Blueprints allows simulation entities to dynamically request clearance.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void ExecuteAASHandshake();

    /**
     * Creates a new file in the specified path with the provided content.
     * Bypasses the conversational chat LLM loop and directly issues a request to the bridge.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void CreateSimulationFile(const FString& FilePath, const FString& Content);

    /**
     * Dynamically generates a new D&D character sheet, updates character index, and indexes RAG on-the-fly.
     * @param CharacterName     The name of the character to generate.
     * @param bRandomGenerate   If true, uses the LLM to randomly generate rich, creative attributes and backstory.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void GenerateDNDPersona(const FString& CharacterName, bool bRandomGenerate);

    /** Delegate triggered when a chat response is received from the bridge */
    UPROPERTY(BlueprintAssignable, Category = "Sovereign|Bridge")
    FOnSovereignChatResponse OnChatResponseReceived;

    /** Triggered whenever the polling loop retrieves a message from the mailbox */
    UPROPERTY(BlueprintAssignable, Category = "Sovereign|Bridge")
    FOnSovereignAIChatPushed OnAIChatPushed;

    /** Polling Interval in seconds (default: 5.0f) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bridge")
    float MailboxPollInterval = 5.0f;

    /** Starts the automated mailbox polling loop */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void StartMailboxPolling(const FString& ActorName);

    /** Stops the mailbox polling loop */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Bridge")
    void StopMailboxPolling();

    /** If true, the bridge will store a permanent copy of the chat in AI_Nexus/Memories/ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Bridge")
    bool bEnableRemoteHistory = false;

    /** Registers a Sovereign Soul with the bridge for simulation-wide tracking */
    void RegisterEntity(USovereignSaveableEntityComponent* Soul);

    /** Unregisters a Sovereign Soul */
    void UnregisterEntity(USovereignSaveableEntityComponent* Soul);

    /** Debugging: Returns the number of registered Sovereign Entities */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Debug")
    int32 GetRegisteredEntityCount() const;

private:
    /** Internal struct to buffer telemetry while handshake is pending */
    struct FPendingTelemetry
    {
        FGuid EntityID;
        float PSTAScore;
        FString BlackBoxJson;
    };

    /** Queue for telemetry data sent before the initial handshake completes */
    TArray<FPendingTelemetry> TelemetryQueue;

    /** Active entities currently registered in the simulation */
    UPROPERTY()
    TArray<TWeakObjectPtr<USovereignSaveableEntityComponent>> RegisteredSovereignEntities;

    /** Flushes buffered telemetry to the bridge */
    void FlushTelemetryQueue();
    /** Path to bridge_config.json in Saved/Config/ */
    FString GetConfigPath() const;

    /** Loads configuration from disk */
    void LoadConfiguration();

    /** Internal HTTP response handler for Check-In */
    void OnCheckInResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /** Internal HTTP response handler for Telemetry */
    void OnTelemetryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /** Internal HTTP response handler for Chat */
    void OnChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /** Bridge Base URL (e.g., http://localhost:8000) */
    UPROPERTY()
    FString BridgeBaseUrl = TEXT("http://127.0.0.1:8000");

    /** Indicates if the handshake has been established */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Bridge", meta = (AllowPrivateAccess = "true"))
    bool bHandshakeActive = false;

    FTimerHandle MailboxTimerHandle;
    FString PollingActorName;

    /** Tracks the active in-flight request so we can cancel it if needed */
    TWeakPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveMailboxRequest;

    /** Executes the HTTP GET request to check the mailbox */
    void QueryMailbox();
    void OnMailboxResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
