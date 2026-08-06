// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
# Research Node: PSTA Core (Provable Trust Framework)

## 🔬 1. The Philosophical "Why"
*   **Vision:** To transition the Sovereign Framework from simple **State Observation** to **Risk Mitigation**. The goal is to establish **Provable Trust**—a mathematical guarantee that the system is operating within safe, verifiable bounds across multiple dimensions of reality.
*   **Metaphor:** The **Four Pillars of Mission Integrity**. If any one pillar (Psychological, Social, Technical, or Administrative) collapses, the structural integrity of the entire mission is compromised, regardless of the health of the remaining pillars.

---

## 📐 2. The Mathematical "How"

### 2.1 The Dimensional Choice $\{P, S, T, A\}$
The system monitors four core dimensions to determine total mission health:
*   **Psychological (P):** Internal human variables (cognitive load, stress, biometric data).
*   **Social (S):** Inter-agent dynamics (trust, cohesion, communication latency).
*   **Technical (T):** Hard constraints (hardware health, sensor drift, latency).
*   **Administrative (A):** Soft constraints (policy compliance, budget variance, and temporal authority).

### 2.2 Dimension Health ($D_i$) & $N$-Bit Topology
The Dimension Score $D_i \in [0, 1]$ represents the overall health of a single dimension. To account for sensor redundancy and prevent hardware glitches from triggering false aborts, factors are refactored into **$N$-Bit Sensor Clusters**.

$$D_i = \Omega_{anchor} \cdot \left( \frac{1}{W_i} \sum_{j=1}^{K_i} \left( \bar{x}_{ij} \cdot \Phi_{ij} \cdot w_{ij} \right) \right)$$

*   **Operational Cluster Mean ($\bar{x}_{ij}$):** The average of all active, non-malfunctioning sensors within a cluster.
    $$\bar{x}_{ij} = \frac{1}{\| \mathbf{b}_{ij} \|_1} \sum_{k=1}^{N} (x_{ij, k} \cdot b_{ij, k})$$
    Where $\mathbf{b}_{ij} \in \{0, 1\}^N$ is the **Topology Mask** (1 = Operational, 0 = Faulty).
*   **Dynamic Coherence Coefficient ($\Phi_{ij}$):** Represents the hardware integrity of the cluster.
    $$\Phi_{ij} = \text{step}\left( \Delta_{\text{fault}} - \left| x_{ij, L} - x_{ij, R} \right| \right)$$
    In a "Sovereign" implementation, $\Phi$ utilizes **Exponential Trust Recovery**:
    *   *Failure:* Immediate snap to 0.0 upon $\Delta_{fault}$ breach.
    *   *Recovery:* $\Phi(t) = 1 - e^{-t/\tau}$ (Trust is earned back slowly over time $t$ as sensors maintain agreement).

**Hardening - The Void Safety Rule:**
*   If $W_i = 0$, $D_i = 0$. Untracked reality is untrusted reality.

**Hardening - Anchor Tags ($\Omega_{anchor}$):**
*   Critical factors ($x_{anchor}$) possess "Override Authority." If any anchor's coherence-adjusted value hits zero, $\Omega_{anchor} = 0$, forcing $D_i = 0$ instantly.

### 2.3 Temporal Momentum & Dual-Baseline Detection
To account for the speed of failure and "Slow-Drift" anomalies, the system utilizes a Dual-Baseline Normalization strategy. This prevents the "Normalization of Deviance" where a slowly worsening environment is incorrectly accepted as the new normal.

*   **Strategic Baseline ($\mu_{strat}$):** The "Calm Sea" reference. A factory-calibrated or mission-start "Absolute Truth" representing the ideal safe operating state (e.g., 25°C, zero pitch/yaw).
*   **Tactical Baseline ($\mu_{tact}$):** The "Current Reading." A high-frequency moving average representing the vessel's technical reality *right now*.
*   **Risk Velocity ($V_i$):** $V_i = \frac{\Delta D_i}{\Delta t}$. High velocity triggers warnings even if absolute health is within nominal thresholds.
*   **Baseline Conflict ($\Psi_{drift}$):** Calculated as $|\mu_{tact} - \mu_{strat}|$.
    *   If $\Psi_{drift} > \epsilon$ (the Drift Tolerance), the **Administrative (A) Pillar** receives a "Normalization of Deviance" alert.
    *   This alert signals that the vessel is orienting itself to a reality that has fundamentally shifted from its safe baseline (e.g., a sustained lists/tilt during a storm).

