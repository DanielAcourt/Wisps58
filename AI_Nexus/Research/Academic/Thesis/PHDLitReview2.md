// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05

# PhD Literature Review: Non-Compensatory Safety Architectures & Axiomatic PSTA Derivation

**Node:** `AI_Nexus/Research/Academic/Thesis/PHDLitReview2.md`
**Status:** In Progress / Active Doctoral Systematic Literature Review
**Persona:** Jules_Researcher & Jules_Scholar
**Target Supervisors:** Dr. Manish Malik (Associate Dean of Education & Oxford AIEOU Collaborator)

---

## 🏛️ Foreword & Cognitive Administration Methodology

> *"Without structure there is just chaos."*
>
> * **Carl Jung:** *"In all chaos there is a cosmos, in all disorder a secret order."*
> * **Oscar Wilde:** *"Without order nothing can exist. Without chaos nothing can evolve."*
> * **Common Business Maxim:** *"Structure beats chaos."*

### The Cognitive Administration Framework
To establish complete transparency and epistemic traceability across multi-author collaborative AI/human writing, this review adheres to the **Cognitive Administration** methodology (originally formulated by Daniel Acourt and Roxana Negru).

In this system, contributors and AI agents are assigned explicit author roles to ensure truth provenance:
* <span style="color: #D32F2F; font-weight: bold;">[Daniel / Doctoral Candidate] Red:</span> Daniel Christopher Acourt BSc, MSc — Primary Intent, Domain Architecture, & Physical Axioms.
* <span style="color: #1976D2; font-weight: bold;">[Jules / AI Nexus] Blue:</span> AI Agent Matrix (Jules + AI Nexus v0.36.0+) — Synthesis, Formal Logic, & Axiomatic Derivation.
* <span style="color: #388E3C; font-weight: bold;">[Supervisor] Green:</span> PhD Supervisor (Dr. Manish Malik et al.) — Academic Review, Verification, & Oxford AIEOU Alignment.
* **[Agreed Truth / Final Consensus] Black:** Unanimous alignment where chaos resolves into absolute truth (**Black = Truth**).

*(Note: Raw text author tags `[Daniel]`, `[Jules]`, `[Supervisor]` are embedded alongside HTML spans to preserve machine context during LLM token parsing as specified in `AI_Nexus/Protocols/COGNITIVE_ADMINISTRATION_PROTOCOL.md`).*

