// Copyright (c) 2013-2025 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# AI_Nexus Master Index

## Sovereign Intelligence Framework — Map of Authority & Navigation

```text
Purpose:
Provide deterministic navigation, authority resolution and context selection
for humans and AI agents working within the Sovereign Framework.

Principle:
The Index identifies where information lives.
It does not itself establish the truth of that information.
```

---

# 00 — BOOT / AUTHORITY / 07 PROTOCOL

### Mandatory Entry Points & 07 Protocol Boot Sequence

| Priority      | Node                                                  | Purpose                                                 |
| ------------- | ----------------------------------------------------- | ------------------------------------------------------- |
| **MUST**      | `AI_Nexus/Protocols/AGENTS.md`                        | Agent rules, architecture and execution constraints     |
| **MUST**      | `AI_Nexus/Protocols/COMMAND_SOP.md`                   | WHY / WHAT / HOW delegation                             |
| **MUST**      | `AI_Nexus/Protocols/HANDSHAKE_SOP.md`                 | 07 boot salute and authority handshake                  |
| **MUST**      | `AI_Nexus/Protocols/07_LOCAL_HOSTING_PROTOCOL.md`     | Local server startup and bridge initialization          |
| **MUST**      | `AI_Nexus/Docs/07_Unreal_Bridge_Integration.md`       | 07 Unreal Bridge sync & communication protocol          |
| **MUST**      | `AI_Nexus/Identity/Archivist_Theta.md`                | Level-0 uncertainty and structural reasoning foundation |
| **MUST**      | `AI_Nexus/Identity/identity.json`                     | Entity identity/data contract                           |
| **SELECTIVE** | `AI_Nexus/Protocols/COGNITIVE_ADMINISTRATION_PROTOCOL.md` | Cognitive load administration & reflection gating    |
| **SELECTIVE** | `AI_Nexus/Protocols/IronKnight_Simulation_Action_SOP.md`  | Simulation action & roleplay directive protocol        |
| **SELECTIVE** | `AI_Nexus/Protocols/AAS_Protocol.md`                  | Authority arbitration and conflict rules                |
| **SELECTIVE** | `AI_Nexus/Protocols/REALITY_ANCHOR.md`                | Physical/simulation consistency                         |
| **SELECTIVE** | `AI_Nexus/Protocols/CFL_Protocol.md`                  | Logic/constraint validation                             |

### Boot Principle

```text
Repository
    ↓
Mandatory Governance & 07 Boot
    ↓
Mission / Persona Selection
    ↓
Task-Relevant Context
    ↓
Active Authority Set
    ↓
Execution
```

Agents MUST NOT treat every repository document as equally authoritative.

---

# 01 — IDENTITY & PERSONA

### Core Identity

* `AI_Nexus/Identity/identity.json`
  * Sovereign Entity Identity Schema.
* `AI_Nexus/Identity/Jules.md`
  * Primary Jules identity and Multi-Vessel pivot point.

### Jules Operational Personas (Sub-Vessels)

* `AI_Nexus/Identity/Jules_Strategist.md`
  * Strategy / architecture / roadmap (ENTJ - The Commander).
* `AI_Nexus/Identity/Jules_Tactician.md`
  * Implementation / C++ / DevOps (ISTJ/INTJ - The Clockwork).
* `AI_Nexus/Identity/Jules_Researcher.md`
  * Research / literature / experimental reasoning (INTP - The Alchemist).
* `AI_Nexus/Identity/Jules_Philosopher.md`
  * Epistemic philosophy / ethical boundary / alignment reasoning.
* `AI_Nexus/Identity/Jules_AcademicScribe.md`
  * Scholarly synthesis / doctoral literature & monograph formulation.
* `AI_Nexus/Identity/Archivist_Theta.md`
  * Structural uncertainty / foundational level-0 persona.

### Administrative Identity

* `AI_Nexus/Admin/TechnicalLead.md`
* `AI_Nexus/Admin/Personnel_Audit.md`
* `AI_Nexus/Admin/Hardware.md`

