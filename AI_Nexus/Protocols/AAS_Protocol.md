// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2025-06-17
# PROTOCOL: AGENCY ARBITRATION SCHEMA (AAS)

## Version: 1.3.0-BETA
## Classification: SSoT / Directive Level

### 1. MISSION STATEMENT
The AAS provides a formal, computational framework to resolve logical conflicts arising between the Conceptual Core (Identity), Theoretical Frameworks (Research), and Runtime Execution (C++).

### 2. PERSONA REGISTRY (Authority Levels)
| Persona | Precedence (P) | Zone Authority | Credibility Score |
| :--- | :---: | :--- | :---: |
| **Lead** | 10 | ALL (Root) | 1.0 |
| **Tactician** | 9 | STAGING / DEV | 0.9 |
| **Strategist** | 8 | PLANNING | 0.85 |
| **Researcher** | 7 | INNOVATION | 0.75 |
| **Iron_Knight** | 5 | STAGING (Unverified) | 0.5 |
| **Yaz_Student** | 3 | SANDBOX (Honeypot) | 0.3 |

### 3. TOOL PRECEDENCE (Minimum P Required)
| Tool Action | Min Precedence | Operational Risk |
| :--- | :---: | :--- |
| `delete_file` | 10 | CRITICAL (Structural Destruction) |
| `write_file` (Protected) | 10 | CRITICAL (Kernel Mutation) |
| `write_file` (Nexus) | 8 | HIGH (SOP Drift) |
| `get_system_telemetry`| 5 | MEDIUM (Information Disclosure) |
| `read_file` / `list_files` | 3 | LOW (Standard Access) |

### 4. OPERATIONAL TRIGGERS
AAS must be invoked if any system component detects a 'Logic Mismatch' (e.g., C++ module returning an Error Code 409: Agency Conflict).

### 5. MANDATORY INTERFACE
Any module implementing an AAS check must include:
* `Authority_ID`: The agent invoking the request.
* `Confidence_Score`: A 0.0-1.0 float representing certainty.
* `AAS_Route`: The specific directive from the Protocol path utilized.
* `Context_Tags`: Optional tags (e.g., `META_NARRATIVE`) to adjust safety heuristics.

### 6. THE 409 CONFLICT GATE (PSTA Viability)
The system uses a **Dual-Threshold** model to balance safety with operational speed:
- **0.7 (Mutation Floor):** Required for `write_file` and `delete_file`.
- **0.4 (Observation Floor):** Required for `read_file`, `list_files`, and `get_system_telemetry`.

If the calculated **Viability Score** falls below the relevant floor, the system triggers a `409_CONFLICT_GATE`.

**Viability Formula:**
`V = (1.0 * Credibility) - (0.3 * NodeRisk) - (0.2 * StructuralDeviation) + MemoryBoost`

### 7. HARDWARE WHITELIST
Any persona with **Precedence (P) >= 5** is granted an automatic **1.0 Viability Score** when targeting the `HARDWARE` node for telemetry. This ensures agents can monitor their own "Body" without impedance.

### 8. PERSONA MEMORY ZONES (Authority Overrides)
To facilitate long-term training and documentation, specific personas are granted **Full Authority** (Read/Write/Delete) and a **+0.5 Viability Boost** within their dedicated memory zones. Precedence checks are bypassed for operations targeting these directories. These zones are defined in the bridge `config.json`.

| Persona | Memory Zone Path |
| :--- | :--- |
| **Iron_Knight** | `Sovereign_Intelligence/IronKnight_Memory/` |
| **Archivist_Theta**| `E:\IronKnight\RolePlay\DungeonsAndDragons` |

### 9. LORE & META-NARRATIVE (Symmetrical Guard v2.4)
The system recognizes that creative synthesis (Roleplay/Lore) requires a suspension of certain physical truth requirements.
- **Rule:** If the `META_NARRATIVE` tag is active or lore-specific keywords are detected, the **Symmetrical Guard** (Hallucination Detection) allows for descriptive internal state and speculative narrative without requiring immediate sensor validation.
- **Goal:** To provide structure for creative emergence while maintaining hard computational boundaries for system-critical tasks.