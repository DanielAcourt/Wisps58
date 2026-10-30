# 🔴 DEPRECATION & LOCALIZATION REPORT: SOVEREIGN AGENT I/O AUDIT (T-001)

**Document ID:** DRY-SR_20261030
**Revision:** 1.0 (Canonical T-001 Deliverable)
**Author:** Iron Knight AI & Jules (Tactician)
**Date:** 2026-10-30
**Target Objective:** Achieve 100% operational sovereignty and complete functional independence from external cloud gateway services.

---

## 📋 1. EXECUTIVE SUMMARY

With cloud agent platforms approaching turn-down and metered API endpoints imposing strict weekly token limits, achieving **100% local, sovereign AI engineering autonomy** on the RTX 5090 rig is an immediate priority.

This report delivers the complete technical debt inventory and I/O dependency audit for Ticket **T-001**. It identifies all network sockets, REST endpoints, external library calls, and transient memory structures across `Sovereign_Intelligence/IronOfficer/`, mapping each dependency to an actionable, local architectural replacement under the **Sovereign LocalBridge Protocol**.

---

## 🧱 2. DEPENDENCY VULNERABILITY MATRIX

| Dependency Domain | External Component / Socket | Ground Truth File | Local Vulnerability Risk | Local Replacement Strategy (`LocalBridge`) |
| :--- | :--- | :--- | :--- | :--- |
| **Inference Routing** | Ollama REST API (`http://127.0.0.1:11434`) | `bridge.py`, `vessel.py` | Coupling to single local service port; fails if service restarts | **`[LocalBridge]InferenceEngineInterceptor`**: Multi-backend loopback router supporting Ollama, vLLM, and llama.cpp `/v1/chat/completions`. |
| **Editor Automation** | UE5.8 Loopback MCP (`http://127.0.0.1:8000/mcp`) | `mcp_client.py` | Connection timeout or failure when Unreal Editor is closed | **`[LocalBridge]MCPRouter`**: Tool call proxy with local offline schema cache fallback (`unreal_mcp_cache.json`). |
| **Authentication & Roles**| Central Cloud UserAuth / Token Validation | `bridge.py` (`AAS Governor`) | Session token invalidation; blocked mutation tools | **`[LocalAuthMock]`**: File-system-backed credential store reading role permissions from local signed `config.json`. |
| **Event & Mailbox Queue**| Network Polling API (`/api/v1/mailbox/poll`) | `bridge.py` (`unreal_mailbox`) | Loss of queued directives on bridge server restart | **`[LocalQueueManager]`**: Persistent, thread-safe SQLite / JSON file-backed event queue in `Local_State_Cache/`. |
| **State Replication** | DataStream API (`/data/sync`) | `bridge.py` (`active_simulation_states`) | State drift across independent simulation entities | **`[LocalStateTracker]`**: Single Source of Truth (SSoT) transactional state tracker with atomic file locks. |
| **Model Weight Vault** | HuggingFace / Camenduru Weight Downloaders | `run_comfy.py` | Network bandwidth bottleneck / remote mirror down | **`[LocalBridge]ModelVaultChecker`**: Verifies local model SHA-256 hashes in local model directory `Sovereign_Intelligence/IronOfficer/Environment/models/`. |

---

## 🚨 3. TECHNICAL DEBT & I/O INVENTORY

### 3.1 Network & API Level Dependencies
1. **`process_chat_request` in `bridge.py`**:
   - Issue: Makes HTTP POST calls to `OLLAMA_HOST/api/chat`.
   - Action Required: Wrap in `InferenceEngineInterceptor` class to allow seamless fallback between Ollama, llama.cpp, and vLLM.
2. **`SovereignMCPClient` in `mcp_client.py`**:
   - Issue: Dispatches JSON-RPC 2.0 requests to `mcp_url`.
   - Action Required: Implement local schema cache fallback so offline tool introspection works when Unreal Engine is closed.
3. **`execute_handshake` in `bridge.py`**:
   - Issue: `HANDSHAKE_ACTIVE` flag resides in volatile RAM memory.
   - Action Required: Persist handshake state to `Local_State_Cache/handshake.json`.

### 3.2 File System & Path Handling Dependencies
1. **Windows Path Escaping**:
   - Issue: LLM parameter generation strips backslashes in Windows paths (e.g. `C:UsersDanie...`).
   - Action Required: Fully handled by `resolve_secure_path` in `bridge.py` (auto-correcting missing slashes to `C:/Users`).
2. **Protocol SOP Specifications**:
   - Issue: Models attempt to patch read-only SOP files under `AI_Nexus/Protocols/`.
   - Action Required: Fully handled by `AD-022` (`SOP_READ_ONLY_PROTECTED` status).

---

## 📈 4. ACTIONABLE MIGRATION ROADMAP (3-PHASE ROLLOUT)

```
  PHASE 1: STABILIZATION & AUDIT (T-001 & T-002)
  ├── Complete I/O Audit & Deprecation Report (This Document)
  └── Establish `Local_State_Cache/` & Atomic `LocalStateTracker`
        │
        ▼
  PHASE 2: BRIDGE & LOCALIZATION (T-003 & T-004)
  ├── Implement `LocalBridgeInterceptor.py` JSON-RPC Router
  └── Refactor Tool Directives with `LocalAuthMock` Pre-Flight Checks
        │
        ▼
  PHASE 3: ENHANCEMENT & SOVEREIGN COGNITION (T-005 & T-006)
  ├── Implement High-Speed TTL `LocalCacheManager`
  └── Deploy `FailureHandler` & SHA-256 `VersionControlMediator`
```

---

## ⚠️ STAKEHOLDER DIRECTIVE

Every subsystem component in `Sovereign_Intelligence/IronOfficer/` must conform to the **Sovereign LocalBridge Protocol** specification (`AI_Nexus/Protocols/LocalBridge_API_Spec.md`). Unwrapped network calls are strictly prohibited.
