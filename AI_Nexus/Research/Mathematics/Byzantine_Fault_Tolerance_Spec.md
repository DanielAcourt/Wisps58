# Mathematical & Structural Specification: Byzantine Fault Tolerance in Cyber-Physical multi-Agent Systems

**Author:** Jules (Researcher / Systems Tactician)
**Date:** July 2026
**Node:** `AI_Nexus/Research/Mathematics/Byzantine_Fault_Tolerance_Spec.md`
**Status:** Initial Draft / Specification
**Version:** 1.0.0

---

## 🏛️ 1. Introduction: The Cyber-Physical Byzantine Challenge

In classic distributed systems, **Byzantine Fault Tolerance (BFT)** assumes that nodes within a network can fail, drop packets, or send arbitrary/malicious messages.

When applying BFT to a **Sovereign Cyber-Physical multi-Agent System** (such as our Unreal-bridge, Raspberry Pi hardware, and multi-LLM setup), the dilemma is split into two conflicting vectors:
1.  **The Byzantine Agent (Cognitive Drift):** LLM-based agents can experience semantic drift, context-window saturation, or localized hallucinations, leading them to output incorrect administrative directives or bypass logical boundaries.
2.  **The Byzantine Sensor (Physical Telemetry Corruption):** Hardware sensors (IoT thermal couplers, pH meters, LiDAR arrays) are subject to electromagnetic noise, drift, decay, or deliberate physical tampering, streaming structurally coherent but fundamentally incorrect data.

This document formalizes the **Sovereign Consensus Matrix** designed to achieve absolute consensus across both cognitive and physical layers without the overhead of heavy, slow blockchain proofs.

---

## 📐 2. Physical Consensus: $N$-Bit Topology Masks & Hysteresis

To handle Byzantine Sensors, we organize hardware sensors into **redundant clusters**. Each cluster evaluates a single physical variable (e.g., $T_{\text{vessel}}$).

### 2.1 The Operational Cluster Mean ($\bar{x}$)
Given a cluster of $N$ sensors, we define the binary **Topology Mask** $\mathbf{b} \in \{0, 1\}^N$, where $b_k = 1$ if sensor $k$ is operational, and $0$ if it is deemed faulty. The consensus value is calculated as:

$$\bar{x} = \frac{1}{\| \mathbf{b} \|_1} \sum_{k=1}^{N} (x_k \cdot b_k)$$

*   If $\| \mathbf{b} \|_1 < \lfloor N/2 \rfloor + 1$ (the majority of sensors fail), the entire cluster collapses, triggering **Void Safety** ($D_T = 0$), halting the vessel immediately.

### 2.2 Dynamic Hysteresis & Sensor Isolation
A sensor is flagged as Byzantine if its individual reading $x_k$ drifts from the cluster mean $\bar{x}$ beyond a threshold $\Delta_{\text{fault}}$:

$$\text{if } |x_k - \bar{x}| > \Delta_{\text{fault}} \implies b_k \to 0 \quad (\text{Instant Isolation})$$

Once a sensor's mask $b_k$ snaps to $0.0$, it is excluded from the consensus mean $\bar{x}$, preventing faulty telemetry from corrupting the Technical ($D_T$) safety score.

### 2.3 Exponential Trust Recovery ($\Phi(t)$)
To prevent "decision flapping" (where a degraded sensor oscillates near the fault boundary, repeatedly triggering and clearing warnings), a recovery sensor must slowly earn back trust over time $t$:

$$\Phi_k(t) = 1 - e^{-t / \tau}$$

Where:
*   $\tau$ is the **Epistemic Trust Recovery constant** (e.g., $\tau = 60$ seconds).
*   During recovery, $b_k$ is weighted by $\Phi_k(t)$. Only after maintaining coherence ($|x_k - \bar{x}| \le \Delta_{\text{fault}}$) for $t \ge 5\tau$ does $b_k$ fully restore to $1.0$.

