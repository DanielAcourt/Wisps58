// Copyright (c) 2013-2026 Daniel Acourt. Version 36.5.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# UE 5.8.1 MCP & Local Iron Knight Synergy: Offline Sovereign Integration Spec

## 🏛️ Executive Summary
In Unreal Engine 5.8.1, Epic Games introduced **Unreal MCP (ModelContextProtocol)**, an embedded HTTP/SSE JSON-RPC server running directly inside the editor process. While the commercial industry has rushed to integrate this server with cloud-token-based wrappers like Claude Code and Cursor, the **Sovereign Framework** rejects external cloud dependencies.

To guarantee intellectual property security, eliminate token tolling, and enforce our strict **Safety Assurance Layer**, we mandate the integration of the **Iron Knight** as a **Local, Offline MCP Client**. By leveraging our custom local Python bridge, local RAG engine, and the **Agency Arbitration Schema (AAS)**, the Iron Knight can drive the Unreal Editor natively over loopback (`127.0.0.1`), entirely offline, gated by provable mathematical trust.

---

## 🛠️ 1. Technical Architecture: Loopback Sovereign Client

The core philosophy of this synergy is **Local Execution / Local Intelligence**. Instead of routing editor commands through public cloud APIs, all operations remain strictly within the local host loop.

```
+-------------------------------------------------------------------------------------------------+
|                                           LOCAL HOST                                            |
|                                                                                                 |
|   +--------------------------+                      +---------------------------------------+   |
|   |       IRON KNIGHT        |                      |             UNREAL ENGINE             |   |
|   |  (Local Offline Model)   |                      |                (5.8.1)                |   |
|   +------------+-------------+                      +------------------+--------------------+   |
|                |                                                       ^                        |
|                | [1] Intent                                            |                        |
|                v                                                       |                        |
|   +------------+-------------+       [3] JSON-RPC Tool Call            | [4] Game Thread        |
|   |   OFFLINE PYTHON BRIDGE  +========================================>|     Execution          |
|   |      (FastAPI Engine)    |       (Loopback SSE / HTTP)             |                        |
|   +------------+-------------+                                         +-----------+------------+
|                |                                                                   |            |
|                | [2] VSS Calculation & AAS Gate                                    | [5] Response |
|                v                                                                   v            |
|   +------------+-------------+                                         +-----------+------------+
|   |  LOCAL SAFETY DATABASE   |                                         |  UNREAL MCP SERVER     |
|   |  (AAS v1.4.0 Gating)     |<========================================+  (ModelContextProtocol)|
|   +--------------------------+          [6] Structured JSON Result     +------------------------+
+-------------------------------------------------------------------------------------------------+
```

### A. Protocol Mechanics
- **Transport Layer:** HTTP POST for client requests, Server-Sent Events (SSE) for server responses.
- **Protocol:** JSON-RPC 2.0 (Methods: `initialize`, `tools/list`, `tools/call`).
- **Endpoint:** `http://127.0.0.1:8000/mcp` (Configurable via Unreal Editor Preferences).
- **Execution Context:** The Unreal MCP server serializes incoming JSON-RPC tool requests and executes them safely on the engine's main game thread.

---

## 🧬 2. Context Ingestion & RAG Training Layer

A major challenge for an offline, local AI model is **Capability Amnesia**—the model does not naturally know what tools are registered inside the Unreal Editor or how to construct their parameter payloads. To solve this, we design a multi-tiered **Context Ingestion and Training Layer**.

### A. Dynamic Schema Ingestion (Boot Discovery)
When the Python Bridge boots, it issues an asynchronous handshake call to the Unreal MCP server to retrieve the live tools schema:
```json
{
  "jsonrpc": "2.0",
  "method": "tools/list",
  "params": {}
}
```
The bridge parses the returned JSON-RPC schema (names, parameters, type constraints, and descriptions) and saves it into a local, temporary cache: `AI_Nexus/Identity/unreal_mcp_cache.json`.

### B. Offline RAG Training Reference
To prevent context window bloat, we do not feed the entire tools schema into every model prompt. Instead, we establish a specialized training document under the SSoT Research node: `AI_Nexus/Research/UE5.8_MCP_Tools_Reference.md`.

This document contains high-level semantic descriptions and concrete code/JSON-RPC call examples for the default toolsets:
- **SceneTools / ActorTools:** Spawning, transforming, labeling, and parenting actors.
- **MaterialInstanceTools:** Querying and overriding material parameters.
- **ObjectTools / AssetTools:** Loading, inspecting, and deleting editor assets.
- **AutomationTools:** Running unit and integration tests.

