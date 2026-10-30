# Comprehensive Technical Audit & Recommendations: Building a Dependable Local Programming Agent (Iron Knight)

**Document Version:** 2.0.0
**Author:** Jules (Agentic Technical Lead)
**Target Architecture:** Iron Knight / Iron Officer (RTX 5090 32GB VRAM Local Suite)
**Date:** 2026-10-30

---

## 1. Executive Summary

Transitioning from cloud-based AI coding agents (such as Jules) to a **100% offline, dependable local programming agent** on the RTX 5090 rig requires expanding Iron Knight's capabilities beyond an Unreal Engine API bridge into a **standalone software engineering execution engine**.

Currently, Iron Knight excels at RAG knowledge retrieval, PSTA viability arbitration, and Unreal Engine simulation bridge directives. However, for general programming tasks (editing C++/Python, running tests, executing git commands, and debugging), it relies on single-turn human prompting and lacks CLI subprocess execution capabilities.

This document presents a complete architectural audit and actionable recommendations to upgrade Iron Knight into a highly dependable, autonomous local software engineer.

---

## 2. Technical Gap Analysis: What Iron Knight Needs for Local Programming

| Feature Domain | Current State (Unreal Bridge Focus) | Target State (Local Engineer Focus) | Priority |
| :--- | :--- | :--- | :--- |
| **Model Selection** | Llama 3.1 70B (General Chat) | **Qwen 2.5 Coder 32B Instruct** (FP8 / Q8_0) | **CRITICAL** |
| **Command Execution** | None (File I/O and REST API only) | **VSS-Gated CLI Subprocess Runner** (`run_command`) | **CRITICAL** |
| **Execution Loop** | Single-turn request/response (`/v1/chat`) | **Autonomous Multi-Turn Agent Loop** (`iron_coder.py`) | **CRITICAL** |
| **Code Editing** | Whole file write & basic string `patch_file` | **Git Merge Diff / Unified Patch Applicator** | **HIGH** |
| **Workspace Navigation** | Basic `list_files` & regex `search_files` | **Git-Aware Directory Mapper & Structural Grep** | **MEDIUM** |
| **Quality Control** | AAS VSS score arbitration | **Automated Pre-Commit Verification & Test Runner** | **HIGH** |

---

## 3. Top 6 Actionable Upgrade Recommendations

### 1. Deploy Qwen 2.5 Coder 32B Instruct on RTX 5090 (Model Layer)
- **Why**: General chat models (Llama 3.1 70B) frequently hallucinate tool outputs or struggle with multi-file refactoring and JSON tool call formatting.
- **Solution**: Load **Qwen 2.5 Coder 32B Instruct** (FP8 or Q8_0 GGUF/EXL2) into Ollama / llama.cpp / vLLM on the RTX 5090.
- **Performance**: Qwen 2.5 Coder 32B scores 90%+ on HumanEval and MBPP, matching or exceeding Claude 3.5 Sonnet on coding tasks while executing locally at 100+ tokens/sec with $0 token cost.

### 2. Implement the VSS-Gated Subprocess Execution Tool (`run_command`)
- **Why**: An agent cannot debug or verify code without running compiler commands (`cmake`, `UnrealBuildTool.exe`), unit tests (`pytest`, `unittest`), or version control commands (`git status`, `git diff`, `git commit`).
- **Solution**: Add `tool_run_command(command, working_dir)` to `bridge.py`:
  - Enforce AAS VSS safety thresholds (VSS >= 0.7 for build/test commands, VSS >= 1.0 for destructive actions).
  - Execute commands in a sandboxed subprocess and return stdout/stderr to the model context.

### 3. Build the Autonomous Multi-Turn Execution Loop (`iron_coder.py`)
- **Why**: Single-turn chat requires human hand-holding after every command. Real coding tasks require an iterative loop: **Plan $\rightarrow$ Tool Call $\rightarrow$ Inspect Output $\rightarrow$ Correct Errors $\rightarrow$ Verify Tests $\rightarrow$ Reflect $\rightarrow$ Finalize**.
- **Solution**: Implement `Sovereign_Intelligence/IronOfficer/iron_coder.py` as an autonomous CLI loop driver that accepts high-level prompts, maintains task state, executes tools in sequence, and iterates until unit tests pass.

### 4. Implement Unified Merge Diff / Patch Applicator Tool
- **Why**: Simple string replacement (`content.replace(search, replace)`) fails when whitespace or duplicate code blocks exist, or when editing large multi-line functions.
- **Solution**: Add `tool_apply_patch(filepath, patch_diff)` to `bridge.py` supporting standard Git merge diffs (`<<<<<<< SEARCH`, `=======`, `>>>>>>> REPLACE`) and unified diff formats with automatic `.bak` backup creation.

### 5. Add Workspace Exploration & Git Integration Tools
- **Why**: Iron Knight needs fast, structured visibility into repository structure without getting bogged down in `.git`, `__pycache__`, or `Intermediate` build folders.
- **Solution**: Add `tool_git_status()`, `tool_git_diff()`, and update `map_directory` to automatically respect `.gitignore` rules.

### 6. Embed Automated Pre-Commit Verification & Pre-Flight Checks
- **Why**: Prevent broken builds or syntax errors from being committed to the repository.
- **Solution**: Before `iron_coder.py` finishes a task, automatically trigger:
  1. Syntax compilation checks (`python3 -m py_compile` or C++ build dry-run).
  2. Test suite execution (`unittest` / `pytest`).
  3. Pre-commit check verification.

---

## 4. Hardware Allocation Strategy on RTX 5090 (32GB VRAM)

| Resource | Memory Allocation | Purpose |
| :--- | :--- | :--- |
| **Qwen 2.5 Coder 32B (FP8)** | ~20 GB VRAM | High-speed local reasoning & code generation |
| **KV Cache Context Window (32k tokens)** | ~6 GB VRAM | Long file context & multi-file reasoning |
| **Local RAG Embedding Model** | ~1 GB VRAM | Instant semantic retrieval across 12+ years of notes |
| **Headless ComfyUI Service (Port 8188)** | ~5 GB VRAM (Dynamic) | Photoreal image generation (`/imagegen`) |
| **Total System Reserve** | **32 GB VRAM** | **100% Utilized / 0 Cloud Dependency** |

---

## 5. Summary Roadmap

By implementing tickets **`AD-021`** (Tool Error Disambiguation & Handshake Context Sync), **`AD-022`** (Protocol Read-Only Grounding), **`AD-023`** (Iron Coder Loop Harness), and **`T-001` through `T-006`** (Sovereign Local Bridge & State Persistence) in `CURRENT_SPRINT.md`, Iron Knight will evolve from an Unreal API bridge into a **100% sovereign, highly dependable local software engineer**—saving 100% of cloud token costs and eliminating reliance on external rate limits.
