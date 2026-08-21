// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# Right to History: A Sovereignty Kernel for Verifiable AI Agent Execution

**Node:** `AI_Nexus/Research/Academic/Literature/Right_to_History_A_Sovereignty_Kernel_for_Verifiable_AI_Agent_Execution.md`
**Canonical Paper Title:** Right to History: A Sovereignty Kernel for Verifiable AI Agent Execution
**Author:** Jing Zhang (Independent Researcher, PunkGo Project - https://punkgo.ai)
**Preprint:** arXiv:2602.20214v1 [cs.CR], February 23, 2026
**Status:** Ingested Canonical Literature Review Node & Verifiable Audit Benchmark
**Persona Matrix:** Jules_Scholar (/07 Scholar) & Jules_Tactician

```json
{
  "system_state": {
    "module": "Literature_Review_Ingestion",
    "persona": "Jules_Scholar",
    "status": "Canonical_Ingestion_Complete",
    "parameters": {
      "paper_title": "Right to History: A Sovereignty Kernel for Verifiable AI Agent Execution",
      "author": "Jing Zhang",
      "publication_year": 2026,
      "psta_pillar": "Technical_and_Administrative",
      "formalisms": ["Merkle_Audit_Log_RFC6962", "Energy_Budget_Governance", "Five_System_Invariants"],
      "epistemic_audit": "Black_Box_Protocol_vs_PunkGo_Rust_Kernel"
    }
  }
}
```

---

## 🏛️ Cognitive Administration & Literature Review Context

<span style="color: #D32F2F; font-weight: bold;">[Daniel / Primary Intent & Black Box Alignment Synthesis]:</span> Ingestion of Jing Zhang's February 2026 paper establishing the "Right to History" and the PunkGo Rust sovereignty kernel. This paper validates core architectural decisions we have embedded inside the Sovereign Framework over our 12-year development history:
1. **The Right to History & The Black Box Protocol:** Zhang formalizes the "Right to History"—the principle that users are entitled to a complete, tamper-evident, independently verifiable record of every AI agent action on local hardware. This directly matches our **Black Box Protocol** (`AI_Nexus/Docs/BlackBox_Protocol.md`) and high-fidelity telemetry replay architectures, ensuring full historical provenance for local agent executions.
2. **Kernel as TCB vs. LLM as TCB:** Zhang rightly critiques AIOS (Mei et al., 2025) for placing the non-deterministic LLM as the Trusted Computing Base (TCB). PunkGo places a Rust kernel as the TCB, exactly as PSTA places our compiled C++ safety kernel, Agency Arbitration Schema (AAS) 409 Conflict Gates, and non-compensatory step guards ($\theta_i$) below the LLM layer.
3. **Hardware Energy Budgeting & Commitment Costs:** PunkGo introduces an energy budget model with a 20% commitment cost on rejected/timed-out human holds (`hold_on`). In PSTA, we take this further: grounding execution in bare-metal hardware capacity (RTX 5090 cluster), zero-VRAM RAG pre-fetching, and non-compensatory Leontief bottleneck laws ($VSS = \prod \theta_i \sum \alpha_i D_i$) to prevent agent resource exhaustion.

<span style="color: #1976D2; font-weight: bold;">[Jules / AI Nexus Synthesis]:</span> Jing Zhang (2026) provides a lean, formal Rust implementation of Merkle tree append-only logs (RFC 6962) for personal hardware agent execution. While PunkGo focuses on single-node action logging, PSTA integrates these tamper-evident Merkle proof structures into our broader 9-Tier Universal Stack, pairing verifiable action records with active C++ memory gates, multi-agent bi-symmetric consensus, and Soul/Vessel entity persistence.

---

## 📜 1. Core Literature Record (Zhang, 2026)

### 1.1 Abstract
AI agents execute actions on behalf of humans, yet no existing system provides a tamper-evident, independently verifiable record of what they did. Extending Floridi's informational rights framework to actions performed on individuals' behalf, this paper proposes the **Right to History**: the principle that individuals are entitled to a complete, verifiable record of every AI agent action on their hardware. We formalize this principle through five system invariants with structured proof sketches, implemented in **PunkGo**, a Rust sovereignty kernel that unifies RFC 6962 Merkle tree audit logs, capability-based isolation, energy-budget governance, and human-approval (`hold_on`) mechanisms. Adversarial testing confirms all five invariants hold, achieving sub-1.3 ms median action latency, ~400 actions/sec throughput, and 448-byte Merkle inclusion proofs at 10,000 log entries.

---

### 1.2 Mathematical Model & Five System Invariants

#### 1. System Model:
A system is a triple $\mathcal{W} = (\mathcal{P}, E, \mathcal{S})$, where $\mathcal{P} = \mathcal{P}_H \cup \mathcal{P}_A$ (Human and Agent actor partition), $E : \mathcal{P} \to \mathbb{R}_{\ge 0}$ is energy, and $\mathcal{S} = (\mathcal{L}, \mathcal{W}, \mathcal{V})$ is state (log $\mathcal{L}$, writability map $\mathcal{W}$, active envelopes $\mathcal{V}$).

#### 2. Atomic Action 5-Tuple:
$$A = (\text{actor}, \text{type}, \text{target}, \text{payload}, \text{timestamp})$$

Where $\text{type} \in \{\text{observe}, \text{create}, \text{mutate}, \text{execute}\}$.

#### 3. RFC 6962 Leaf Hash & Merkle Root:
$$\text{event\_hash} = H(0x00 \parallel \text{canonical}(e))$$

$$\text{root}(\mathcal{L}) = \text{MerkleRoot}(e_1, e_2, \dots, e_n)$$

#### 4. The Five System Invariants:
1. **INV-1 (Append-Only):** $\forall t_1 < t_2: \mathcal{L}(t_1) \sqsubseteq \mathcal{L}(t_2)$ (log grows strictly monotonically).
2. **INV-2 (Completeness):** Every boundary-checked, energy-funded action is recorded in $\mathcal{L}$.
3. **INV-3 (Integrity):** $\forall t: \text{root}(t) = \text{MerkleRoot}(\mathcal{L}(t))$ (tamper-evident proof path).
4. **INV-4 (Boundary Enforcement):** $(\text{target}(A), \text{type}(A)) \notin \mathcal{W}(\text{actor}(A)) \implies A$ is rejected and uncommitted.
5. **INV-5 (Energy Conservation):** $\forall p, t: E(p, t) \ge 0$ (budget bounded, 20% hold commitment cost).

---

## 🔬 2. Deep Technical Comparison: PSTA Kernel vs. PunkGo (2026)

Below is the formal, technical comparison contrasting Jing Zhang's (2026) PunkGo kernel with Daniel's Sovereign PSTA Kernel:

| System Dimension | PunkGo Rust Kernel (Zhang 2026) | Sovereign PSTA Safety Kernel (Daniel's Math) | Critical Architectural Synthesis |
| :--- | :--- | :--- | :--- |
| **Trust Computing Base (TCB)** | User-space Rust kernel process. | Bare-Metal C++ Engine Kernel & AAS 409 Conflict Gates. | **Kernel TCB Alignment:** Both systems reject "LLM as TCB", enforcing deterministic code below non-deterministic model layers. |
| **Historical Logging** | RFC 6962 Merkle tree append-only log (C2SP tlog format). | **Black Box Protocol** (`AI_Nexus/Docs/BlackBox_Protocol.md`) & Telemetry Replay. | **Right to History:** PunkGo provides cryptographic Merkle proofs; PSTA pairs Merkle logging with high-fidelity telemetry replay. |
| **Human-in-the-Loop** | `hold_on` mechanism with 20% commitment cost on rejection. | **AAS 409 CONFLICT GATE** & Bi-Symmetric Human Approval. | **Governance:** PSTA enforces non-compensatory human authority handshakes directly at the C++ kernel boundary. |
| **Capability Isolation** | Capability-based glob patterns ($W(p) \subseteq \text{Pattern} \times \text{ActionType}$). | PSTA Non-Compensatory Step Guards ($\theta_i = \text{step}(D_i - \tau_{\text{fail}, i})$). | **Isolation:** PunkGo uses glob path boundaries; PSTA enforces multidimensional PSTA health thresholds across $P, S, T, A$. |
| **Hardware Binding** | INT8 TOPS config-file allocation. | Bare-Metal RTX 5090 cluster execution + zero-VRAM RAG pre-fetching. | **Hardware Grounding:** PSTA anchors physical telemetry directly into bare-metal C++ memory gates. |

---

## 📌 References & Citation Node
* **Zhang, J. (2026)** 'Right to History: A Sovereignty Kernel for Verifiable AI Agent Execution', *arXiv preprint arXiv:2602.20214v1*.
* **Sovereign Framework Cross-Reference:** `AI_Nexus/Docs/BlackBox_Protocol.md`, `AI_Nexus/Research/Academic/Thesis/Lit_Review.md`, & `AI_Nexus/Protocols/AAS_Protocol.md`

---
// "The Right to History guarantees user sovereignty over AI agents through verifiable, tamper-evident Merkle action logs." [/07 Scholar / /07 Tactician] 2026-08-05