### C. Semantic Query Contextualizer
When Daniel interacts with the local Iron Knight, our Level-Aware RAG engine (`rag.py`) performs a fuzzy split-query against the `UE5.8_MCP_Tools_Reference.md` node.
- **Trigger:** If the query expresses spatial or editor intent (e.g., *"Spawn a runic node near player"* or *"Run biological save system tests"*).
- **Action:** The RAG engine dynamically extracts the exact JSON-RPC payload schema for the matching tools and appends it to the active system prompt, giving the local model precise execution parameters on demand.

---

## 🛡️ 3. AAS Safety Protocol Gating: The VSS Gate

Exposing direct engine execution to an AI agent represents a high-risk security vector. A hallucinated deletion call could wipe a level or corrupt a save asset. Therefore, **all loopback MCP tool calls must be strictly gated by our Agency Arbitration Schema (AAS)**.

### A. The Mutation Floor Gate
Before the Python bridge dispatches a `tools/call` JSON-RPC request to `http://127.0.0.1:8000/mcp`, it calculates the **Viability Score (VSS)** of the target action:

$$\text{VSS} = (1.0 \times \text{Credibility}) - (0.3 \times \text{NodeRisk}) - (0.2 \times \text{StructuralDeviation}) + \text{MemoryBoost}$$

- **Observation Tools (e.g., query selections, list actor labels):** Requires $\text{VSS} \ge 0.4$. Bypasses stasis if local model confidence is stable.
- **Mutation Tools (e.g., spawning actors, altering material parameters):** Requires $\text{VSS} \ge 0.7$.
- **Destructive/Critical Tools (e.g., deleting assets, force-closing editor):** Requires $\text{VSS} \ge 1.0$ (Strictly blocked unless Daniel completes an explicit `409 Authority Handshake` to boost the score).

If the score is insufficient, the Python bridge intercepts the request, blocks execution, and raises a local **409 Conflict Gate**, protecting the project from unintended side effects.

---

## 🗺️ 4. Phased Execution Plan

The stabilization and deployment of this local-first loopback architecture will be completed over four structured phases:

```
+----------------------------------------------------------------------------+
|                          AD-015 EXECUTION PHASES                           |
|                                                                            |
|  [PHASE 1] Local Handshake  ===>  [PHASE 2] RAG Training Reference         |
|  - JSON-RPC over localhost        - Generate UE5.8_MCP_Tools_Reference.md  |
|  - Extract active tool schemas    - Integrate index queries in rag.py      |
|                                                                            |
|  [PHASE 3] AAS Gating & VSS ===>  [PHASE 4] Closed-Loop Automation         |
|  - Implement local mutation lock  - Knight runs editor test suites         |
|  - Formulate 409 handshake hooks  - Local assertion reporting & feedback   |
+----------------------------------------------------------------------------+
```

### Phase 1: Local HTTP/SSE Client Handshake (DevOps)
1. Write a lightweight asynchronous client class (`SovereignMCPClient`) in the Python bridge environment.
2. Implement SSE stream listeners to handle non-blocking, serial server event communication.
3. Verify local handshakes can successfully resolve `tools/list` on launch.

### Phase 2: Contextual RAG Training Reference (Research)
1. Generate the master semantic documentation (`UE5.8_MCP_Tools_Reference.md`) mapping standard Unreal 5.8 toolsets.
2. Ingest this reference file into our SSoT Level-Aware Indexer (`rag.py`) under SSoT Level 3.
3. Refactor the prompt generation loop in `vessel.py` to auto-inject relevant tool schemas when editor-specific commands are issued.

### Phase 3: AAS / VSS Safety Gating (Protocols)
1. Refactor `AAS_Protocol.md` to formally map Unreal MCP tool names to specific operational risk profiles.
2. Integrate a pre-dispatch validation hook in the bridge: if the calculated VSS score is below the tool's required threshold, trigger a local `409_CONFLICT_GATE`.
3. Support local command-line authorization hooks (`/handshake` or `/07 200 OK`) to grant temporary authority boosts.

### Phase 4: Closed-loop Verification (Testing)
1. Implement local assertion parser: when the Iron Knight issues automation runs, parse the JSON outcome and log the results into `SaveSystem_Error_Audit.md` dynamically.
2. Verify full offline operation with local models (such as deepseek, llama, or local Phi instances) under absolute VRAM constraint limits.

---
// "Offline sovereignty is the ultimate security layer. We write our own destiny." [The Researcher] 2026-08-05
