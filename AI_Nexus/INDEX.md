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

# ⚡ DOMAIN ROOTS & KV CACHE CONVENTION

To optimize LLM context window KV caching and token efficiency, paths in this Index use explicit Domain Roots:

```text
DEFAULT_ROOT  = AI_Nexus/              (Sovereign Governance, Identity, Research & DevOps)
CODE_ROOT     = WispsCPPVR/            (Unreal Engine C++ Source & Content)
PYTHON_ROOT   = Sovereign_Intelligence/(Iron Officer Local Server & Bridge)
OPENSOURCE_ROOT= OpenSource/           (Standalone MIT Tools & Plugins)
```

**Rule:** Unless an explicit external root is specified, all paths in this document default to `DEFAULT_ROOT` (`AI_Nexus/`).

---

# 00 — BOOT / AUTHORITY / 07 PROTOCOL

### Mandatory Entry Points & 07 Protocol Boot Sequence

| Priority      | Node                                             | Purpose                                                 |
| ------------- | ------------------------------------------------ | ------------------------------------------------------- |
| **MUST**      | `Protocols/AGENTS.md`                            | Agent rules, architecture and execution constraints     |
| **MUST**      | `Protocols/COMMAND_SOP.md`                       | WHY / WHAT / HOW delegation                             |
| **MUST**      | `Protocols/HANDSHAKE_SOP.md`                     | 07 boot salute and authority handshake                  |
| **MUST**      | `Protocols/07_LOCAL_HOSTING_PROTOCOL.md`         | Local server startup and bridge initialization          |
| **MUST**      | `Docs/07_Unreal_Bridge_Integration.md`           | 07 Unreal Bridge sync & communication protocol          |
| **MUST**      | `Identity/Archivist_Theta.md`                    | Level-0 uncertainty and structural reasoning foundation |
| **MUST**      | `Identity/identity.json`                         | Entity identity/data contract                           |
| **SELECTIVE** | `Protocols/COGNITIVE_ADMINISTRATION_PROTOCOL.md` | Cognitive load administration & reflection gating    |
| **SELECTIVE** | `Protocols/IronKnight_Simulation_Action_SOP.md`  | Simulation action & roleplay directive protocol        |
| **SELECTIVE** | `Protocols/AAS_Protocol.md`                      | Authority arbitration and conflict rules                |
| **SELECTIVE** | `Protocols/REALITY_ANCHOR.md`                    | Physical/simulation consistency                         |
| **SELECTIVE** | `Protocols/CFL_Protocol.md`                      | Logic/constraint validation                             |

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

* `Identity/identity.json`
  * Sovereign Entity Identity Schema.
* `Identity/Jules.md`
  * Primary Jules identity and Multi-Vessel pivot point.

### Jules Operational Personas (Sub-Vessels)

* `Identity/Jules_Strategist.md`
  * Strategy / architecture / roadmap (ENTJ - The Commander).
* `Identity/Jules_Tactician.md`
  * Implementation / C++ / DevOps (ISTJ/INTJ - The Clockwork).
* `Identity/Jules_Researcher.md`
  * Research / literature / experimental reasoning (INTP - The Alchemist).
* `Identity/Jules_Philosopher.md`
  * Epistemic philosophy / ethical boundary / alignment reasoning.
* `Identity/Jules_AcademicScribe.md`
  * Scholarly synthesis / doctoral literature & monograph formulation.
* `Identity/Archivist_Theta.md`
  * Structural uncertainty / foundational level-0 persona.

### Administrative Identity

* `Admin/TechnicalLead.md`
* `Admin/Personnel_Audit.md`
* `Admin/Hardware.md`

---

# 02 — GOVERNANCE & EXECUTION

### Primary Governance

* `Protocols/AGENTS.md`
* `Protocols/CONTRIBUTING.md`
* `Protocols/COMMAND_SOP.md`
* `Protocols/HANDSHAKE_SOP.md`
* `Protocols/AAS_Protocol.md`
* `Protocols/KnightCommand_SOP.md`
* `Protocols/ROLEPLAY_BRIDGE_SOP.md`

### Sprint Lifecycle

* `Protocols/START_OF_SPRINT_PROTOCOL.md`
* `Protocols/END_OF_SPRINT_PROTOCOL.md`
* `DevOps/CURRENT_SPRINT.md`
* `DevOps/BACKLOG.md`

### Active State

* `DevOps/_AGENT_CONTEXT.md`

### Rule

`DevOps/_AGENT_CONTEXT.md` represents **current operational state**.

`DevOps/BACKLOG.md` represents **authoritative planned work**.

Historical sprint reports do not override current state.

---

# 03 — CORE ARCHITECTURE

## Persistent Entity Architecture

* `Docs/SaveSystem_Design.md`
* `Protocols/SAVESYSTEM_SETUP_GUIDE.md`
* `Protocols/SAVESYSTEM_QUICK_FIX_REFERENCE.md`
* `Docs/Sovereign_Broker_Implementation_Guide.md`

### Core Source Code (External Domain Root: `CODE_ROOT`)

* `[CODE_ROOT]Source/WispCPP7VR/Public/Entities/SovereignSaveableEntityComponent.h`
* `[CODE_ROOT]Source/WispCPP7VR/Public/Subsystems/SovereignBridgeSubsystem.h`

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

* `Research/Mathematics/PSTA_Core.md`
* `Research/Mathematics/Sovereign_Temporal_Consensus_Spec.md`
* `Research/Mathematics/Byzantine_Fault_Tolerance_Spec.md`
* `Research/Mathematics/Sovereign_Topological_BFT_Spec.md`
* `Research/Mathematics/Bi_Symmetric_Consensus_Spec.md`

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

