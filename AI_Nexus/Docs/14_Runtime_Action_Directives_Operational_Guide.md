// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Sovereign Framework: Runtime Action Directives & AI Agent Control Plane Operational Guide

**Document ID:** `AI_Nexus/Docs/14_Runtime_Action_Directives_Operational_Guide.md`
**Version:** `1.0.0` (Grounded in Tickets AD-025 – AD-035)
**Author:** Sovereign Framework Research & DevOps

---

## 1. Executive Summary & Control Plane Architecture

The **Sovereign Control Plane Directive Pipeline** provides a 2-way, real-time command bridge between local Large Language Models (LLMs running via Ollama/Llama3 and `bridge.py`) and virtual entities inside Unreal Engine 5.8.

### Key Architectural Principles
1. **Decoupled Control vs. Narrative Plane**:
   - **Narrative Chat Mailbox** (`/v1/unreal/mailbox`): Handles dialogue, roleplay, and UI subtitles.
   - **Control Plane Directives** (`/v1/unreal/directive` & `/v1/unreal/directives/poll`): Dedicated, high-priority JSON queue (`unreal_directives_queue`) for physical simulation commands (possession, navigation, interaction).
2. **Macro Intent vs. Micro Execution**:
   - The LLM acts at the **Macro Level** (deciding *what* to do, e.g., `"Move to Dragon Egg"` or `"Possess Antelope"`).
   - Unreal Engine C++ components act at the **Micro Level** (handling tick-by-tick pathfinding via `AAIController`, collision avoidance, and NavMesh navigation).
3. **Option C Component Mobility Guard**:
   - Every simulation entity possesses a `USovereignSaveableEntityComponent` ("Soul Hub").
   - `bIsMobile = true/false` dictates whether an entity can execute spatial navigation directives. Static interactables (chests, terminals, shrines) cleanly reject movement commands.

---

## 2. Complete Directive Catalog & Schemas

LLMs issue directives by calling the `send_unreal_directive` function tool registered in `Sovereign_Intelligence/IronOfficer/bridge.py`.

```json
{
  "sender_id": "SIM_IronKnight",
  "target_entity": "MySovereignIronKnightAgent_C_1",
  "action_name": "MoveToLocation",
  "parameters": {
    "location": { "X": 1280.0, "Y": 750.0, "Z": 400.0 }
  },
  "authority_token": "AAS_BOOST_1.0"
}
```

### Directive Reference Matrix

| Directive Action | Purpose | Required Parameters | Example Payload |
|:---|:---|:---|:---|
| `PerformAgentPossession` | Attaches agent control to a target vessel actor. | `target_vessel` | `{"target_vessel": "BPI_DragonEgg_C_3"}` |
| `EjectAgentPossession` | Severs possession from current target vessel. | *None* | `{}` |
| `MoveToLocation` | Navigates mobile entity to 3D vector coordinates. | `location` (`X`, `Y`, `Z`) | `{"location": {"X": 1280.0, "Y": 750.0, "Z": 400.0}}` |
| `MoveToActor` | Navigates mobile entity to a target actor. | `target_vessel` or `target_actor` | `{"target_vessel": "BPI_DragonEgg_C"}` |
| `InteractWithObject` | Triggers `IInteractionInterface::OnInteract` on target. | `target_object`, `target_vessel`, or `target_actor` | `{"target_object": "SovereignSaveTerminal_2"}` |

---

## 3. Prioritized Target Resolution Hierarchy

When a directive arrives in Unreal Engine C++ (`USovereignBridgeSubsystem::ProcessRuntimeDirective`), target string parameters are resolved against all registered `USovereignSaveableEntityComponent` instances using a strict 3-tier matching hierarchy:

```
[Incoming Directive Target Parameter]
                 │
                 ▼
 ┌───────────────────────────────────────────────┐
 │ Priority 1: EntityID / GUID Match             │
 │ Checks EntityID.ToString() & Metadata Tags     │
 └───────────────────────┬───────────────────────┘
                         │ (If not found)
                         ▼
 ┌───────────────────────────────────────────────┐
 │ Priority 2: Exact Actor Name Match            │
 │ Checks GetName() == TargetString              │
 └───────────────────────┬───────────────────────┘
                         │ (If not found)
                         ▼
 ┌───────────────────────────────────────────────┐
 │ Priority 3: Substring Fallback Match          │
 │ Checks GetName().Contains(CleanTargetString)  │
 └───────────────────────────────────────────────┘
```

* **Single Active Vessel Rule (AD-034)**: If `ASovereignIronKnightAgent` receives a `PerformAgentPossession` directive while already possessing a vessel, it automatically calls `EjectAgentPossession()` first to clear $B-044$ bidirectional states (`bIsBeingPossessed = false`) before attaching to the new target vessel.

---

## 4. In-Engine Debugging & Diagnostic Log Traces

All directive parsing and execution steps output structured logs to Unreal Engine's **Output Log** under category `LogTemp` with prefix `SovereignBridge`.

### Sample Diagnostic Log Sequence

```text
LogTemp: Warning: SovereignBridge: Control Plane Directive Received: SIM_IronKnight -> MySovereignIronKnightAgent_C_1 (MoveToActor)
LogTemp: Warning: SovereignBridge: Processing Runtime Directive: [DIRECTIVE:MoveToActor] sender=SIM_IronKnight target=MySovereignIronKnightAgent_C_1 params={"target_vessel": "MySovereignBaseInteractableTest1_C_3"}
LogTemp: Warning: SovereignBridge: Directive Parsed - Action: 'MoveToActor', Target: 'MySovereignIronKnightAgent_C_1'
LogTemp: Warning: SovereignBridge: Resolved target vessel [MySovereignBaseInteractableTest1_C_3] via Priority 2 (Exact Name match).
LogTemp: Warning: SovereignBridge: Projected 3D target X=1280.000 Y=750.000 Z=400.000 to NavMesh floor location X=1280.000 Y=750.000 Z=10.000
LogTemp: Warning: SovereignBridge: Executed NavMesh AAIController::MoveToLocation on [MySovereignIronKnightAgent_C_1] targeting X=1280.000 Y=750.000 Z=10.000 (Result: 2)
```

### Common Troubleshooting Checklist

1. **Directive Not Received by Actor**:
   - Verify `StartDirectivePolling("SIM_IronKnight")` or `StartDirectivePolling(GetName())` was executed on `BeginPlay`.
   - Confirm FastAPI bridge server (`bridge.py`) is running on `http://127.0.0.1:8000`.
2. **"Movement Directive REJECTED - Subject actor has bIsMobile=false"**:
   - Check the target actor's `USovereignSaveableEntityComponent` in the Details Panel. Set `bIsMobile = True`.
3. **Actor Accepts Move Directive But Stays Stationary**:
   - Verify a `NavMeshBoundsVolume` is placed in the level and built (`P` key in Viewport shows green NavMesh).
   - Ensure the actor's `RootComponent` mobility is set to `Movable`.
