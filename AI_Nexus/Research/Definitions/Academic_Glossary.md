// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
# Sovereign Academic Writing Glossary
## Academic Translation Layer for Research, Literature Review and Doctoral Writing

### Purpose
This glossary translates the Sovereign Framework's internal architectural language into academically recognisable language while preserving the underlying conceptual structure.

- The **Engineering Glossary** (`Glossary.md`) optimizes for execution.
- This **Academic Glossary** (`Academic_Glossary.md`) optimizes for scholarly interpretation.

The objective is not to make Daniel's writing sound like an LLM or to erase the original voice. The objective is to make a complex architectural argument reconstructable by an external researcher.

---

## 1. Core Writing Principle

### Architectural Language
The internal conceptual language used to reason about the system.
*Example:* "Unknown must not become authority."

### Academic Language
A formulation that makes the proposition explicit and testable.
*Example:* "An epistemically unresolved state should not, without additional evidence, confer unrestricted operational authority."

### Formal Language
The mathematical representation.

$$ \mathcal{V}(\perp) = 0 $$

### Evidence Language
The statement of what has actually been demonstrated.
*Example:* "Under assumptions A–C, the proposed operator prevents an unresolved state from satisfying the defined authority condition."

> **Rule:** Do not collapse these layers.

---

## 2. Academic Translation Map

| Sovereign Language | Academic Translation | Formal / Technical Layer |
| :--- | :--- | :--- |
| **Soul** | Persistent system-level state authority / single source of truth | `USovereignSaveableEntityComponent` |
| **Vessel** | Host computational or physical entity | Actor / vehicle / platform |
| **Spirit** | Controlling or decision-making agent | Operator / autonomous controller |
| **Void** | Epistemically unresolved state | $(\perp)$ |
| **Safety Clamp** | Non-compensatory authority constraint | Hard gate |
| **Symmetrical Guard** | Independent runtime safety monitor | $(\Delta_{sym})$ |
| **Severance** | Capability revocation / safety intervention | State transition |
| **Silent Observer** | Independent monitoring and provenance subsystem | Telemetry observer |
| **Black Box** | Survivable event and telemetry record | Timestamped log |
| **Anchor** | Dominant or overriding constraint | Hard constraint |
| **Sovereign Core** | Trusted computing / authority boundary | Kernel / assurance layer |
| **Trust Recovery** | Time-dependent restoration of operational confidence | $(\Phi(t))$ |
| **Risk Velocity** | Temporal derivative of a system-health variable | $(V_i(t))$ |
| **Cognitive Administration** | Provenance-aware research and governance process | Metadata / authorship / evidence model |
| **Mirror Loop** | Self-referential epistemic reinforcement | AI feedback contamination |
| **Dinosaur Bone Problem** | Bounded information retrieval / clarification under resource constraints | Query constraint |
| **Local Sovereignty** | Autonomous local decision authority under communication constraints | Runtime autonomy |
| **Truth Record** | Provenance-preserving observation record | $(O(t)) + \text{metadata}$ |

---

## 3. Claim Language

- **Observation:** Something directly recorded, measured, or reproduced.
  - *Preferred:* "Sensor S17 reported 41 knots at $t$."
  - *Avoid:* "The vessel was travelling at 41 knots." (unless evidence establishes the physical claim).
- **Interpretation:** A model-derived explanation of an observation.
  - *Preferred:* "The observation was interpreted as an airspeed estimate." (Do not present interpretation as raw telemetry).
- **Hypothesis:** A proposition proposed for investigation but not yet established.
  - *Preferred:* "We hypothesise that..."
- **Claim:** A proposition being presented for evaluation.
  - *Preferred:* "The proposed architecture treats..."
- **Result:** A finding supported by the stated experiment, analysis, or proof.
  - *Preferred:* "Under the specified test conditions, the implementation produced..."
- **Conclusion:** A claim justified by the combined evidence. (Do not use "conclusion" merely because a section has ended).

---

## 4. Evidence Language

