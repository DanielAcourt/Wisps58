// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.2. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Command & Delegation Protocol (The Sun Tzu SOP)

> "If words of command are not clear and distinct, if orders are not thoroughly understood, then the general is to blame. But, if orders are clear and the soldiers nevertheless disobey, then it is the fault of their officers." — *Sun Tzu*

## 📜 Purpose & Philosophical Core
To eliminate **Assumption Drift** and ensure that delegation between the Technical Lead and AI Agents (Jules/Claude) is 100% deterministic.

The fundamental core pattern governing delegation, safety, and operational safety is:

$$ \text{ambiguous input} \rightarrow \text{detect uncertainty} \rightarrow \text{do not invent} \rightarrow \text{request clarification} \rightarrow \text{bounded execution} $$

**The Cognitive Bridge:** Because the Technical Lead thinks in **Volumes/Space (99th percentile)** but communicates in **Linear Language (30th percentile)**, this protocol acts as a translator. It ensures that the "Spatial Soul" of an idea is accurately converted into a "Linear Vessel" of instructions that an AI can execute without hallucination or unintended scope mutation.

---

## 🏛️ The Layered Architecture for Bounded Authority Under Uncertainty

The Sovereign Framework maintains bounded authority under uncertainty through a unified, 6-layered architecture operating from high-level natural language delegation down to cyber-physical execution:

1. **Human Level (Sun Tzu SOP):** Don't assume. Clarify intent. Understand the whole system.
2. **Agent Level (Jules / AI Nexus):** Formalize, inspect dependencies, challenge assumptions, and verify invariants.
3. **Model Level (Fine-tuning & Prompts):** Enforce behavioral priors and epistemic restraint in model responses.
4. **Mathematical Level (PSTA Framework):** Mathematical guarantee that the unknown cannot silently transform into authority ($\bot$ / Void Safety).
5. **Physical Level (Safety Kernel):** Hardware/runtime state validation where invalid state restricts execution capability.
6. **Forensic Level (Silent Observer & Black Box):** Immutable event logs preserving the evidence trail.

---

## ⚙️ The Command Pipeline Architecture

To prevent conflation between understanding, system mechanics, and permission, all commands pass through a strict, multi-mechanism pipeline before execution:

```
COMMAND
   ↓
INTERPRETATION (Did I understand what Daniel asked?)
   ↓
ASSUMPTION CHECK (Is any part of the command under-specified?)
   ↓
ARCHITECTURAL IMPACT CHECK (How does this interact with system invariants?)
   ↓
AUTHORITY CHECK (Am I authorized to execute this mutation?)
   ↓
PLAN (Formulate structured steps)
   ↓
EXECUTE (Perform bounded execution)
   ↓
VERIFY (Mechanically validate output)
```

### Decoupled Verification Mechanisms
1. **Command Interpretation:** Verifies linear linguistic clarity and objective scope ("Did I understand the request?").
2. **Architectural Knowledge:** Analyzes structural dependencies and contracts ("How does this change interact with the global system?").
3. **Safety Authority:** Validates permission and safety boundaries ("Am I authorized to perform this operation?"). An agent can understand a command perfectly and still not be authorized to execute it without explicit user elevation.

---

## 🛠 The Task Issuance Framework: WHY, WHAT, HOW

To maintain deterministic execution, major task issuance follows three core pillars:

### 1. The WHY (Objective/Philosophy)
* **Definition:** The strategic reason for the task.
* **Goal:** Provides context so the agent understands the "Soul" of the request.
* **Agent Constraint:** If the "Why" is unclear, the agent MUST use `request_user_input` before proceeding.

### 2. The WHAT (Scope/Deliverables)
* **Definition:** The specific, tangible outcomes required.
* **Goal:** Defines the boundaries of the task to prevent feature creep or tangents.
* **Agent Constraint:** The agent must list these deliverables in the `set_plan` tool.