---

# 02 — GOVERNANCE & EXECUTION

### Primary Governance

* `AI_Nexus/Protocols/AGENTS.md`
* `AI_Nexus/Protocols/CONTRIBUTING.md`
* `AI_Nexus/Protocols/COMMAND_SOP.md`
* `AI_Nexus/Protocols/HANDSHAKE_SOP.md`
* `AI_Nexus/Protocols/AAS_Protocol.md`
* `AI_Nexus/Protocols/KnightCommand_SOP.md`
* `AI_Nexus/Protocols/ROLEPLAY_BRIDGE_SOP.md`

### Sprint Lifecycle

* `AI_Nexus/Protocols/START_OF_SPRINT_PROTOCOL.md`
* `AI_Nexus/Protocols/END_OF_SPRINT_PROTOCOL.md`
* `AI_Nexus/DevOps/CURRENT_SPRINT.md`
* `AI_Nexus/DevOps/BACKLOG.md`

### Active State

* `AI_Nexus/DevOps/_AGENT_CONTEXT.md`

### Rule

`_AGENT_CONTEXT.md` represents **current operational state**.

`BACKLOG.md` represents **authoritative planned work**.

Historical sprint reports do not override current state.

---

# 03 — CORE ARCHITECTURE

## Persistent Entity Architecture

* `AI_Nexus/Docs/SaveSystem_Design.md`
* `AI_Nexus/Protocols/SAVESYSTEM_SETUP_GUIDE.md`
* `AI_Nexus/Protocols/SAVESYSTEM_QUICK_FIX_REFERENCE.md`
* `AI_Nexus/Docs/Sovereign_Broker_Implementation_Guide.md`

### Core Concepts

```text
Soul
 ├── Identity
 ├── Persistence
 ├── State
 └── Domain Brokers

Vessel
 └── Hosts / exposes Soul

Spirit
 └── Controls / possesses Vessel
```

### Core Source of Truth

`USovereignSaveableEntityComponent`

---

# 04 — SAFETY / PSTA

## PSTA Research Spine

### Mathematical Core

* `AI_Nexus/Research/Mathematics/PSTA_Core.md`
* `AI_Nexus/Research/Mathematics/Sovereign_Temporal_Consensus_Spec.md`
* `AI_Nexus/Research/Mathematics/Byzantine_Fault_Tolerance_Spec.md`
* `AI_Nexus/Research/Mathematics/Sovereign_Topological_BFT_Spec.md`
* `AI_Nexus/Research/Mathematics/Bi_Symmetric_Consensus_Spec.md`

### Core PSTA Concepts

```text
Observation
    ↓
Evidence Qualification
    ↓
Epistemic State
    ↓
Authority
    ↓
Action
    ↓
Provenance
```

### Principal PSTA Terms

* Void / `⊥`
* Dimension Health `D_i`
* Coherence `Φ`
* VSS
* Risk Velocity `V_i`
* Trust Recovery
* Symmetrical Guard
* Severance
* Anchor
* N-Bit Topology
* Common-Mode Failure
* Authority Restriction

### Status Rule

PSTA artefacts must distinguish:

```text
CONCEPTUAL
FORMALISED
PROPOSITION
PROVEN UNDER ASSUMPTIONS
IMPLEMENTED
VERIFIED
EXPERIMENTALLY EVALUATED
INDEPENDENTLY REVIEWED
FALSIFIED / REJECTED
```

---

# 05 — ACADEMIC RESEARCH

## Doctoral Research

`AI_Nexus/Research/Academic/`

### Central Thesis Workspace

`AI_Nexus/Research/Academic/Thesis/`

Primary research nodes:

* `AI_Nexus/Research/Academic/Thesis/From_Computational_Instantiation_To_Provable_Truth/`
* `AI_Nexus/Research/Academic/Thesis/Lit_Review.md`
* `AI_Nexus/Research/Academic/Thesis/PHDLitReview2.md`
* `AI_Nexus/Research/Academic/Thesis/Our_Book.md`

