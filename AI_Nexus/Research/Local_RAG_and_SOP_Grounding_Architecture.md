# Architectural Specification: AI_Nexus, SOP & Protocol Grounding for Local Offline LLMs

**Document Version:** 1.0.0
**Author:** Jules (Agentic Technical Lead)
**Target System:** Iron Knight / Iron Officer (RTX 5090 Local AI Engine)
**Date:** 2026-10-30

---

## 1. The Operational Challenge

Cloud-scale models (e.g. Claude 3.5 Sonnet / Jules) rely on vast parameter scales and long context windows to follow complex project SOPs and update C++ / Unreal Engine code.

When transitioning to a **100% local, offline setup on the RTX 5090**, the core question is:
*How do we ensure a local open-weights model (Qwen 2.5 Coder 32B / Llama 3.1 70B) maintains exact adherence to `AI_Nexus` protocols, SOPs, and C++ codebases without hallucinating or missing context?*

---

## 2. The 4-Pillar Local Grounding Architecture

```
 +-------------------------------------------------------------------------+
 |                            USER / TASK PROMPT                           |
 +-------------------------------------------------------------------------+
                                      |
                                      v
 +-------------------------------------------------------------------------+
 | PILLAR 1: Level-Aware CPU RAG Engine (`rag.py`)                        |
 | Scans AI_Nexus, identifies SSoT Levels (0-4), retrieves exact SOP chunks|
 +-------------------------------------------------------------------------+
                                      |
                                      v
 +-------------------------------------------------------------------------+
 | PILLAR 2: Context Injection & 32k Token KV Cache                        |
 | Injects SSoT ground truth + active file anchors into RTX 5090 VRAM      |
 +-------------------------------------------------------------------------+
                                      |
                                      v
 +-------------------------------------------------------------------------+
 | PILLAR 3: Agentic Execution Loop Driver (`iron_coder.py`)              |
 | Multi-turn loop: Plan -> RAG Retrieve -> Tool Call -> Inspect Output   |
 +-------------------------------------------------------------------------+
                                      |
                                      v
 +-------------------------------------------------------------------------+
 | PILLAR 4: Scribe Tool Validation & Pre-Commit Verification             |
 | Pre-flight sanity checks, VSS safety gating, atomic .bak, unit tests    |
 +-------------------------------------------------------------------------+
```

---

## 3. Detailed Pillar Analysis

### Pillar 1: Level-Aware CPU RAG Engine (`rag.py`)
- **Zero VRAM Overhead**: Runs entirely in system CPU RAM, preserving 100% of the RTX 5090's 32GB VRAM for model weights and KV cache.
- **SSoT Level Mapping**:
  - **Level 0**: Timeline & Sprint Reviews (`AI_Nexus/Timeline/`)
  - **Level 1**: SOPs, Rules & Protocols (`AI_Nexus/Protocols/END_OF_SPRINT_PROTOCOL.md`, `AAS_Protocol.md`)
  - **Level 2**: Identity, Governance & Precedence
  - **Level 3**: Research, Math & Design Specifications
  - **Level 4**: DevOps, Backlogs & Active Code (`CURRENT_SPRINT.md`, C++ / Python headers)
- **Automatic Relevance Retrieval**: Before the LLM processes a turn, `rag.py` performs a level-boosted TF-IDF search on `AI_Nexus` and silently injects matching SOP sections into the prompt under `[GROUND TRUTH SYSTEM GROUNDING]`.

### Pillar 2: 32k Token Context Window in VRAM (RTX 5090)
- The RTX 5090's 32 GB VRAM allocation:
  - **Qwen 2.5 Coder 32B FP8**: ~20 GB VRAM
  - **32k Context KV Cache**: ~8 GB VRAM
  - **System Overhead / RAG**: ~4 GB
- A 32,000 token context window accommodates the full task prompt, system instructions, RAG-grounded SOPs, and C++ header definitions simultaneously without truncation.

### Pillar 3: Multi-Turn Execution Loop Driver (`iron_coder.py`)
- Cloud agents succeed because they don't stop at single responses; they execute an interactive loop:
  1. **Plan**: Formulate numbered execution steps.
  2. **Retrieve**: Read target SOPs and C++ source files via RAG and tools.
  3. **Act**: Execute file tools (`write_file`, `patch_file`, `run_command`).
  4. **Verify**: Check compiler warnings, build output, or test execution logs.
  5. **Reflect & Correct**: If an error occurs, inspect stdout/stderr and apply a targeted fix.

### Pillar 4: Deterministic Bridge Guardrails & Scribe Protection
- Local models need unambiguous tool outputs:
  - **Pre-Flight Validation**: Disallow `search == replace` patch calls.
  - **Path Normalization**: Auto-correct Windows drive letters (`C:Users` $\rightarrow$ `C:/Users`).
  - **Explicit Tool Output Messages**: Return `"NO_CHANGES_MADE: search equals replace"` instead of vague `"No data"`.
  - **Atomic `.bak` Creation**: Every file write creates an automatic backup before mutation.

---

## 4. How Unreal Engine Integration Translates to Local Operations

| Operation Type | Cloud Workflow (Sonnet) | Local Offline Workflow (Iron Knight) |
| :--- | :--- | :--- |
| **SOP Discovery** | Model relies on pre-prompted file lists | `rag.py` retrieves exact SOP sections on demand |
| **C++ Header Analysis** | Manual `read_file` calls | `search_files` + RAG indexing across `Source/` |
| **End of Sprint Closeout**| Reads protocol SOP $\rightarrow$ Writes `SR_YYYYMMDD.md` | RAG injects `END_OF_SPRINT_PROTOCOL.md` $\rightarrow$ `iron_coder.py` writes `SR_YYYYMMDD.md` |
| **Build & Test Verification**| Interactive user feedback | `run_command` executes `pytest` or `UnrealBuildTool` locally |

---

## 5. Conclusion

By combining **Level-Aware RAG (`rag.py`)**, **32k VRAM context caching**, **`iron_coder.py` multi-turn loop driver**, and **deterministic bridge guardrails**, local open-weights models running on the RTX 5090 will achieve the exact same SOP adherence, context grounding, and C++ code reliability as cloud-based agents.
