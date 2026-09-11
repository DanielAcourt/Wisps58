// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# 13: Connecting Unreal AI Actors to Action Directives Guide (AD-029)

> **Architectural Guide: Bridging LLM Chat Directives to C++ Actors via Runtime Mailbox Polling**

---

## 🌌 Overview

This guide details the complete 2-way runtime execution pipeline connecting the **Iron Knight AI Bridge** (`Sovereign_Intelligence/IronOfficer/bridge.py`) to simulated C++ actors inside Unreal Engine 5.8 (`ASovereignIronKnightAgent`, `ASovereignBaseCharacter`).

---

## 🏛️ 1. Automatic Mailbox Polling Setup

For an AI Agent or actor (like `ASovereignIronKnightAgent`) to receive directives sent from chat or API endpoints, it must initialize active mailbox polling on `BeginPlay`:

```cpp
void ASovereignIronKnightAgent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        if (USovereignBridgeSubsystem* BridgeSubsystem = World->GetSubsystem<USovereignBridgeSubsystem>())
        {
            // Automatically poll the bridge for directives addressed to "SIM_IronKnight"
            BridgeSubsystem->StartMailboxPolling(TEXT("SIM_IronKnight"));
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
         │  2. unreal_runtime_directive() queues "[DIRECTIVE:EjectAgentPossession]" into unreal_mailbox["IronKnight"]
         ▼
[ USovereignBridgeSubsystem (Unreal C++) ]
         │
         │  3. QueryMailbox() polls GET /v1/unreal/mailbox?actor_name=SIM_IronKnight every 5.0s
         ▼
[ USovereignBridgeSubsystem::ProcessRuntimeDirective ]
         │
         │  4. Parses ActionName ("EjectAgentPossession") & Target ("SIM_IronKnight")
         │  5. Finds ASovereignIronKnightAgent in RegisteredSovereignEntities
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

## 🛠️ 3. Supported Directives & Capability Mapping

| Action Directive | Target Class | C++ Execution Method | Effect |
| :--- | :--- | :--- | :--- |
| **`PerformAgentPossession`** | `ASovereignIronKnightAgent` | `PerformAgentPossession(Target)` | Attaches agent to vessel, sets `bIsPossessingTarget = true` and updates `TargetSoul->bIsBeingPossessed = true`. |
| **`EjectAgentPossession`** | `ASovereignIronKnightAgent` | `EjectAgentPossession()` | Breaks agent possession, clears target references, and sets `bIsPossessingTarget = false`. |
| **`Interact`** | `ASovereignBaseInteractable` / Character | `IInteractionInterface::Execute_OnInteract` | Executes contextual interaction on target object or NPC. |
| **`MoveToLocation`** | AI Character / Agent | `UAIBlueprintHelperLibrary::SimpleMoveToLocation` | Issues 3D coordinate movement vector to actor AI Controller. |

---

## 📜 4. Operational Best Practices
* **Target Naming Alignment:** Ensure the `target_entity` string passed to `send_unreal_directive` matches the actor's in-editor name (e.g. `SIM_IronKnight` or `MySovereignIronKnightAgent_C_1`).
* **Active Polling Maintenance:** Ensure `StartMailboxPolling` is running during PIE. If PIE ends or restarts, polling automatically re-initializes on `BeginPlay`.
