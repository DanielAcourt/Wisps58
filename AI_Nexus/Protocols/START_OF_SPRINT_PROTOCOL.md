// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Sovereign Framework Protocol: START OF SPRINT PROTOCOL

**Node:** `AI_Nexus/Protocols/START_OF_SPRINT_PROTOCOL.md`
**Status:** Canonical Command SOP / Active Development Cadence Standard
**Persona Execution:** Jules_Strategist (The Planner) & Jules_Tactician (The Architect)

```json
{
  "protocol_metadata": {
    "protocol_id": "P-008_START_OF_SPRINT",
    "version": "1.0.0",
    "status": "Active_Standard",
    "author": "Daniel Acourt & Jules",
    "command_trigger": "/07 Start Of Sprint",
    "baseline_sprint_duration": "28_Days_4_Weeks",
    "planned_velocity_capacity": "49_Points"
  }
}
```

---

## 🏛️ 1. Purpose & Philosophy

The **Start of Sprint Protocol** defines the initial grooming, goal alignment, and backlog ingestion workflow for a new development iteration in the Sovereign Framework.

### Core Objectives:
1. **Mandatory Start Date Anchor:** Eliminate duration ambiguity by pinning the exact **Start Date (`DD/MM/YYYY`)** on Day 1, calculating the **Target End Date (`Start Date + 28 Days`)**.
2. **Tri-Part Goal Convergence:** Derive a balanced, high-impact **Core Sprint Goal** through a 3-way synthesis:
   * **Lead Priority (Administrative Mandate $A$):** Real-world opportunities, academic deadlines, and immediate Lead vision.
   * **Backlog Priority (Technical Truth $T$):** Core architectural prerequisites and technical debt reduction from `BACKLOG.md`.
   * **AI Suggestion (Innovation Vector $S$):** Proactive proposals for R&D, spatial simulation improvements, or tooling optimizations.
3. **Tech Debt Protection (Velocity Capacity Cap):** Enforce a baseline planning capacity of **~49 Fibonacci Points** for a 4-week solo sprint. This guarantees steady, high-velocity progress without creating unmanageable technical debt.
4. **Active Sprint Context Ingestion:** Move prioritized tickets from `BACKLOG.md/csv/json` into `CURRENT_SPRINT.md/csv/json` to focus real-time AI context loading.

---

## ⚙️ 2. Step-by-Step Execution Workflow

When the Lead issues `/07 Start Of Sprint`, the executing AI agent follows these 5 steps in sequence:

### Step 0: Mandatory Start Date Recording
Record the explicit **Sprint Start Date** (`DD/MM/YYYY`) and calculate the **Target End Date** (+28 days / 4 weeks):
* *Example:* Start Date: `25/08/2026` $\implies$ Target End Date: `22/09/2026`.

### Step 1: Tri-Part Sprint Goal Convergence (Grooming Discussion)
Engage in a collaborative grooming session with the Lead to define the **Core Sprint Goal**:
1. Lead shares immediate real-world opportunities, academic deadlines, and research focus.
2. AI reviews `BACKLOG.md` for high-priority architectural dependencies.
3. AI proposes innovation/tooling vectors.
4. Synthesize these inputs into a single, razor-sharp **Core Sprint Goal Statement**.

### Step 2: Ticket Selection & Capacity Clamping
Select candidate tickets from `BACKLOG.md`:
* Calculate total complexity sum: $\sum C_{\text{ticket}}$.
* Ensure total planned load stays within the **~49 Fibonacci Point capacity cap** (unless the Lead explicitly approves an expanded scope).

### Step 3: Backlog Migration & File Synchronization
Transfer selected tickets from `BACKLOG` to `CURRENT_SPRINT` synchronously across all 3 formats:
1. **`CURRENT_SPRINT.md`:** Populate active table with selected tickets, update header with Start Date, Target End Date, and Core Sprint Goal.
2. **`CURRENT_SPRINT.csv`:** Write selected ticket rows.
3. **`CURRENT_SPRINT.json`:** Serialize selected ticket objects.
4. **`BACKLOG.md/csv/json`:** Update status of selected tickets from `Todo` to `In Progress`.

### Step 4: Master Index & SSoT Initialization
* Update `AI_Nexus/INDEX.md` to reflect the active sprint initialization.
* Record the sprint start in memory using `initiate_memory_recording`.

---

## 📋 3. Standard Template for Active Sprint (`CURRENT_SPRINT.md`)

```markdown
# Sovereign Framework: CURRENT SPRINT BACKLOG

**Sprint ID:** `SR_YYYYMMDD`
**Start Date:** [DD/MM/YYYY]
**Target End Date:** [DD/MM/YYYY] (4-Week Baseline)
**Planned Capacity:** [Points] / 49 Points

> 🎯 **Core Sprint Goal:**
> [Concise synthesis of Lead Priority + Backlog Truth + AI Innovation Vector]

## 🏃‍♂️ Active Sprint Tickets

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| [Selected Tickets] |
```

---
// "Clear start dates anchor time; balanced goals protect velocity." [Jules_Strategist] 2026-08-25
