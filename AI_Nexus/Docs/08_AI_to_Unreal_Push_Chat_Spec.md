# Architectural Specification: AI-Initiated Chat to Unreal Simulation (AI Talking Back)

**Version:** 1.0.0
**Status:** Approved Architectural Plan (AD-005b Roadmap)
**Author:** Jules (Researcher / Systems Architect)
**Date:** July 2026

---

## 🏛️ 1. The Core Challenge: Bi-Directional Web Topology

In standard REST-based architectures, Unreal Engine acts as the HTTP Client (initiating requests) and the FastAPI Bridge acts as the HTTP Server (responding to requests).

If an AI running on the bridge wants to **initiate dialogue** with Unreal (e.g., reacting spontaneously to an alarming sensor reading or a critical telemetry threshold), the bridge cannot easily send a raw HTTP request back to Unreal because:
1. Unreal is typically running on a local loopback without a listening web server.
2. Managing a listening TCP port or HTTP listener inside Unreal requires heavy C++ threading and socket handling code which can introduce instability, access violations, or cross-platform socket crashes.

---

## 📐 2. The Solution: The Mailbox Polling Pattern

To maintain 100% engine stability and zero-dependency portability, we establish **The Mailbox Polling Pattern**. This decouples the AI's generation event from Unreal's tick rate.

### A. The Mailbox State (Bridge-Side)
We declare a global in-memory dictionary in `bridge.py`:
```python
# Map actor_name to list of pending messages queued by the AI
unreal_mailbox: Dict[str, List[str]] = {}
```

### B. The AI Push Tool
We expose a new tool `push_chat_to_unreal` to the AI's function schemas:
```python
async def tool_push_chat_to_unreal(actor_name: str, message: str, persona: str = "Unknown"):
    """
    Pushes an AI-formed chat string directly into the mailbox queue of a specific Unreal actor.
    """
    if not actor_name or not message:
        return {"error": "Invalid arguments."}

    clean_actor = actor_name.replace("SIM_", "") # Support both standard and prefixed formats
    if clean_actor not in unreal_mailbox:
        unreal_mailbox[clean_actor] = []

    unreal_mailbox[clean_actor].append(message)
    logger.info(f"07 MAILBOX: Queued push chat for SIM_{clean_actor}: '{message[:40]}...'")

    return {
        "status": "success",
        "queued": True,
        "actor_name": f"SIM_{clean_actor}",
        "pending_count": len(unreal_mailbox[clean_actor])
    }
```

### C. The Mailbox API Endpoints
We expose two endpoints in `bridge.py`:

1.  **`GET /v1/unreal/mailbox`**:
    Used by Unreal to retrieve and clear pending messages.
    ```python
    @app.get("/v1/unreal/mailbox")
    async def get_unreal_mailbox(actor_name: str):
        clean_actor = actor_name.replace("SIM_", "")
        messages = unreal_mailbox.pop(clean_actor, [])
        return {
            "actor_name": f"SIM_{clean_actor}",
            "messages": messages,
            "count": len(messages)
        }
    ```

2.  **`POST /v1/unreal/push_chat`**:
    Allow administrators, developers, or external scripts to manually queue messages for Unreal:
    ```python
    class PushChatPayload(BaseModel):
        actor_name: str
        message: str

    @app.post("/v1/unreal/push_chat")
    async def push_chat_manually(payload: PushChatPayload):
        return await tool_push_chat_to_unreal(payload.actor_name, payload.message, persona="Lead")
    ```

---

## 🕹️ 3. Unreal Engine C++ Implementation

### A. Updating the Subsystem Header
We expand `USovereignBridgeSubsystem` in `SovereignBridgeSubsystem.h` with:

```cpp
/** Delegate triggered when a proactive AI message is received from the mailbox */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSovereignAIChatPushed, const FString&, Message);

UCLASS()
class WISPCPP7VR_API USovereignBridgeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // ... Existing members ...

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

private:
    FTimerHandle MailboxTimerHandle;
    FString PollingActorName;

    /** Executes the HTTP GET request to check the mailbox */
    void QueryMailbox();
    void OnMailboxResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
```

### B. Implementing the Polling Loop
In `SovereignBridgeSubsystem.cpp`:

```cpp
void USovereignBridgeSubsystem::StartMailboxPolling(const FString& ActorName)
{
    PollingActorName = ActorName;
    GetWorld()->GetTimerManager().SetTimer(
        MailboxTimerHandle,
        this,
        &USovereignBridgeSubsystem::QueryMailbox,
        MailboxPollInterval,
        true
    );
    UE_LOG(LogTemp, Warning, TEXT("SovereignBridge: Started Mailbox Polling loop for [%s] every %f seconds."), *PollingActorName, MailboxPollInterval);
}

void USovereignBridgeSubsystem::StopMailboxPolling()
{
    GetWorld()->GetTimerManager().ClearTimer(MailboxTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("SovereignBridge: Stopped Mailbox Polling."));
}

void USovereignBridgeSubsystem::QueryMailbox()
{
    if (PollingActorName.IsEmpty()) return;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &USovereignBridgeSubsystem::OnMailboxResponse);

    // Construct Query Parameters
    FString TargetURL = FString::Printf(TEXT("%s/v1/unreal/mailbox?actor_name=%s"), *BridgeBaseUrl, *PollingActorName);
    Request->SetURL(TargetURL);
    Request->SetVerb(TEXT("GET"));
    Request->ProcessRequest();
}

void USovereignBridgeSubsystem::OnMailboxResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* MessagesArray;
            if (JsonObject->TryGetArrayField(TEXT("messages"), MessagesArray))
            {
                for (auto& MsgVal : *MessagesArray)
                {
                    FString PushedMessage = MsgVal->AsString();
                    UE_LOG(LogTemp, Warning, TEXT("SovereignBridge: AI Pushed Proactive Chat: %s"), *PushedMessage);

                    // Broadcast to UI Widgets, subtitles, or character dialogue systems
                    OnAIChatPushed.Broadcast(PushedMessage);
                }
            }
        }
    }
}
```

---

## ⚡ 4. Blueprint UI Integration

Once implemented, the setup inside Unreal Engine's Event Graph is clean and declarative:

1.  **On BeginPlay:**
    - Call `GetSubsystem<USovereignBridgeSubsystem>()`.
    - Call `StartMailboxPolling(ActorName = "PlayerWisp")`.
2.  **Bind to Delegate:**
    - Bind a Custom Event `OnAIIncomingMessage` to the subsystem's `OnAIChatPushed` delegate.
    - Inside the event, pass the `Message` string directly to `WBP_DialogueBubble` or `SovereignMasterHUD` to render text in the 3D VR environment or floating UI.

This ensures seamless bi-directional integration where the AI can monitor telemetry, analyze SSoT RAG documentation, and proactively "speak" back to the user or player entities.