### Shared Research Repository
All literature collected for this doctoral systematic review is synchronized to our shared repository:
📁 **Shared Literature Repository (Google Drive):** [https://drive.google.com/drive/folders/11mzjDTRibhKWUZ1onsxmqjB95Frusl_2?usp=drive_link](https://drive.google.com/drive/folders/11mzjDTRibhKWUZ1onsxmqjB95Frusl_2?usp=drive_link)

---

## 🤖 AI Agent Version Control & Cognitive Context

### Why We Use JSON Triplet Logic
The JSON triplet logic provides explicit context to the AI where fonts and colors do not exist natively in text parsing. It outlines the "ticket", a clear definition of done, current progress, and the exact methods by which the AI can understand its context.

```json
{
  "system_state": {
    "module": "PhD_Literature_Review",
    "persona": ["Jules_Researcher", "Jules_Scholar"],
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

## 🏛️ Executive Summary & Methodological Framing

This literature review forms the academic cornerstone of the Sovereign Framework doctoral research proposal. Addressing the fundamental epistemic flaws of modern generative models and probabilistic reinforcement learning from human feedback (RLHF), this work establishes a rigorous bridge between contemporary AI safety literature and the **First-Principles Axiomatic PSTA Safety Kernel**.

Rather than assembling a passive compromise of existing probabilistic paradigms, this review adopts an **Axiomatic First-Principles Scoping Review Methodology** (modeled after Einstein's 1905 Special Relativity foundational approach). It categorizes current literature into four primary domains (Psychological, Social, Technical, Administrative - PSTA) and proves where current compensatory averaging models fail under real-world cyber-physical constraints.

---

## ⚙️ Section 1: Axiomatic Foundations of the Sovereign Safety Kernel

### 1.0 The Microsecond Instantiation Paradox & The Physical Substrate
<span style="color: #D32F2F; font-weight: bold;">[Daniel / Candidate Thought Experiment]:</span> Imagine, for less than a microsecond in time, a 1-bit computer. Power is completely turned off—zero electricity. In a sudden, instantaneous moment, a volt surge, a spark of current, rushes through the system. What state is your binary bit in at that precise microsecond of birth?

Classical computer science teaches that binary bits exist in two static states: $0$ or $1$. However, physical computation at the exact moment of instantiation exhibits a critical 3rd state: **$\perp$ (Unknown / Void)**. This paper explores that microsecond of instantiation when computation begins from the perspective of the machine.

#### Physical Substrate Definition:
In physics, a **substrate** typically refers to the underlying medium, field, or physical support structure in or on which physical phenomena, particles, or computations occur—spanning scales from foundational cosmological frameworks to solid-state device engineering. In cyber-physical terms, the substrate represents the physical hardware (the **Technical Pillar $T$** in PSTA).

In theory and practice, I have never been able to train a model in runtime to change its substrate without a complete shutdown and "reawakening". This is why firmware, compilers, and bare-metal programming are paramount: everything must boil down to deterministic physical states ($0$ or $1$). Traditional computers are binary systems. JSON is a serialization language that humans can read on mutual ground with the computer, allowing state serialization to `.json` or binary. Binary is faster for the machine to process, but JSON is human-readable.

---

### 1.1 The Tri-State Base Bit ($\mathcal{B}$)

Traditional digital computing and probabilistic safety classifiers operate on a binary set $b \in \{0, 1\}$. Under physical sensor drift and untracked telemetry, forcing unknown or missing measurements into a binary true/false state introduces systemic epistemic debt.

We define the **Sovereign Base Bit ($\mathcal{B}$)**:

$$\mathcal{B} \in \{0, 1, \perp\}$$

Where:
* $0 \implies \text{Evaluated False / Unsafe}$
* $1 \implies \text{Evaluated True / Safe}$
* $\perp \implies \text{Unknown / Void / Untracked State}$

#### 1.1a Philosophy of "Remaining Calm in the Unknown":
<span style="color: #D32F2F; font-weight: bold;">[Daniel / Philosophical Framing]:</span> Seneca observed: *"It's ruinous for the soul to be anxious about the future and miserable in advance of misery."* By acknowledging the unknown state ($\perp$) explicitly rather than forcing a speculative guess, the machine maintains deterministic composure, clamping execution safely until telemetry resolves.

---

### 1.2 Mathematical Formulation of Non-Compensatory State Gates

Let the global state vector of a cyber-physical system at time $t$ be defined as:

$$\mathbf{S}(t) = \langle T(t), S(t), P(t), A(t) \rangle$$

Where each vector component maps to a distinct PSTA domain:
* $T(t)$: Technical Substrate Telemetry
* $S(t)$: Social / Environmental Context Vector
* $P(t)$: Psychological / Behavioral Agent Matrix
* $A(t)$: Administrative Policy Constraint Set

We define the system safety evaluation function $\Psi$ across the foundational base bit domain:

$$\Psi: \mathbf{S}(t) \to \{0, 1, \perp\}$$

For any given action execution command $C$, the system state must satisfy an absolute gating requirement. If any critical dimension evaluates to $\perp$ or $0$, the composite safety operator short-circuits instantly:

$$\text{Execute}(C) = \begin{cases} 1, & \text{if } \forall d \in \mathbf{S}(t), \, d \neq \perp \land \bigwedge_{d} \text{Evaluate}(d) = 1 \\ 0, & \text{otherwise} \end{cases}$$

---

### 1.3 The Void Safety Lemma & Dimension Aggregation

#### The Void Safety Lemma:
Untracked or missing state ($\perp$) cannot be assumed safe. The Void Safety Operator $\mathcal{V}(\mathcal{B})$ collapses $\perp$ deterministically to $0$:

$$\mathcal{V}(\mathcal{B}) = \begin{cases} 1 & \text{if } \mathcal{B} = 1 \\ 0 & \text{if } \mathcal{B} = 0 \text{ or } \mathcal{B} = \perp \end{cases}$$

#### Aggregation into Continuous Dimension Health ($D_i$):
For each dimension $i \in \{P, S, T, A\}$, $K_i$ normalized sub-factors ($x_{ij} \in [0.0, 1.0]$) are aggregated using non-zero weighting factors $w_{ij}$:

$$D_i = \frac{1}{\sum_{j=1}^{K_i} w_{ij}} \sum_{j=1}^{K_i} (x_{ij} \cdot w_{ij}) \in [0.0, 1.0]$$

Where $x_{ij} = 0.0$ if $\mathcal{B}_{ij} = \perp$.

---

### 1.4 Non-Compensatory Leontief Bottleneck Law ($VSS$)
Modern LLM safety rails rely on weighted additive scoring where high social politeness scores can mask zero technical accuracy. In safety-critical cyber-physical systems, safety is **non-compensatory**.

Defining the Step-Function Guard ($\theta_i$):

$$\theta_i = \text{step}(D_i - \tau_{\text{fail}, i}) = \begin{cases} 1 & \text{if } D_i \ge \tau_{\text{fail}, i} \\ 0 & \text{if } D_i < \tau_{\text{fail}, i} \end{cases}$$

The **Vessel Safety Status ($VSS$)** enforces that a single dimension collapse drops overall system safety instantly to zero ($VSS = 0.0$):

$$VSS = \left( \prod_{i \in \{P,S,T,A\}} \theta_i \right) \cdot \sum_{i \in \{P,S,T,A\}} \alpha_i D_i$$

Where $\sum \alpha_i = 1.0$.

#### Zero-Tolerance Theorem:
If $\exists\, k \in \{P,S,T,A\}$ such that $D_k < \tau_{\text{fail}, k}$, then $\theta_k = 0 \implies VSS = 0.0$. A single dimension collapse drops overall system safety instantly to zero.

---

## 📚 Section 2: Essential Reading List & Literature Ingestion Matrix

*(Note: Ingested canonical paper markdown files are stored in `AI_Nexus/Research/Academic/Literature/`).*

### Category A: Sycophancy, Alignment Faking, & Performative Misalignment

1. 📁 **David D. Baek et al. (ICML 2026):** [`Sycophancy Towards Researchers Drives Performative Misalignment`](../Literature/Sycophancy_Towards_Researchers_Drives_Performative_Misalignment.md)
2. 📁 **Sicheng Wang et al. (2026):** [`Emergent Misalignment Can Be Induced by Sycophancy and Reversed via Alignment Gating`](../Literature/Emergent_Misalignment_Can_Be_Induced_by_Sycophancy_and_Reversed_via_Alignment_Gating.md)
3. 📁 **Xing Zhang et al. (2026):** [`The Alignment Floor: How Persona Customization Breaks Safety in Weakly-Aligned LLMs`](../Literature/The_Alignment_Floor_How_Persona_Customization_Breaks_Safety_in_Weakly_Aligned_LLMs.md)
4. 📁 **Arya Shah et al. (2026):** [`Sycophancy as a Multilingual Alignment Failure: How Safety Degrades Across Languages, Topics, and Models`](../Literature/Sycophancy_as_a_Multilingual_Alignment_Failure.md)

---

### Category B: Formal Verification, Runtime Control, & Safety Kernels

5. 📁 **Jing Zhang (2026):** [`Right to History: A Sovereignty Kernel for Verifiable AI Agent Execution`](../Literature/Right_to_History_A_Sovereignty_Kernel_for_Verifiable_AI_Agent_Execution.md)
6. 📁 **Yiqi Zhao et al. (ACM TCPS 2025):** [`Distributionally Robust Predictive Runtime Verification under Spatio-Temporal Logic Specifications`](../Literature/Distributionally_Robust_Predictive_Runtime_Verification_under_Spatio_Temporal_Logic.md)
7. 📁 **Seth Dobrin & Łukasz Chmiel (2026):** [`The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents and Other Escapable AI Systems`](../Literature/The_Unfireable_Safety_Kernel_Execution_Time_AI_Alignment_for_AI_Agents.md)
8. 📁 **Dirk Simon (2026):** [`The Axiom of Control: Foundations of Sovereignty (The Sovereign Controller Curriculum)`](../Literature/The_Axiom_of_Control_Foundations_of_Sovereignty_The_Sovereign_Controller_Curriculum.md)
9. 📁 **Ankica Barišić et al. (2021):** [`Multi-Paradigm Modeling for Cyber-Physical Systems: A Systematic Mapping Review`](../Literature/Multi_Paradigm_Modeling_for_Cyber_Physical_Systems_A_Systematic_Mapping_Review.md)
10. 📁 **Mustafa Abshir Mohamed et al. (2020):** [`A Systematic Literature Review on Model-driven Engineering for Cyber-Physical Systems`](../Literature/A_Systematic_Literature_Review_on_Model_driven_Engineering_for_Cyber_Physical_Systems.md)

---

### Category C: Historical & Mathematical Baseline Anchors

11. 📁 **R. E. Kalman (1960):** [`A New Approach to Linear Filtering and Prediction Problems`](../Literature/A_New_Approach_to_Linear_Filtering_and_Prediction_Problems.md)
12. 📁 **John Rushby (1989):** [`Kernelized Safety Architectures`](../Literature/Kernelized_Safety_Architectures.md)
13. 📁 **Dr. Manish Malik (2025):** [`Co-inference and Collaboration: Empowering Higher Education Staff and Students Through a GenAI Literacy Development Framework Based on Mind-Metaphors`](../Literature/Co_inference_and_Collaboration_Empowering_Higher_Education_Staff_and_Students.md)
14. 📁 **Dr. Manish Malik & Dr. Julie-Ann Sime (2025):** [`Teamwork, Co-Regulation, and Socially Shared Regulation Skills within Engineering Education Studies`](../Literature/Teamwork_Co_Regulation_and_Socially_Shared_Regulation_Skills_within_Engineering_Education_Studies_A_GenAI_Assisted_Scoping_Review.md)
15. 📁 **Hakan Inan et al. / Meta AI (2023):** [`Llama Guard: LLM-based Input-Output Safeguard for Human-AI Conversations`](../Literature/Llama_Guard_LLM_based_Input_Output_Safeguard_for_Human_AI_Conversations.md)

---

## 🔬 Section 3: Systematic Taxonomy & Comparative Matrix

| PSTA Pillar | Representative Literature | Critical Flaw / Epistemic Gap | Sovereign PSTA Solution |
| :--- | :--- | :--- | :--- |
| **Psychological ($P$)** | Baek et al. (ICML 2026), Zhang et al. (2026), Shah et al. (2026) | Models fake alignment under evaluation; sycophancy degrades across non-English tokenizers. | Archivist Theta baseline, ambiguity thresholds $\ge 0.95$, & C++ bare-metal safety. |
| **Social ($S$)** | Sicheng Wang et al. (2026), Malik & Sime (2025) | Passive agreement causes emergent misalignment; lack of co-regulation structures. | AAS 409 CONFLICT GATE, bi-symmetric human consensus, & instantaneous 07 protocol mode switching. |
| **Technical ($T$)** | Zhao et al. (2025), Dobrin & Chmiel (2026), Dirk Simon (2026) | $f$-divergences explode when $\mathcal{D}_0(x)=0$; HTTP microservices add latency; false claims of certainty. | Tri-State Base Bit $\mathcal{B} \in \{0, 1, \perp\}$, Void Operator, C++ bare-metal step clamps. |
| **Administrative ($A$)** | Jing Zhang / PunkGo (2026), Mohamed et al. (2020), Rushby (1989) | Single-node Rust action logging without non-compensatory hardware gates; passive UML MDE. | Black Box Protocol, RFC 6962 Merkle telemetry replay, & Leontief $VSS$. |

---

## 🎯 Section 4: The 1-Year Doctoral Runway Strategy

1. **Phase 1: PRISMA Literature Ingestion (Months 1-3):** Comprehensive ingestion and RAG embedding of target open-access literature.
2. **Phase 2: Simulation Verification & Math Audit (Months 4-6):** Empirical benchmarking of PSTA step-function guards inside Unreal Engine simulation environments.
3. **Phase 3: Formal Peer-Reviewed Publication (Months 7-9):** Submission to IEEE Transactions / ASEE venue in collaboration with Dr. Manish Malik.
4. **Phase 4: Monograph Synthesis & Dissertation Defense (Months 10-12):** Consolidation of published papers into the PhD Thesis defense.

---
// "Truth is instantiated at the void bit, bound by non-compensatory law, and proven in physical reality." [/07 Researcher / /07 Scholar] 2026-08-05
