// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# SOP: IRON KNIGHT SIMULATION ACTION & ROLEPLAY DIRECTIVE PROTOCOL (AD-025)

## 🌌 Overview
This Standard Operating Procedure (SOP) governs how the Iron Knight AI agent executes in-game runtime actions (e.g. Possession, Movement, Contextual Interaction, Biological/Cultivation routines) during Play-In-Editor (PIE) or Standalone simulation play.

---

## 🏛️ 1. EDITOR MCP VS. RUNTIME SIMULATION PIPELINE

To maintain absolute system stability and prevent state collisions, the Sovereign Framework enforces a strict architectural boundary between Editor and Runtime execution:

| Domain | Architecture / Endpoint | Purpose / Scope |
| :--- | :--- | :--- |
| **Editor Context** | **Unreal Native MCP** (`http://127.0.0.1:8000/mcp`) | Asset management, Level editing, Build automation, EUW packaging when simulation is **stopped**. |
| **Runtime Context** | **Sovereign Directive API** (`POST /v1/unreal/directive`) | Real-time actor possession, movement, interaction, and roleplay actions during **active simulation play**. |

---

## 📜 2. ACTION CAPABILITY MANIFEST (VERIFICATION)

An Iron Knight agent or persona **cannot** command a target vessel to perform an action unless that target vessel explicitly registers the action in its **Soul Hub (`USovereignSaveableEntityComponent`) Action Capability Manifest**.

### Registered Action Capabilities:
*   `PerformAgentPossession`: Target vessel attaches to Iron Knight spatial entity and sets `bIsBeingPossessed = true`.
*   `EjectAgentPossession`: Target vessel detaches from Iron Knight and restores independent control.
*   `Interact`: Triggers `IInteractionInterface::Execute_OnInteract` on sensed or specified target actors.
*   `MoveToLocation`: Issues a 3D coordinate movement directive to the target vessel's navigation/movement component.
*   `Rest`: Triggers bio-metabolic recovery on `USovereignBioComponent`.
*   `CondenseQi`: Triggers cultivation/qi manipulation on `USovereignQiComponent`.

---

## 🔌 3. STRUCTURED JSON DIRECTIVE SCHEMA

All runtime action directives issued from the local model/bridge to Unreal Engine must adhere to the following JSON schema:

```json
{
  "sender_id": "SIM_IronKnight",
  "target_entity": "BP_Humanoid_C_1",
  "action_name": "PerformAgentPossession",
  "parameters": {
    "vessel_tag": "BP_Humanoid_C_1",
    "behavior_state": "ActiveObservation"
  },
  "authority_token": "AAS_BOOST_1.0"
}
```

---

## 🛡️ 4. EXECUTION FLOW & AAS GOVERNANCE

1.  **Directive Formulation:** The Iron Knight selects a target vessel and action based on the ingested World Manifest.
2.  **Capability Check:** The bridge verifies that the target vessel supports `action_name`.
3.  **FastAPI Dispatch:** The bridge POSTs the directive to `POST /v1/unreal/directive`.
4.  **C++ Router Execution:** `USovereignBridgeSubsystem` receives the directive payload, finds the target `USovereignSaveableEntityComponent`, and dispatches the mapped C++/Blueprint function.
5.  **State Verification:** The target vessel updates its `SaveDataComponent` tags and acknowledges action completion back to the bridge telemetry pipeline.
