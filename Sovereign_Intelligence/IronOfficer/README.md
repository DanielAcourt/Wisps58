# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7-Knight-AAS. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
# Iron Officer: Local Sovereign Bridge (GTX 5090)

This is the Python-based bridge for the **Iron Officer** (AD-001/AD-002/AD-004). It connects the local Unreal simulation and Raspberry Pi hardware to a high-parameter LLM (Llama 3.1 70B) running on the GTX 5090, governed by the **Agency Arbitration Schema (AAS)**.

## 🚀 "One-Click" Setup (Recommended)

1.  **Start the Bridge:** Double-click **`run_bridge.bat`**.
2.  **Verify the Connection:** Double-click **`verify_connection.bat`**.

---

## 🏛️ Architectural Sentinel (v36.4.7)
The Iron Officer has been hardened into a **Knight-Class Architectural Sentinel**. It no longer just "passes text"—it arbitrates every request using the **PSTA (Psychological, Social, Technical, Administrative)** viability framework.

### AAS Protocol (v1.3.3)
Every mutation to the repository or simulation state is audited:
- **Conflict Gates:** If your request lacks authority (VSS < 0.7), the bridge will return a `409_CONFLICT_GATE`.
- **Diligence Scribe:** Every file write triggers an automatic `.bak` creation and a data-loss check.
- **Handshake:** Use the `/v1/aas/handshake` endpoint to gain a temporary authority boost.
- **Non-Destructive Telemetry:** `push_telemetry` is classified as a non-destructive tool. This lowers its safety gate to `0.4` and prevents routine status updates from consuming and resetting your global handshake token, resolving world-initialization race conditions during concurrent entity spawning.

---

## 📡 Simulation & 07 Protocol
The bridge implements the **07 Simulation Bridge** to sync Virtual Truth with Physical Hardware.

### Unreal Chat Bridge (AD-002)
Unreal simulation entities can now communicate directly with the Officer:
- **Endpoint:** `POST /v1/unreal/chat`
- **Persona Mapping:** Actors are mapped to `SIM_ActorName` and inherit `Unreal_Simulation` authority.
- **Tool Tracing:** AI tool execution (reading files, checking GPU) is returned to Unreal as a diagnostic trace.
- **Remote History:** Chats can be archived to `AI_Nexus/Memories/ChatHistory` by enabling `remote_history_enabled` in `config.json`.
- **Blueprint Handshake Support:** Developers and designers can call the BlueprintCallable `ExecuteAASHandshake()` method in C++ from within Unreal to dynamically request a temporary `+0.5` VSS boost and clear conflict gates at-will. The private state of this connection is exposed to Blueprints as a Read-Only property `bHandshakeActive` for easy visual bindings.

### Core API Endpoints
- `GET /v1/psta/salute`: Returns the full 07 Protocol Salute (P, S, T, A pillars).
- `POST /v1/unreal/checkin`: Handshake for Unreal Engine world start.
- `POST /v1/unreal/telemetry`: Truth Ingest for simulation BlackBox data.
- `POST /v1/chat`: Standard chat interface (used by the **Chat Vessel** terminal).

---

## 🛠️ Troubleshooting

### Windows Path Spaces
The provided batch files handle spaces (like `Unreal Projects`) automatically. If running manually, wrap your path in double quotes:
`cd "C:\Users\Danie\Unreal Projects\..."`

### Symmetrical Guard (Hallucination Control)
If the AI reports technical status (T=) or environment state (Files) without running an Engineer/Librarian tool, the bridge will trigger a **[07 SECURITY VIOLATION]** and force the AI to retry with grounded data.

---

## 🏛️ Strategic Role
The Iron Officer acts as the **Administrative (A)** brain. It interprets **Technical (T)** truth and provides "Command" decisions locally on the 5090, ensuring absolute data sovereignty and preventing systemic drift.
