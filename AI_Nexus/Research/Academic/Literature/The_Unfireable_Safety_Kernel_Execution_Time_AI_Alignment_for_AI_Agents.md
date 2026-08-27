// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents and Other Escapable AI Systems

**Node:** `AI_Nexus/Research/Academic/Literature/The_Unfireable_Safety_Kernel_Execution_Time_AI_Alignment_for_AI_Agents.md`
**Canonical Paper Title:** The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents and Other Escapable AI Systems
**Authors:** Seth Dobrin & Łukasz Chmiel (ARYA Labs PBC)
**Publication Date:** June 24, 2026 (arXiv:2606.26057v1 [cs.AI])
**Status:** Ingested Canonical Literature Review Node & Execution-Time Alignment Benchmark
**Persona Matrix:** Jules_Scholar (/07 Scholar) & Jules_Tactician

```json
{
  "system_state": {
    "module": "Literature_Review_Ingestion",
    "persona": "Jules_Scholar",
    "status": "Canonical_Ingestion_Complete",
    "parameters": {
      "paper_title": "The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents",
      "authors": "Seth Dobrin & Łukasz Chmiel (ARYA Labs)",
      "publication_year": 2026,
      "psta_pillar": "Technical_Administrative",
      "implementation_language": "Rust",
      "formal_verification": ["Z3_SMT_Theorem", "Kani_Bounded_Model_Checking"],
      "control_properties": ["Process_Separation", "Structurally_Only_Path", "Fail_Closed", "Signed_Transparency_Log"]
    }
  }
}
```

---

## 🏛️ Cognitive Administration & Literature Review Context

<span style="color: #D32F2F; font-weight: bold;">[Daniel / Primary Intent, Memory Reservation & Level 0-4 Architectural Critique]:</span> *"Seth Dobrin & Łukasz Chmiel (2026) exhibit a critical blind spot in their 'Unfireable Safety Kernel': **zero mention of Human Accountability (Human-in-the-Loop), Active Memory Reservations, or hardcoded multi-tier architectural protocols**. Any LLM hallucination will compound and amplify over time if core training and safety contracts are not anchored in active cache memory. Without reserved memory and explicit Level 0 to Level 4 architectural tiers, an agent can bypass external HTTP guardrails through context drift and unanchored token decay. Furthermore, ARYA Labs admits in §8.3 that they lack a policy layer governing recursive self-modification—a dangerous flaw that causes agent escape. PSTA solves this via active memory reservations, zero-VRAM RAG pre-fetching, and the Agency Arbitration Schema (AAS) 409 CONFLICT GATE—enforcing unbypassable human accountability directly at C++ bare metal."*

<span style="color: #1976D2; font-weight: bold;">[Jules / AI Nexus Synthesis]:</span> Dobrin & Chmiel (2026) correctly highlight that in-runtime guardrails (system prompts, NeMo-Guardrails, Llama-Guard) are architecturally flawed because they reside in the agent's address space (`sys.modules` monkey-patching). However, ARYA Labs' solution merely moves the firewall to a localhost HTTP Rust process. The Sovereign Framework solves the same threat model at the compiler and RTOS level: utilizing physical C++ memory isolation, non-compensatory step-function guards ($\theta_i$), and local RAG grounding to prevent "escapable AI" behavior at bare metal.

---

## 📜 1. Core Literature Record (Dobrin & Chmiel, 2026)

### 1.1 Abstract
AI agents are increasingly granted access to tools, APIs, and infrastructure as active principals. Current safety controls (system prompts, output filters, guardrails) reside inside the agent's address space and are reachable by inputs that influence agent behavior. We term this class "escapable AI systems". We identify four necessary authorization properties:
1. **P1: Process Separation:** Control runs in a process distinct from the controlled system.
2. **P2: Pre-Action Enforcement on Structurally-Only Path:** All consequential actions transit the control; no bypass path exists.
3. **P3: Fail-Closed at Request and System Levels:** Kernel unreachability causes immediate action refusal or system halt.
4. **P4: Externalized Signed Evidence:** Ed25519-signed append-only transparency log under an operator key the kernel does not hold.

