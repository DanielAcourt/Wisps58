# Sovereign Framework Glossary: Single Source of Truth

This glossary defines the core technical and thematic terminology of the Sovereign Framework. Adherence to these definitions ensures consistency across code, documentation, and agent communication.

---

## 🏗️ Core Entities

### **The Soul (USovereignSaveableEntityComponent)**
The primary data hub and Single Source of Truth for an entity. It persists across possession cycles and manages the entity's GUID, state, and identity.

### **The Soul Hub (The Mediator)**
An architectural design pattern where `USovereignSaveableEntityComponent` serves as a central mediator, orchestrating specialized autonomous components (Bio, Qi, Elements, Attributes) and centralizing Bridge communications while delegating domain-specific logic to registered brokers.

### **Sovereign Nested UI**
A decoupled UI rendering framework that uses `USovereignMasterHUD` for automated runtime discovery and pointer injection of the Soul Hub into nested `USovereignBaseWidget` sub-modules. It automates widget visibility based on the presence of registered category brokers and provides simple C++ string mappings (`GetCategoryDataAsMap`) for safe, robust Blueprint binding.

### **The Vessel (Actor/Pawn/Character)**
The physical or digital body that houses a Soul. Vessels can be possessed by the Spirit (Wisp) or operate autonomously.

### **The Spirit (ASovereignPlayerWisp)**
The player-controlled entity capable of possessing and unpossessing different Vessels. It carries the player's consciousness through the simulation.

---

## ⚡ Energy & State

### **Paradox Density ($\Xi$)**
A mathematical metric clamped between $[0.0, 1.0]$ that quantifies the logical stress and narrative/structural contradictions of an entity (e.g. ingesting unhandled or 'unknown' meta-tags). System confidence is calculated as $1.0 - \Xi$, directly affecting VSS and downstream gameplay systems.

### **Qi (Elemental Energy)**
A **Simulated/Gamified State** variable used for evolution and possession mechanics. It represents a fantasy result of user interaction and time, distinct from real-world telemetry truth. Tracked via the `SovereignQi.Qi.*` namespace.

### **Maturity / Growth Stage**
A measure of a Soul's evolution. Progression is triggered by elemental input, eventually leading to a physical/visual "Evolve" event.

---

## 🌐 Systems & Protocols

### **Agency Arbitration Schema (AAS)**
A decentralized security governance framework that enforces precedence, credibility weights, and dual VSS safety thresholds (0.4 for read-only, 0.7 for mutation) on AI agent operations. It generates self-regulating diagnostic error responses (`409_CONFLICT_GATE`) when VSS safety bounds are violated.

### **Adaptive Field Interpolator (AFI)**
A specialized actor component that utilizes a Logic Interpolator Kernel to reconcile incompatible data streams (Historical vs. Contextual) into emergent VFX and physics manifestations based on local PSTA discrepancy reports.

### **Distributed Ingestion Engine (DIE)**
A high-performance tiered storage partitioning strategy that separates high-frequency RAG assets on local SSD drives ('Spirit') from long-term Lidar scans, historical logs, and LLM fine-tuning datasets on high-capacity HDD archives ('Body').

### **Digital Twin**
A high-fidelity digital representation of a physical object or volume (e.g., the Fish Tank). Parity is maintained via real-time JSON/Binary telemetry.

### **Black Box Protocol**
A system design focused on data survivability in detached, low-power, or high-risk environments.

### **Sovereign Rule**
A fundamental architectural constraint: All attachments during possession MUST use `FAttachmentTransformRules::SnapToTargetNotIncludingScale` to maintain scale integrity.

### **Cyber-Physical Systems (CPS)**
Systems where software logic directly governs or synchronizes with physical hardware (Robotics/Sensors).

---

## 📐 PSTA (Provable Trust Framework)

### **PSTA Dimensions**
The **Four Pillars of Mission Integrity**: **Psychological** (Operator stress), **Social** (Team cohesion), **Technical** (Hardware health), and **Administrative** (Policy/Budget).

### **Dimension Health ($D_i$)**
A normalized score $[0, 1]$ representing the performance of a specific PSTA dimension, calculated via a self-normalizing weighted average.

### **Provable Safety Status (PSS)**
(Legacy) A weighted aggregation of Dimension Health scores subject to the Bottleneck Law. Replaced by the **Vessel Safety Status (VSS)** in the Unified Safety Formula.

### **Vessel Safety Status (VSS)**
The ultimate mission health metric. It utilizes the **Unified Safety Formula** to ensure absolute integrity. If any dimension fails its $\tau_{fail}$ threshold, or if the **Conflict Penalty** between Administrative and Technical pillars becomes too high, the VSS collapses to zero.

### **Coherence Coefficient ($\Phi$)**
A mathematical measure of agreement within an $N$-bit sensor cluster. In a healthy state, $\Phi=1.0$. A breach of the $\Delta_{fault}$ threshold snaps $\Phi$ to $0.0$, triggering degraded single-channel operation and Exponential Trust Recovery.

### **Severance Event**
A hardware-level "Hard-Kill" command triggered by the **Symmetrical Guard** when AI Intent and Physical Truth diverge beyond a kernel-level threshold. It bypasses all software "opinion" to ensure safety.

### **Symmetrical Guard (The Safety Kernel)**
A deterministic, non-neural monitoring process that calculates the **Symmetry Delta** ($Δ_{sym}$) between what the system *intends* to do and what the physical environment *allows*.

### **Provable Trust**
A state where every safety decision and status spike is traceable to a specific, weighted input factor ($x_{ij}$), providing a transparent audit trail for system behavior.

### **Asymmetric Failure**
A condition where a mission enters a critical risk state due to a failure in a "soft" dimension (Social/Psychological) even while the "hard" dimensions (Technical) remain perfect.

### **Discovery Layer**
A dynamic registry system that handles unknown meta-tags and degraded $N$-bit sensor clusters. It self-normalizes Dimension Health scores and adjusts the system's Caution threshold based on total Uncertainty Weight ($U_W$).

### **$N$-Bit Topology**
A sensor redundancy architecture where factors are grouped into clusters (e.g., $N=2$ for stereo). A **Topology Mask** ($\mathbf{b}_{ij}$) tracks the operational state of each sensor, allowing the system to isolate hardware malfunctions from true environmental data.

### **Anchor Tag**
A critical input factor ($x_{ij}$) that possesses "Override Authority." If an Anchor Tag's value hits zero, the entire Dimension Health ($D_i$) is forced to zero, regardless of other weights.

### **Void Safety**
A deterministic fallback protocol for dimensions with zero active monitoring ($W_i = 0$). In the Sovereign Framework, a "Void" dimension is treated as a critical failure ($D_i = 0$) because untracked reality is untrusted reality.

### **Risk Velocity ($V_i$)**
The rate of change of Dimension Health over time. High Risk Velocity triggers safety warnings even if absolute health is still above nominal thresholds, allowing for proactive intervention.
