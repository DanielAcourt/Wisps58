# Deterministic Non-Compensatory Safety Kernels for Autonomous Space Probes: The PSTA Framework

**Daniel Acourt**
*Sovereign Systems Laboratory*
Email: research@sovereign-systems.org

---

## Abstract

As space exploration missions extend beyond inner-solar-system radio coverage into deep-space, planetary shadow, and high-latency loss-of-signal (LOS) environments, autonomous vessels must transition from earth-gated command loops to Local Sovereignty. In these microsecond transition windows, probabilistic machine learning models, soft AI guardrails, and additive scoring mechanisms fail due to mathematical assumptions of statistical continuity and distribution shift continuity.

This paper introduces the Provable Trust Safety Aware (PSTA) framework: a deterministic, non-compensatory execution kernel engineered specifically for autonomous space probes. PSTA replaces binary logic $b \in \{0, 1\}$ and probabilistic confidence scores with the physical Tri-State Base Bit ($\mathcal{B} \in \{0, 1, \perp\}$), enforcing an unbypassable Void Safety Operator ($\mathcal{V}(\mathcal{B}) = 0$) and a Leontief Non-Compensatory Step-Guard ($\theta_i$).

We demonstrate PSTA's performance during deep-space telemetry blackouts and sensor degradation, proving that PSTA prevents catastrophic "sensor trust averaging" (where high confidence in nominal sub-systems masks isolated sensor failure). Furthermore, we map PSTA directly to NASA-STD-8719.13C software safety hazard requirements, establishing a formal mathematical foundation for certified autonomous space probe architectures compliant with DO-178C Level A and AUTOSAR C++14 standards.

***Index Terms*—Autonomous Space Probes, Software Safety, Non-Compensatory Logic, Loss of Signal, Local Sovereignty, Tri-State Logic, NASA-STD-8719.13C.**

---

## I. INTRODUCTION AND PROBLEM STATEMENT

### A. The Challenge of Deep-Space Loss-of-Signal (LOS) Transitions
Deep-space exploration vessels (e.g., Europa surface landers, interstellar probes, or Mars entry-descent-landing vehicles) operate in environments where round-trip light time (RTLT) ranges from tens of minutes to hours. During critical flight maneuvers—such as atmospheric entry, orbital insertion, or planetary shadow occlusion—earth-based ground control intervention is physically impossible ($RTLT > 0$).

At the precise microsecond when ground communication drops ($LOS = 1$), the vessel undergoes a state transition from Remote Command Sovereignty to Local Sovereignty. In this state, the probe's onboard autonomy software must independently evaluate:
1) Which physical sensors remain trustworthy under environmental radiation or thermal shock.
2) How to process unvetted, missing, or corrupted telemetry streams.
3) Whether to execute thruster fires or science payloads under partial system degradation.

### B. The Failure of Probabilistic and Additive Safety Architectures
Modern machine learning safety approaches (e.g., RLHF-aligned LLMs, Conformal Prediction, Guardrails AI, or Bayesian belief networks) evaluate safety using continuous weighted averages or statistical confidence bounds ($1-\delta$).

Under real-world spaceflight anomaly conditions, these systems suffer from three catastrophic epistemic failures:

1) *Compensatory Averaging Trap:* An additive safety function $S_{\text{standard}} = \sum w_i x_i$ allows exceptionally high scores in structural or thermal health (e.g., 99%) to mask a fatal drop in reaction-control fuel pressure or star-tracker alignment (e.g., 0%), resulting in a high composite score ($S_{\text{standard}} > \tau$) that authorizes a lethal maneuver.
2) *Distribution Shift Breakdown:* Statistical validation methods (such as conformal prediction) rely on Radon-Nikodým derivatives ($\frac{d\mathcal{D}}{d\mathcal{D}_0}$) assuming absolute continuity between inflight telemetry ($\mathcal{D}$) and ground simulation models ($\mathcal{D}_0$). When unexpected space environment phenomena occur ($\mathcal{D}_0(x) = 0$), Radon-Nikodým derivatives explode ($\to \infty$), rendering probabilistic confidence bounds meaningless.
3) *The Binary Forced-State Dilemma:* Standard digital systems force unvetted or dropped telemetry into binary true or false ($0$ or $1$), introducing unquantified epistemic debt.