- **Demonstrated:** Use only when an experiment, proof, or reproducible procedure supports the statement.
- **Formally established:** Use when the proposition follows from a stated formal model and its assumptions.
- **Empirically observed:** Use when something was measured experimentally.
- **Consistent with:** Use when evidence agrees with a proposition but does not establish it.
- **Suggests:** Use when evidence provides a meaningful but incomplete indication.
- **Plausible:** Use when the proposition has a coherent rationale but insufficient evidence.
- **Unresolved:** Use when the available evidence cannot distinguish competing explanations.
- **Unsupported:** Use when a proposition lacks adequate evidence.

---

## 5. Words That Require Operational Definitions

The following terms should not appear in doctoral thesis or academic papers without being explicitly defined in the local research context:

$$\text{trust, safety, authority, intelligence, autonomy, truth, uncertainty, reliability, robustness, confidence, coherence, risk, agency, alignment, failure, recovery, independence, provenance}$$

*Example:*
Do not write: *"PSTA increases trust."*
Write: *"Within this study, trust denotes the defined operational quantity ($T$), calculated from [specified variables], and does not imply physical truth or epistemic certainty."*

---

## 6. Mathematical Writing

- **Equation:** A compact representation of a relationship.
- **Definition:** States what a mathematical object means.
- **Assumption:** States what must be accepted for the following derivation or claim to hold.
- **Proposition:** A formally stated claim.
- **Lemma:** A supporting result used to establish a larger proposition.
- **Theorem:** A formally demonstrated result under explicit assumptions.
- **Corollary:** A consequence that follows from an established result.
- **Model:** A mathematical representation of some aspect of a system.
- **Policy:** A rule applied to the output of a model.

> *Note:* This distinction is critical for PSTA. For example, $\mathcal{V}(\perp)=0$ may be a policy concerning authority; it does not, by itself, establish that $\text{physical state} = 0$.

---

## 7. Proof Status

Every major mathematical idea should eventually carry one of these 8 formal statuses:

1. **CONCEPTUAL:** The idea exists but has not been formally specified.
2. **FORMALISED:** Variables, domains, and relationships have been defined.
3. **PROPOSITION:** A specific property has been stated.
4. **PROVEN UNDER ASSUMPTIONS:** A valid derivation exists.
5. **IMPLEMENTED:** The formal model has a software representation.
6. **VERIFIED:** Implementation behaviour has been checked against the specification.
7. **EXPERIMENTALLY EVALUATED:** The system has been tested under specified conditions.
8. **INDEPENDENTLY REVIEWED:** An external researcher has examined the result.

> **Rule:** Never use "proven" to mean merely "implemented."

---

## 8. Literature Language

- **Prior Art:** Existing work that addresses a materially similar problem.
- **Research Neighbour:** A related field or method that informs the problem without necessarily solving the exact same problem.
- **Overlap:** A meaningful conceptual or technical similarity.
- **Novelty Threat:** Existing work that may substantially reduce or eliminate a claimed contribution.
- **Research Gap:** A question that remains insufficiently addressed after examination of the literature.
- **Contribution:** The specific addition made by the present research.
- **Differentiation:** The precise property that distinguishes the proposed work from existing approaches.

*Preferred Pattern:*
Do not write: *"No one has solved this problem."*
Write: *"Existing literature addresses X and Y; however, the reviewed approaches do not appear to address Z under the stated conditions."*

---

## 9. Academic Discipline for Strong Claims

- Replace *"This proves..."* $\rightarrow$ *"This establishes, under assumptions A–C, that..."*
- Replace *"This is novel."* $\rightarrow$ *"The reviewed literature indicates a potential distinction in..."*
- Replace *"The system is safe."* $\rightarrow$ *"The system satisfies the defined safety condition under..."*
- Replace *"The sensor is wrong."* $\rightarrow$ *"The observation is inconsistent with the defined reference condition."*
- Replace *"The AI hallucinated."* $\rightarrow$ *"The generated output contained an unsupported factual assertion."*
- Replace *"The model became crazy."* $\rightarrow$ *"The model entered a state in which its generated output was inconsistent with the defined evidence or authority conditions."*

---

## 10. Provenance Language

Every major claim should be traceable to one of six sources:
1. Human proposition
2. Literature
3. Formal derivation
4. Implementation
5. Experiment
6. External review

