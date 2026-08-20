// Copyright (c) 2013-2025 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# Sovereign Framework: WispCPP7VR (v37.0.0-Knight-AAS)

## 🌌 Overview
The **Sovereign Framework** (WispCPP7VR) is a high-performance, modular C++ architecture for Unreal Engine designed for immersive VR experiences. It provides a robust foundation for entity possession, data-driven simulations, and real-time environmental telemetry. The framework blends rigorous technical standards with a spiritual, meditative theme centered around "Wisps," "Souls," and "Possession."

---

## 🤖 Sovereign Intelligence: The Iron Knight Bridge (`Sovereign_Intelligence/`)
The `Sovereign_Intelligence/` folder is the administrative and cognitive engine of the framework, running locally to interface your Unreal Engine simulation directly with local hardware acceleration (e.g. your GTX 5090).

*   **`IronOfficer/bridge.py` (The Bridge):** A local high-performance FastAPI server. It acts as the gatekeeper for system telemetry, AAS (Agency Arbitration Schema) protocol validation, and PSTA (Provable Trust) evaluation.
*   **`IronOfficer/vessel.py` (The HMI):** A lightweight console chat medium allowing you to communicate directly with the local AI model (using Ollama and Llama3.1) and issue `/report` commands.

### 🏛️ Sovereign Alignment Core
To ensure absolute epistemic grounding and prevent systemic hallucinations, the bridge enforces two fundamental architectural pillars:

| Principle / Guard | Academic & Systems Lexicon | Operational Manifestation |
| :--- | :--- | :--- |
| **The Soul & The Vessel** | *Ontological Decoupling / State Survivability* | We decouple the **Cognitive State (The Soul)** from the **Physical Instance (The Vessel)**. The AI's identity and data contracts survive even if the physical render or hardware node collapses. |
| **The Symmetrical Guard** | *Epistemic Grounding / Symmetrical Co-inference* | We prevent hallucinations by forcing a **Symmetrical Guard**. The AI cannot make claims about its environment without executing a physical measurement tool first. It grounds 'thinking' in 'doing.' |

### 🔌 Iron Knight Simulation API
The bridge exposes specialized REST endpoints for real-time simulation synchronization:
*   `POST /v1/unreal/checkin`: Performs a 07 Protocol check-in handshake to verify connectivity and initialize the global authority boost.
*   `POST /v1/unreal/telemetry`: Ingests multi-component BlackBox telemetry data and PSTA viability parameters from simulation souls.
*   `POST /v1/unreal/chat`: Supports stateful/stateless dialogue between simulation-born actors (e.g. starting with `SIM_`) and the AI, returning detailed diagnostic tool execution logs.
*   `POST /v1/aas/handshake`: Dynamically re-arms a single-use global authority boost (`+0.5` VSS) to clear administrative logical barriers.

---

## 🏛️ Sovereign Intelligence Hierarchy (SSoT)
The project adheres to a strict **Single Source of Truth (SSoT)** hierarchy, managed within the `AI_Nexus/` directory to ensure alignment between high-level intent and low-level implementation. Refer to the [AI Nexus Master Index](AI_Nexus/INDEX.md) for a complete file map.

*   **Level 0:** [Timeline & Evolution](AI_Nexus/Timeline/MD.md). Tracks the project's historical context and long-term vision.
*   **Level 1:** [The Rulebook](AI_Nexus/Protocols/AGENTS.md). Standard operating procedures, multi-agent coordination protocols, and architectural constraints.
*   **Level 2:** [Identity & Internal Logic](AI_Nexus/Identity/identity.json). Defines the "Current Truth" for every actor in the world via the `USovereignSaveableEntityComponent` (The Soul).

---

## 👻 The "Modular Soul" Architecture (`USovereignSaveableEntityComponent`)
Evolved from a monolithic entity component, the **Sovereign Soul** operates on a decoupled, **Mediator-based Hub** pattern (The Soul Hub):

*   **Central Mediator:** `USovereignSaveableEntityComponent` registers specialized brokers on `BeginPlay` and orchestrates all serialization and Bridge transmission.
*   **Specialized Components:**
    *   `USovereignBioComponent` (Biology, Lineage, Maturity, Metabolism)
    *   `USovereignQiComponent` (Core Cultivation, Qi Capacity, Alignment, Resonance)
    *   `USovereignElementComponent` (Body Sockets, Resistance, Affinities)
    *   `USovereignAttributeComponent` (Core RPG attributes, Health)
*   **Categorized Truth:** All telemetry is organized into clean named root categories (`Identity`, `Bio`, `Qi`, `Elements`, `Attributes`, `UnknownTags`) preventing data pollution.

---