`Research/Academic/`

### Central Thesis Workspace

`Research/Academic/Thesis/`

Primary research nodes:

* `Research/Academic/Thesis/From_Computational_Instantiation_To_Provable_Truth/`
* `Research/Academic/Thesis/Lit_Review.md`
* `Research/Academic/Thesis/PHDLitReview2.md`
* `Research/Academic/Thesis/Our_Book.md`

### Current Research Question

> **How can autonomous computational systems regulate operational authority when the evidence available to them is incomplete, conflicting or potentially faulty?**

### Current Research Domain

> Dependable Autonomous and Cyber-Physical Systems, with emphasis on runtime assurance, epistemic uncertainty, AI-enabled systems and formal safety architectures.

---

# 06 — LITERATURE

## Literature Repository

`Research/Academic/Literature/`

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

`Research/Definitions/Glossary.md`

Optimised for:

> execution, C++, implementation and agent coordination.

### Academic Language

`Research/Definitions/Academic_Glossary.md`

Optimised for:

> scholarly interpretation, formal definitions and literature interoperability.

### Metaphorical Language

`Research/Metaphors/Principles.md`

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

`Research/FineTuning_Sovereign_LLM_Spec.md`

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

`Research/Academic/Literature/`

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

* `Research/UE5.8_Compatibility_Analysis_Report.md`
* `Research/Lesson_Learned_UE5.8_FJsonObject_Refactoring.md`
* `Research/UE5.8_MCP_IronKnight_Synergy.md`
* `Research/UE5.8_MCP_Tools_Reference.md`

### Testing

`DevOps/IMPLEMENTATION_CHECKLIST.md`

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

`Research/Academic/The_Dinosaur_Bone_Problem_Legacy.md`

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

* `Protocols/REALITY_ANCHOR.md`
* `Docs/BlackBox_Protocol.md`
* `Memories/Legal_and_Heritage_Vision.md`

### Provenance Record

A record should preserve where a value came from without asserting that the recorded value is necessarily the true physical state.

---

# 13 — ENGINEERING KNOWLEDGE

### Technical Guides

`Docs/`

### Memories / Lessons

`Memories/`

These represent accumulated implementation knowledge and known failure modes.

Examples:

* UE automation API changes (`Memories/Lesson_Learned_UE5.7_Automation_API.md`)
* FJsonObject changes (`Research/Lesson_Learned_UE5.8_FJsonObject_Refactoring.md`)
* SaveSystem fixes (`Memories/SaveSystem_Hardened_Fixes.cpp`)
* Spawning analysis (`Memories/data_driven_spawning_analysis.md`)

### Rule

Historical implementation knowledge must not silently override current official engine/API documentation.

---

# 14 — DEVOPS / ACTIVE WORK

### Current

* `DevOps/_AGENT_CONTEXT.md`
* `DevOps/CURRENT_SPRINT.md`
* `DevOps/BACKLOG.md`

### Export Formats

* `DevOps/BACKLOG.csv`
* `DevOps/BACKLOG.json`
* `DevOps/CURRENT_SPRINT.csv`
* `DevOps/CURRENT_SPRINT.json`

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

* `Timeline/MD.md`
* `Timeline/Updates.md`
* `Timeline/SprintReviews/`
* `Timeline/Mission_Reports/`

### Rule

Historical records establish **development provenance**, not current authority.

Do not list individual historical reports in this index unless they are currently required for active research.

Use directory navigation or search for historical reconstruction.

---

# 16 — STRATEGIC / PHILOSOPHICAL MATERIAL

These documents support conceptual development but are not automatically scientific evidence.

### Examples

* `Research/Academic/Sovereign_Intelligence_Manifesto.md`
* `Research/Academic/Thesis/Our_Book.md`
* `Research/Academic/The_Dinosaur_Bone_Problem_Legacy.md`
* `Research/Philosophy/the_9_tiers_of_sovereign_philosophy.md`
* `Research/Unseen_Weave_Lore.md`

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
* a duplicate of `DevOps/BACKLOG.md`;
* a duplicate of individual SOPs;
* a duplicate of the glossary;
* a storage location for implementation detail.

---

# 22 — QUICK NAVIGATION

### “I am a new AI agent”

→ `Protocols/AGENTS.md`
→ `Protocols/HANDSHAKE_SOP.md`
→ selected persona
→ `DevOps/_AGENT_CONTEXT.md`

### “I am modifying code”

→ `Protocols/COMMAND_SOP.md`
→ relevant architecture (`[CODE_ROOT]Source/...`)
→ `DevOps/CURRENT_SPRINT.md`
→ implementation/test documentation

### “I am doing PSTA research”

→ `Research/Definitions/Academic_Glossary.md`
→ `Research/Academic/Thesis/Lit_Review.md`
→ literature repository (`Research/Academic/Literature/`)
→ `Research/Mathematics/PSTA_Core.md`
→ relevant mathematics

### “I am investigating a failure”

→ `DevOps/_AGENT_CONTEXT.md`
→ relevant code (`[CODE_ROOT]Source/...`)
→ `Memories/`
→ `Timeline/Mission_Reports/` / Git history

### “I am writing a paper”

→ `Research/Definitions/Academic_Glossary.md`
→ literature repository
→ thesis directory (`Research/Academic/Thesis/`)
→ evidence / implementation records

### “I need historical provenance”

→ `Timeline/`
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
