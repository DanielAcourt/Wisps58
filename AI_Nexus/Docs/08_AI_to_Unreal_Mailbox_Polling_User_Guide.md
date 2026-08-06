# User Guide: AI-to-Unreal Mailbox Polling System (B-026)

**Version:** 1.0.0
**Author:** Jules (Systems Architect / Tactician)
**Date:** July 2026
**Context:** Sovereign Framework (AD-005b Roadmap)

---

## 🏛️ 1. Overview of the Mailbox Polling System

In standard web architectures, Unreal Engine acts as the HTTP Client (initiating requests) and the FastAPI Bridge acts as the HTTP Server (responding to requests). Because Unreal runs on a local loopback without a listening web server, the AI cannot easily push messages *directly* to Unreal without heavy, unstable socket listening code in C++.

To solve this, we establish **The Mailbox Polling Pattern**. This architecture decouples the AI's generation event from Unreal's tick rate using an in-memory "mailbox" queue on the bridge and a lightweight, periodic polling loop on the Unreal side.

```
                  ┌────────────────────────────────────────┐
                  │          FastAPI AI Bridge             │
                  │                                        │
                  │   ┌────────────────────────────────┐   │
                  │   │  Global unreal_mailbox Dict    │   │
                  │   │  "SIM_PlayerWisp": [Msgs...]   │   │
                  │   └───────────────▲────────────────┘   │
                  └───────────────────┼────────────────────┘
                       POST push_chat │ GET mailbox (pull & clear)
                                      │
                  ┌───────────────────▼────────────────────┐
                  │          Unreal Engine                 │
                  │                                        │
                  │   ┌────────────────────────────────┐   │
                  │   │  USovereignBridgeSubsystem     │   │
                  │   │  - MailboxTimerHandle          │   │
                  │   │  - OnAIChatPushed (Delegate)   │   │
                  │   └────────────────────────────────┘   │
                  └────────────────────────────────────────┘
```

---

## ⚙️ 2. Dynamic Polling Lifecycle (Best Practice: Option C)

To minimize network traffic, eliminate loopback server overhead, and keep local uvicorn console logs completely clean, we utilize **Option C: Dynamic Polling**.

Instead of running the polling loop continuously in the background throughout the entire game session, you should **dynamically start and stop polling** based on player proximity, interactive levels, or dialogue-active states.

### Why Dynamic Polling is Best:
- **Zero Overhead:** No CPU cycles or HTTP requests are made when the player is exploring, fighting, or in a non-interactive area.
- **Clean Console Logs:** Your FastAPI terminal will remain completely quiet and only scroll when active dialogue is taking place.
- **Level-Aware Design:** You can decide precisely which levels have AI-initiated conversations active and which do not.

---

## 🕹️ 3. How to Connect and Control Polling in Unreal Blueprints

The mailbox polling controls are fully exposed as `BlueprintCallable` methods and `BlueprintAssignable` delegates on the `USovereignBridgeSubsystem`.

### Step A: Starting Polling (Dynamic Activation)
Trigger polling only when the player enters an interactive zone (e.g., a dialogue trigger volume, a save terminal room, or a puzzle area):

1. Inside your **Character Blueprint** (e.g., `BP_PlayerWisp`), **Level Blueprint**, or **Interaction Volume Blueprint**:
2. Get a reference to the `SovereignBridgeSubsystem` using **Get Subsystem** (World Subsystem).
3. Call **`Start Mailbox Polling`**.
   - **`Actor Name`**: Pass `"PlayerWisp"` (or the name of your character).
   - *Normalization Note:* The C++ subsystem automatically prefixes this with `SIM_` (e.g., `"SIM_PlayerWisp"`) to match standard simulation identity protocols.
   - **`Mailbox Poll Interval`**: By default, this is set to `5.0` seconds. You can override it to be faster (e.g., `3.0`) or slower (e.g., `10.0`) on the subsystem variable if desired.

```
[BeginPlay / OnOverlap] ──► [Get SovereignBridgeSubsystem] ──► [Start Mailbox Polling (ActorName: "PlayerWisp")]
```

### Step B: Binding to the Message Delegate (Visual Manifestation)
To display fetched messages on-screen or in a 3D float bubble, you must bind an event to the subsystem's delegate:

1. Drag off the `SovereignBridgeSubsystem` reference and search for **`Bind Event to On AI Chat Pushed`**.
2. Connect this node to your initialization sequence (e.g., immediately after `BeginPlay` or inside your UI construction graph).
3. Create a **Custom Event** (e.g., `OnProactiveAIChatReceived`) and connect it to the delegate bind.
4. This event will trigger with a single parameter: **`Message`** (FString).
5. Pass this `Message` string into your UI Widget (e.g., using **Create Widget** -> **Add to Viewport** or updating a **WBP_DialogueBubble** floating component above your wisp character).

```
[Get Subsystem] ──► [Bind Event to OnAIChatPushed]
                            │
                            └──► [Custom Event: OnProactiveAIChatReceived (Message)] ──► [Create WBP_DialogueBubble (Text: Message)]
```

### Step C: Stopping Polling (Dynamic Deactivation)
Turn polling off immediately when the player leaves the zone, closes a terminal, or enters a gameplay-only state to prevent unnecessary network background loops:

1. When the player leaves an overlap volume, finishes a conversation, or the level changes:
2. Get the `SovereignBridgeSubsystem` reference.
3. Call **`Stop Mailbox Polling`**. This automatically clears the timer handle and cancels any active, in-flight HTTP GET requests, ensuring perfect memory stability.

```
[EndPlay / OnEndOverlap] ──► [Get SovereignBridgeSubsystem] ──► [Stop Mailbox Polling]
```

---

## ⚡ 4. How the AI Initiates Chat (Pushing Messages)

The AI can proactively send messages into Unreal through two primary routes:

### Method 1: AI Tool Execution (Autonomous "Whispering")
During any standard or simulation chat conversation, the AI model has access to the `push_chat_to_unreal` tool. If the AI wants to proactively whisper to you or another simulation actor, it executes the tool automatically:

* **Tool Name:** `push_chat_to_unreal`
* **Parameters:**
  - `actor_name` (e.g., `"SIM_PlayerWisp"`)
  - `message` (e.g., `"Warning: High energy fluctuations detected in the Qi Pool."`)
* **Security & AAS Profile:**
  - Classified as a **non-destructive tool** (AAS VSS threshold lowered to `0.4` so simulation characters with baseline `0.44` authority can use it).
  - Precedence level set to `5`, preventing it from consuming your single-use `HANDSHAKE_ACTIVE` boost token during multi-agent dialogues.

### Method 2: Manual REST POST Ingestion (External / Admin Scripting)
Developers or automated external scripts can manually inject messages into any simulation actor's mailbox on demand:

* **Endpoint:** `POST /v1/unreal/push_chat`
* **Content-Type:** `application/json`
* **Payload:**
  ```json
  {
    "actor_name": "SIM_PlayerWisp",
    "message": "hello world"
  }
  ```
* **Response:**
  ```json
  {
    "status": "success",
    "queued": true,
    "actor_name": "SIM_BP_PlayerWisp_C_0",
    "pending_count": 1
  }
  ```

Once pushed, the next time Unreal's dynamic polling timer fires, it retrieves the message, clears it from the bridge memory, and displays it inside your VR headset!
