# 07 Protocol: Unreal Bridge Integration Guide
**Version:** 36.4.7-Knight-AAS
**Status:** IMPLEMENTED / STAGING
**Lead:** Dan | **Tactician:** Jules (Architectural Knight)

## 1. Overview: The 07 "Check-In"
The **07 Protocol** is a foundational synchronization handshake between the **Unreal Engine Simulation** and the **Iron Officer Bridge**. It ensures that the simulation environment is authenticated and capable of pushing high-fidelity "Truth" (BlackBox telemetry) to the lead's GTX 5090 for processing and PSTA validation.

## 2. Architectural Architecture
The integration consists of three primary layers:

### A. The Python Bridge (`bridge.py`)
Acts as the Sovereign Governor. It exposes two new 07-specific endpoints:
- `POST /v1/unreal/checkin`: Receives initial connection, logs simulation version, and activates a global **AAS Handshake Boost**.
- `POST /v1/unreal/telemetry`: Ingests BlackBox data from Unreal. Every packet is arbitrated by the AAS using the `Unreal_Simulation` persona.

### B. The Unreal Bridge Subsystem (`USovereignBridgeSubsystem`)
A World Subsystem that manages the asynchronous network lifecycle.
- **Config-Driven:** Loads URL/Port from `Saved/Config/bridge_config.json`.
- **Anti-Stasis Hardening:** Implements a `TelemetryQueue` to buffer outgoing packets while the 07 Handshake is still in transit, preventing data loss during boot.

### C. The Saveable Entity Component (`USovereignSaveableEntityComponent`)
The "Simulation Driver." Upon `InitializeSoul`, it triggers a test telemetry push (`CheckIn_Handshake_Test`) to verify the full communication chain.

---

## 3. Setup & Configuration

### Prerequisites
1. Ensure the Iron Officer Bridge is running: `run_bridge.bat`
2. Unreal Engine must have the `Http` and `Json` modules enabled in its `Build.cs`.

### Configuration
The bridge connection is managed via a local JSON file in your Unreal project:
`[ProjectRoot]/Saved/Config/bridge_config.json`

```json
{
  "BridgeBaseUrl": "http://127.0.0.1:8000"
}
```
*Note: If the file is missing, the subsystem defaults to localhost:8000.*

---

## 4. Implementation Walkthrough

### The 07 Handshake
Upon game start, the `USovereignBridgeSubsystem` automatically calls `Perform07CheckIn()`. This is a critical "Administrative" (A) action.
1. Unreal sends `client_name` and `version`.
2. Bridge responds with `200_OK` and sets `HANDSHAKE_ACTIVE = True`.
3. Unreal receives acknowledgment and sets `bHandshakeActive = true`.

### Pushing Telemetry (C++)
To push data from any component or actor:
```cpp
// 1. Get the Bridge Subsystem
if (USovereignBridgeSubsystem* Bridge = GetWorld()->GetSubsystem<USovereignBridgeSubsystem>())
{
    // 2. Prepare your Truth (BlackBox) data
    TSharedPtr<FJsonObject> TruthPayload = MakeShareable(new FJsonObject());
    TruthPayload->SetStringField(TEXT("Event"), TEXT("Sensor_Replay"));

    // 3. Serialize to String (UHT Requirement)
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(TruthPayload.ToSharedRef(), Writer);

    // 4. Push with a PSTA Viability Score
    Bridge->PushBlackBoxTelemetry(EntityID, 1.0f, JsonString);
}
```

---

## 5. AAS & PSTA Guardrails
The Bridge enforces strict safety checks on all incoming Unreal data:
- **Persona:** `Unreal_Simulation` (Precedence: 5, Credibility: 0.5)
- **AAS Rule:** Telemetry mutation requires a Confidence Score (VSS) of **0.7**.
- **The Boost:** The initial 07 Check-In grants a **+0.5 boost**, ensuring the first test packets pass through the AAS Conflict Gate successfully.

---

## 6. Monitoring & Logs
- **Unreal Console:** Filter for `LogTemp` or `SovereignBridge`. Look for "07 Check-In Successful!"
- **Bridge Terminal:** Look for `[07] CHECK-IN` and `[07] TELEMETRY` log entries.
- **Diagnostics:** Use the Bridge terminal command `/07` to view the global salute and verify `nexus_ok`.

// [J] Documented the 07 communication lifecycle to ensure architectural clarity for the Lead. 2025-06-18