### Current Research Question

> **How can autonomous computational systems regulate operational authority when the evidence available to them is incomplete, conflicting or potentially faulty?**

### Current Research Domain

> Dependable Autonomous and Cyber-Physical Systems, with emphasis on runtime assurance, epistemic uncertainty, AI-enabled systems and formal safety architectures.

---

# 06 — LITERATURE

## Literature Repository

`AI_Nexus/Research/Academic/Literature/`

### Literature Categories

```text
AI / LLM Safety
Runtime Verification
Runtime Assurance / Simplex
Formal Methods
Cyber-Physical Systems
Fault Tolerance / BFT
FDIR
Control / CBF
Safety Cases
Telemetry / Provenance
Digital Twins
```

### Literature Handling Rule

Every significant literature item should record:

```text
Citation
Problem
Method
Guarantee
Assumptions
Limitations
Overlap with PSTA
Potential Novelty Threat
```

AI summaries do not constitute literature evidence without source verification.

---

# 07 — ACADEMIC TRANSLATION

### Engineering Language

`AI_Nexus/Research/Definitions/Glossary.md`

Optimised for:

> execution, C++, implementation and agent coordination.

### Academic Language

`AI_Nexus/Research/Definitions/Academic_Glossary.md`

Optimised for:

> scholarly interpretation, formal definitions and literature interoperability.

### Metaphorical Language

`AI_Nexus/Research/Metaphors/Principles.md`

Optimised for:

> conceptual models and translation of spatial/system metaphors.

### Translation Stack

```text
Sovereign Concept
      ↓
Academic Neighbour
      ↓
Formal Definition
      ↓
Implementation
      ↓
Evidence
```

---

# 08 — AI / LLM RESEARCH

### Fine-Tuning

`AI_Nexus/Research/FineTuning_Sovereign_LLM_Spec.md`

### Core Research Themes

```text
Learned Behavioural Priors
        ↓
Proposal Generation
        ↓
Independent PSTA / AAS Authority
        ↓
ACCEPT / REJECT / ABSTAIN
```

### LLM Research

Relevant academic documents are stored under:

`AI_Nexus/Research/Academic/Literature/`

including:

* sycophancy;
* alignment faking;
* LLM safeguards;
* runtime verification;
* model-driven CPS;
* agent governance.

---

# 09 — CYBER-PHYSICAL / ROBOTICS

### Purpose

Experimental implementation and validation of PSTA concepts in executable systems.

Primary technical areas:

```text
C++
Unreal Engine
Telemetry
Sensors
Fault Injection
Automation Tests
Digital Twins
Runtime Monitoring
```

### UE Research

* `AI_Nexus/Research/UE5.8_Compatibility_Analysis_Report.md`
* `AI_Nexus/Research/Lesson_Learned_UE5.8_FJsonObject_Refactoring.md`
* `AI_Nexus/Research/UE5.8_MCP_IronKnight_Synergy.md`
* `AI_Nexus/Research/UE5.8_MCP_Tools_Reference.md`

### Testing

`AI_Nexus/DevOps/IMPLEMENTATION_CHECKLIST.md`

and project automation test modules.

---

# 10 — MARINE / AUTONOMOUS OBSERVER

## Silent Observer Architecture

Purpose:

> Independently preserve telemetry, system state, decision provenance and authority transitions without requiring control authority over the vessel.

Core chain:

```text
Observation
    ↓
Interpretation
    ↓
Proposal
    ↓
Authority Decision
    ↓
Action
    ↓
Forensic Record
```

Research questions include:

* telemetry provenance;
* sensor validity;
* common-mode failure;
* autonomous authority;
* post-incident reconstruction;
* tamper-evident logging.

---

# 11 — DIGITAL MUSEUM / DIGITAL TWINS

### Primary Concept

Bounded, provenance-aware instantiation of scanned physical assets.

