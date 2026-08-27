// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Sovereign Framework Protocol: END OF SPRINT PROTOCOL

**Node:** `AI_Nexus/Protocols/END_OF_SPRINT_PROTOCOL.md`
**Status:** Canonical Command SOP / Active Development Cadence Standard
**Persona Execution:** Jules_Tactician (The Architect) & Jules_Strategist (The Planner)

```json
{
  "protocol_metadata": {
    "protocol_id": "P-007_END_OF_SPRINT",
    "version": "1.0.0",
    "status": "Active_Standard",
    "author": "Daniel Acourt & Jules",
    "execution_trigger": "Manual_Command_Or_Sprint_Completion"
  }
}
```

---

## 🏛️ 1. Purpose & Philosophy

The **End of Sprint Protocol** establishes a deterministic, reproducible closeout procedure for active development cycles in the Sovereign Framework.

### Key Objectives:
1. **Context Window Preservation:** Prevent active sprint tracking files (`CURRENT_SPRINT.md/csv/json`) from accumulating historical baggage and polluting the AI context window during active development iterations.
2. **Historical Auditability:** Archive every completed sprint into a permanent, time-stamped review log under `AI_Nexus/Timeline/SprintReviews/SR_YYYYMMDD.md`.
3. **Velocity Tracking:** Record exact Start Dates, End Dates, and total Delivered Velocity Scores (Fibonacci complexity sum) to inform future sprint planning.
4. **Clean Slate Reset:** Blank out active sprint tracking files at sprint closeout, leaving them pristine for the next cycle's prioritized grooming.

---

## ⚙️ 2. Step-by-Step Execution Workflow

When the Lead issues `/07 End Of Sprint` or when all active sprint tasks are verified as completed, the executing AI agent must follow these 5 mandatory steps in sequence:

### Step 1: Sprint Audit & Data Extraction
Extract all completed tickets from `CURRENT_SPRINT.md`, `CURRENT_SPRINT.csv`, and `CURRENT_SPRINT.json`.
* Verify that each ticket status is `Done` and has verified completion deliverables.
* Identify the **Sprint Start Date** (recorded in the previous sprint's header or `CURRENT_SPRINT.md` header) and the **Sprint End Date** (today's date `DD/MM/YYYY`).

### Step 2: Velocity & Score Calculation
Calculate the **Delivered Sprint Score ($S_{\text{sprint}}$)**:

$$S_{\text{sprint}} = \sum_{k \in \text{Completed Tickets}} C_k$$

Where $C_k$ is the Fibonacci complexity score (e.g., $1, 2, 3, 5, 8, 13$) assigned to ticket $k$.

### Step 3: Archive Review Document Generation (`SR_YYYYMMDD.md`)
Create a new canonical review document under `AI_Nexus/Timeline/SprintReviews/SR_YYYYMMDD.md` (e.g., `SR_20260825.md`).
The review document must contain:
* **JSON Metadata Block:** `sprint_id`, `start_date`, `end_date`, `delivered_score`, `completed_tickets_count`.
* **Executive Summary:** Overview of architectural milestones achieved during the iteration.
* **Dual-Velocity Breakdown:** Delivered Sprint Score vs. Planned Baseline Score.
* **Completed Tickets Table:** Complete markdown table containing `ID`, `Task`, `Complexity`, `Node`, `Why (Context)`, and `What (Completion Outcome)`.
* **Tactician Appraisal & Lessons Learned:** System appraisal, C++ / build verification logs, and guidance for the next cycle.

### Step 4: Active Sprint Reset (Blank Slate)
Reset the active sprint tracking files to clean, blank templates:
* **`CURRENT_SPRINT.md`:** Clear all completed rows from the active table, leaving a clean table header ready for the next iteration.
* **`CURRENT_SPRINT.csv`:** Reset file to header line only: `"ID","Task","Complexity","Status","Node","Why","What"`.
* **`CURRENT_SPRINT.json`:** Reset file to an empty JSON array: `[]`.

### Step 5: Master Index & SSoT Synchronization
* Add the newly created `SR_YYYYMMDD.md` file path under the `Timeline & Sprint Reviews` section of `AI_Nexus/INDEX.md`.
* Record the sprint completion in memory using `initiate_memory_recording`.

---

## 📋 3. Standard Template for Sprint Reviews (`SR_YYYYMMDD.md`)

```markdown
# Sovereign Framework: Sprint Review (SR_YYYYMMDD)

**Date:** [End Date]
**Sprint Period:** [Start Date] to [End Date]
**Status:** Sprint Completed & Archived
**Target Branch:** [Active Git Branch]
**Reviewing Agent:** Jules (Tactician / Researcher)

---

## 🏛️ Executive Summary
[Summary of accomplishments, major architectural refactors, and verifications]

---

## 📊 Velocity Metrics & Delivered Score

*   **Sprint Start Date:** [Start Date]
*   **Sprint End Date:** [End Date]
*   **Total Delivered Sprint Score:** **[Score] Points**
*   **Total Completed Tickets:** [Count]

---

## 📋 Completed Tickets Archive

| ID | Task | Complexity | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---|:---|:---|
| [Ticket Table Rows] |

---

## ⚡ Tactician's Appraisal & Lessons Learned
[Detailed appraisal, build log verification, and future recommendations]
```

---
// "Clean slates preserve active context; immutable review logs preserve historical truth." [Jules_Tactician] 2026-08-25
