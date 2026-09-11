// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# 13: Connecting Unreal AI Actors to Action Directives Guide (AD-029 & AD-032)

> **Architectural Guide: Bridging LLM Chat Directives to C++ Actors via Control Plane Polling**

---

## 🌌 Overview

This guide details the complete 2-way runtime execution pipeline connecting the **Iron Knight AI Bridge** (`Sovereign_Intelligence/IronOfficer/bridge.py`) to simulated C++ actors and Blueprints inside Unreal Engine 5.8 (`ASovereignIronKnightAgent`, `ASovereignBaseCharacter`).

---

## 🏛️ 1. Automatic Dual-Channel Polling Setup

For an AI Agent or actor (like `ASovereignIronKnightAgent`) to receive both conversation messages and physical Control Plane directives, it initializes active polling on `BeginPlay`:

```cpp
void ASovereignIronKnightAgent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        if (USovereignBridgeSubsystem* BridgeSubsystem = World->GetSubsystem<USovereignBridgeSubsystem>())
        {
            // Channel 1: Narrative Chat Mailbox Polling
            BridgeSubsystem->StartMailboxPolling(TEXT("SIM_IronKnight"));

            // Channel 2: Control Plane Action Directive Polling (AD-032)
            BridgeSubsystem->StartDirectivePolling(TEXT("SIM_IronKnight"));
        }
    }
}
```

---

## 🔄 2. The 2-Way Directive Execution Sequence

```
[ LLM Chat / API Call ]
         │
         │  1. LLM invokes tool: send_unreal_directive("SIM_IronKnight", "EjectAgentPossession")
         ▼
[ FastAPI Bridge (bridge.py) ]
         │
         │  2. tool_send_unreal_directive calls unreal_runtime_directive()
         │     - Queues structured JSON into unreal_directives_queue["IronKnight"]
         │     - Logging: "07 DIRECTIVE: Received action directive 'EjectAgentPossession'..."
         ▼
[ USovereignBridgeSubsystem (Unreal C++) ]
         │
         │  3. QueryDirectives() polls GET /v1/unreal/directives/poll?actor_name=SIM_IronKnight every 2.0s
         ▼
[ USovereignBridgeSubsystem::OnDirectivesResponse ]
         │
         │  4. Parses FSovereignDirective struct (ActionName="EjectAgentPossession", Target="SIM_IronKnight")
         │  5. Broadcasts OnDirectiveReceived delegate (for Blueprint listeners)
         │  6. Executes Agent->EjectAgentPossession()
         ▼
[ In-Engine C++ State Update ]
         │
         │  - bIsPossessingTarget flips to false
         │  - PossessedTargetActor resets to nullptr
         │  - PossessedTargetName resets to "None"
         │  - Details Window updates immediately in Unreal Engine 5.8
```

---

## 🔍 3. Understanding Bridge API Endpoint Logs

When reviewing `bridge.py` console logs during simulation play:

* **Why don't I see an external HTTP `POST /v1/unreal/directive` log?**
  When you chat with the AI in PIE, the LLM executes the Python tool `send_unreal_directive` internally inside `bridge.py`. It calls the `unreal_runtime_directive()` function directly (logging `07 DIRECTIVE: Received action directive...`).
* **When is `POST /v1/unreal/directive` used?**
  External tools, automation scripts, or web dashboards issue HTTP POST requests directly to `http://127.0.0.1:8000/v1/unreal/directive`.
* **When is `GET /v1/unreal/directives/poll` used?**
  Unreal Engine's `USovereignBridgeSubsystem::QueryDirectives()` sends an HTTP GET request to `http://127.0.0.1:8000/v1/unreal/directives/poll?actor_name=SIM_IronKnight` every 2.0 seconds to retrieve and flush queued Control Plane directives.

---

## 🛠️ 4. Supported Directives & Capability Mapping

| Action Directive | Target Class | C++ Execution Method | Effect |
| :--- | :--- | :--- | :--- |
| **`PerformAgentPossession`** | `ASovereignIronKnightAgent` | `PerformAgentPossession(Target)` | Attaches agent to vessel, sets `bIsPossessingTarget = true` and updates `TargetSoul->bIsBeingPossessed = true`. |
| **`EjectAgentPossession`** | `ASovereignIronKnightAgent` | `EjectAgentPossession()` | Breaks agent possession, clears target references, and sets `bIsPossessingTarget = false`. |
| **`Interact`** | `ASovereignBaseInteractable` / Character | `IInteractionInterface::Execute_OnInteract` | Executes contextual interaction on target object or NPC. |
| **`MoveToLocation`** | AI Character / Agent | `UAIBlueprintHelperLibrary::SimpleMoveToLocation` | Issues 3D coordinate movement vector to actor AI Controller. |

---

## 🎨 5. Blueprint Delegate Integration (`OnDirectiveReceived`)

In addition to automatic C++ execution, `USovereignBridgeSubsystem` exposes a `BlueprintAssignable` delegate for custom game-specific Blueprint logic:

```cpp
/** Delegate triggered when a runtime directive is polled via Control Plane (AD-032) */
UPROPERTY(BlueprintAssignable, Category = "Sovereign|Directive")
FOnSovereignDirectiveReceived OnDirectiveReceived;
```

### How to Bind Custom Blueprint Events:
1. Open your Level Blueprint or Actor Blueprint (e.g. `BP_Humanoid` or `BP_DragonVessel`).
2. On `BeginPlay`, retrieve **Sovereign Bridge Subsystem** (`GetSubsystem<USovereignBridgeSubsystem>`).
3. Drag off and call **Assign On Directive Received**.
4. Drag off `Directive.ActionName` into a **Switch on String**:
   * `ActionName == "Sleep"` -> Play sleep animation / montage.
   * `ActionName == "OpenGate"` -> Trigger door timeline.
   * `ActionName == "CastSpell"` -> Spawn Niagara particle effect.

---

## 📜 6. Operational Best Practices
* **Target Naming Alignment:** Ensure the `target_entity` string passed to `send_unreal_directive` matches the actor's in-editor name (e.g. `SIM_IronKnight` or `MySovereignIronKnightAgent_C_1`).
* **Active Polling Maintenance:** Ensure both `StartMailboxPolling` and `StartDirectivePolling` are active during PIE.
