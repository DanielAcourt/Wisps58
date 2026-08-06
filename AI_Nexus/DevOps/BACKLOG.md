// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.9. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-04
# Sovereign Framework: PRODUCT BACKLOG

This backlog tracks long-term product tasks using Fibonacci scoring. To optimize context loading during active cycles, all currently active/prioritized tickets have been moved to `CURRENT_SPRINT.md`.

## 🎯 Defined Backlog Tickets (Todo Archive)

| ID | Task | Complexity | Status | Node | Why (Context) | What (Completion Outcome) |
|:---|:---|:---:|:---:|:---|:---|:---|
| B-003 | Lidar Intelligence (Matrix Museum Summoning) | 13 | Todo | Research | Realize the 12-year vision of summoning historical artifacts into a spatial office. | Runtime ingestion and mesh reconstruction of high-fidelity Lidar datasets. |
| B-004 | "Corpse Possession" State Persistence | 3 | Todo | DevOps | Allow the Wisp to possess and reactivate dead or inactive vessels. | Possession lifecycle correctly restores state and controls for previously "dead" actors. |
| B-005 | Qi/Maturity Evolution Milestones | 5 | Todo | DevOps | Track the gamified "Reflection" state of entities over long timeframes. | Milestone system that triggers visual/logical evolution based on persisted Qi/Time. |
| B-006 | VR Interaction Polish (Possession Lifecycle) | 5 | Todo | Protocols | Ensure the core "Possession" mechanic is comfortable and intuitive for VR. | Zero-hitch possession transitions with localized haptic and visual feedback. |
| B-007 | Performance Baseline (1000+ Entities) | 8 | Todo | DevOps | Stress-test the USovereignSaveableEntityComponent for massive swarm scenarios. | Stable 60fps in VR while tracking and saving 1000+ active Sovereign entities. |
| B-008 | PSTA C++: N-Bit Topology Matrix | 8 | Todo | DevOps | Harden the T-pillar against single-sensor hardware glitches. | FPSTACluster implemented to evaluate health based on N sensors with agreement masks. |
| B-009 | PSTA C++: Exponential Trust Recovery | 5 | Todo | DevOps | Prevent "Decision Flapping" when a sensor returns to nominal range. | Hysteresis logic that slowly regains trust (Phi) using an exponential recovery curve. |
| B-010 | PSTA C++: Risk Velocity (Vi) Detection | 5 | Todo | DevOps | Detect failure before it happens by tracking the speed of health decay. | High-frequency derivative check (dVSS/dt) that triggers Caution on rapid descent. |
| B-011 | PSTA C++: Symmetrical Guard (Severance) | 8 | Todo | DevOps | Provide an autonomous "Hard-Kill" if AI intent violates physical reality. | Kernel-level check that triggers hardware severance when Intent/Truth delta is breached. |
| B-012 | Pi Kernel: Headless Safety Node | 13 | Todo | DevOps | Deploy the PSTA safety kernel on external Raspberry Pi hardware. | C++ PSTA implementation running headlessly on Pi 4 with physical relay control. |
| B-013 | Data Bridge: Pi-Unreal Telemetry Link | 8 | Todo | DevOps | Mirror real-world hardware truth in the Unreal reflection. | UDP/TCP listener subsystem in Unreal ingesting JSON telemetry from the Pi. |
| B-014 | Simulation: "Fish Tank" Visual Mapping | 5 | Todo | DevOps | Provide a high-fidelity visual representation of Pi-linked key species. | Dynamic materials and animations in Unreal that reflect live Pi sensor data. |
| B-015 | Replay: Time-Dilation scrubbing | 8 | Todo | DevOps | Enable forensic analysis of Black Box logs at various speeds. | Standardized replay interface for scrubbing through historical PSTA sessions. |
| R-001 | PSTA Math: Exponential Trust Hysteresis | 5 | Todo | Research | Define the mathematical bounds for trust recovery across all pillars. | Formalized Phi(t) equations for recovery and failure snapping. |
| R-002 | PSTA Math: Disagreement & Conflict Penalties | 8 | Todo | Research | Penalize VSS when human intent (A) and machine truth (T) diverge. | Mathematical definition of the Disagreement Penalty based on A/T vectors. |
| R-003 | PSTA Math: Residual Transparency Logging | 3 | Todo | Research | Ensure the "Why" of every safety decision is auditable. | Protocol for logging the specific dimension residuals that caused a VSS collapse. |
| R-004 | PSTA Math: Multi-Node Consensus (N > 2) | 8 | Todo | Research | Extend safety logic to clusters with more than two redundant sensors. | Generalization of the Operational Cluster Mean formula for N-bit agreement. |
| R-005 | PSTA Math: Weighted Epistemic Uncertainty | 5 | Todo | Research | Adjust caution thresholds based on how much the system "doesn't know." | Uncertainty Weight (Uw) formula that shifts safety bounds based on unknown tags. |
| R-007 | PSTA Math: Common-Mode Failure Signatures | 8 | Todo | Research | Identify when multiple sensors fail for the same underlying reason. | Pattern recognition logic for detecting correlated cluster failures. |
| R-008 | PSTA Math: Kernel-Level Severance Params | 5 | Todo | Research | Determine the physical bounds for autonomous AI severance. | Calculated thresholds for the Symmetrical Guard based on hardware limits. |
| L-001 | Legal: Deterministic Safety & Heritage Compliance | 13 | Todo | Admin | Ensure the framework meets international safety and heritage standards. | Whitepaper/Audit document proving the Sovereign Framework's compliance. |
| AD-005 | Iron Officer: Spatial Reflection (UE) | 13 | Todo | Admin/DevOps | Mirror the Iron Officer persona inside the Unreal simulation. | High-fidelity 3D agent manifestation responding to live PSTA telemetry. |
| B-018 | Lidar: .DLB Ingestion Pipeline | 13 | Todo | Research | Stream Lidar data from Museum APIs into the Diagnostic Broker. | Automated manifest ingestion mapping "Unknown" tags to high-fidelity mesh assets. |
| B-021 | Tool: `scope_validation` | 8 | Todo | DevOps | Enable agents to verify external path availability without risking operational stalls. | Proactive path testing tool with Temporal Dependency Mapping (TDM) and TOS reporting. |
| B-022 | Tool: `introspection_schema` | 5 | Todo | DevOps | Provide agents with verifiable structural blueprints of system subsystems. | Schema retrieval tool with Degradation Forecasting and Failure Mode Simulation (DFS). |
| B-023 | Tool: `cross_protocol_analyzer` | 13 | Todo | Research | Facilitate semantic translation between disparate system outputs and standards. | Semantic mapping engine with Constraint-Optimized Transformation (COTE) matrix. |
| B-024 | CFL: Temporal Paradox Mitigation | 8 | Todo | Research | Address Ticket #934-Temporal. Prevent logic violations of linear time in the SDK. | Implement a 'Causality Dampener' component handling temporal gradient differentials. |
| B-025 | CFL: Adaptive Field Interpolator | 13 | Todo | Research | Address Ticket #102-Dynamic. Handle 'Unknown' abilities on runtime via interpolation. | Unreal component that reconciles incompatible data streams into emergent VFX/Physics. |
| B-026 | AI-to-Unreal Mailbox Polling | 8 | Done | DevOps | Implement proactive AI chat capabilities using the Mailbox Polling pattern. | Integration of /v1/unreal/mailbox HTTP loop in USovereignBridgeSubsystem and push_chat tool. |
| B-027 | AAS v1.4.0 Hardening | 3 | Todo | Research | Refactor the hardcoded diligence score to dynamically verify backup files on disk. | Diligence score calculates actual .bak coverage ratios dynamically. |
| B-028 | C++ Sensor Consensus Integration | 5 | Todo | Research | Migrate FSovereignSensorPair and EvaluateBiSymmetry logic into UDiagnosticBroker C++. | Complete sensor consensus state-space machine evaluating anomalies vs coherent hazards. |
| AD-005a | Spatial Sense: Save State Ingestion | 5 | Todo | Research/DevOps | Ingest the serialized component save packet during `/v1/unreal/chat` calls. | Map serialized JSON save payloads to the LLM's system context prompt framing. |
| AD-005b | Spatial Sense: Unreal Agent Manifestation | 8 | Todo | DevOps | Create a 3D representation of the Iron Knight agent inside the levels. | Blueprint actor capable of requesting handshakes and rendering dynamic UI status. |
| AD-005c | Spatial Sense: Paradox Visualizer (AFI) | 5 | Todo | Research | Reconcile local state discrepancy reports to drive visual particles. | Drive material parameters and Niagara particles based on live `ParadoxDensity` ($\Xi$). |
| B-041 | C++ Bio: Diminished Draconic Beast Fallback | 8 | Todo | DevOps | Penalize premature egg laying or parental death during gestation by regressing the offspring into a lesser beast. | Implement biological regression resolving early eggs to Wyvern/Pygmy Wyvern classifications with restricted stats and no legendary actions. |

---

## 🏛️ Strategic Alignment
- **Complexity 1-3:** "Pipe" work. Localized logic, UI tweaks, or minor hardening.
- **Complexity 5-8:** "Officer" work. Multi-system synchronization, architectural refactors, or new "Vessel" implementation.
- **Complexity 13+:** "General" work. Massive research goals, deep spatial math, or framework-shifting paradigms.
