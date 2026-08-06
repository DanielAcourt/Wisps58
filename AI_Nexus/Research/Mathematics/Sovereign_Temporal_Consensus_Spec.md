# Mathematical Proposal: Sovereign Temporal Consensus (PSTA vs. Classic Byzantine Fault Tolerance)

**Author:** Jules (Researcher / Systems Tactician)
**Date:** July 2026
**Node:** `AI_Nexus/Research/Mathematics/Sovereign_Temporal_Consensus_Spec.md`
**Status:** Canonical Proposal
**Version:** 1.0.1-Beta-Temporal

---

## 🏛️ 1. Executive Summary: The Static Byzantine Flaw

The classic **Byzantine Generals Problem** (formulated by Lamport, Shostak, and Pease in 1982) is fundamentally **static and stateless**. It seeks to achieve consensus on a single, isolated decision at a single instant in time: *Do we attack or retreat?*

In a real-world, high-integrity cyber-physical system, this static assumption fails. It introduces two critical system vulnerabilities:
1.  **The Flapping Sensor / Over-Sensitivity:** A minor, temporary physical spike in a single sensor (e.g., an electromagnetic surge causing a temperature spike for $10\text{ms}$) is treated as a "Byzantine lie," triggering immediate, costly node isolation or system-wide shutdown.
2.  **The Autopilot-Pilot Authority Paradox:** If the system is programmed to take deterministic, automated action based *solely* on sensor consensus, a conflict occurs when a human pilot attempts to overtake an autopilot. The autopilot treats the pilot's input as an "out-of-tolerance anomaly," locking the pilot out of control.

We propose **Sovereign Temporal Consensus**. By integrating **Temporal Baseline Accumulation** and mapping each sensor to a **Multi-Pillar 2-Bit State Matrix**, we shift the PSTA framework from a single-point automated pilot override to an **objective, time-series Ledger of Truth**.

---

## 📐 2. The 2-Bit, 4-State PSTA Sensor Profile & Theta Alignment

Instead of treating a sensor as a binary input (1 = Operational, 0 = Faulty), every physical sensor $x_k$ evaluates its local state across a **two-bit binary matrix** representing four distinct operational states.

