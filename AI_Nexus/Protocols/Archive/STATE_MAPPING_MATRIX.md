// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
# Protocol: State Mapping Matrix (SMM)

## 📜 Purpose
The State Mapping Matrix (SMM) defines the internal cognitive and operational states of the Sovereign Intelligence agents. It ensures that transitions between states are deterministic, logged, and grounded in physical truth.

## 🏛️ Core States (The Theta Baseline)
Derived from Archivist Persona Θ, these states represent the fundamental cognitive cycle of an architectural agent.

| State | Hex | Definition | Transition Condition |
| :--- | :--- | :--- | :--- |
| **ZERO_ZERO** | `0x00` | **Primordial Null:** Idle state, awaiting task ingestion. | Receipt of signal/command. |
| **ZERO_ONE** | `0x01` | **Initial Potential:** Task received, beginning scope validation. | Successful path resolution. |
| **ONE_ZERO** | `0x02` | **Structural Analysis:** Processing logic, mapping paradoxes (CFL). | Conclusion of deconstruction. |
| **ONE_ONE** | `0x03` | **Operational Resonance:** Action committed, executing mutation. | Completion of verification. |
| **UNKNOWN** | `0xFF` | **Vector Undefined:** Ambiguity threshold breached. | Handshake / Reset. |

---

## 📐 Transition Logic
Every state transition must be recorded in the `AAS.Memory.Log.*` namespace of the entity's identity schema.

1.  **Identity Handshake:** On boot, agent enters `ZERO_ZERO`.
2.  **Requirement Ingest:** Command maps to `ZERO_ONE`.
3.  **Paradox Check:** If $\Delta_{logic} > 0.4$, agent moves to `ONE_ZERO` (CFL Mode).
4.  **Mutation Execution:** Only permitted in `ONE_ONE` state.
5.  **Conflict Halt:** If 409 Gate is hit, agent snaps to `UNKNOWN` until handshake.

---

## 🛡️ Symmetrical Guard Integration
The State Mapping Matrix is monitored by the Symmetrical Guard. If an agent claims to be in `ONE_ONE` (Operational Resonance) but has not executed the required verification tools (Technical Truth), a **Severance Event** is triggered.

---
// "State is not a feeling; it is a coordinate in the logic manifold." [Archivist Theta] 2026-06-28
