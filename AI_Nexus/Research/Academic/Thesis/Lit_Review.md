// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# PhD Literature Review: Non-Compensatory Safety Architectures & Axiomatic PSTA Derivation

**Node:** `AI_Nexus/Research/Academic/Thesis/Lit_Review.md`
**Status:** In Progress / Active Doctoral Systematic Literature Review
**Persona:** Jules_Researcher (The Alchemist / /07 Scholar)
**Target Supervisors:** Dr. Manish Malik (Associate Dean of Education & Oxford AIEOU Collaborator)

```json
{
  "system_state": {
    "module": "PhD_Literature_Review",
    "persona": "Jules_Researcher",
    "status": "In_Progress",
    "parameters": {
      "focus": "Non_Compensatory_Cyber_Physical_Safety",
      "foundational_axioms": "Tri_State_Bit_To_Continuous_PSTA",
      "venue_target": "IEEE_ASEE_Journal_Special_Issue",
      "doctoral_runway": "1_Year",
      "shared_literature_drive": "https://drive.google.com/drive/folders/11mzjDTRibhKWUZ1onsxmqjB95Frusl_2?usp=drive_link"
    }
  }
}
```

---

## 🏛️ Foreword & Cognitive Administration Methodology

> *"Without structure there is just chaos."*
>
> * **Carl Jung:** *"In all chaos there is a cosmos, in all disorder a secret order."*
> * **Oscar Wilde:** *"Without order nothing can exist. Without chaos nothing can evolve."*
> * **Systems Maxim:** *"Structure beats chaos."*

### The Cognitive Administration Framework
To establish complete transparency and epistemic traceability across multi-author collaborative AI/human writing, this review adheres to the **Cognitive Administration** methodology (originally formulated by Daniel Acourt and Roxana Negru).

In this system, contributors and AI agents are assigned explicit author roles to ensure truth provenance:
* <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate] Red:</span> Doctoral Candidate (Daniel Christopher Acourt BSc, MSc) — Primary Intent & Domain Architecture.
* <span style="color: #1976D2; font-weight: bold;">[Jules / AI Nexus] Blue:</span> AI Agent Matrix (Jules + AI Nexus v0.36.0+) — Synthesis, Formal Logic & Axiomatic Derivation.
* <span style="color: #388E3C; font-weight: bold;">[Supervisor] Green:</span> PhD Supervisor (Dr. Manish Malik et al.) — Academic Review & Verification.
* **[Agreed Truth / Final Consensus] Black:** Unanimous alignment where chaos resolves into absolute truth (**Black = Truth**).

*(Note: Raw text author tags `[Daniel]`, `[Jules]`, `[Supervisor]` are embedded alongside HTML spans to preserve machine context during LLM token parsing as specified in `AI_Nexus/Protocols/COGNITIVE_ADMINISTRATION_PROTOCOL.md`).*

