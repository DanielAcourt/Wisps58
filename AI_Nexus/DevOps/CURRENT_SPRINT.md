// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07
# Sovereign Framework: CURRENT SPRINT BACKLOG

**Sprint ID:** `SR_20261026`
**Start Date:** `07/10/2026`
**Target End Date:** `21/10/2026` (2-Week Recovery Baseline)
**Planned Capacity:** 40 Points (Ultra-Light Refactoring & Recovery Iteration)
**Completed Capacity / Velocity:** 36 Points

> 🎯 **Core Sprint Goal:**
> **"Telemetry Subsystem Decoupling, Entity Registration Hardening, Base Entity Modularization & Dynamic Vessel Infusion"**
> Refactor IoT sensor telemetry (`TemperatureCelsius`, `PhValue`, `WaterDepthMM`) out of `ASovereignBaseInteractable` into a standalone Black Box telemetry component/subsystem (`B-046`), harden entity module registration and attribute sync (`B-043`), modularize `ASovereignBaseEntity` component hierarchy (`B-047`), implement dynamic vessel infusion & component attachment pipeline (`B-049`), expand species D&D attribute template mapping (`B-048`), define utility-gated need-driven entity perception architecture (`B-050`), implement adaptive bridge mailbox polling (`AD-019`), enforce simulation reality anchor guardrails (`AD-020`), and harden AAS backup verification (`B-027`).

## 🏃‍♂️ Active Sprint Tickets

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| B-046 | Standalone Black Box Telemetry Subsystem & Base Class Decoupling | 8 | Completed | DevOps | IoT telemetry variables (`TemperatureCelsius`, `PhValue`, `WaterDepthMM`) are currently hardcoded on `ASovereignBaseInteractable`, burdening all interactable base classes with sensor properties when only specific Digital Twin hardware models receive telemetry. | Extract IoT sensor telemetry into a standalone `USovereignTelemetryComponent` or `USovereignBlackBoxTelemetrySubsystem`, removing hardcoded sensor properties from `ASovereignBaseInteractable` and providing clean UDP/TCP/Serial hardware ingestion. |
| B-043 | C++ Entity Module Registration & Attribute Sync Hardening | 5 | Completed | DevOps | Ensure living creature blueprints like BP_Antelope and BP_Humanoid automatically register USovereignAttributeComponent to USovereignSaveableEntityComponent on BeginPlay. | Living creature entities reliably serialize their full D&D attribute block (STR, DEX, CON, INT, WIS, CHA) into the world manifest. |
| B-047 | Modular ASovereignBaseEntity Component Hierarchy Refactoring | 5 | Completed | DevOps | ASovereignBaseEntity currently instantiates Bio, Qi, Element, and Attribute components by default, burdening non-living simulation entities (rocks, save terminals) with unnecessary component overhead. | Refactor ASovereignBaseEntity to only construct SaveDataComponent by default, leaving specialized vessel components (Bio, Attribute, Qi, Element) to living subclasses (ASovereignBaseCharacter / ASovereignLivingEntity) and dynamic attachment via Soul Hub discovery. |
| B-048 | USovereignSpeciesData D&D Attribute Expansion & Sync Pipeline | 3 | Completed | DevOps | FSovereignGrowthStage in USovereignSpeciesData only defines BaseStrength, BaseConstitution, and BaseAgility floats, lacking the full D&D attribute suite (Dexterity, Intelligence, Wisdom, Charisma, Luck) for species templates. | Expand USovereignSpeciesData with full integer D&D attributes (STR, DEX, CON, INT, WIS, CHA, LCK) and update USovereignAttributeComponent::SyncStatsFromEntity() to dynamically copy species defaults on spawn. |
| B-049 | Dynamic Vessel Infusion & Component Attachment Pipeline | 5 | Completed | DevOps | Non-living base entities (e.g. Rocks) can be animated or granted magic/life by Wisps during simulation gameplay, requiring dynamic component creation (`InfuseMagic`, `InfuseLife`, `InfuseAttributes`) and automatic component instantiation on save load. | `USovereignSaveableEntityComponent` provides `InfuseMagic()`, `ExtractMagic()`, `InfuseLife()`, `ExtractLife()`, `InfuseAttributes()`, and `ExtractAttributes()` API, automatically creating/registering components at runtime and on save state restoration. |
| B-050 | Utility-Gated Need-Driven Entity Perception Architecture | 5 | In Research | DevOps | Continuous per-frame `GetSensedActor()` sphere sweeps inside `ASovereignBaseCharacter::Tick` cause unnecessary CPU physics overhead when scaled across AI herds. AI perception must be gated by internal biological vitals (`USovereignBioComponent`) and state needs rather than unconditional per-frame polling. | Isolate per-frame reticle line-of-sight traces strictly to locally controlled player pawns (`IsPlayerControlled() && IsLocallyControlled()`), and gate AI perception to low-frequency, need-driven heartbeat checks (`OnSovereignHeartbeat`) triggered when vitals drop below threshold (e.g. Hunger < 40). |
| AD-019 | Adaptive Mailbox Polling & Bridge Traffic Throttling | 3 | Todo | DevOps | Reduce HTTP traffic spikes from QueryMailbox by implementing adaptive polling intervals in USovereignBridgeSubsystem. | Adaptive polling fires every 5.0s during idle gameplay and speeds up to 1.0s only when active messages are queued, reducing traffic by ~70%. |
| AD-020 | Simulation Reality Anchor & C++ Mutation Disambiguation | 3 | Todo | Research/DevOps | Add strict Reality Anchor prompt guardrails for Unreal_Simulation chats to prevent LLM hallucination of code execution. | LLM clearly differentiates between suggesting C++ code refactors and executing actual file changes. |
| B-027 | AAS v1.4.0 Hardening | 3 | Todo | DevOps | Refactor hardcoded diligence score to dynamically verify backup files on disk. | Diligence score calculates actual .bak coverage ratios dynamically. |

---

## 🏛️ Strategic Alignment
- **Active Iteration Load:** **22 Points** (Ultra-Light 2-Week Recovery)
- **Previous Completed Sprint Review:** `AI_Nexus/Timeline/SprintReviews/SR_20260928.md` (135 Points Delivered)