---

## II. AXIOMATIC DERIVATION OF THE PSTA KERNEL

To solve these systemic failures, the Sovereign PSTA Framework abstracts safety enforcement out of probabilistic models into a bare-metal, mathematically provable C++ execution kernel.

```
       +-------------------------------------------------------+
       |             PHYSICAL TELEMETRY SUBSTRATE              |
       |  (Star Trackers, IMUs, Thruster Sensors, Biometrics)  |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |      TRI-STATE BASE BIT REGISTRATION: B in {0,1,perp} |
       |     0 = Unsafe | 1 = Nominally Safe | perp = Void     |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |             VOID SAFETY OPERATOR: V(B)                |
       |           V(1) = 1  |  V(0) = 0  |  V(perp) = 0       |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |  PSTA DOMAIN HEALTH DERIVATION (P, S, T, A Dimensions)|
       |   D_i = Sum(x_ij * w_ij) / Sum(w_ij)  in [0.0, 1.0]   |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |   LEONTIEF NON-COMPENSATORY STEP GUARD: theta_i       |
       |    theta_i = step(D_i - tau_fail,i) in {0, 1}          |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |        VESSEL SAFETY STATUS (VSS) CALCULATION         |
       |     VSS = ( Prod theta_i ) * Sum( alpha_i * D_i )     |
       |          IF ANY theta_k == 0  ==>  VSS = 0.0          |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |        AAS 409 CONFLICT GATE / HARDWARE EXECUTION      |
       |  Execute Command ONLY IF VSS >= Tau_Required AND      |
       |                   All theta_i == 1                    |
       +-------------------------------------------------------+
```

### A. The Physical Substrate and The Microsecond Instantiation Paradox
In physical computation, a hardware bit undergoing initial power surge or severe cosmic radiation bit-flip does not instantaneously exist in static $0$ or $1$. At the microsecond of state evaluation, physical telemetry exists in an unvetted, transitional state: Void ($\perp$).

We define the Sovereign Tri-State Base Bit ($\mathcal{B}$):

$$\mathcal{B} \in \{0, 1, \perp\}$$

Where:
* $0 \implies \text{Evaluated Unsafe / Nominal Failure}$
* $1 \implies \text{Evaluated Nominally Safe}$
* $\perp \implies \text{Void / Untracked / Telemetry Loss / Radiation Occlusion}$

### B. The Void Safety Operator
In space mission control, untracked or corrupted state telemetry cannot be assumed nominal. The Void Safety Operator $\mathcal{V}(\mathcal{B})$ deterministically maps unknown states to hard zero:

$$\mathcal{V}(\mathcal{B}) = \begin{cases} 1 & \text{if } \mathcal{B} = 1 \\ 0 & \text{if } \mathcal{B} = 0 \text{ or } \mathcal{B} = \perp \end{cases}$$

This operator ensures that if radiation knocks out a star-tracker telemetry bus ($\mathcal{B} = \perp$), the kernel does not attempt to extrapolate or guess; it drops the sensor health evaluation instantly to $0.0$.

### C. Continuous Domain Health Matrix ($D_i$)
Telemetry inputs are organized into four orthogonal domain dimensions representing the complete operational context of the autonomous probe:

$$\mathbf{S}_{\text{sys}}(t) = \langle P(t), S_{\text{soc}}(t), T(t), A(t) \rangle$$