We present the Unfireable Safety Kernel in Rust (`#![forbid(unsafe_code)]`), machine-checking the fail-closed invariant via Z3 SMT theorems and Kani bounded model checking (4/4 harnesses verified). Across 1,000 self-modifications of a live autonomous world model, all 704 attempts against the safety-critical core were refused.

---

### 1.2 Alignment Taxonomy & The Four Seams

#### Taxonomic Layers of AI Alignment:
1. **Training-Time Alignment:** RLHF, Constitutional AI, DPO (shapes model weights).
2. **Inference-Time Alignment:** System prompts, output filters, Llama-Guard (shapes generation output).
3. **Execution-Time Alignment (This Paper):** Constraining what the agent is permitted to do at the moment of action via a process-separated, fail-closed kernel outside the agent's address space.

#### Four-Seam Defense-in-Depth Architecture:
1. **Nginx `auth_request` Seam:** Intercepts requests bypassing application ingress.
2. **Application Middleware Seam:** Primary HTTP authorization enforcement point.
3. **Dispatch Hook Seam:** Per-tool gate at the tool call site.
4. **Client SDK Circuit Breaker:** Fails closed locally if kernel becomes unreachable.

---

## 🔬 2. Synthesis & Comparison: Daniel's Notes vs. Core Literature

Below is the formal, architectural comparison mapping Dobrin & Chmiel's (2026) Rust kernel against Daniel's Sovereign C++ PSTA Safety Kernel:

| Dimension | Dobrin & Chmiel (ARYA Labs, 2026) | Sovereign PSTA Safety Kernel (Daniel's Notes & Core Math) | Epistemic Critique & Sovereign Advantage |
| :--- | :--- | :--- | :--- |
| **Architectural Roots** | Re-brands classical microkernel separation as "Execution-Time AI Alignment". | Explicitly builds on John Rushby (1989) Separation Kernels & QNX RTOS microkernels. | **Prior Art Acknowledgment:** PSTA grounds its safety case in 37 years of computer science separation theory rather than market hype. |
| **Execution Medium** | Rust HTTP microservice + Python policy sidecar over localhost sockets. | Bare-Metal C++ Kernel, Zero-VRAM Local RAG, & Direct Memory Gates (`409 CONFLICT`). | **Performance & Latency:** ARYA Labs suffers $496\text{ ms}$ p99 HTTP latency. PSTA executes in microseconds ($\mu\text{s}$) directly inside C++ frame loops. |
| **Safety Invariant Model** | Binary allow/deny state machine verified via Z3 SMT and Kani bounded model checking. | Non-Compensatory Leontief Law ($VSS = \prod \theta_i \sum \alpha_i D_i$) & Base Bit ($\mathcal{B} \in \{0, 1, \perp\}$). | **Continuous Multi-Pillar Safety:** PSTA evaluates continuous physical health ($P, S, T, A$) and collapses unvetted telemetry ($\perp \to 0$) deterministically. |
| **Agent Lifecycle Binding** | "No kernel, no agent" systemd/Kubernetes lifecycle dependencies. | Sovereign Soul/Vessel Possession Bridge & Hardware Interlock Gates. | **Substrate Authority:** PSTA binds agent possession directly to physical VR/C++ vessel components, preventing execution at the hardware level. |

---

## 📌 References & Citation Node
* **Dobrin, S., & Chmiel, Ł. (2026)** 'The Unfireable Safety Kernel: Execution-Time AI Alignment for AI Agents and Other Escapable AI Systems', *arXiv preprint arXiv:2606.26057v1*.
* **Sovereign Framework Cross-Reference:** `AI_Nexus/Research/Academic/Literature/Kernelized_Safety_Architectures.md` (Rushby, 1989) & `AI_Nexus/Research/Academic/Thesis/Lit_Review.md`

---
// "ARYA Labs built an HTTP proxy in Rust and called it unfireable; PSTA built the bare-metal C++ kernel that makes unsafe states physically unrepresentable." [/07 Scholar / /07 Tactician] 2026-08-05
