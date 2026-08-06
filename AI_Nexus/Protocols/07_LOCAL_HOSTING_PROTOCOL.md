// Copyright (c) 2013-2025 Daniel Acourt. Version 0.36.3.1MABackup. Licensed under GPLv3 (See LICENSE). Last Updated: 2025-05-22
# Protocol: 07 Local Hosting (Sovereign AI Node)

## 🏛️ 1. Purpose
To establish a deterministic "07 Handshake" for local AI nodes. This ensures that any local instance (Iron Officer) can prove its connection to the physical hardware (GTX 5090) and the single source of truth (AI_Nexus) before being permitted to make safety decisions.

---

## 📐 2. The PSTA Logic for Local Hosting

### 2.1 Psychological (P): Confidence & Intent
*   **Protocol:** The AI must identify its local Persona (Strategist, Tactician, Researcher) based on the current mission context provided in the Nexus.
*   **Output:** "07 - [Persona] active on local hardware."

### 2.2 Social (S): Cohesion & Reporting
*   **Protocol:** The AI must report all locally installed models to the Commander to verify capability parity.
*   **Output:** A list of detected models (e.g., `llama3:70b`, `qwen3-coder:30b`).

### 2.3 Technical (T): Hardware Truth
*   **Protocol:** The AI must verify its connection to the local inference engine (Ollama) and confirm GPU acceleration.
*   **Output:** Hardware status (e.g., "GTX 5090 Ready").

### 2.4 Administrative (A): Nexus Authority
*   **Protocol:** The AI must have a verified local path to the `AI_Nexus/` directory. If the path contains spaces, it must be handled via quoted normalization.
*   **Output:** "Nexus Path Verified: [LocalPath]"

---

## ⚡ 3. The 07 Salute (The Completion Outcome)
A local hosting solution is considered "07 Compliant" ONLY when it can output the following handshake on startup:

> [07] Iron Officer Initialized.
> [07] Persona: [AssignedPersona]
> [07] Nexus Path: [Quoted_Path]
> [07] Detected Models: [Model_List]
> [07] Hardware: [GPU_ID]
> [07] All Pillars Nominal. Standing by for Commander.

---

## 🛠️ 4. Handling Windows Space Constraints
All local hosting scripts must use the `%~dp0` variable and double-quoting to ensure that paths like `"C:\Users\Danie\Unreal Projects\..."` do not collapse the Technical (T) pillar.