**Logging Hierarchy:**
1. The **Tactical Reading** is logged for every sensor to maintain the high-fidelity Black Box "movie."
2. The **Baseline Conflict** is logged as a discrete event to provide "Provable Reason" for system state changes (e.g., shifting to Caution because the "Calm Sea" baseline is no longer valid).

### 2.4 Vessel Safety Status (VSS)
The VSS is the final holistic metric that determines if the mission is "Safe." It transitions the system from simple "Provable Safety" (PSS) to a "Non-Compensatory" Unified Safety Logic.

**The Unified Safety Formula (VSS):**
To ensure that critical failure in any single dimension cannot be "hidden" by success in others, we apply a hard step-function product:

$$VSS = \left( \prod_{i \in \{P,S,T,A\}} \text{step}(D_i - \tau_{fail, i}) \right) \cdot \sum_{i=1}^{n} \alpha_i D_i$$

*   **The Kill Switch:** The product term acts as a binary gate. If any $D_i$ falls below its dimension-specific failure threshold $\tau_{fail, i}$, the entire VSS collapses to **0.0**, proving mission failure.
*   **Non-Compensatory Logic:** Social (S) or Psychological (P) health cannot "average out" a Technical (T) failure.

---

## 🏗️ 3. The Structural Integration (The Discovery Layer)

### 3.1 Handling Unknown & Degraded Tags (Weighted Uncertainty)
The system calculates the **Uncertainty Weight** ($U_W$) to manage systemic vigilance.
*   **Uncertainty Sum:** $U_W = \sum w_{unknown} + \sum w_{degraded}$.
*   **Degraded Logic:** A sensor cluster ($x_{ij}$) is considered "Degraded" if its Topology Mask $\mathbf{b}_{ij}$ has lost bits (e.g., [1, 0] instead of [1, 1]).
*   **Weighted Discovery Ratio:** This ensures that losing a high-weight navigation sensor (Lidar) shifts the safety posture more aggressively than losing a low-weight decorative sensor.

**Threshold Adjustment:**
If the total Uncertainty Weight ($U_W$) relative to the Total Weight ($W_{total}$) exceeds a threshold, the system shifts $t_{caut}$ upward to enforce a safety-first posture.

$$t_{caut, adj} = t_{caut, base} + \gamma \cdot \left( \frac{U_W}{W_{total}} \right)$$

Where **Epistemic Skepticism ($\gamma$)** is a sensitivity factor determining the aggressiveness of the posture.

### 3.2 Epistemic Paradox & Temporal Authority (E)
To account for logic-breaking events identified via the **CFL Protocol**, the system incorporates **Paradox Penalties**.

*   **Temporal Anchor Violation:** If the internal simulation time deviates from the **External Temporal Anchor** (Lead Mandate), the **Administrative (A)** pillar is penalized by a factor of $\Psi_{temp}$.
    $$\Psi_{temp} = |T_{sim} - T_{anchor}| \cdot \lambda$$
    Where $\lambda$ is the Temporal Rigidity constant.
*   **Logical Inconsistency ($\Xi$):** Represents the presence of unresolvable paradoxes (e.g., The Liar Paradox) within the active SDK configuration.
    $$VSS_{adj} = VSS \cdot (1 - \Xi)$$
    Where $\Xi \in [0, 1]$ is the **Paradox Density** reported by the Researcher.

### 3.3 Technical (T) Hardening: Physical Truth vs. Simulated State
The **Technical (T)** pillar distinguishes between **Absolute Truth** (Hardware sensors) and **Simulated State** (Gamified variables).

*   **Physical Truth (The Black Box):** High-fidelity logs derived from real-world sensors (Lidar, IoT Telemetry). This is the only source of truth for PSTA integrity verification.
*   **Simulated State (The Save File):** Gamified elements such as **Qi (Energy)** or **Growth Stages**. These are fantasy-based results of user interaction and time; they are persisted for simulation continuity but are not treated as "Mission Truth."
*   **The "Sandwich" Proof:** By successfully persisting unknown tags (e.g., `Sandwhich: True`), the system demonstrates its ability to act as a **Deterministic Data Bridge** for both real-world sensors and simulated metadata.
*   **Namespace Scoping:** Data is scoped (e.g., `Telemetry.*` for Truth vs. `Qi.*` for Simulation). This ensures the PSTA system only assesses risk based on physical telemetry while maintaining the gamified simulation.

### 3.4 Automated Discovery Layer (Vision Integration)
To mitigate the "Labor of Labeling," the Sovereign Framework integrates with spatial sensors (Computer Vision/Lidar):
*   **Semantic Transcription:** Objects identified via Image Recognition (e.g., a "High-Voltage Hazard") are automatically registered as $x_{ij}$ factors in the **Administrative (A)** or **Technical (T)** dimensions.
*   **Autonomous Priority Mapping:** Known visual archetypes carry pre-defined impact weights, allowing the system to scale its risk assessment without manual human input.

