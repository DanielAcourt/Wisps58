# Architectural Specification: Local RAG Benchmarking, SSoT Authority Precedence & Verification Engine

**Document Version:** 1.0.0
**Author:** Jules (Agentic Technical Lead) & Daniel Acourt (Lead)
**Target System:** Iron Knight / Iron Officer (RTX 5090 Local AI Engine)
**Date:** 2026-10-30

---

## 1. Executive Overview

This specification establishes the official response and technical implementation plan addressing the 9-point architectural review of Iron Knight's local context ingestion and execution pipeline.

While local open-weights models (such as **Qwen 2.5 Coder 32B Instruct**) provide high-speed local inference on the RTX 5090, giving a local model access to documentation does not automatically guarantee equivalent reasoning or accuracy compared to cloud models.

To ensure local operations meet the highest standards of reliability, this document formalizes:
1. **Explicit SSoT Authority Precedence Rules** overriding raw TF-IDF similarity scores.
2. **Context Provenance & Structured Knowledge Injection**.
3. **Adaptive Context Window Budgeting (32k Tokens)**.
4. **RTX 5090 Local Model Hardware Benchmarking Methodology**.
5. **The 6-Stage Independent Verification Pipeline**.

---

## 2. Explicit SSoT Authority Precedence Hierarchy

To prevent historical sprint review logs or outdated research notes from overriding active protocol specifications, `rag.py` and `bridge.py` enforce a **5-Tier Authority Hierarchy**:

```
 ┌────────────────────────────────────────────────────────────────────────┐
 │ TIER 1: CANONICAL PROTOCOLS & GOVERNANCE                               │
 │ Node: AI_Nexus/Protocols/*.md (AAS_Protocol.md, AGENTS.md)            │
 │ Authority: Absolute. Governs permitted operations and safety rules.   │
 └────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │ TIER 2: TECHNICAL SPECIFICATIONS & GAME DESIGN DOCUMENTS               │
 │ Node: AI_Nexus/Docs/*.md, AI_Nexus/Research/*.md                       │
 │ Authority: High. Establishes intended architecture and features.       │
 └────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │ TIER 3: CURRENT SOURCE CODE & ENGINE HEADER IMPLEMENTATIONS            │
 │ Node: WispsCPPVR/Source/*.cpp, WispsCPPVR/Source/*.h                   │
 │ Authority: Ground Truth. Establishes currently implemented reality.    │
 └────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │ TIER 4: ACTIVE SPRINT BACKLOG & WORK PRIORITIES                        │
 │ Node: AI_Nexus/DevOps/CURRENT_SPRINT.md                                │
 │ Authority: Active. Establishes current task priorities and scope.      │
 └────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │ TIER 5: HISTORICAL TIMELINE & SPRINT REVIEW ARCHIVES                  │
 │ Node: AI_Nexus/Timeline/SprintReviews/*.md                             │
 │ Authority: Contextual Only. CANNOT override Tiers 1-4.                 │
 └────────────────────────────────────────────────────────────────────────┘
```

### Precedence Conflict Resolution Rule:
If a Tier 5 document (e.g. `SR_20260928.md`) has a 0.90 similarity score, but conflicts with a Tier 1 Protocol or Tier 3 Source File with a 0.50 similarity score, **Tier 1 and Tier 3 strictly supersede Tier 5**.

---

## 3. Context Provenance & Structured Knowledge Injection

Rather than silently injecting raw markdown into system prompts, `bridge.py` wraps all RAG-retrieved knowledge in structured, auditable provenance blocks:

```markdown
[RETRIEVED PROJECT KNOWLEDGE]
Source: AI_Nexus/Protocols/AAS_Protocol.md
SSoT Level: Level 1 (Protocols)
Authority Tier: Tier 1 (Canonical Protocol)
Relevance Score: 0.88
Section: ## 3. Viability Safety Score Calculation

Content:
[Chunk text here]
--------------------------------------------------
[END RETRIEVED PROJECT KNOWLEDGE]
```

This ensures the LLM recognizes retrieved text as reference material rather than system prompt overrides.

---

## 4. Context Window Budgeting Strategy (32k Tokens)

To prevent context saturation or "lost-in-the-middle" attention degradation during complex C++ coding tasks, `iron_coder.py` enforces an adaptive **32k Token Context Budget**:

| Context Category | Token Allocation | Target Percentage | Purpose |
| :--- | :--- | :--- | :--- |
| **System & AAS Governance** | ~3,200 Tokens | 10% | Core system prompts, AAS safety rules, persona |
| **Task & Conversation History** | ~4,800 Tokens | 15% | Multi-turn user prompts, agent thoughts |
| **Retrieved Documentation (RAG)**| ~6,400 Tokens | 20% | Top matching Tier 1-3 SSoT knowledge chunks |
| **Active Source Code** | ~12,800 Tokens | 40% | C++ header files, implementation code |
| **Tool Outputs & Scratchpad** | ~4,800 Tokens | 15% | Terminal command outputs, compiler errors |
| **Total VRAM Reserve** | **32,000 Tokens** | **100%** | **Optimal VRAM KV Cache Allocation** |

---

## 5. RTX 5090 Local Model Hardware Benchmarking

### Model Comparison on 32GB VRAM Rig

| Model Candidate | Quantization | VRAM Footprint | KV Cache VRAM (32k) | Total VRAM Required | Performance on RTX 5090 | Recommendation |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Qwen 2.5 Coder 32B Instruct** | FP8 / Q8_0 | ~20 GB | ~8 GB | **~28 GB** | **110+ tok/sec** (Native GPU) | **PRIMARY RECOMMENDED** |
| **DeepSeek Coder 33B** | Q8_0 | ~21 GB | ~8 GB | **~29 GB** | **100+ tok/sec** (Native GPU) | **SECONDARY ALTERNATIVE** |
| **Llama 3.1 70B Instruct** | Q4_K_M | ~38 GB | ~8 GB | **~46 GB** | **12-18 tok/sec** (Requires CPU Offload) | **RESERVED FOR ARCHITECTURE** |

---

## 6. The 6-Stage Independent Verification Pipeline

An agent cannot claim a programming task is completed simply because a tool executed. `iron_coder.py` enforces a **6-Stage Verification Pipeline**:

```
 [1. SOURCE MODIFIED] ──> [2. SYNTAX COMPILED] ──> [3. UBT BUILD SUCCESS]
                                                            │
 [6. EVIDENCE LOCKED] <── [5. GIT DIFF VERIFIED] <── [4. UNIT TESTS PASSED]
```

### Verification States:
1. `SOURCE_MODIFIED`: File changed on disk (`patch_file` / `write_file`).
2. `SYNTAX_COMPILED`: Python syntax verified via `py_compile` or C++ header parsed.
3. `UBT_BUILD_SUCCESS`: UnrealBuildTool or C++ compiler returns exit code 0.
4. `UNIT_TESTS_PASSED`: Relevant unit tests (`pytest` / `unittest`) pass 100%.
5. `GIT_DIFF_VERIFIED`: Git diff inspected to confirm no unintended side-effects.
6. `EVIDENCE_LOCKED`: Full execution trace logged to `AI_Nexus/Memories/ExecutionLogs/`.

*If any step fails, the agent status remains `NOT_VERIFIED` and triggers a bounded repair loop.*
