// Copyright (c) 2013-2026 Daniel Acourt. Version 36.5.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# UE 5.8.1 MCP Local Training & Reference Map

This document serves as the local semantic training data mapped into the **Sovereign RAG Indexer**. It provides the local model with the precise execution contexts, parameters, and expected JSON-RPC interfaces for standard Unreal Engine toolsets.

---

## 🏛️ 1. Core Toolsets Mapping

### A. Scene & Actor Tools (`toolset_registry.toolsets.core.actor`)
Exposes tools for querying, selecting, and modifying actors inside the active editor level.

#### 1. `get_selected_actors`
- **Description:** Returns the list of currently selected actors in the editor.
- **Parameters:** None.
- **Expected Return Type:** `list[str]` (paths or labels of selected actors).

#### 2. `spawn_actor`
- **Description:** Spawns a new actor of the specified class at the target location.
- **Parameters:**
  - `class_path` (`str`): The full asset reference path (e.g., `"/Game/RunicSystem/Blueprints/BP_RunicNode.BP_RunicNode_C"`).
  - `location` (`dict`): Vector3 coordinates `{"x": float, "y": float, "z": float}`.
  - `rotation` (`dict`): Rotator angles `{"pitch": float, "yaw": float, "roll": float}`.
- **Expected Return Type:** `str` (the name of the spawned actor).

#### 3. `set_actor_transform`
- **Description:** Moves or rotates an actor.
- **Parameters:**
  - `actor_name` (`str`): The name or label of the target actor.
  - `location` (`dict`, optional): `{"x": float, "y": float, "z": float}`.
  - `rotation` (`dict`, optional): `{"pitch": float, "yaw": float, "roll": float}`.

---

### B. Material & Appearance Tools (`toolset_registry.toolsets.core.material`)
Exposes tools for modifying material parameters dynamically.

#### 1. `override_scalar_parameter`
- **Description:** Modifies a scalar parameter on a material instance.
- **Parameters:**
  - `material_path` (`str`): Full asset reference path to the Material Instance.
  - `param_name` (`str`): Name of the scalar parameter (e.g., `"EmissiveIntensity"`).
  - `value` (`float`): The new parameter value.

#### 2. `override_vector_parameter`
- **Description:** Modifies a vector/color parameter on a material instance.
- **Parameters:**
  - `material_path` (`str`): Full asset reference path to the Material Instance.
  - `param_name` (`str`): Name of the vector parameter (e.g., `"BaseColor"`).
  - `value` (`dict`): `{"r": float, "g": float, "b": float, "a": float}`.

---

### C. Asset & Content Tools (`toolset_registry.toolsets.core.asset`)
Exposes tools for searching, loading, and inspecting assets.

#### 1. `find_assets_by_class`
- **Description:** Searches the Content Browser for assets of a given class.
- **Parameters:**
  - `class_name` (`str`): Class name (e.g., `"Blueprint"`, `"Texture2D"`).
  - `path_prefix` (`str`, optional): Restricts the search (e.g., `"/Game/Blueprints"`).

---

### D. Automation & Testing Tools (`toolset_registry.toolsets.core.automation`)
Exposes editor automation and unit-testing capabilities.

#### 1. `run_automation_tests`
- **Description:** Executes a list of unit or integration tests inside the Unreal test runner.
- **Parameters:**
  - `test_filter` (`str`): The filter string matching target tests (e.g., `"WispCPP7VR.SaveSystem"`).
- **Expected Return Type:** `dict` (detailed test outcome with pass/fail and assertions).

---

## 📡 2. Expected JSON-RPC Handshake Format

When communicating with `http://127.0.0.1:8000/mcp`, the offline bridge must wrap all calls using standard **JSON-RPC 2.0**.

### Example JSON-RPC Request (Spawn Actor):
```json
{
  "jsonrpc": "2.0",
  "id": 101,
  "method": "tools/call",
  "params": {
    "name": "spawn_actor",
    "arguments": {
      "class_path": "/Game/RunicSystem/Blueprints/BP_RunicNode.BP_RunicNode_C",
      "location": {
        "x": 1250.0,
        "y": -500.0,
        "z": 110.0
      },
      "rotation": {
        "pitch": 0.0,
        "yaw": 180.0,
        "roll": 0.0
      }
    }
  }
}
```

### Example JSON-RPC Response (Success):
```json
{
  "jsonrpc": "2.0",
  "id": 101,
  "result": {
    "content": [
      {
        "type": "text",
        "text": "BP_RunicNode_C_1"
      }
    ]
  }
}
```

---

## 🏛️ 3. SSoT INDEX.md Registration
The file `AI_Nexus/INDEX.md` must register these documents to prevent **Knowledge Drift**:
*   `AI_Nexus/Research/UE5.8_MCP_IronKnight_Synergy.md` (Level 3 - Mathematical & Research)
*   `AI_Nexus/Research/UE5.8_MCP_Tools_Reference.md` (Level 3 - Mathematical & Research)

---
// "Structured knowledge precedes automated mastery." [The Researcher] 2026-08-05