### 3.5 Classification and Threshold Mapping
The system maps the continuous VSS score to a discrete operational status.

| Status | Range | Logic with "Unknown" Discovery |
| :--- | :--- | :--- |
| **Critical** | $0 \le VSS < t_{crit}$ | **Immediate Abort.** Provable failure detected. |
| **Warning** | $t_{crit} \le VSS < t_{warn}$ | **Human-in-the-loop (HITL):** Robot or human must verify. |
| **Caution** | $t_{warn} \le VSS < t_{caut}$ | **Performance throttled.** System is investigating data. |
| **Nominal** | $t_{caut} \le VSS \le 1$ | **Standard Ops.** Requires high tag-certainty. |

**The Discovery Layer Constraint (Weighted Unknown Tag Penalty):**
If the total Uncertainty Weight ($U_W$) relative to the Total Weight ($W_{total}$) exceeds a threshold, the system shifts $t_{caut}$ upward.

$$t_{caut, adj} = t_{caut, base} + \gamma \cdot \left( \frac{U_W}{W_{total}} \right)$$

*   **Epistemic Skepticism ($\gamma$):** A sensitivity factor that determines the aggressiveness of the safety-first posture.
*   **Conflict Penalty:** If the **Administrative (A)** pillar (Human Intent) and **Technical (T)** pillar (Sensor Truth) disagree beyond a conflict threshold, a **Disagreement Penalty** is applied to the final VSS, signaling a breakdown in mission integrity.

---

## 📡 4. Truth vs. Reflection (The Sim-to-Real Bridge)

The PSTA system operates across two distinct but synchronized perspectives:

*   **The Truth (Real-World Vessel):** The physical entity (e.g., Robot, Pi-driven sensor) gathering relative truths from its environment. It generates the raw Black Box logs.
*   **The Reflection (Unreal Simulation):** A high-fidelity "movie" of the Truth. It replays the logs to reconstruct the event, using the same PSTA math to verify that the reconstruction matches the intended historical truth.

---

## ⚡ 5. The Symmetrical Guard: Autonomous Severance

The final layer of the Sovereign Framework is the **Safety Kernel**— a low-level "Interlinear" process that monitors for **Asymmetric Intent Detection**.

### 5.1 The Severance Event
If the system's **Administrative Intent** (the AI's "opinion" or command) diverges significantly from the **Physical Constraint** (the Technical Truth), the Kernel triggers an immediate **Severance Event**.

*   **Symmetry Delta ($\Delta_{sym}$):** The absolute difference between the Intent Vector ($\vec{I}$) and the current Physical State Vector ($\vec{S}$).
    $$\Delta_{sym} = \| \vec{I} - \vec{S} \|$$
*   **The Severance Logic:**
    $$\text{if } \Delta_{sym} > \text{Threshold}_{kernel} \implies \text{ExecuteSeverance()}$$

### 5.2 The Hard-Kill Boundary
*   **Non-Neural Execution:** The severance is a direct, deterministic write to the hardware-level E-Stop register. It intentionally bypasses the neural network/AI to prevent "The Safety Paradox" (where a failing AI overrides its own safety triggers).
*   **Black Box Enforcement:** Every Severance Event is logged with the raw Symmetrical Delta and the specific Intent Vector that caused the violation, providing an indisputable forensic record.

### 5.3 Asymmetric Failure Awareness
If the **Administrative (A)** pillar (Human Intent) and **Technical (T)** pillar (Sensor Truth) disagree beyond a conflict threshold, a **Conflict Penalty** is applied to the final VSS, signaling a breakdown in mission integrity. This ensures that social or psychological health cannot "average out" a fundamental alignment failure.

---

## 🧪 6. Training Data & Scenarios

### 4.1 Definitive Terms
*   **Anchor Tag:** A high-priority input that can autonomously zero a dimension score.
*   **Void Safety:** Fallback logic for dimensions with zero active data points.
*   **Risk Velocity:** The rate at which health is deteriorating.

### 4.2 Test Scenarios
*   **The Island Stress Test:** Isla's biometric stress spikes. $D_P$ drops. If $V_P$ (velocity) is too high, VSS enters **Warning** immediately, even if $D_P$ is still $0.7$.
*   **The Engine Anchor:** A technical sensor for fuel pressure is marked as an **Anchor**. If it hits 0, $D_T$ becomes 0 instantly, aborting the mission via the Bottleneck Law.
