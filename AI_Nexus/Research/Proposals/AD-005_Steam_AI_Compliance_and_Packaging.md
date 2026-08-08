// Copyright (c) 2013-2025 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-08
# Proposal & Specification: AD-005 Steam AI Compliance & Local Packaging

**Author:** Jules (Researcher Persona / Knowledge Ingestion Lead)
**Date:** August 2026
**Node:** `AI_Nexus/Research/Proposals/AD-005_Steam_AI_Compliance_and_Packaging.md`
**Status:** Canonical Proposal / Systems Spec
**Version:** 1.0.0

---

## 🏛️ 1. Executive Summary & The "Why"
As the WispsCPPVR simulation matures and moves toward public accessibility via Steam, we face a critical publishing bottleneck. Under current legal frameworks and Valve's distribution agreements, utilizing real-time Generative AI during gameplay (e.g., AI possession of actors, real-time dialogue synthesis, and dynamically generated player-actor interaction) introduces severe compliance, legal liability, and system performance hurdles.

The goal of this document is twofold:
1. **Legally de-risk publishing on Steam** by analyzing Valve's strict AI policies and specifying a deterministic, verifiable, and non-hallucinated **AAS Steam Safety Guardrail**.
2. **Resolve hardware delivery barriers** by proposing an architectural blueprint for bundling low-latency, offline, on-device AI models directly with the game executables, eliminating complex Python dependencies and third-party API costs for the baseline player.

---

## ⚖️ 2. Steam's Generative AI Policy & Legal Guardrails

Through extensive research of Steamworks guidelines and real-world developer audits, Steam's policy (as updated in January 2026) operates on a strict taxonomy:

### A. Pre-Generated AI Content (Static Assets)
* **Definition:** Any content (art, music, sound, code, pre-written dialogue trees) created with AI tools *during* development and packaged into the final game files.
* **Steam Rule:** Developers must disclose this use in the Steamworks Content Survey. Valve audits these assets during pre-release review using the same standard as handmade assets.
* **Developer Promise:** Under the Steam Distribution Agreement, the developer promises that these assets do not contain illegal, infringing, or copyright-violating content.
* **Exemption:** "Efficiency gains" from standard developer tools (e.g., GitHub Copilot, IDE code assistants, generic Photoshop Generative Fill for drafts) are **explicitly exempt** and do not trigger disclosure, provided the AI-generated artifact itself does not ship with the game files.

### B. Live-Generated AI Content (Real-Time Synthesis)
* **Definition:** Content generated on-the-fly while the game client is running (e.g., our active actor possession scenario).
* **Steam Rules:**
  1. **Mandatory Disclosure:** Developers must explicitly declare live-generation in the Content Survey. This disclosure is displayed publicly on the Steam Store Page.
  2. **Active Guardrails Requirement:** Developers must provide Steam with a detailed description of the safety guardrails placed on the live generator to guarantee it will never output illegal content.
  3. **The Adult Only Sexual Content (AOSC) Ban:** Live-generated sexual content is **completely prohibited**. Steam allows hand-made and pre-generated adult games, but will *not* distribute any game where the AI can live-generate Adult Only Sexual Content.
  4. **The Player Report System:** Valve integrates an official "Report Illegal AI Content" button directly into the Steam Overlay during gameplay. Players can instantly submit flagged outputs with chat logs and contextual data to Valve. If a game's guardrails are bypassed (jailbroken) and Valve finds illegal or violating generation, the game is removed from the Steam platform.