1) *Psychological / Cognition Domain ($P$):* Onboard AI alignment velocity, KV-cache memory reservation integrity, and instruction processing drift.
2) *Social / Inter-Agent Domain ($S_{\text{soc}}$):* Inter-probe mesh communications, satellite relay latency, and swarm consensus cohesion.
3) *Technical Substrate Domain ($T$):* Physical sensor voltage, thruster chamber pressure, thermal gradients, and radiation dosage.
4) *Administrative / Mission Policy Domain ($A$):* Mission trajectory bounds, planetary protection protocol constraints, and power budget limits.

For each dimension $i \in \{P, S, T, A\}$, $K_i$ normalized telemetry sub-factors ($x_{ij} \in [0.0, 1.0]$) are aggregated using non-zero weighting factors $w_{ij}$:

$$D_i = \frac{1}{\sum_{j=1}^{K_i} w_{ij}} \sum_{j=1}^{K_i} \left( \mathcal{V}(\mathcal{B}_{ij}) \cdot x_{ij} \cdot w_{ij} \right) \in [0.0, 1.0]$$

Notice that if $\mathcal{B}_{ij} = \perp$, then $\mathcal{V}(\mathcal{B}_{ij}) = 0$, contributing zero health credit to $D_i$.

---

### D. The Leontief Non-Compensatory Step-Guard and Vessel Safety Status ($VSS$)

To eliminate compensatory averaging, PSTA applies economic Leontief production bottleneck principles to physical hardware safety.

We define the Step-Function Guard ($\theta_i$) for each domain:

$$\theta_i = \text{step}(D_i - \tau_{\text{fail}, i}) = \begin{cases} 1 & \text{if } D_i \ge \tau_{\text{fail}, i} \\ 0 & \text{if } D_i < \tau_{\text{fail}, i} \end{cases}$$

Where $\tau_{\text{fail}, i}$ represents the non-negotiable critical failure threshold for domain $i$.

The composite Vessel Safety Status ($VSS$) is defined as:

$$VSS = \left( \prod_{i \in \{P, S, T, A\}} \theta_i \right) \cdot \sum_{i \in \{P, S, T, A\}} \alpha_i D_i$$

Where $\sum \alpha_i = 1.0$ and $\alpha_i > 0$.

#### Theorem 1 (Zero-Tolerance Short-Circuit):
*If there exists any domain $k \in \{P, S, T, A\}$ such that $D_k < \tau_{\text{fail}, k}$, then $\theta_k = 0$, which implies:*

$$VSS = 0 \cdot \sum_{i} \alpha_i D_i = 0.0$$

*Proof:* Matrix multiplication with scalar zero yields identically zero. Even if three domains evaluate to maximum health ($D_P = 1.0, D_{S_{\text{soc}}} = 1.0, D_A = 1.0$), a single catastrophic failure in Technical Substrate ($D_T < \tau_{\text{fail}, T} \implies \theta_T = 0$) forces $VSS = 0.0$ instantly. $\blacksquare$

---

### E. Risk Velocity ($V_i$) and Predictive Acceleration

Safety is not static; telemetry decays over time. PSTA calculates the dynamic Risk Velocity ($V_i$) using finite time step differentials:

$$V_i(t) = \frac{\Delta D_i}{\Delta t} = \frac{D_i(t) - D_i(t - \Delta t)}{\Delta t}$$

To prevent division by zero during high-frequency execution cycles (e.g., 1000Hz C++ simulation ticks), $\Delta t$ is strictly clamped: $\Delta t \ge 0.001\text{s}$.

If Risk Velocity exceeds dynamic rate limits ($V_i < -\rho_i$), the system predicts imminent threshold violation before $D_i$ drops below $\tau_{\text{fail}, i}$, preemptively tripping the step guard $\theta_i \to 0$.

---

## III. DEEP-SPACE SCENARIO: TELEMETRY LOSS AND LOCAL SOVEREIGNTY TRANSITION

### A. Scenario Setup
Consider an autonomous space probe conducting an orbital insertion maneuver behind Europa (Jupiter moon occlusion).