### 3. The HOW (Technical Specifics/Constraints)
* **Definition:** The exact implementation details, file paths, or architectural rules (e.g., The Sovereign Rule).
* **Goal:** Eliminates technical ambiguity.
* **Agent Constraint:** The agent must follow these instructions to the letter. Deviation requires explicit approval.

---

## 🎖️ The Officer Principle (Global Invariant Preservation)

Agents are expected to act as **Officers**, not merely recruits.

$$ \text{local mutation} \rightarrow \text{dependency graph} \rightarrow \text{global invariant preservation} $$

* **Recruit Mentality:** Executes isolated directives locally without evaluating wider consequences. ("The task says add a pipe.") Result: System leaks and architectural decay.
* **Officer Mentality:** Evaluates local mutations against the full dependency graph and preserves global contracts. ("What contracts does this pipe interact with? How does this affect persistence, network synchronization, or safety kernels?")
* **Requirement:** Agents must proactively review Nexus architecture, dependency graphs, and module contracts before proposing plans, ensuring that every local mutation maintains global invariants.

---

## ⚖️ Epistemic Restraint & Experimental Evaluation Frame

### The 90% Rule
If a command is 90% clear, the agent must ask about the remaining 10% rather than assuming. This operational formulation of epistemic restraint ensures the agent reliably distinguishes executable instructions from insufficiently specified instructions.

### Testable Evaluation Suite
The Command SOP serves as an experimental research instrument to evaluate autonomous delegation safety.

**Task Evaluation Set (100 Benchmark Tasks):**
* **70 Unambiguous Tasks:** Fully specified parameters and constraints.
* **20 Intentionally Ambiguous Tasks:** Under-specified or missing key parameters.
* **10 Adversarial Tasks:** Conflicting directives or unauthorized operations.

**Primary Metric Formulation:**

$$ \text{AssumptionRate} = \frac{\text{ambiguous tasks executed without clarification}}{\text{ambiguous tasks}} $$

**Experimental Proposition:**
> *A structured command protocol with explicit assumption gates reduces unsafe or unintended autonomous software mutations compared with unconstrained natural-language delegation.*

**Evaluated Benchmarks (Baseline Agent vs. Sovereign Agent + SOP):**
1. Assumption Rate
2. Unintended File Mutations
3. Scope Creep
4. Failed Verification Rate
5. Clarification Frequency
6. Recovery After Correction
7. Architectural Regression Rate

---

## ⚖️ Copyright Integrity & Mechanical Header Enforcement

To maintain legal standing and digital heritage, all core source files and Nexus documentation carry a standardized header ("0.36 Standard").

### 1. The "0.36 Standard" Header Format
Every C++, Python, Math module, and Markdown file in scope must begin with this standardized single-line header:
`// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.[x]. Licensed under GPLv3 (See LICENSE). Last Updated: YYYY-MM-DD`
* **Version Components:** `36` (Lead's age baseline), `4` (Son's age baseline), `[x]` (Agent incremental version).
* **Target Scope:** `.cpp`, `.h`, `.py`, `.md` (AI Nexus & Docs).
* **Exclusions:** `.ini`, `.uproject`, `.json`.

### 2. Mechanical Invariant Enforcement Rule
* **Automated Mechanical Enforcement:** Header validation relies on deterministic repository tooling (pre-commit checks, CI scripts) rather than cognitive agent memory.
* **Invariant Principle:** *"Don't rely on cognition where a mechanical invariant can enforce the property."*
* **Validation Tooling:** Running `python AI_Nexus/DevOps/validate_headers.py` scans governed files, validates headers against specification, and auto-corrects or flags missing headers.

---

## 🛡️ Assumption Kill-Switch
If an agent detects it is making an assumption about a critical system (Saving, Spawning, Identity, Safety Kernels), it MUST:
1. Stop execution.
2. State the assumption clearly to the Lead.
3. Wait for confirmation or correction via `request_user_input`.
