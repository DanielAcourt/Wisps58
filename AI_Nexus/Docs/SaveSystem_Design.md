# Sovereign Save System: Architecture of Continuity

## ⛩️ 1. The Core Philosophy
The Sovereign Save System is not merely a "game state" recorder; it is the **Architecture of Continuity** for the "Message in a Bottle" research vision. It bridges the gap between high-fidelity 3D simulation and decoupled historical truth.

### 1.1 The Soul and the Vessel
*   **The Soul (`USovereignSaveableEntityComponent`):** The Single Source of Truth (SSoT) for an entity's identity, lineage, and metadata. It exists independently of its visual representation.
*   **The Vessel (`AActor`):** The physical container in Unreal Engine. It is a **Reflection of Truth**, responsible for rendering, physics, and local simulation logic.

---

## 🧬 2. The Dynamic DNA (Unknown Meta-Tags)

### 2.1 The "Sandwich" Proof
To verify the system's ability to persist data without pre-defined C++ members, we use the **"Sandwich" Tag** (Note: spelled as `Sandwhich` in test logs for deterministic matching).
*   **The Logic:** A tag named `Sandwhich` is injected into the entity at runtime (via `AddUnknownTag` or `AActor::Tags`).
*   **The Proof:** The presence of `"Sandwhich": "True"` in the generated `.json` proves that the "Soul" can ingest, carry, and manifest "Unknown Truths" that have no hard-coded equivalent in the C++ Vessel.
*   **Strategic Value:** This is the cornerstone of the **PSTA Technical (T)** pillar. It proves the system can ingest novel sensor data (e.g., Lidar artifacts, IoT telemetry) without requiring a recompile of the core engine.

#### 💡 Forensic Evidence (JSON Proof)
```json
{
    "GUID": "C69C257843DCF78767898AA819929D9D",
    "ParentID": "93B96D334E9DC507F0162C8CA927B6FA",
    "Class": "/Game/Blueprint/Actors/Living/BP_Antelope.BP_Antelope_C",
    "MetaTags":
    {
        "Sandwhich": "True", // <-- THE PROOF: Dynamic DNA persisting in the Soul
        "AttributeComponent.STR": "10",
        "QiComponent.Qi.Current": "0.0",
        "Telemetry.temp_c": "0.0"
    }
}
```

### 2.2 Ingestion & Replay
Unknown tags allow the framework to act as a **Deterministic Replayer**. If an external Raspberry Pi sends a new telemetry key like `Telemetry.FluidDensity`, the Sovereign Framework:
1.  Captures it in the `UnknownMetaTags` map.
2.  Persists it to the Black Box log.
3.  Allows Unreal to "Reflect" this truth visually (e.g., changing material density) during replay.

---

## 🏗️ 3. The Save Hierarchy (The Sandwich Layering)

The save process operates in a layered "Sandwich" approach to ensure data integrity:

*   **Pass A: Identity & Identity Tags:** Scrapes `AActor::Tags` and `IGameplayTagAssetInterface` to establish "Who" the entity is.
*   **Pass B: Vessel DNA (Physicality):** Scrapes components implementing `ISovereignSaveInterface` (e.g., `AttributeComponent`, `QiComponent`) to establish "What" the entity is currently doing.
*   **Pass C: The Meta-Suitcase:** Packs all recognized and unrecognized data into a flat JSON structure, ensuring zero data loss during the Sim-to-Real transition.

---

## 🕰️ 4. Truth vs. Reflection (The Digital Museum)

The Sovereign Framework operates on a **Truth-First** paradigm:
*   **External Truth:** The Lidar scans of the Titanic or the Pi logs of a maritime journey are the "Absolute Truth."
*   **Unreal Reflection:** Unreal Engine is the **Simulation Layer**. It uses the Black Box logs to reconstruct the journey, using high-fidelity meshes (.pak/.DLB) to render a visual approximation of the historical data.

This distinction is critical for the **Minimum Viable Soul (MVS)**:
*   The **MVS** is the point where the "Soul" (JSON Metadata) and the "Vessel" (Refined 3D Model) synchronize perfectly to tell a historical story through spatial intelligence.

---

## 📊 5. PSTA Integration (The Technical Pillar)
The Save System is the primary feeder for the **Technical (T)** dimension of the PSTA framework.
*   **Integrity Verification:** By comparing the expected "Truth" in the JSON with the actual "Reflection" in the simulation, the PSTA system calculates an **Integrity Rating**.
*   **Decision Making:** If the delta between Truth and Reflection exceeds a threshold, the Technical Pillar collapses, triggering the PSTA Bottleneck Law and notifying the Lead of a simulation drift.
