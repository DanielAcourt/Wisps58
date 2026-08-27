// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-05
# PROTOCOL: COGNITIVE ADMINISTRATION (COLOR & AUTHORSHIP)

## Version: 1.0.0-PROPOSED
## Node: `AI_Nexus/Protocols/COGNITIVE_ADMINISTRATION_PROTOCOL.md`
## Persona Target: Jules_Researcher / /07 Scholar & Multi-Agent Matrix

---

## 🏛️ 1. OVERVIEW & MISSION STATEMENT

The **Cognitive Administration Protocol** was developed by Daniel Acourt and Roxana Negru to establish unambiguous truth provenance and multi-author collaboration in academic, technical, and creative writing.

When human authors, PhD supervisors, and AI agents collaborate on a single living document (such as a doctoral dissertation or framework specification), tracking the precise origin, review state, and consensus level of every sentence is critical.

The ultimate goal of Cognitive Administration is to **transform all text into "Black" (Agreed Truth / Final Consensus)** through iterative agreement.

```
[Red: Candidate/Lead]   \
[Blue: AI Agents]        ===> [Iterative Alignment & Verification] ===> [Black: Final Consensus / Truth]
[Green: PHD Supervisor] /
```

---

## 🎨 2. AUTHORSHIP COLOR ROLES & HEX MAPPINGS

Since standard text and Markdown files (`.md`) do not render visual colors by default, Cognitive Administration uses **HTML inline CSS spans** for rendered views (e.g. previewing in IDEs or web platforms) combined with **Markdown text markers** for raw text and LLM context parsing.

| Role | Entity | Hex Color | Rendered Style | Markdown Fallback Prefix |
| :--- | :--- | :---: | :--- | :--- |
| **Doctoral Candidate / Lead** | Daniel Christopher Acourt | `#D32F2F` | <span style="color:#D32F2F; font-weight:bold;">Red</span> | `[Daniel / Candidate]:` |
| **AI Agents & Nexus** | Jules + AI Nexus v0.36.0+ | `#1976D2` | <span style="color:#1976D2; font-weight:bold;">Blue</span> | `[Jules / AI Nexus]:` |
| **PhD Supervisor** | Target Supervisor (Dr. Manish Malik et al.) | `#388E3C` | <span style="color:#388E3C; font-weight:bold;">Green</span> | `[Supervisor]:` |
| **Agreed Truth / Final** | Unanimous Consensus | `#000000` | Standard Text (Black) | *(No prefix)* |

---

## 💻 3. MARKUP IMPLEMENTATION SYNTAX

To ensure both human readability in visual editors and strict machine readability during LLM context ingestion, all non-black text blocks MUST be formatted with both HTML inline styling and explicit text prefixes.

### Example Markdown Source Syntax:
```markdown
<span style="color: #D32F2F;">[Daniel / Candidate]: In physics, a substrate typically refers to the underlying medium, field, or physical support structure in which physical phenomena occur.</span>

<span style="color: #1976D2;">[Jules / AI Nexus]: We formalize this computationally under Axiom 1 as the Tri-State Base Bit $\mathcal{B} \in \{0, 1, \perp\}$, where untracked substrate states default to void ($\perp$).</span>

<span style="color: #388E3C;">[Supervisor]: Ensure you ground this definition against QNX Neutrino RTOS microkernel hardware isolation literature.</span>

This sentence represents verified, unanimous consensus and has been accepted into black text.
```

---

## 🔄 4. LIFECYCLE & TRUTH CONVERGENCE WORKFLOW

1. **Instantiation (Red / Blue / Green):**
   * Any new proposal, claim, or hypothesis added by a specific participant is tagged with their respective author color and prefix.
2. **Review & Challenge:**
   * Collaborators comment or refine the text in their respective colors below or inline.
3. **Consensus & Conversion (Turning to Black):**
   * Once all parties (Doctoral Candidate, PhD Supervisor, and AI Verification Engine) agree on a statement, the HTML spans and author prefixes are removed.
   * The text reverts to standard Markdown (Black text), denoting **Agreed Universal Truth**.

---

## 🧪 5. EXPERIMENTAL ROADMAP & GOOGLE DRIVE CONVERSION (AD-018)

As logged in ticket `AD-018`, implementation tools and conversion scripts for this protocol will be prototyped in subsequent experimental phases:
* **Google Docs Importer/Exporter Script:** Tooling to map Google Docs font highlight/text colors directly to Cognitive Administration HTML spans and vice versa.
* **RAG Parsing Rule:** Custom parser in `rag.py` to extract author metadata tags (`[Daniel]`, `[Jules]`, `[Supervisor]`) as dedicated metadata fields during vector embedding.

---
// "Without structure there is chaos; through alignment, color becomes truth." [Cognitive Administration Protocol] 2026-08-05