## 🎨 Sovereign Nested UI & Blueprint Handshake
To support flexible, non-blocking visual feedback during active VR play sessions:
*   **`USovereignMasterHUD`:** Handles automated runtime discovery of registered specialized component brokers and injects the Soul Hub pointer into nested sub-widgets.
*   **`USovereignBaseWidget`:** Automates UI element show/hide logic based on active category brokers and converts serialized states to simple C++-handled `TMap<FString, FString>` maps for safe, robust Blueprint binding.
*   **`USovereignBridgeSubsystem::ExecuteAASHandshake`:** Exposes a `BlueprintCallable` action to request an authority boost directly from Unreal. This enables actors to dynamically lift `409 CONFLICT` gates when telemetry pushes collide with single-use handshake tokens during concurrent world-initialization.

---

## 🛸 Possession Lifecycle & SOP
Possession is a critical mechanic governed by a strict **Standard Operating Procedure (SOP)** to prevent actor-state corruption and ensure visual consistency in VR. The lifecycle is managed by `HandlePossessionLifecycle()`, which toggles between entering and exiting a host.

### The "Sovereign Rule" for Transforms
All attachments during the possession lifecycle **must** use:
`FAttachmentTransformRules::SnapToTargetNotIncludingScale`
This ensures the possessing Spirit does not inherit the scale of its host Vessel, maintaining visual integrity.

### Technical Implementation Sequence:

#### 1. Entering a Host (`AttemptPossession`)
*   **Targeting:** Performs a `SphereTraceSingleForObjects` to find a valid Vessel.
*   **Validation:** Checks `CanBePossessed()` via the `IInteractionInterface`.
*   **Attachment:** Uses the "Sovereign Rule" to attach to the target's `GetPossessionAttachmentComponent()`.
*   **Control Handover:** The Player Controller `Possess()`es the target Pawn.
*   **Spirit State:** The Spirit actor is hidden, and collision is set to `QueryOnly` to avoid physical interference while inhabiting the host.

#### 2. Exiting a Host (`EjectFromHost`)
*   **Unhide & Physics:** Restores the Spirit's visibility and `QueryAndPhysics` collision.
*   **Detachment:** Calls `DetachFromActor(FDetachmentTransformRules::KeepWorldTransform)`.
*   **Safe Offset:** Applies a safe world offset (e.g., `Z + 80.0f`) to prevent the Spirit from spawning inside the host's collision.
*   **Re-possession:** Control is handed back to the Spirit actor via `PC->Possess(this)`.

---

## 🐟 Digital Twin & Fish Tank Alpha (FTA)
The Sovereign Framework features advanced "Digital Twin" capabilities, allowing it to consume real-world IoT data.

**Fish Tank Alpha (FTA)** is the flagship implementation of this system, mapping live environmental telemetry from `DansOfficeTank_0` to Unreal Actor properties:
*   `temp_c`: Water temperature (Celsius).
*   `ph_val`: pH level.
*   `water_depth_mm`: Water depth in millimeters.

---

## 📦 Black Box Protocol (Data Survivability)
The **Black Box Protocol** (B-002) is a specialized persistence layer designed for "Mission-Critical Truth." It ensures that high-fidelity telemetry survives even if the simulation "Soul" is decoupled from its current "Vessel."

*   **Delta-Based Logging:** Only records data when changes exceed a defined threshold (default: 0.2), minimizing storage bloat while capturing significant events.
*   **Truth Ingestion (The Replay):** Supports replaying external telemetry (e.g., from a Raspberry Pi) back into Unreal actors, enabling high-fidelity 3D reconstruction and historical visualization.
*   **Atomic Persistence:** Uses the `USovereignBlackBoxSubsystem` to perform atomic writes to standalone JSON files in `/Saved/BlackBox/`, protecting against data corruption.
*   **Identity-Linked:** Logs are partitioned by `EntityID` (Guid), creating a permanent historical record for every sovereign entity.

See the [Black Box Protocol Documentation](AI_Nexus/Docs/BlackBox_Protocol.md) for technical implementation and usage details.

---

## 🎮 Input Conventions
The framework enforces a standardized input model to ensure a consistent user experience:
*   **[F] Key:** Possession lifecycle (Initiate Possession / Unpossess / Eject). Bound to `HandlePossessionLifecycle`.
*   **[E] Key:** Contextual Interaction (Harvesting, Activating, Conversing). Bound to `Interact`.

---

## 🏗️ Framework Capabilities (The StoreFront)
*   **Interface-Based Possession:** Decoupled `IInteractionInterface` allows any Actor to become a possessable Vessel.
*   **Asynchronous Asset Loading:** Uses `FStreamableManager` for zero-hitch mesh updates and entity evolution.
*   **Data-Driven Spawning:** `USovereignSpawnManager` utilizes `USovereignSpeciesData` for class-agnostic, async actor instantiation.
*   **Identity Guarding:** A "Logic Gate" check ensures the physical Actor class matches its logical Identity Signature before spawning.

---

## 📜 Architectural Standards & Guidelines
*   **AI_Nexus/:** The "Neutral Zone" for AI coordination, shared data, and high-level documentation.
*   **Polite Persistence:** Saveable components extract only their relevant keys from the "Suitcase" (`TMap<FString, FString>`), ensuring clean data management.