*Preferred Structure:*
"The conceptual proposition originated in [research record]. It was formalised as [definition/equation], implemented in [component], and evaluated using [experiment]."

---

## 11. AI-Assisted Writing

AI-generated wording must not silently acquire the status of human reasoning.

Maintain the explicit chain:

$$ \text{Human Intent} \rightarrow \text{AI Formalisation} \rightarrow \text{Human Review} \rightarrow \text{Accepted Wording} $$

AI may assist with: restructuring, terminology, literature discovery, mathematical exposition, counterexamples, grammar, and translation between representational layers. AI must **not** be treated as independent evidence merely because multiple models agree.

---

## 12. Anti-"AI Cadence" Rule

Avoid prose that is polished but semantically empty.

Do **not** use:
- *"This groundbreaking work..."*
- *"In today's rapidly evolving landscape..."*
- *"It is important to note that..."*
- *"This represents a paradigm shift..."*
- *"By leveraging..."*
- *"Furthermore"* on every paragraph
- Repeated three-part rhetorical lists
- Conclusions that simply restate the introduction

*Prefer:* Specific subject $\rightarrow$ specific claim $\rightarrow$ evidence $\rightarrow$ qualification.

---

## 13. Preserve Daniel's Voice

Academic translation must not remove conceptual ownership. Retain distinctive architectural terms when they carry useful structure.

- *Preferred:* "We refer to this unresolved evidence state as the Void ($(\perp)$). In runtime-verification literature, it is closely related to an inconclusive verdict."
- *Avoid:* "The Void concept is replaced by inconclusive semantics."

---

## 14. Writing from a High-Dimensional Model

When a concept is difficult to explain linearly, use the sequence:

$$ \text{Problem} \rightarrow \text{Diagram} \rightarrow \text{Definition} \rightarrow \text{Equation} \rightarrow \text{Example} \rightarrow \text{Counterexample} \rightarrow \text{Evidence} $$

Do not attempt to encode every relationship into one paragraph. A paragraph is a projection of the model, not the entire model.

---

## 15. The Academic Reader's Stairs

Present complex ideas across 8 accessible tiers:
- **Level 1 — One sentence:** What is the claim?
- **Level 2 — Conceptual explanation:** Why does it matter?
- **Level 3 — Diagram:** How are the components related?
- **Level 4 — Formal definition:** What exactly does each term mean?
- **Level 5 — Mathematics:** What relationship is being asserted?
- **Level 6 — Implementation:** How is it realised?
- **Level 7 — Evidence:** What happened when it was tested?
- **Level 8 — Limitation:** Where does the claim stop?

---

## 16. Translation of Daniel's Natural Research Style

When Daniel writes:
*"The machine doesn't know if the sensor is telling the truth, so it should stop trusting itself."*

Translate conceptually into:
*"A sensor observation may be recorded without being sufficient to justify the corresponding physical-state proposition. Where the evidence fails the defined admissibility conditions, operational authority should be restricted rather than inferred from model confidence."*

Preserving the original core concept:

$$ \text{record observation} \neq \text{authorise interpretation} $$

---

## 17. The Four-Status Rule

For every important statement, ask:
1. What was **observed**?
2. What was **inferred**?
3. What was **authorised**?
4. What remains **unknown**?

This distinction should survive throughout the thesis and research literature.

---

## 18. Final Academic Writing Rule

Do not make the language more sophisticated than the evidence. The purpose of academic prose is not to sound intelligent—it is to make the reasoning recoverable by another researcher.

A good sentence should allow the reader to identify:

$$ \text{who/what} + \text{did/observed what} + \text{under which conditions} + \text{with what evidence} + \text{with what limitation} $$

---

## Operational Summary

- **Engineering Glossary (`Glossary.md`):** Optimizes for execution.
- **Academic Glossary (`Academic_Glossary.md`):** Optimizes for interpretation.
- **Formal Notation:** Optimizes for precision.
- **Code:** Optimizes for execution.
- **Evidence:** Determines what may be claimed.
- **Human Review:** Determines what survives independent scrutiny.

The objective is not to make all languages identical; the objective is to make them interoperable without semantic drift.
