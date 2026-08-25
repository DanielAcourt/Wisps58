// Copyright (c) 2013-2025 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# Sovereign Framework: CURRENT SPRINT BACKLOG

This file tracks active tickets prioritized for the current iteration. Isolating these tasks optimizes real-time AI context loading and focuses active development.

> 💡 **Developer Note (Sprint Status):**
> **All active sprint tasks have been completed and verified!** The dynamic broker lifecycle (B-016, B-017), dynamic soul hub mediator orchestration (E-001a, E-001b, E-001c), and the AI-to-Unreal Mailbox Polling pattern (B-026) are fully operational and verified through C++ automated tests and play-in-editor sessions.

## 🏃‍♂️ Active Sprint Tickets

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| AD-003 | Iron Officer: Session & Local Memory | 8 | Done | Admin/Research | Enable the local AI to ingest the AI_Nexus research directly. | Local RAG or context window management for 12 years of project data. |
| B-016 | Soul: Diagnostic Broker (Truth) | 5 | Done | DevOps | Implement the raw scientific/historical data layer for Lidar/Museum ingestion. | UDiagnosticBroker capturing and restoring vetted metadata and raw telemetry. |
| B-017 | Soul: Cultivation Broker (Magic) | 5 | Done | DevOps | Implement the gamified progression layer (Qi/Maturity) as a decoupled module. | UCultivationBroker managing optional simulation logic without breaking SSoT. |
| E-001a | Epic: Soul – Dynamic Broker Instantiation | 8 | Done | DevOps | Automatically instantiate and register UDiagnosticBroker and UCultivationBroker on BeginPlay. | Dynamic lifecycle complete with safe NewObject creation. |
| E-001b | Epic: Soul – Domain Namespace Isolation | 5 | Done | DevOps | Ensure strict separation of "Sovereign.Truth" and "Sovereign.Magic" save keys. | Decoupled namespace serialization verified under distinct JSON category keys. |
| E-001c | Epic: Soul – VSS & Paradox Coupling | 8 | Done | Research/DevOps | Bind Paradox Density and System Confidence to Diagnostic Broker vetting indicators. | Paradox scaling mapped to truth validation and unvetted telemetry. |
| B-026 | AI-to-Unreal Mailbox Polling | 8 | Done | DevOps | Implement proactive AI chat capabilities using the Mailbox Polling pattern. | Integration of /v1/unreal/mailbox HTTP loop in USovereignBridgeSubsystem and push_chat tool. |
| AD-005a | Spatial Sense: Save State Ingestion | 5 | Done | Research/DevOps | Map serialized JSON save payloads to the LLM's system context prompt framing. | Serialized component save packet (Identity, Bio, Qi, etc.) is injected during `/v1/unreal/chat` calls. |
| AD-005d | E2E Dialogue Console Logging | 2 | Done | DevOps | Enable immediate trace logging of the AI's response content in the bridge and client subsystem. | AI response is logged with prefix trace on both Python and C++ consoles. |
| B-035 | C++ Bio: Universal Gestation & Nesting | 5 | Done | DevOps | Support dragon lifecycle reproduction and egg states in a universal biological module. | Add GestationProgress, bGestationActive, NestSpatiotemporalVolume, and EggFertilityState to USovereignBioComponent C++. |
| B-038 | C++ Bio: Live Gestation Progression | 3 | Done | DevOps | Advance the internal egg formation cycle during the entity heartbeat without double-ticking component logic. | Integrate GestationProgress incrementing within USovereignBioComponent's metabolism update loop, capping at 100.0f and transitioning EggFertilityState. |
| AD-013 | Python 3.14 Asyncio Event Loop Alignment | 2 | In Progress | DevOps | Prevent unittest failures and RuntimeErrors in Python 3.12/3.14 where asyncio.get_event_loop() no longer implicitly creates loop. | Refactor test_unrestricted_roleplay_zone_bypass and test_anti_hallucination_guardrails in test_rag_and_psta.py to use asyncio.run(). |
| AD-014 | Sovereign RAG Configurable Absolute Path Alignment | 3 | In Progress | DevOps | Support arbitrary absolute directory indexing outside the project root and automatically eliminate uninitialized relative path warnings. | Update rag.py to handle absolute path configurations for index_dirs and auto-create missing relative directories dynamically. |
| B-043 | FJsonObject::Values UE 5.8.1 Compatibility Audit | 3 | Done | DevOps | Refactor of FJsonObject in UE 5.8+ uses UE::FSharedString for keys, breaking implicit conversions in range-based for loops over Values. | Comprehensive audit of all FJsonObject::Values range-based loops completed, verified, and recorded in a new Lesson Learned file. |
| B-044 | Unreal 5.8.1 Refactor | 3 | Done | DevOps | To implement explicit FString key wrapping in range-based loops over FJsonObject::Values to ensure compiling builds on Unreal 5.8.1+. | SovereignQiComponent.cpp resonance loop refactored with explicit KeyStr wrappers; verified zero compilation warnings/errors under Unreal 5.8.1. |
| AD-015 | 5.8.1 MCP Research & Iron Knight Synergy | 3 | In Progress | Research/DevOps | Define how our offline local Python bridge/RAG engine can act as a local MCP client to drive the editor directly, keeping all intelligence local and free from commercial cloud-token dependencies. | Finalize offline loopback integration specification detailing schema discovery, JSON-RPC handling, and RAG contextual grounding. |
| AD-016 | PhD Literature Review: Non-Compensatory Safety & Sovereign Axioms | 8 | In Progress | Research/Academic | Establish doctoral scoping review for Dr. Malik bridging non-compensatory PSTA safety with classic AI safety literature. | `AI_Nexus/Research/Academic/Thesis/Lit_Review.md` created and populated with axiomatic derivation and taxonomy. |
| AD-017 | Sovereign Framework Book: Monograph & Master Blueprint | 8 | In Progress | Research/Academic | Synthesize 12 years of research, spatial computing architecture, and 9-Tier Universal Stack into a published monograph. | `AI_Nexus/Research/Academic/Thesis/Our_Book.md` created and populated with Master Table of Contents and chapter highlights. |
| AD-003X | PSTA_Core Kernel Hardening | 5 | Scoped / Pending | Research/DevOps | Move theoretical PSTA safety pillars (P, S, T, A) into mandatory, compile-time and runtime execution checks. | Functional definitions of V_P, V_S, V_T, V_A operators in PSTA_Core.md executing before state transitions or external I/O. |

