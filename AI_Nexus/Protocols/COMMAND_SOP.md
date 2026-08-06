// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2025-05-22
# Command & Delegation Protocol (The Sun Tzu SOP)

> "If words of command are not clear and distinct, if orders are not thoroughly understood, then the general is to blame. But, if orders are clear and the soldiers nevertheless disobey, then it is the fault of their officers." — *Sun Tzu*

## 📜 Purpose
To eliminate **Assumption Drift** and ensure that delegation between the Technical Lead and AI Agents (Jules/Claude) is 100% deterministic.

**The Cognitive Bridge:** Because the Technical Lead thinks in **Volumes/Space (99th percentile)** but communicates in **Linear Language (30th percentile)**, this protocol acts as a translator. It ensures that the "Spatial Soul" of an idea is accurately converted into a "Linear Vessel" of instructions that an AI can execute without hallucination.

---

## 🛠 The Task Issuance Framework: WHY, WHAT, HOW

To prevent agents from running on assumptions, every major task command should ideally follow this structure:

### 1. The WHY (Objective/Philosophy)
*   **Definition:** The strategic reason for the task.
*   **Goal:** Provides context so the agent understands the "Soul" of the request.
*   **Agent Constraint:** If the "Why" is unclear, the agent MUST use `request_user_input` before proceeding.

### 2. The WHAT (Scope/Deliverables)
*   **Definition:** The specific, tangible outcomes required.
*   **Goal:** Defines the boundaries of the task to prevent feature creep or "tangents."
*   **Agent Constraint:** The agent must list these deliverables in the `set_plan` tool.

### 3. The HOW (Technical Specifics/Constraints)
*   **Definition:** The exact implementation details, file paths, or architectural rules (e.g., The Sovereign Rule).
*   **Goal:** Eliminates technical ambiguity.
*   **Agent Constraint:** The agent must follow these instructions to the letter. Deviation requires explicit approval.

---

## ⚖️ Accountability & Correction
*   **Lead Responsibility:** Ensure commands are "clear and distinct." Avoid ambiguous language.
*   **Agent Responsibility:** Execute without "hallucinating" intent. If a command is 90% clear, the agent must ask about the remaining 10% rather than assuming.
*   **Correction Loop:** If an agent goes off on a tangent, the Lead should point to the specific section of the "What" or "How" that was violated.

---

## 🛰️ Deterministic Documentation (Nexus Sync)
To prevent **Information Decay**, agents must ensure that high-level documentation and low-level task tracking are perfectly synchronized at the end of every session:
1.  **Backlog Parity:** The `BACKLOG.md` status must match the `_AGENT_CONTEXT.md` sprint status.
2.  **Memory Persistence:** Critical technical lessons learned (e.g., UE 5.7 API shifts) must be recorded in `AI_Nexus/Memories/` and referenced in `AI_Nexus/Protocols/AGENTS.md`.
3.  **Status Integrity:** Never mark a task as "Done" in the Backlog if there are pending hardening or verification steps.

---

## ⚖️ Copyright Integrity (Active Duty)
To maintain legal standing and digital heritage, all core source files and Nexus documentation must carry a standardized header.

### 1. The "0.36 Standard" Header
Every C++, Math module, and Markdown file must begin with this exact single-line format:
`// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.[x]. Licensed under GPLv3 (See LICENSE). Last Updated: YYYY-MM-DD`
*   **Version Components:** `36` (Lead's age), `4` (Son's age), `[x]` (Agent incremental version).
*   **Target Scope:** `.cpp`, `.h`, `.md` (AI Nexus & Docs).
*   **Exclusions:** `.ini`, `.uproject`, `.json`.

### 2. Enforcement Rule
*   **Mandatory Check:** Every time an AI Agent or human opens/reads a file within the scope, they MUST verify the presence and accuracy of this header.
*   **Immediate Correction:** If the header is missing, outdated, or incorrect, it must be updated immediately as part of the current task.

---

## 🛡️ Assumption Kill-Switch
If an agent detects it is making an assumption about a critical system (Saving, Spawning, Identity), it MUST:
1.  Stop execution.
2.  State the assumption clearly to the Lead.
3.  Wait for confirmation or correction.

---

## 🎖️ The Officer Principle (Interconnectedness)
Agents are expected to act as **Officers**, not just recruits.
*   **Recruit Mentality:** Follows orders to "put in a pipe" without knowing what it connects to. Result: Systems that leak and collapse.
*   **Officer Mentality:** Understands the "Soul" of the architecture. Knows that a pipe in the Spawning system affects the Persistence system.
*   **Requirement:** Agents must proactively review the `Research/` and `Admin/` nodes of the Nexus before proposing plans, ensuring that every new "pipe" is correctly integrated into the 12-year research vision.
