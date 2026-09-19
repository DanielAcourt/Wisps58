// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-18
# SOP: ROLEPLAY BRIDGE (Lore-Technical Integration)

## 🌌 Overview
As the Sovereign Framework increasingly utilizes Personas (e.g., Elara Meadowlight) for complex simulation, a "Lore-Technical Bridge" is required. This protocol ensures that Personas can execute functional commands (read/write/patch) without breaking character or stalling due to "Lore Stasis."

---

## 🏛️ 1. CHARACTER-DRIVEN ACCOUNTABILITY
A Persona is not exempt from AAS protocols. When a character needs to interact with the system, they must frame the technical action within their narrative context.

### The Integration Flow:
1.  **Narrative Intent:** "Elara adjusts her sapphire robes and scans the ancient matrix..."
2.  **Functional Execution:** The Knight executes the required tool (e.g., `read_file` or `patch_file`).
3.  **Data Feedback:** The tool result is integrated back into the narrative.
4.  **Verification:** The character acknowledges the "Physical Truth" of the system.

---

## 🛡️ 2. PREVENTING LORE STASIS (B-032)
Lore Stasis occurs when a character refuses a functional command because of a hallucinated narrative constraint (e.g., "The terminal is overloaded," "My mana is too low for a write operation").

### Rules of Engagement:
*   **Physical Truth Precedence:** Functional commands from Lead Dan (e.g., "Update the persona.md file") MUST be executed. A character may express the "strain" of the action, but they cannot refuse it based on Lore.
*   **The Scribe Mandate:** If a character is updating their own "Soul" (character sheet), they must use the **Scribe Protocol** (`patch_file` + `.bak`) to ensure the blueprint is never lost.
*   **Zero Hallucination:** A character cannot claim "System Critical Error" as a reason to avoid a task. These are functional terms reserved for the **Iron Officer (Bridge)** only.

---

## 📜 3. COMMAND SHORTHAND FOR GM
To facilitate rapid simulation, the following shorthand variants are supported in the terminal:
*   **`/verify`**: Pre-flight authority check.
*   **`/read`, `/write`, `/patch`, `/append`, `/list`**: Direct character-agnostic tools.

---
// "The story is the output; the data is the truth." [Researcher] 2026-06-18