* *Phase 1 (Nominal Line-of-Sight):* Probe receives Earth telemetry and GPS-like optical navigation anchors. $VSS = 0.94$.
* *Phase 2 (Eclipse / Loss of Signal):* Solar glare or icy moon occlusion obscures primary optical star-trackers. Primary IMU experiences thermal drift. LOS goes from $0 \to 1$.

```
Telemetry Stream:
  - Optical Star Tracker A: [CORRUPTED / LOS] ==> B_starA = perp
  - Optical Star Tracker B: [CORRUPTED / GLARE] ==> B_starB = perp
  - Backup Gyroscope IMU:   [NOMINAL, DRIFTING] ==> B_imu = 1, x_imu = 0.62
  - Engine Chamber Pressure:[NOMINAL] ==> B_press = 1, x_press = 0.98
```

### B. Execution Trace Comparison

#### 1) Method 1: Standard Probabilistic / Weighted Model
In a standard system, the optical tracker failures are assigned an estimated confidence or averaged with nominal pressure and power metrics:

$$S_{\text{standard}} = (0.0 \cdot 0.25) + (0.62 \cdot 0.25) + (0.98 \cdot 0.25) + (0.95 \cdot 0.25) = 0.6375$$

Because $S_{\text{standard}} = 0.6375 > 0.50$ (standard threshold), the standard system authorizes the orbital thruster burn, unaware that optical orientation is completely lost ($\perp$). *Result: Probe fires thrusters in wrong orientation, tumbling into planetary collision.*

#### 2) Method 2: Sovereign PSTA Kernel
Under PSTA, the missing star-tracker streams evaluate to $\mathcal{B}_{\text{starA}} = \perp$ and $\mathcal{B}_{\text{starB}} = \perp$.

1) *Void Safety Application:*
   $$\mathcal{V}(\mathcal{B}_{\text{starA}}) = 0, \quad \mathcal{V}(\mathcal{B}_{\text{starB}}) = 0$$

2) *Technical Domain Health Calculation ($D_T$):*
   $$D_T = \frac{(0 \cdot 1.0) + (0 \cdot 1.0) + (0.62 \cdot 0.8) + (0.98 \cdot 1.0)}{1.0 + 1.0 + 0.8 + 1.0} = \frac{1.476}{3.8} = 0.388$$

3) *Step Guard Evaluation ($\theta_T$):*
   Given critical technical threshold $\tau_{\text{fail}, T} = 0.70$:
   $$\theta_T = \text{step}(0.388 - 0.70) = 0$$

4) *Composite Vessel Safety Status ($VSS$):*
   $$VSS = (\theta_P \cdot \theta_{S_{\text{soc}}} \cdot 0 \cdot \theta_A) \cdot \sum \alpha_i D_i = 0.0$$

5) *Local Sovereignty Arbitration:*
   The PSTA kernel trips the Agency Arbitration Schema (AAS) 409 CONFLICT GATE. The probe immediately aborts the main thruster burn, places itself in stable gyroscopic sun-pointing orientation, and shifts to Safe-Hold Local Sovereignty until telemetry resolves. *Result: Mission and probe survive.*

---

## IV. NASA SOFTWARE SAFETY STANDARD COMPLIANCE MAPPING

To demonstrate flight-readiness for space missions, PSTA maps directly to NASA-STD-8719.13C (Software Safety Standard) requirements, as detailed in Table I.

TABLE I
NASA-STD-8719.13C SOFTWARE SAFETY COMPLIANCE MATRIX

