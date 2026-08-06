# Mathematical Proposal: Bi-Symmetric Sensor Consensus & Hazard Detection (PSTA-T Hardening)

**Author:** Jules (Researcher / Systems Tactician)
**Date:** July 2026
**Node:** `AI_Nexus/Research/Mathematics/Bi_Symmetric_Consensus_Spec.md`
**Status:** Canonical Proposal
**Version:** 1.0.0

---

## 🏛️ 1. Executive Summary: The Biological Premise

Legacy aerospace and robotic architectures often use $N$-modular redundancy (e.g., three sensors voting) to isolate a single point of failure. While algebraically clean, this is heavy and expensive.

We look instead to biological evolution: **Bi-Symmetric Semicoupling**.

The human body does not use triple-redundancy. It operates on **pairs**: two eyes, two ears, two kidneys, two vestibular systems. By pairing an equal and opposite sensor channel, we can achieve high-fidelity consensus and, more importantly, **instantly distinguish between an internal sensor failure and a true external environmental hazard.**

Using the **Fish Tank Alpha (FTA)** water temperature telemetry as a concrete case study, this document formalizes the mathematical and logical structures of Bi-Symmetric PSTA Consensus.

---

## 📐 2. The Bi-Symmetric Mathematical Formulation

Let a physical telemetry variable (e.g., Water Temperature) be monitored by a symmetric pair of physical sensors, $S_A$ and $S_B$, yielding readings $x_A(t)$ and $x_B(t)$.

### 2.1 The Core Variables
*   **The Observed Value ($x(t)$):** The simple average of the paired readings.
    $$x(t) = \frac{x_A(t) + x_B(t)}{2}$$
*   **The Symmetry Delta ($\Delta_{\text{sym}}(t)$):** The absolute difference between the pair.
    $$\Delta_{\text{sym}}(t) = |x_A(t) - x_B(t)|$$
*   **The Safe Operating Envelope ($\mathcal{E}$):** Defined by low and high thresholds:
    $$\mathcal{E} = [T_{\text{low}}, T_{\text{high}}]$$

---

## ⚡ 3. Anomaly vs. Hazard: The 2D State-Space

By evaluating the relationship between the **Symmetry Delta ($\Delta_{\text{sym}}$)** and the **Envelope Violation**, the system dynamically categorizes state into two mutually exclusive zones:

```
                  +-----------------------------------+
                  |   Symmetry Delta (Δ_sym)          |
                  |   Low (< ε)        |  High (>= ε) |
+-----------------+--------------------+--------------+
| Envelope (x)    |                    |              |
| Inside [TL, TH] |     1. NOMINAL     | 3. ANOMALY   |
|                 |  (Perfect Sync)    | (Sensor Fail)|
+-----------------+--------------------+--------------+
| Outside Envelope|     2. HAZARD      | 4. UNKNOWN   |
| (x > TH / < TL) |   (Real Threat)    | (Total Chaos)|
+-----------------+--------------------+--------------+
```

### 1. NOMINAL STATE (Perfect Sync)
*   **Conditions:** $\Delta_{\text{sym}}(t) < \epsilon$ AND $x(t) \in \mathcal{E}$.
*   **System Action:** Both sensors agree, and the reading is safe. Trust ($\Phi$) is $1.0$. The Technical pillar ($D_T$) operates at maximum health.

### 2. COHERENT HAZARD (External Environmental Threat)
*   **Conditions:** $\Delta_{\text{sym}}(t) < \epsilon$ AND $x(t) \notin \mathcal{E}$.
*   **System Action:** **The sensors back each other up perfectly, but they agree that the environment is unsafe.**
    *   Because they agree, the **Trust Metric remains $1.0$ (High Certainty)**.
    *   Because the value is unsafe, the **Technical Pillar $D_T$ collapses immediately**.
    *   This represents a real, verified external danger (e.g., the water heater failed and the tank is dropping to 10°C). The system triggers immediate mitigation (e.g., alerts, power cycle).