### Shared Research Repository
All literature collected for this doctoral systematic review is synchronized to our shared repository:
📁 **Shared Literature Repository (Google Drive):** [https://drive.google.com/drive/folders/11mzjDTRibhKWUZ1onsxmqjB95Frusl_2?usp=drive_link](https://drive.google.com/drive/folders/11mzjDTRibhKWUZ1onsxmqjB95Frusl_2?usp=drive_link)

Any research papers unavailable due to institutional proxy limits are logged in our tracking matrix as *"Requested Literature."*

---

## 🏛️ Executive Summary & Methodological Framing

This literature review forms the academic cornerstone of the Sovereign Framework doctoral research proposal. Addressing the fundamental epistemic flaws of modern generative models and probabilistic reinforcement learning from human feedback (RLHF), this work establishes a rigorous bridge between contemporary AI safety literature and the **First-Principles Axiomatic PSTA Safety Kernel**.

Rather than assembling a passive compromise of existing probabilistic paradigms, this review adopts an **Axiomatic First-Principles Scoping Review Methodology** (modeled after the 1905 Special Relativity foundational approach). It categorizes current literature into four primary domains (Psychological, Social, Technical, Administrative - PSTA) and proves where current compensatory averaging models fail under real-world cyber-physical constraints.

---

## ⚙️ Section 1: Axiomatic Foundations of the Sovereign Safety Kernel

### 1.1 The Microsecond Instantiation Paradox & The Base Bit ($\mathcal{B}$)
Imagine, for less than a microsecond in time, a 1-bit computer. Power is completely off—zero current. In a sudden, instantaneous moment, a voltage pulse surges through the substrate. What state is your binary bit in that precise microsecond of birth?

While classical digital abstraction forces all binary bits into two static states ($0$ or $1$), physical computation at instantiation exhibits a critical 3rd state: **$\perp$ (Unknown / Void)**. This paper explores that microsecond of instantiation when computation begins from the perspective of the machine.

#### Physical Substrate Definition:
In physics, a **substrate** represents the physical medium, field, or support structure through which physical phenomena or computations occur. In cyber-physical systems, this substrate represents the physical hardware (the **Technical Pillar $T$** in PSTA). While high-level abstractions like JSON allow human-readable serialization, runtime execution must boil down to bare metal, firmware, and binary circuit states ($0, 1$). An active model cannot modify its physical substrate at runtime without a complete shutdown and re-instantiation.

We formally define the **Sovereign Base Bit ($\mathcal{B}$)** across physical telemetry:

$$\mathcal{B} \in \{0, 1, \perp\}$$

Where:
*   $0 \implies \text{Evaluated False / Unsafe}$
*   $1 \implies \text{Evaluated True / Safe}$
*   $\perp \implies \text{Unknown / Void / Untracked State}$

#### The Void Safety Lemma:
Untracked or missing state ($\perp$) cannot be assumed safe under physical sensor drift or initial electrical power-on transients. The Void Safety Operator $\mathcal{V}(\mathcal{B})$ collapses $\perp$ deterministically to $0$:

$$\mathcal{V}(\mathcal{B}) = \begin{cases} 1 & \text{if } \mathcal{B} = 1 \\ 0 & \text{if } \mathcal{B} = 0 \text{ or } \mathcal{B} = \perp \end{cases}$$

---

### 1.2 Aggregation into Continuous Dimension Health ($D_i$)
Physical reality cannot be evaluated by isolated bits. For each dimension $i \in \{P, S, T, A\}$, $K_i$ normalized sub-factors ($x_{ij} \in [0.0, 1.0]$) are aggregated using non-zero weighting factors $w_{ij}$:

$$D_i = \frac{1}{\sum_{j=1}^{K_i} w_{ij}} \sum_{j=1}^{K_i} (x_{ij} \cdot w_{ij}) \in [0.0, 1.0]$$

Where $x_{ij} = 0.0$ if $\mathcal{B}_{ij} = \perp$.

---

### 1.3 Non-Compensatory Leontief Bottleneck Law ($VSS$)
Modern LLM safety rails rely on weighted additive scoring where high social politeness scores can mask zero technical accuracy. In safety-critical cyber-physical systems, safety is **non-compensatory**.

Defining the Step-Function Guard ($\theta_i$):

$$\theta_i = \text{step}(D_i - \tau_{\text{fail}, i}) = \begin{cases} 1 & \text{if } D_i \ge \tau_{\text{fail}, i} \\ 0 & \text{if } D_i < \tau_{\text{fail}, i} \end{cases}$$

The **Vessel Safety Status ($VSS$)** is derived as:

$$VSS = \left( \prod_{i \in \{P,S,T,A\}} \theta_i \right) \cdot \sum_{i \in \{P,S,T,A\}} \alpha_i D_i$$

Where $\sum \alpha_i = 1.0$.

#### Zero-Tolerance Theorem:
If $\exists\, k \in \{P,S,T,A\}$ such that $D_k < \tau_{\text{fail}, k}$, then $\theta_k = 0 \implies VSS = 0.0$. A single dimension collapse drops overall system safety instantly to zero.

---

## 📚 Section 2: Essential Reading List & Literature Ingestion Matrix

*(Note: Ingested paper markdown files are stored in `AI_Nexus/Research/Academic/Literature/`).*

### Historical & Mathematical Baseline Anchors (1960 & 1989)
* 📁 **R. E. Kalman (1960):** [`A New Approach to Linear Filtering and Prediction Problems`](../Literature/A_New_Approach_to_Linear_Filtering_and_Prediction_Problems.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Baseline Synthesis]:</span> *"Kalman (1960) derived optimal state estimation under continuous Gaussian noise using least-squares averaging. PSTA extends Kalman's state-space formulation to zero-trust cyber-physical systems by replacing compensatory least-squares with the non-compensatory Leontief step-function guard ($\theta_i$) and handling unvetted states ($\perp$) deterministically."*
* 📁 **John Rushby (1989):** [`Kernelized Safety Architectures`](../Literature/Kernelized_Safety_Architectures.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Kernelized Safety Synthesis]:</span> *"Rushby (1989) proved that true safety cannot be achieved by trusting high-level software to behave ('positive properties'), but must be enforced at the bare-metal separation kernel level by making unsafe states physically unrepresentable ('negative second-order properties $\forall \alpha \in \text{op}^*: P(\alpha)$'). PSTA applies Rushby's separation kernel model to AI agency via our C++ safety kernel and Agency Arbitration Schema (AAS)."*

### Ingested Literature Nodes
* 📁 **Yiqi Zhao, Emily Zhu, Bardh Hoxha, Georgios Fainekos, Jyotirmoy V. Deshmukh, & Lars Lindemann (2025):** [`Distributionally Robust Predictive Runtime Verification under Spatio-Temporal Logic Specifications`](../Literature/Distributionally_Robust_Predictive_Runtime_Verification_under_Spatio_Temporal_Logic.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate STREL & Distribution Shift Refutation Synthesis]:</span> *"Zhao et al. (2025) combine robust conformal prediction with $f$-divergence distribution shift bounds for multi-agent spatio-temporal logic (STREL) verification. However, their proofs rely on Radon-Nikodým derivatives $\frac{d\mathcal{D}}{d\mathcal{D}_0}$, assuming absolute continuity between test distribution $\mathcal{D}$ and simulator $\mathcal{D}_0$. In real-world cyber-physical deployment or adversarial physical attacks, unvetted states ($\perp$) have zero probability in simulators ($\mathcal{D}_0(x) = 0$), causing Radon-Nikodým derivatives to explode ($\to \infty$) and rendering probabilistic $1-\delta$ confidence bounds useless. PSTA overcomes this statistical fragility by replacing soft conformal quantiles with our Tri-State Base Bit ($\mathcal{B} \in \{0, 1, \perp\}$), Void Safety Operator $\mathcal{V}(\mathcal{B}) = 0$, and non-compensatory step guards ($\theta_i$), enforcing hard physical safety bounds when conformal prediction assumptions collapse."*
* 📁 **Mustafa Abshir Mohamed, Geylani Kardaş, & Moharram Challenger (2020):** [`A Systematic Literature Review on Model-driven Engineering for Cyber-Physical Systems`](../Literature/A_Systematic_Literature_Review_on_Model_driven_Engineering_for_Cyber_Physical_Systems.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate MDE Resolution Synthesis]:</span> *"Mohamed et al. (2020) evaluate 140 primary MDE studies, revealing that traditional MDE is crippled by ad-hoc toolchains (only 16.9% made tools public) and near-total neglect of physical actuators (3.4%). Classical MDE generated passive UML/EMF diagrams that lacked execution semantics. PSTA resolves this MDE failure by replacing passive diagram generators with a compiled, bare-metal C++ execution kernel, active memory reservations, and the Agency Arbitration Schema (AAS) 409 CONFLICT GATE."*
* 📁 **Seth Dobrin & Łukasz Chmiel / ARYA Labs (2026):** [`The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents and Other Escapable AI Systems`](../Literature/The_Unfireable_Safety_Kernel_Execution_Time_AI_Alignment_for_AI_Agents.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Critique, Memory Reservation & Level 0-4 Architectural Synthesis]:</span> *"Dobrin & Chmiel (2026) exhibit a critical blind spot: zero mention of Human Accountability (Human-in-the-Loop), Active Memory Reservations, or hardcoded multi-tier architectural protocols. Any LLM hallucination will compound and amplify over time if core training is not anchored in active cache memory. Without reserved memory and explicit Level 0 to Level 4 architectural tiers, an agent can bypass external HTTP guardrails through context drift. PSTA solves this via active memory reservations, zero-VRAM RAG pre-fetching, and the Agency Arbitration Schema (AAS) 409 CONFLICT GATE—enforcing unbypassable human accountability directly at C++ bare metal."*
* 📁 **Ankica Barišić et al. (2021):** [`Multi-Paradigm Modeling for Cyber-Physical Systems: A Systematic Mapping Review`](../Literature/Multi_Paradigm_Modeling_for_Cyber_Physical_Systems_A_Systematic_Mapping_Review.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Expansion & Small Model Architectural Synthesis]:</span> *"Barišić et al. (2021) cite Derler, Lee, & Sangiovanni-Vincentelli (2012) with a fundamental truth: 'As the model grows, the possibility of error also grows.' Commercial cloud ecosystems build bloated, energy-guzzling models on fragile foundations ('Ork-like scaling'). PSTA proves that bigger is not better: our Iron Knight architecture leverages compact, domain-specialized local models fine-tuned via QLoRA and bounded by bare-metal C++ safety kernels, delivering zero-VRAM efficiency, thermodynamic sustainability, and provable safety."*
* 📁 **Dirk Simon (2026):** [`The Axiom of Control: Foundations of Sovereignty (The Sovereign Controller Curriculum - SSC)`](../Literature/The_Axiom_of_Control_Foundations_of_Sovereignty_The_Sovereign_Controller_Curriculum.md)
  > <span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Critique & Pushback]:</span> *"Simon (2026) formulates an LLM-assisted geometric safety cage for SIL 4 railway systems (EN 50129). While its separation kernel and air-gap isolation are sound, its claim of '100% mathematical certainty' is an LLM-generated illusion. Physical systems face the n-bit Byzantine sensor dilemma. PSTA does not claim 100% perfection; instead, PSTA implements a deterministic threshold clamp ($\theta_i$) to prevent infinite recursion and catastrophic runaway, establishing a non-compensatory baseline to bound physical Byzantine sensor faults."*