To maintain absolute, systemic consistency across our Single Source of Truth, these 2-bit values are mapped directly to **Archivist Persona Θ's cognitive state enums**:
*   `00` $\to$ **ZERO_ZERO:** Primordial Null / Critical Collapse (The system's baseline ground-state).
*   `01` $\to$ **ZERO_ONE:** Initial Potential / Triggered (Exceeds baseline threshold; degraded state).
*   `10` $\to$ **ONE_ZERO:** Structural Analysis / Processing (Under active audit; saturated context).
*   `11` $\to$ **ONE_ONE:** Operational Resonance / Committed (The peak state of stable, synchronized execution; Nominal standard operation).

```
              +-----------------------------------------------+
              |            Two-Bit State Matrix               |
              +-----------------------+-----------------------+
              |          00           |          01           |
              |       CRITICAL        |        DEGRADED       |
              |     [ZERO_ZERO]       |       [ZERO_ONE]      |
              +-----------------------+-----------------------+
              |          10           |          11           |
              |       SATURATED       |        NOMINAL        |
              |      [ONE_ZERO]       |       [ONE_ONE]       |
              +-----------------------+-----------------------+
```

### 2.1 The Four State Mappings:
1.  **`00` - CRITICAL COLLAPSE (Void/E-Stop):** [ZERO_ZERO] Telemetry is absent, corrupt, or exceeds the catastrophic hardware threshold. The $D_i$ score for this sensor's assigned pillar snaps immediately to $0.0$.
2.  **`01` - DEGRADED (Investigation/Caution):** [ZERO_ONE] The reading exceeds the strategic baseline drift tolerance ($\Psi_{\text{drift}} > \epsilon$), but remains within physical structural safety. Performance is throttled, and the Symmetrical Guard flags a caution.
3.  **`10` - SATURATED (Warning):** [ONE_ZERO] The sensor is operational, but has reached its maximum physical/computational limits (e.g., chat context length approaching 16k characters, or thermal sensors operating at the high end of their continuous-duty threshold).
4.  **`11` - NOMINAL (Standard Operation):** [ONE_ONE] The sensor value aligns perfectly with the dual tactical and strategic baselines, committing state in perfect operational resonance.

Because each sensor reports its state through this 2-bit matrix across all four PSTA dimensions, **any drop in telemetry or state change is localized to its specific pillar, rather than corrupting the entire system's decision loop.**

---

## 🕒 3. Temporal Baseline Accumulation (Hysteresis Over Time)

To prevent the system from reacting erratically to instantaneous spikes, we introduce **Temporal Integration**. We do not make safety decisions based on a single sample $x(t)$. Instead, we monitor the **cumulative deviation** over a sliding time-window $[t - W, t]$.

### 3.1 Cumulative Drift Integration
We define the **Temporal Discrepancy Integral** ($I_k$) for sensor $k$:

$$I_k(t) = \int_{t-W}^{t} |x_k(\tau) - \mu_{\text{strat}}(\tau)| \, d\tau$$

Where:
*   $\mu_{\text{strat}}$ is the Strategic Baseline (the long-term historical norm).
*   $W$ is the sliding temporal window.

### 3.2 The Threshold Spike Trigger
An instantaneous spike is allowed to pass as noise. The sensor's state is only degraded to `01` or `00` if the integrated discrepancy exceeds the maximum allowed energy envelope $\theta_{\text{energy}}$:

$$\text{State}_k(t) = \begin{cases}
      00 & \text{if } I_k(t) > \theta_{\text{energy}} \quad (\text{Sustained Failure}) \\
      01 & \text{if } I_k(t) > \theta_{\text{caution}} \quad (\text{Slow Drift}) \\
      11 & \text{otherwise} \quad (\text{Nominal / Transients Ignored})
   \end{cases}$$

This mathematical formulation mimics the thermal capacity of physical systems. A momentary spike in temperature doesn't melt a wire; a sustained, high-current draw does. By integrating over time, we eliminate false Byzantine alerts.

---

## 🏛️ 4. The Ledger of Truth vs. Autopilot Override

The classic Byzantine dilemma forces an immediate, automated decision (e.g., the autopilot locking out the pilot). In the Sovereign Framework, we decouple **Risk Evaluation** from **Control Authority**:

```
+------------------------+      +------------------------+
|   Physical Sensors     | ---> |   Local Bridge (PSTA)  |
|  - 2-Bit State Matrix  |      |  - Calculates VSS      |
+------------------------+      |  - Write-Only Ledger   |
                                +-----------+------------+
                                            |
                                            v
+------------------------+      +------------------------+
|   Control Decoupling   | <--- |   Asymmetric Alert     |
|  - NO AUTO-OVERRIDE    |      |  - Informs Pilot (A)   |
|  - State Auditing      |      |  - Adjusts Thresholds  |
+------------------------+      +------------------------+
```

### 4.1 Non-Intrusive Risk Auditing
The PSTA framework acts primarily as an **Objective Ledger of Truth** rather than a primary controller.
*   **The Pilot's Authority (A-Pillar):** The human operator remains the ultimate authority.
*   **The Auditing Role:** If the pilot's input causes a flight control surface to move in a way that contradicts sensor consensus, the system does not forcibly hijack control (which would cause a catastrophic pilot-autopilot fight).
*   **The Logging Action:** Instead, the PSTA engine writes an **Asymmetric Conflict Event** to the append-only Scribe ledger, lowers the global Vessel Safety Status ($VSS$), adjusts UI warning indicators, and throttles peak engine output safely while leaving the pilot with manual override authority.

This guarantees that we maintain **provable trust and complete accountability** after the event, without risking an automated autopilot locking out human judgment in a split-second crisis.

---

## 🏁 5. Comparative Matrix: Sovereign PSTA vs. Standard BFT

| Vector | Classic Byzantine Generals (BFT) | Sovereign Temporal Consensus (PSTA) |
| :--- | :--- | :--- |
| **Temporal Nature** | **Stateless:** Evaluates a single instant in time. | **Time-Series:** Integrates cumulative deviation over window $W$. |
| **Sensor States** | **Binary:** Operational or Faulty. | **2-Bit Matrix:** Four states (`00`, `01`, `10`, `11`) mapped to Theta Enums. |
| **Noise Filtering** | None. A single lie can trigger consensus rounds. | **Hysteresis:** momentary spikes are ignored via integral $I_k(t)$. |
| **Control Action** | **Deterministic:** Automated node voting/override. | **Decoupled:** Objective ledgering; human-in-the-loop priority. |
| **Data Preservation**| State overwritten upon consensus. | **Immutable Scribe:** Every discrepancy committed to `.bak` log. |

---

## 🚀 6. Mathematical Actions for Implementation

To implement this temporal consensus model in the next cycle, we will create the following C++ components:
1.  **`FSovereignTemporalBuffer`:** A sliding queue that stores the last $N$ seconds of telemetry values ($x_k(t)$).
2.  **`UPSTAEnergyEvaluator`:** A utility class that calculates the cumulative discrepancy integral $I_k(t)$ and updates the 2-bit state matrix on a 10Hz tick.
3.  **`FConflictLedgerScribe`:** An append-only serialization helper that writes asymmetric agent-sensor disputes directly to `/Saved/BlackBox/` without blocking the main game thread.
