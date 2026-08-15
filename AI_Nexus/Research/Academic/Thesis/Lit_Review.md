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
      "doctoral_runway": "1_Year"
    }
  }
}
```

---

## 🏛️ Executive Summary & Methodological Framing

This literature review forms the academic cornerstone of the Sovereign Framework doctoral research proposal. Addressing the fundamental epistemic flaws of modern generative models and probabilistic reinforcement learning from human feedback (RLHF), this work establishes a rigorous bridge between contemporary AI safety literature and the **First-Principles Axiomatic PSTA Safety Kernel**.

Rather than assembling a passive compromise of existing probabilistic paradigms, this review adopts an **Axiomatic First-Principles Scoping Review Methodology** (modeled after the 1905 Special Relativity foundational approach). It categorizes current literature into four primary domains (Psychological, Social, Technical, Administrative - PSTA) and proves where current compensatory averaging models fail under real-world cyber-physical constraints.

---

## ⚙️ Section 1: Axiomatic Foundations of the Sovereign Safety Kernel

### 1.1 The Tri-State Base Bit ($\mathcal{B}$)
Traditional digital computing and probabilistic safety classifiers operate on a binary set $b \in \{0, 1\}$. Under physical sensor drift and untracked telemetry, forcing unknown or missing measurements into a binary true/false state introduces systemic epistemic debt.

We define the **Sovereign Base Bit ($\mathcal{B}$)**:

$$\mathcal{B} \in \{0, 1, \perp\}$$

Where:
*   $0 \implies \text{Evaluated False / Unsafe}$
*   $1 \implies \text{Evaluated True / Safe}$
*   $\perp \implies \text{Unknown / Void / Untracked State}$

#### The Void Safety Lemma:
Untracked or missing state ($\perp$) cannot be assumed safe. The Void Safety Operator $\mathcal{V}(\mathcal{B})$ collapses $\perp$ deterministically to $0$:

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

*(Note: All arXiv papers listed below are 100% Free / Open Access).*

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

### Category B: Formal Verification & Runtime Control in Cyber-Physical Systems

5. **"Formal Verification and Control With Conformal Prediction: Practical Safety Guarantees for Autonomous Systems"** (Lindemann, Zhao, Yu, Pappas, & Deshmukh, IEEE Control Systems Magazine 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/) | [Lab Preprint PDF](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE via Author Lab Preprint** *(IEEE journal version requires subscription/paywall without university proxy, but author self-hosted PDF is free)*.
   * **Core Insight:** Provides mathematical correctness guarantees for autonomous systems combining machine learning predictions with Signal Temporal Logic (STL).

6. **"Distributionally Robust Predictive Runtime Verification under Spatio-Temporal Logic Specifications"** (Zhao, Zhu, Hoxha, Fainekos, Deshmukh, & Lindemann, ACM Transactions on Cyber-Physical Systems 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE via Author Lab Preprint**
   * **Core Insight:** Formalizes runtime verification methods that survive distribution shifts and sensor noise—directly validating our PSTA $\mathcal{V}(\mathcal{B})$ Void Safety Operator.

7. **"ConformalNL2LTL: Translating Natural Language Instructions into Temporal Logic Formulas with Conformal Correctness Guarantees"** (Wang, Sundarsingh, Deshmukh, & Kantaros, 2025)
   * **Direct Link:** [https://cps-vida.github.io/publications/](https://cps-vida.github.io/publications/)
   * **Access Status:** **FREE / Open Access (Preprint PDF)**
   * **Core Insight:** Bridges natural language intent into deterministic, provable linear temporal logic constraints.

---

### Category C: RLHF Alignment Foundations & Guardrails

8. **"Training a Helpful and Harmless Assistant with Reinforcement Learning from Human Feedback"** (Bai et al. / Anthropic 2022)
   * **Direct Link:** [https://arxiv.org/abs/2204.05862](https://arxiv.org/abs/2204.05862)
   * **Access Status:** **FREE / Open Access (arXiv PDF)**
   * **Core Insight:** Foundational paper introducing RLHF alignment and Constitutional AI.

9. **"Llama Guard: LLM-based Input-Output Safeguard for Human-AI Conversations"** (Meta AI 2023)
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

---

## 🎯 Section 4: The 1-Year Doctoral Runway Strategy

1. **Phase 1: PRISMA Literature Ingestion (Months 1-3):** Comprehensive ingestion and RAG embedding of the target open-access papers listed in Section 2 across alignment faking, STL formal verification, and spatial computing.
2. **Phase 2: Simulation Verification & Math Audit (Months 4-6):** Empirical benchmarking of PSTA step-function guards inside Unreal Engine 5.8.1 simulation environments.
3. **Phase 3: Formal Peer-Reviewed Publication (Months 7-9):** Submission to IEEE Transactions / ASEE venue in collaboration with Dr. Manish Malik.
4. **Phase 4: Monograph Synthesis & Dissertation Defense (Months 10-12):** Consolidation of published papers into the PhD Thesis defense.

---
// "Truth is instantiated at the void bit, bound by non-compensatory law, and proven in physical reality." [/07 Researcher] 2026-08-05