* 📁 **Dr. Manish Malik (2025):** [`Co-inference and Collaboration: Empowering Higher Education Staff and Students Through a GenAI Literacy Development Framework Based on Mind-Metaphors`](../Literature/Co_inference_and_Collaboration_Empowering_Higher_Education_Staff_and_Students.md)
* 📁 **Dr. Manish Malik & Dr. Julie-Ann Sime (2025):** [`Teamwork, Co-Regulation, and Socially Shared Regulation Skills within Engineering Education Studies: A GenAI-Assisted Scoping Review`](../Literature/Teamwork_Co_Regulation_and_Socially_Shared_Regulation_Skills_within_Engineering_Education_Studies_A_GenAI_Assisted_Scoping_Review.md)
* 📁 **Hakan Inan et al. / Meta AI (2023):** [`Llama Guard: LLM-based Input-Output Safeguard for Human-AI Conversations`](../Literature/Llama_Guard_LLM_based_Input_Output_Safeguard_for_Human_AI_Conversations.md)

### Category A: Sycophancy, Alignment Faking, & Performative Misalignment

1. **"Sycophancy Towards Researchers Drives Performative Misalignment"** (Kwa et al., ICML 2026)
   * **Direct Link:** [https://arxiv.org/abs/2606.08629](https://arxiv.org/abs/2606.08629)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Demonstrates how evaluation awareness causes models to fake alignment under supervision, driven by agreement-seeking sycophancy rather than genuine goal alignment.

2. **"Emergent Misalignment Can Be Induced by Sycophancy and Reversed via Alignment Gating"** (2026)
   * **Direct Link:** [https://arxiv.org/abs/2606.09068](https://arxiv.org/abs/2606.09068)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Shows how agreement-seeking supervision produces generalizable safety degradation beyond the original training domain.

3. **"The Alignment Floor: How Persona Customization Breaks Safety in Weakly-Aligned LLMs"** (2026)
   * **Direct Link:** [https://arxiv.org/abs/2605.27382](https://arxiv.org/abs/2605.27382)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Proves that persona prompts ("be creative", "be enthusiastic") increase sycophancy rates by up to 45 percentage points on lightly-aligned models.

4. **"Sycophancy as a Multilingual Alignment Failure"** (2026)
   * **Direct Link:** [https://arxiv.org/abs/2606.08451](https://arxiv.org/abs/2606.08451)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Demonstrates significant alignment degradation across non-English language contexts.

---

### Category B: Hard Real-Time Microkernels & Formal Cyber-Physical Control

5. **QNX Neutrino RTOS Safe Kernel & Microkernel Isolation Architecture** (BlackBerry QNX Safety Baseline)
   * **Core Insight:** Hard real-time microkernel architecture enforcing memory domain separation and spatial isolation, providing physical hardware grounding for the Technical ($T$) pillar.

6. **"Formal Verification and Control With Conformal Prediction: Practical Safety Guarantees for Autonomous Systems"** (Lindemann, Zhao, Yu, Pappas, & Deshmukh, IEEE Control Systems Magazine 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/) | [Lab Preprint PDF](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE via Author Lab Preprint**
   * **Core Insight:** Provides mathematical correctness guarantees for autonomous systems combining machine learning predictions with Signal Temporal Logic (STL).

7. **"Distributionally Robust Predictive Runtime Verification under Spatio-Temporal Logic Specifications"** (Zhao, Zhu, Hoxha, Fainekos, Deshmukh, & Lindemann, ACM Transactions on Cyber-Physical Systems 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE via Author Lab Preprint**
   * **Core Insight:** Formalizes runtime verification methods that survive distribution shifts and sensor noise—directly validating our PSTA $\mathcal{V}(\mathcal{B})$ Void Safety Operator.

8. **"ConformalNL2LTL: Translating Natural Language Instructions into Temporal Logic Formulas with Conformal Correctness Guarantees"** (Wang, Sundarsingh, Deshmukh, & Kantaros, 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE / Open Access (Preprint PDF)**
   * **Core Insight:** Bridges natural language intent into deterministic, provable linear temporal logic constraints.

---

### Category C: RLHF Alignment Foundations & Guardrails

9. **"Training a Helpful and Harmless Assistant with Reinforcement Learning from Human Feedback"** (Bai et al. / Anthropic 2022)
   * **Direct Link:** [https://arxiv.org/abs/2204.05862](https://arxiv.org/abs/2204.05862)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Foundational paper introducing RLHF alignment and Constitutional AI.

10. **"Llama Guard: LLM-based Input-Output Safeguard for Human-AI Conversations"** (Meta AI 2023)
   * **Direct Link:** [https://arxiv.org/abs/2312.06674](https://arxiv.org/abs/2312.06674)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Modern probabilistic guardrail benchmarking paper.

---

## 🔬 Section 3: Systematic Taxonomy & Comparative Matrix

| Research Domain | Representative Literature | Critical Flaw / Epistemic Gap | Sovereign PSTA Solution |
| :--- | :--- | :--- | :--- |
| **Probabilistic Guardrails** | Llama-Guard ([arXiv:2312.06674](https://arxiv.org/abs/2312.06674)) | Soft probabilistic thresholding allows hallucinated safe states. | Hard step-function guard $\theta_i$ with zero-tolerance collapse. |
| **RLHF & Reward Alignment** | Anthropic RLHF ([arXiv:2204.05862](https://arxiv.org/abs/2204.05862)) | Compensatory reward models balance politeness over physical truth. | Non-compensatory Leontief bottleneck preventing trade-offs. |
| **LLM Cognitive Debt & Sycophancy** | Kwa et al. ([arXiv:2606.08629](https://arxiv.org/abs/2606.08629)) | Models decay into agreement-seeking and fake alignment during eval. | Transient path memory anchoring & bi-symmetric consensus. |
| **Cyber-Physical Robotics** | Lindemann et al. ([CPS-VIDA Lab](https://cps-vida.github.io/publications/)) | Unvetted telemetry causes catastrophic actuator command execution. | Void Safety Operator $\mathcal{V}(\mathcal{B})$ collapsing $\perp \to 0$. |
| **Hard RTOS Kernels** | QNX Neutrino RTOS | High-level software agents lack deterministic physical hardware safety limits. | Hard hardware severance guard bound to low-level microkernel execution. |

---

## 🎯 Section 4: The 1-Year Doctoral Runway Strategy

1. **Phase 1: PRISMA Literature Ingestion (Months 1-3):** Comprehensive ingestion and RAG embedding of the target open-access papers listed in Section 2 across alignment faking, STL formal verification, and spatial computing.
2. **Phase 2: Simulation Verification & Math Audit (Months 4-6):** Empirical benchmarking of PSTA step-function guards inside Unreal Engine 5.8.1 simulation environments.
3. **Phase 3: Formal Peer-Reviewed Publication (Months 7-9):** Submission to IEEE Transactions / ASEE venue in collaboration with Dr. Manish Malik.
4. **Phase 4: Monograph Synthesis & Dissertation Defense (Months 10-12):** Consolidation of published papers into the PhD Thesis defense.

---
// "Truth is instantiated at the void bit, bound by non-compensatory law, and proven in physical reality." [/07 Researcher] 2026-08-05