| NASA-STD-8719.13C Requirement | PSTA Kernel Implementation Mechanism |
| :--- | :--- |
| **Section 5.1.2:** Autonomous Hazard Mitigation without Operator Input | Non-Compensatory Leontief Step-Guards ($\theta_i = 0$) force $VSS = 0.0$, automatically aborting hazardous burns. |
| **Section 5.2.1:** Single Failure Criterion (No single sensor fault leads to mission loss) | Tri-State Base Bit ($\mathcal{B} \in \{0, 1, \perp\}$) and Void Operator $\mathcal{V}(\perp)=0$ isolate single sensor failures instantly. |
| **Section 5.3.4:** Software Determinism & Predictable Execution Timing | Compiled C++ static memory execution, frame-locked via `GFrameCounter` with zero dynamic allocation in the tick loop. |
| **Section 5.8.3:** Independent Telemetry Audit Logging & Replay | High-fidelity RFC 6962 Merkle-tree Black Box telemetry recorder logging exact ($\mathcal{B}_i, D_i, VSS$) states at microsecond resolution. |

---

## V. C++ IMPLEMENTATION AND MEMORY BOTTLENECK CONTROLS

PSTA is implemented directly in C++ for real-time flight controllers and simulation environments (e.g., Unreal Engine 5.8 / Bare-Metal Flight Executive).

### A. Key Architectural Guards in C++
1) *Zero-Dynamic Memory Allocation:* All $D_i$, $\theta_i$, and $V_i$ matrices are pre-allocated in static arrays (`EPSTABaseBit`, `FSOVEREIGN_PSTA_State`) to prevent flight-computer heap fragmentation.
2) *Frame-Lock Throttling (`GFrameCounter`):* Telemetry updates enforce a configurable time-throttle interval ($\Delta t_{\text{min}} = 0.05\text{s}$) to prevent CPU thread starvation during sensor spikes.
3) *Explicit String Key Intermediate Isolation:* To maintain absolute compiler stability across Visual Studio 2026 / C++14/20 standards, JSON serialization uses explicit local string assignments (`FString KeyStr = FString(Elem.Key);`), eliminating temporary object lifetime corruption.

---

## VI. CONCLUSION AND FUTURE ROADMAP

The PSTA Framework bridges the gap between high-level autonomous decision-making and bare-metal software safety requirements. By replacing continuous additive scoring with the Tri-State Base Bit ($\mathcal{B} \in \{0, 1, \perp\}$) and Leontief Step Guards ($\theta_i$), PSTA guarantees that autonomous space probes remain provably safe during microsecond loss-of-signal transitions into Local Sovereignty.

### A. Next Steps for Implementation and Verification
1) *IEEE Journal Submission:* Formal submission to *IEEE Transactions on Aerospace and Electronic Systems*.
2) *Hardware-in-the-Loop (HITL) Simulation:* Integration with flight-computer testbeds (RTX 5090 / ARM Cortex-R flight controller simulation) under simulated radiation bit-flips.
3) *NASA SBIR Grant Proposal:* Submitting PSTA as a software safety kernel proposal for future NASA outer-planet exploration probes.

---

## REFERENCES

[1] National Aeronautics and Space Administration, *Software Safety Standard*, NASA-STD-8719.13C, 2020.
[2] RTCA / EUROCAE, *Software Considerations in Airborne Systems and Equipment Certification*, DO-178C / ED-12C, 2011.
[3] R. E. Kalman, "A new approach to linear filtering and prediction problems," *Journal of Basic Engineering*, vol. 82, no. 1, pp. 35–45, 1960.
[4] J. Rushby, "Kernelized safety architectures," *ACM Computing Surveys*, vol. 21, no. 3, pp. 335–367, 1989.
[5] J. Zhang, "Right to history: A sovereignty kernel for verifiable AI agent execution," *PunkGo Protocol Specifications*, Tech. Rep., 2026.
[6] Y. Zhao, E. Zhu, B. Hoxha, G. Fainekos, J. V. Deshmukh, and L. Lindemann, "Distributionally robust predictive runtime verification under spatio-temporal logic specifications," *ACM Transactions on Cyber-Physical Systems*, vol. 9, no. 2, pp. 101–124, 2025.
[7] D. Acourt, "The Sovereign Framework and Agency Arbitration Schema Protocol Specifications," Sovereign Systems Lab, Tech. Rep. SSL-TR-2026-01, 2026.
