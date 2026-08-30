// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Sovereign Framework: CURRENT SPRINT BACKLOG

**Sprint ID:** `SR_20260922`
**Start Date:** `25/08/2026`
**Target End Date:** `22/09/2026` (4-Week Solo Baseline)
**Planned Capacity:** 85 Points (Expanded Community Scope)

> 🎯 **Core Sprint Goal:**
> **"PSTA Hardware Reality & Hybrid Agent Synergy (Iron Knight + Jules + UE 5.8 MCP Loopback)"**
> Deploy headless Pi hardware safety nodes, bridge physical sensor telemetry into Unreal, and formalize the Jules-to-Iron-Knight local MCP orchestration protocol under Agency Arbitration Schema (AAS) governance.

## 🏃‍♂️ Active Sprint Tickets

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| AD-016 | UE 5.8.1 MCP Loopback Client Bridge | 6 | Completed | Research/DevOps | Connect Iron Knight to Unreal 5.8 embedded MCP server for local, token-free editor automation. | Python MCP client in Sovereign_Intelligence implementing JSON-RPC over loopback http://127.0.0.1:8000/mcp with tool discovery. |
| AD-017 | UE 5.8.1 Python MCP Tool Registration & Scripting Directory | 5 | Completed | Research/DevOps | Create `/Content/Python/` scripting directory and register a live sample tool script exposing editor automation methods to `SovereignMCPClient`. | `/WispsCPPVR/Content/Python/` created with `init_unreal.py` and `sovereign_mcp_tools.py` registering live editor tools accessible via `GET /v1/mcp/tools`. |
| AD-005a | Spatial Sense: Save State Ingestion | 5 | Completed | Research/DevOps | Ingest the serialized component save packet and spatial transforms during `/v1/unreal/chat` calls. | Multi-entity world manifest and 3D spatial transforms (Location, Rotation) serialized into system prompt context for `/v1/unreal/chat`. |
| AD-005b | Spatial Sense: Unreal Agent Manifestation | 8 | Todo | DevOps | Create a 3D representation of the Iron Knight agent inside the levels. | Blueprint actor capable of requesting handshakes and rendering dynamic UI status. |
| AD-005c | Spatial Sense: Paradox Visualizer (AFI) | 5 | Todo | Research | Reconcile local state discrepancy reports to drive visual particles. | Drive material parameters and Niagara particles based on live `ParadoxDensity` ($\Xi$). |
| E-002a | Jules-to-Iron-Knight Hybrid Delegation Protocol | 8 | Todo | Research/DevOps | Formalize JSON-RPC schema allowing Jules to hand off Epics directly to Iron Knight. | Multi-agent SOP and queue handler enabling autonomous delegation from cloud Jules to local Iron Knight. |
| B-012 | Pi Kernel: Headless Safety Node | 13 | Todo | DevOps | Deploy the PSTA safety kernel on external Raspberry Pi hardware. | C++ PSTA implementation running headlessly on Pi 4 with physical relay control. |
| B-013 | Data Bridge: Pi-Unreal Telemetry Link | 8 | Todo | DevOps | Mirror real-world hardware truth in the Unreal reflection. | UDP/TCP listener subsystem in Unreal ingesting JSON telemetry from the Pi. |
| B-028 | C++ Sensor Consensus Integration | 5 | Todo | Research | Migrate FSovereignSensorPair and EvaluateBiSymmetry logic into UDiagnosticBroker C++. | Complete sensor consensus state-space machine evaluating anomalies vs coherent hazards. |
| B-027 | AAS v1.4.0 Hardening | 3 | Todo | DevOps | Refactor hardcoded diligence score to dynamically verify backup files on disk. | Diligence score calculates actual .bak coverage ratios dynamically. |
| AD-012 | Bridge: Portable Roleplay & Gitignore Constraints | 3 | Todo | DevOps | Enable portable roleplay execution without drive-mount dependencies or git bloat. | Modify config.json path variables to relative, and update .gitignore to exclude run-time DND campaigns. |
| B-039 | C++ Bio: Enhanced Gestation & Prestige Condensation | 5 | In Progress | DevOps | Implement the Unseen Weave prestige mechanic where female dragons condense eggs to gain attribute boosts. | Add EnhancedGestationTier and PrestigeCondensationCount properties to USovereignBioComponent, with methods to shrink Nest volume and multiply duration. |
| B-040 | C++ Bio: Progenitor Attribute Imbuement | 5 | Todo | DevOps | Allow wyrmlings to inherit superior attributes based on Mother and Father stats stored in the Save Entity. | Implement attribute copying from parents to egg metadata, granting +1/+2 Ability score modifiers upon hatching. |
| B-042 | C++ Bio: Data-Driven Draconic Spawning & Initialization | 5 | Todo | DevOps | Enable individual draconic species traits and gestation attributes to be initialized dynamically from Species Data templates. | Implement InitializeFromSpeciesTemplate in USovereignBioComponent to parse USovereignSpeciesData traits, map DragonType, and override GestationRate with Gestation.BaseRate dynamic attribute. |
| B-036 | C++ Qi: Universal Domains & Runes | 5 | Todo | DevOps | Support male territory ownership and rune stone node hierarchies in a universal qi module. | Add DomainTier, DomainResonanceRadius, and DomainNetworkSaturation properties to USovereignQiComponent C++. |
| B-037 | SSoT: Unseen Weave Lore Grounding | 3 | Todo | Research | Bake the foundational homebrew rules and dragon subclass metrics into the local RAG database. | Write Unseen_Weave_Lore.md containing dragon classifications, gestation duration tables, and rune grades. |
| B-043 | C++ Entity Module Registration & Attribute Sync Hardening | 5 | Todo | DevOps | Ensure living creature blueprints like BP_Antelope and BP_Humanoid automatically register USovereignAttributeComponent to USovereignSaveableEntityComponent on BeginPlay. | Living creature entities reliably serialize their full D&D attribute block (STR, DEX, CON, INT, WIS, CHA) into the world manifest. |
| B-044 | C++ Possession Target State & Vessel Registration | 5 | Todo | DevOps | Track active possession state bidirectionally by adding PossessedTargetActor and PossessingEntity fields to USovereignSaveableEntityComponent. | Serialized Identity JSON block contains PossessedTargetActorName when possessed, restoring 3D playspace narrative coherence. |
| AD-019 | Adaptive Mailbox Polling & Bridge Traffic Throttling | 3 | Todo | DevOps | Reduce HTTP traffic spikes from QueryMailbox by implementing adaptive polling intervals in USovereignBridgeSubsystem. | Adaptive polling fires every 5.0s during idle gameplay and speeds up to 1.0s only when active messages are queued, reducing traffic by ~70%. |
| AD-020 | Simulation Reality Anchor & C++ Mutation Disambiguation | 3 | Todo | Research/DevOps | Add strict Reality Anchor prompt guardrails for Unreal_Simulation chats to prevent LLM hallucination of code execution. | LLM clearly differentiates between suggesting C++ code refactors and executing actual file changes. |
| AD-021 | Local Art Asset Manifest & Gitignore Strategy | 3 | Completed | DevOps | Establish local art asset management solution using asset_manifest.json, Python sync script, and .gitignore rules for binary art assets. | `.gitignore` rules for art assets, `asset_manifest.json`, `sync_art_assets.py` script, Python MCP integration, and documentation in `AI_Nexus/Docs/12_Local_Art_Asset_Manifest_Guide.md`. |
| AD-022 | Community: Educational Stream & YouTube Video Production | 3 | Todo | Research | Script and produce an educational video ("Stop Paying for Git LFS: Unreal Engine Zero-Bloat Art Staging Workflow") explaining the local asset manifest solution. | Published educational video and stream walkthrough on YouTube and memeseco community channels. |
| AD-023 | Community: Standalone Open-Source Tool Packaging | 3 | Todo | DevOps | Package asset_manifest.json, sync_art_assets.py, and sync_art_assets.bat into a clean standalone open-source GitHub repository. | Independent, MIT/GPL-licensed GitHub repository published for community adoption. |
| AD-024 | Community: Fab Marketplace Plugin & Utility Widget Packaging | 5 | Todo | DevOps | Wrap the Python asset sync engine into an Unreal Engine Editor Utility Widget (EUW) toolbar button and list it on Fab. | Packaged Fab-ready plugin with in-editor UI panel and 1-click toolbar sync button. |

---

## 🏛️ Strategic Alignment
- **Active Iteration Load:** **74 Points** (In Progress)
- **Previous Completed Sprint Review:** `AI_Nexus/Timeline/SprintReviews/SR_20260825.md` (97 Points Delivered)