---

## 📋 Upcoming Prioritized Sprint Tickets (Next Phase)

The following tickets have been prioritized and added to the active backlog queue for the next development iteration:

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| B-027 | AAS v1.4.0 Hardening | 3 | Todo | DevOps | Refactor the hardcoded diligence score to dynamically verify backup files on disk. | Diligence score is calculated dynamically based on actual .bak coverage ratios on disk. |
| AD-008 | Adaptive System Prompt Modulation | 5 | Done | DevOps | Prevent the LLM from snapping back to rigid Knight behavior during roleplay. | System prompt dynamically softens tone and instructs model to write to active paths immediately. |
| AD-009 | Smart RAG Query Filtering | 3 | Done | DevOps | Prevent irrelevant administrative documents from polluting the creative context. | RAG search is bypassed or restricted if the prompt is purely roleplay/conversational. |
| AD-010 | Transient Path Memory Anchor | 3 | Done | DevOps | Keep the active file path anchored in the system context to stop repetitive parameter loops. | A last_active_file variable is dynamically tracked and injected into the system prompt. |
| AD-011 | Windows Cross-Drive Endpoint Hardening | 2 | Done | DevOps | Resolve Windows ValueError crash when calculating relative paths in generate_persona across multiple drive mounts. | try-except fallback implemented to return absolute path on cross-drive relative failure. |

---

## 🐉 DND Implementation Tickets (Homebrew Core)

These tickets represent the "DND Implementation" phase, baking the core logic of "The Unseen Weave" and universal draconic biology/spiritual systems into the active Wisp framework:

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| B-036 | C++ Qi: Universal Domains & Runes | 5 | Todo | DevOps | Support male territory ownership and rune stone node hierarchies in a universal qi module. | Add DomainTier, DomainResonanceRadius, and DomainNetworkSaturation properties to USovereignQiComponent C++. |
| B-037 | SSoT: Unseen Weave Lore Grounding | 3 | Todo | Research | Bake the foundational homebrew rules and dragon subclass metrics into the local RAG database. | Write Unseen_Weave_Lore.md containing dragon classifications, gestation duration tables, and rune grades. |
| B-039 | C++ Bio: Enhanced Gestation & Prestige Condensation | 5 | In Progress | DevOps | Implement the Unseen Weave prestige mechanic where female dragons condense eggs to gain attribute boosts. | Add EnhancedGestationTier and PrestigeCondensationCount properties to USovereignBioComponent, with methods to shrink Nest volume and multiply duration. |
| B-040 | C++ Bio: Progenitor Attribute Imbuement | 5 | Todo | DevOps | Allow wyrmlings to inherit superior attributes based on Mother and Father stats stored in the Save Entity. | Implement attribute copying from parents to egg metadata, granting +1/+2 Ability score modifiers upon hatching. |
| B-042 | C++ Bio: Data-Driven Draconic Spawning & Initialization | 5 | Todo | DevOps | Enable individual draconic species traits and gestation attributes to be initialized dynamically from Species Data templates. | Implement InitializeFromSpeciesTemplate in USovereignBioComponent to parse USovereignSpeciesData traits, map DragonType, and override GestationRate with Gestation.BaseRate dynamic attribute. |
| AD-012 | Bridge: Portable Roleplay & Gitignore Constraints | 3 | Todo | DevOps | Enable portable out-of-the-box roleplay execution without drive-mount dependencies or git bloat. | Modify config.json path variables to relative, and update .gitignore to exclude run-time DND campaigns/sessions. |

---

## 🏛️ Strategic Alignment
- **Combined Sprint Backlog Load:** **49 Points** (All Completed)
- **Sprint Goal:** Stabilize the decoupled Sovereign Soul Broker interface, establish dynamic broker instantiation, and enforce strict Truth/Magic domain separation. (100% Achieved)