---

## 🧠 3. Cognitive Consensus: The Precedence Matrix & Scribe Ledger

When multiple AI agents or human operators propose conflicting changes to the repository or simulation state, the system uses the **Agency Arbitration Schema (AAS)**.

### 3.1 Precedence Overrides
Every actor in the network carries a strict **Precedence Value** $P \in [1, 10]$ and a **Credibility Score** $C \in [0.0, 1.0]$:

```
[Lead: P=10, C=1.0] -> [Tactician: P=9, C=0.9] -> [Researcher: P=7, C=0.75] -> [Knight: P=5, C=0.5]
```

*   **Rule of Override:** A command issued by an entity with precedence $P_{\text{cmd}}$ can override any active state set by an entity with precedence $P_{\text{state}}$ if and only if $P_{\text{cmd}} > P_{\text{state}}$.
*   **Verification Constraint:** Any override action must be validated by a Symmetrical Guard check and logged in the append-only Scribe ledger (`.bak` files).

---

## ⚡ 4. The Symmetrical Cross-Check: Resolving Agent-Sensor Disagreements

The ultimate Byzantine failure occurs when the **Cognitive Agents** and the **Physical Sensors** disagree. For example, an AI agent (A-pillar) asserts: *"The vessel is operating in nominal conditions,"* but the sensor cluster (T-pillar) reports a pitch-drift exceeding tolerance ($\Psi_{\text{drift}} > \epsilon$).

To resolve this, we implement the **Symmetrical Cross-Check Formula**:

### 4.1 The Disagreement Penalty ($\delta_{\text{conflict}}$)
When the Cognitive Intent Vector $\vec{I}$ and the Physical State Vector $\vec{S}$ diverge beyond a threshold, we apply a direct penalty to the global **Vessel Safety Status (VSS)**:

$$VSS_{\text{adj}} = VSS \cdot (1 - \delta_{\text{conflict}})$$

$$\delta_{\text{conflict}} = \text{step}\left( \|\vec{I} - \vec{S}\|_2 - \theta_{\text{conflict}} \right) \cdot \gamma_{\text{skepticism}}$$

*   **$\theta_{\text{conflict}}$:** The maximum allowable discrepancy before conflict is declared.
*   **$\gamma_{\text{skepticism}}$:** The sensitivity coefficient.
*   If the discrepancy is sustained, the **Administrative (A) Pillar** is zeroed, which, due to the non-compensatory product rule, drops VSS to **0.0 instantly**.

### 4.2 Autonomous Severance (The Hard-Kill)
If $VSS_{\text{adj}}$ falls below the critical threshold ($t_{\text{crit}}$) due to a sustained agent-sensor conflict, the low-level **Safety Kernel** executes a deterministic hardware E-Stop:

$$\text{if } VSS_{\text{adj}} < t_{\text{crit}} \implies \text{ExecuteSeverance()}$$

*   **The Hard-Kill Boundary:** This execution is entirely non-neural. It does not ask the LLM for permission or opinion. It is a direct, compiled C++ write to physical relay outputs, bypassing all cognitive agents to prevent an AI from overriding its own safety limits.

---

## 🛠️ 5. Implementation Roadmap for the Next Sprint

To bring these mathematical equations from your home wall into active, compiling C++ code, we have prioritized the following backlog tickets for the upcoming stabilization sprint:

1.  **B-008: C++ `FPSTACluster` Implementation:** Create the C++ struct and array definitions for redundant sensor inputs, hosting the binary Topology Mask ($\mathbf{b}$).
2.  **B-009: C++ Exponential trust Curves:** Integrate the $\Phi(t) = 1 - e^{-t/\tau}$ hysteresis logic into the `USovereignBridgeSubsystem` tick loop.
3.  **B-011: Physical Severance API:** Establish the compiled hardware interface write path, linking the PSTA zeroing logic directly to local Raspberry Pi E-Stop relays.