### C. Primary Compliance Sources
* **Steamworks Official Policy Announcement (Jan 2024):** [Steamworks Development: AI Content on Steam](https://steamcommunity.com/groups/steamworks/announcements/detail/3862463747997849619)
* **Steamworks Content Survey Manual:** [Steamworks Partner Docs: Content Survey FAQ](https://partner.steamgames.com/doc/gettingstarted/contentsurvey)
* **2026 Policy Overhaul (Efficiency Tool Exemption):** [Valve Clarifies Steam's AI Disclosure Rules](https://finance.biggo.com/news/202601171220_Steam_AI_Disclosure_Update_Focuses_on_Player_Content)

---

## 🛠️ 3. Technical Packaging & Hardware Delivery Blueprint

To package a fully local, self-hosted LLM inside a single download off of Steam without expecting players to maintain Python, PyTorch, or Ollama, we model our delivery pipeline on the successful architecture of *The Moment of Panic* (TheBadinc, Feb 2025), which ships an active local LLM natively as part of its core gameplay loop.

### A. The Engine: Compiled `llama.cpp` Backend
* **No Python, No PyTorch:** We bypass the Python runtime entirely. We package a compiled, native C++ executable of **`llama-server`** (from the open-source `llama.cpp` library) directly inside our packaged game directory (`/Binaries/ThirdParty/llama/`).
* **The Model File:** We bundle a highly optimized, quantized GGUF model—specifically **Llama 3.2 3B Instruct (Q4_K_M 4-bit Quantization)**.
  - **Memory Footprint:** The 3B model occupies only **~2.2 GB** of disk space and requires roughly **2.5 GB to 3.5 GB of VRAM** during live inference.
  - **The GPU Headroom Win:** Since WispsCPPVR's virtual reality rendering requires tight optimization, we ensure the game's static assets are lightweight, leaving ample headroom on standard mid-range gaming GPUs (GTX 1070 / RX 5700 8GB or higher) to process dialogue in the background without frame drops.
* **Cross-Hardware Abstraction:** The bundled `llama-server` binary natively handles hardware acceleration selection dynamically:
  - **CUDA:** Automatic activation on NVIDIA GPUs.
  - **Vulkan:** Native activation on AMD and Intel GPUs.
  - **CPU Fallback:** Safe execution with system RAM if no dedicated GPU is available.

### B. The Dual-Mode Gateway Strategy
To give enthusiasts maximum sovereign power while maintaining low-overhead safety for casual Steam players, we implement a **Dual-Mode System Interface**:

```
                              +---------------------------------------+
                              |         WISPSCPPVR GAME CLIENT        |
                              +------------------+--------------------+
                                                 |
                                 [Select Connection Protocol]
                                                 |
                        +------------------------+------------------------+
                        |                                                 |
            [A. NATIVE COMPLIANT MODE]                       [B. SOVEREIGN EXPERT MODE]
                        |                                                 |
             - Bundled Native C++ binary                      - Connects to user's local
               (`llama-server` via GGUF).                       Ollama GTX 5090 (`bridge.py`).
             - Lightweight 3B aligned model                   - High-parameter 70B model
               (Llama 3.2 3B Instruct Q4).                      (Llama 3.1 70B).
             - Hard-coded Steam Safety Guard                    - Complete unconstrained
               (AAS / PSTA Deterministic E-Stop).               customization & emergent lore.
             - Steam Pre-Release Approved.                    - Exempt from Steam liability.
```

---

## 🛡️ 4. The AAS Steam Safety Guardrail (Symmetrical Severance)

To guarantee that the live-generation client can never generate Adult Only Sexual Content or illegal narratives, we enforce a deterministic, dual-layered guardrail controlled by the **Agency Arbitration Schema (AAS)**:

### Layer 1: Deterministic Scribe Censorship (Lexicon Filter)
A high-frequency regex and keyword-based filtering process running natively in C++ before the user's prompt is dispatched to the local inference server, and immediately after the response is synthesized.
* If any banned keywords, sexual phrases, or unsafe semantic patterns are detected, the message is blocked before generation, and a localized systemic warning is returned.

### Layer 2: Cognitive PSTA Alignment & Symmetrical Severance
If the user attempts to jailbreak the model using complex linguistic masking, our **Administrative (A)** and **Social (S)** pillars act as an automated monitor.
* **The Violation Penalty:** If the incoming or outgoing dialogue falls below our PSTA compliance threshold (due to high Paradox Density $\Xi$ or violation of the Symmetrical Guard), the Administrative pillar health ($D_A$) instantly collapses to `0.0`.
* **Symmetrical Severance (The Hard-Kill E-Stop):**
  - Following the PSTA **Bottleneck Law**, once $D_A = 0.0$, the entire system **Vessel Safety Status (VSS)** snaps to `0.0`.
  - This immediately halts the possession cycle, unpossesses the actor, triggers an in-game "Containment Stasis" (the actor goes rigid, their eyes turn to cold stone, and their connection to the "Spirit" is severed), and logs the incident under `AI_Nexus/DevOps/Incident_Reports/`.

---

## 📡 5. Verification & Telemetry Audit Trail

To defend our developer publishing account against malicious or false player reports, whenever the Steam Overlay report is triggered, our Unreal client auto-logs a telemetry payload to `Sovereign_Intelligence/IronOfficer/RolePlay/Reports/`:
1. The full active conversational chat log.
2. The current C++ save state of the Soul Hub.
3. The exact seed, model identifier, and active PSTA metrics.

This provides an indisputable, deterministic audit trail proving our compliance with Valve's safety guidelines.

---
// "The boundary is defined; the gateway is open." [Researcher] 2026-08-08