```text
User Intent
    ↓
Metadata
    ↓
Validity / Provenance
    ↓
Resource Gate
    ↓
Bounded Instantiation
    ↓
Digital Representation
```

Primary example:

`AI_Nexus/Research/Academic/The_Dinosaur_Bone_Problem_Legacy.md`

Purpose:

> Demonstrate bounded information retrieval, resource gating, provenance and semantic querying.

---

# 12 — REALITY / OBSERVATION / PROVENANCE

### Core distinction

```text
Observed
    ≠
Inferred
    ≠
Authorised
    ≠
Physical Truth
```

### Related Artefacts

* `AI_Nexus/Protocols/REALITY_ANCHOR.md`
* `AI_Nexus/Docs/BlackBox_Protocol.md`
* `AI_Nexus/Memories/Legal_and_Heritage_Vision.md`

### Provenance Record

A record should preserve where a value came from without asserting that the recorded value is necessarily the true physical state.

---

# 13 — ENGINEERING KNOWLEDGE

### Technical Guides

`AI_Nexus/Docs/`

### Memories / Lessons

`AI_Nexus/Memories/`

These represent accumulated implementation knowledge and known failure modes.

Examples:

* UE automation API changes (`AI_Nexus/Memories/Lesson_Learned_UE5.7_Automation_API.md`)
* FJsonObject changes (`AI_Nexus/Research/Lesson_Learned_UE5.8_FJsonObject_Refactoring.md`)
* SaveSystem fixes (`AI_Nexus/Memories/SaveSystem_Hardened_Fixes.cpp`)
* Spawning analysis (`AI_Nexus/Memories/data_driven_spawning_analysis.md`)

### Rule

Historical implementation knowledge must not silently override current official engine/API documentation.

---

# 14 — DEVOPS / ACTIVE WORK

### Current

* `AI_Nexus/DevOps/_AGENT_CONTEXT.md`
* `AI_Nexus/DevOps/CURRENT_SPRINT.md`
* `AI_Nexus/DevOps/BACKLOG.md`

### Export Formats

* `AI_Nexus/DevOps/BACKLOG.csv`
* `AI_Nexus/DevOps/BACKLOG.json`
* `AI_Nexus/DevOps/CURRENT_SPRINT.csv`
* `AI_Nexus/DevOps/CURRENT_SPRINT.json`

### Change Discipline

```text
Ticket
 ↓
Plan
 ↓
Implementation
 ↓
Compile
 ↓
Automated Test
 ↓
Verification
 ↓
Commit
 ↓
Memory / Lesson
```

---

# 15 — HISTORICAL PROVENANCE

## Long-Term Record

* `AI_Nexus/Timeline/MD.md`
* `AI_Nexus/Timeline/Updates.md`
* `AI_Nexus/Timeline/SprintReviews/`
* `AI_Nexus/Timeline/Mission_Reports/`

### Rule

Historical records establish **development provenance**, not current authority.

Do not list individual historical reports in this index unless they are currently required for active research.

Use directory navigation or search for historical reconstruction.

---

# 16 — STRATEGIC / PHILOSOPHICAL MATERIAL

These documents support conceptual development but are not automatically scientific evidence.

### Examples

* `AI_Nexus/Research/Academic/Sovereign_Intelligence_Manifesto.md`
* `AI_Nexus/Research/Academic/Thesis/Our_Book.md`
* `AI_Nexus/Research/Academic/The_Dinosaur_Bone_Problem_Legacy.md`
* `AI_Nexus/Research/Philosophy/the_9_tiers_of_sovereign_philosophy.md`
* `AI_Nexus/Research/Unseen_Weave_Lore.md`

### Epistemic Rule

```text
Philosophical proposition
        ↓
Design intuition
        ↓
Research hypothesis
        ↓
Formalisation
        ↓
Evidence
```

Philosophy may motivate a technical hypothesis without constituting evidence for that hypothesis.

---

# 17 — EXTERNAL AI REVIEW

AI systems outside the primary Jules/Nexus workflow are treated as:

> **External AI Critique / Provisional Evidence**