### 3. SENSOR ANOMALY (Internal Component Degradation)
*   **Conditions:** $\Delta_{\text{sym}}(t) \ge \epsilon$ AND $x(t) \in \mathcal{E}$.
*   **System Action:** **The sensors disagree, but the average remains theoretically safe.**
    *   This is an **internal failure**. One of the sensors has experienced electrical drift or hardware failure.
    *   The **Trust Metric ($\Phi$) drops immediately** proportional to the drift:
        $$\Phi(t) = \max\left(0.0, 1.0 - \frac{\Delta_{\text{sym}}(t) - \epsilon}{\Delta_{\text{fault}}}\right)$$
    *   The Technical pillar $D_T$ enters **Caution**, performance is throttled, and the Scribe ledger logs a maintenance alert to replace the degrading sensor. Crucially, the system knows the fish are safe for now; only the *measurement* is broken.

### 4. COMPROMISED UNKNOWN (Total Chaos)
*   **Conditions:** $\Delta_{\text{sym}}(t) \ge \epsilon$ AND $x(t) \notin \mathcal{E}$.
*   **System Action:** The sensors disagree, and the average is unsafe. Complete epistemic failure. The trust score collapses to $0.0$, Void Safety triggers, and VSS drops to $0.0$ instantly to protect the vessel.

---

## 🐟 4. Case Study: Fish Tank Alpha (FTA) Temperature Loop

Let's apply this directly to the Fish Tank Alpha JSON configuration. The water temperature is monitored by two physical DS18B20 temperature probes, $T_1$ and $T_2$.

*   **Envelope Limits:** $T_{\text{low}} = 22.0^{\circ}\text{C}$, $T_{\text{high}} = 26.0^{\circ}\text{C}$.
*   **Symmetry Tolerance ($\epsilon$):** $0.5^{\circ}\text{C}$ (sensors must agree within half a degree).
*   **Catastrophic Fault Limit ($\Delta_{\text{fault}}$):** $2.0^{\circ}\text{C}$ (if they drift 2 degrees apart, one is completely dead).

### Scenario A: The Coherent Hazard (Real Threat)
*   *Readings:* $T_1 = 18.2^{\circ}\text{C}$, $T_2 = 18.4^{\circ}\text{C}$.
*   *Math:*
    *   $x(t) = 18.3^{\circ}\text{C}$ (Outside envelope).
    *   $\Delta_{\text{sym}}(t) = 0.2^{\circ}\text{C}$ (Below $\epsilon = 0.5$, they are in sync).
*   *Conclusion:* **COHERENT HAZARD**. Trust is high. The water is freezing. Immediate emergency notification is sent to the General.

### Scenario B: The Sensor Anomaly (Internal Failure)
*   *Readings:* $T_1 = 24.0^{\circ}\text{C}$, $T_2 = 25.8^{\circ}\text{C}$ (Sensor 2 is drifting high).
*   *Math:*
    *   $x(t) = 24.9^{\circ}\text{C}$ (Inside safe envelope).
    *   $\Delta_{\text{sym}}(t) = 1.8^{\circ}\text{C}$ (Well above $\epsilon = 0.5$).
*   *Conclusion:* **SENSOR ANOMALY**. The average water temperature is fine, but Sensor 2 is failing. Trust drops to:
    $$\Phi(t) = 1.0 - \frac{1.8 - 0.5}{2.0} = 0.35$$
    The PSTA log registers a sensor fault. Auto-control is throttled, but no panic shutdown occurs.

---

## 🚀 5. Blueprint & C++ Spawning Implementation

To support this bi-symmetric pairing in our C++ core, the `UDiagnosticBroker` will be updated with:

1.  **`FSovereignSensorPair` Struct:**
    ```cpp
    struct FSovereignSensorPair {
        FString TelemetryKey; // e.g., "Telemetry.temp_c"
        FString SensorA_ID;
        FString SensorB_ID;
        float SymmetryTolerance; // epsilon
        float FaultLimit; // Delta_fault
        float CurrentTrust; // Phi
    };
    ```
2.  **`EvaluateBiSymmetry` Thread Task:**
    Performs the 2D state-space evaluation asynchronously, updating the active telemetry JSON with distinct `Trust` and `Status` keys (e.g., `Telemetry.temp_c.trust = 0.35`, `Telemetry.temp_c.status = "ANOMALY"`).

This ensures that our **Digital Twin** visual twin does not react erratically to physical wiring glitches, but responds instantly and decisively to actual ecological emergencies.