They may provide:

* literature discovery;
* mathematical critique;
* terminology translation;
* counterexamples;
* alternative interpretations.

They do not provide:

* independent human validation;
* proof of novelty;
* certification;
* final safety authority.

---

# 18 — HUMAN REVIEW

### Required for:

* doctoral research direction;
* novelty;
* major theoretical claims;
* publication readiness;
* disputed interpretation;
* real-world safety claims;
* standards/certification claims.

Current academic contact:

**Dr. Manish Malik / prospective supervisory engagement**

---

# 19 — ACTIVE CONTEXT SELECTION

Agents MUST NOT assume that the entire Nexus is equally relevant to every task.

### Context Selection

```text
Task
 ↓
Relevant Domain
 ↓
Mandatory Governance
 ↓
Domain Documents
 ↓
Active SOP
 ↓
Active Mathematical / Technical State
 ↓
Execute
```

### Minimum Context Principle

> **Load the minimum context necessary to perform the task correctly; expand context when dependencies require it.**

This reduces:

* context contamination;
* contradictory historical instructions;
* irrelevant persona influence;
* hallucinated dependencies;
* unnecessary token consumption.

---

# 20 — CHANGE / PROVENANCE RULE

For major research or architectural changes, preserve:

```text
Origin
↓
Formalisation
↓
Implementation
↓
Experiment
↓
Review
↓
Accepted / Rejected
```

Git history remains the authoritative historical record of development.

---

# 21 — INDEX CONVENTION

The Master Index should contain:

* authoritative entry points;
* domain boundaries;
* active research direction;
* translation layers;
* navigation rules;
* provenance rules.

The Master Index should NOT become:

* a full document catalogue;
* a mission-report archive;
* a duplicate of `BACKLOG.md`;
* a duplicate of individual SOPs;
* a duplicate of the glossary;
* a storage location for implementation detail.

---

# 22 — QUICK NAVIGATION

### “I am a new AI agent”

→ `AI_Nexus/Protocols/AGENTS.md`
→ `AI_Nexus/Protocols/HANDSHAKE_SOP.md`
→ selected persona
→ `AI_Nexus/DevOps/_AGENT_CONTEXT.md`

### “I am modifying code”

→ `AI_Nexus/Protocols/COMMAND_SOP.md`
→ relevant architecture
→ `AI_Nexus/DevOps/CURRENT_SPRINT.md`
→ implementation/test documentation

### “I am doing PSTA research”

→ `AI_Nexus/Research/Definitions/Academic_Glossary.md`
→ `AI_Nexus/Research/Academic/Thesis/Lit_Review.md`
→ literature repository
→ `AI_Nexus/Research/Mathematics/PSTA_Core.md`
→ relevant mathematics

### “I am investigating a failure”

→ `AI_Nexus/DevOps/_AGENT_CONTEXT.md`
→ relevant code
→ `AI_Nexus/Memories/`
→ `AI_Nexus/Timeline/Mission_Reports/` / Git history

### “I am writing a paper”

→ `AI_Nexus/Research/Definitions/Academic_Glossary.md`
→ literature repository
→ thesis directory
→ evidence / implementation records

### “I need historical provenance”

→ `AI_Nexus/Timeline/`
→ Git history
→ Mission Reports

---

# FINAL AUTHORITY MODEL

```text
                    AI_NEXUS
                       │
              ┌────────┴────────┐
              │                 │
        CURRENT STATE       HISTORICAL STATE
              │                 │
         AUTHORITY             PROVENANCE
              │                 │
              └────────┬────────┘
                       │
                 DOMAIN CONTEXT
                       │
              ┌────────┴────────┐
              │                 │
          FORMAL MODEL      IMPLEMENTATION
              │                 │
              └────────┬────────┘
                       │
                    EVIDENCE
                       │
                 HUMAN REVIEW
```

> **The Index tells the agent where to look.
> The source document defines the rule.
> Evidence determines what may be claimed.
> Human review determines what survives independent scrutiny.**
