// Copyright (c) 2013-2026 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

# Sovereign Framework: Dynamic Broker Lifecycle & VSS Coupling Implementation Guide

This guide details the technical specifications, architectural patterns, and validation protocols for the specialized Sovereign data brokers. It outlines the dynamic lifecycle, strict domain namespace isolation, and non-compensatory mathematical coupling of Paradox Density ($\Xi$) with System Confidence.

---

## 🏛️ 1. Architectural Blueprint: The Decoupled Soul

The **Sovereign Soul Mediator** (`USovereignSaveableEntityComponent`) acts as a central orchestrator. Rather than managing domain-specific state monolithically, it delegates responsibilities to modular, testable brokers conforming to `ISovereignBrokerInterface`.

```
                  +-----------------------------------------+
                  |  USovereignSaveableEntityComponent      |
                  |                (Soul)                   |
                  +--------------------+--------------------+
                                       |
                   +-------------------+-------------------+
                   | (Mediator Pattern)|                   |
                   v                                       v
     +-------------+-------------+           +-------------+-------------+
     |     UDiagnosticBroker     |           |    UCultivationBroker     |
     |          (Truth)          |           |          (Magic)          |
     |   - Lidar Telemetry       |           |   - Qi Progression        |
     |   - Curator Sign-offs     |           |   - Simulation Growth     |
     |   - "Sovereign.Truth"     |           |   - "Sovereign.Magic"     |
     +---------------------------+           +---------------------------+
```

### Decomposed Sprint Roadmap (E-001)
To eliminate integration risk and maintain a predictable development cadence, the monolithic 34-complexity Epic `E-001` has been decomposed into three distinct, structured sub-tickets:
1.  **E-001a: Broker Lifecycle & Inception (8 Points):** Automatic dynamic instantiation of `UDiagnosticBroker` and `UCultivationBroker` during the Soul's `BeginPlay()` lifecycle phase using `NewObject<T>(this)`.
2.  **E-001b: Domain Namespace Isolation (5 Points):** Serialization boundaries partitioning scientific/historical measurements under `Sovereign.Truth` and progress variables under `Sovereign.Magic`.
3.  **E-001c: VSS & Paradox Coupling (8 Points):** Mathematical feedback loops where live curation status (`VettedBy`) mitigates unvetted telemetry uncertainty.

---

## 🛠️ 2. C++ Implementation Details

### A. Dynamic Instantiation & Registration (E-001a)
On `BeginPlay()`, the Soul Component checks for broker presence. If absent, it instantiates them with itself as the outer object, preventing garbage collection and ensuring correct ownership boundaries:

```cpp
void USovereignSaveableEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!DiagnosticBroker)
	{
		DiagnosticBroker = NewObject<UDiagnosticBroker>(this);
		RegisterBroker(DiagnosticBroker);
	}
	if (!CultivationBroker)
	{
		CultivationBroker = NewObject<UCultivationBroker>(this);
		RegisterBroker(CultivationBroker);
	}
    // ... rest of initialization
}
```

### B. Namespace Serialization Boundaries (E-001b)
Each broker encapsulates its own saving and loading logic, writing and reading strictly to distinct root JSON category keys:

*   **`UDiagnosticBroker` Key:** `"Sovereign.Truth"`
*   **`UCultivationBroker` Key:** `"Sovereign.Magic"`

Because of strict `HasField` validation during deserialization, either broker can fail to load (or be entirely absent) without causing save corruption or breaking the other domain's state.

### C. Non-Compensatory VSS Curation Coupling (E-001c)
System Confidence evaluates live physical certainty. If unhandled or conflicting meta-tags introduce a non-zero `ParadoxDensity` ($\Xi$), the system confidence falls ($1.0 - \Xi$).

However, if a vetted curator signs off on the telemetry (`VettedBy` is non-empty), 80% of current paradox/uncertainty density is mitigated, boosting system confidence back toward nominal bounds:

$$\text{System Confidence} = 1.0 - (\Xi \times \text{Mitigation Factor})$$

Where $\text{Mitigation Factor} = 0.2$ (80% mitigation) when `VettedBy` contains a valid curator string.

```cpp
float USovereignSaveableEntityComponent::GetSystemConfidence_Implementation() const
{
	float BaseConfidence = 1.0f - ParadoxDensity;

	// Coupling with the Diagnostic Broker's curation status (VettedBy indicator)
	if (DiagnosticBroker && !DiagnosticBroker->VettedBy.IsEmpty())
	{
		// Curator validation mitigates 80% of current paradox/uncertainty density, boosting the system confidence.
		BaseConfidence = 1.0f - (ParadoxDensity * 0.2f);
	}

	return FMath::Clamp(BaseConfidence, 0.0f, 1.0f);
}
```

---

## 🧪 3. Verification & Testing Protocol

To maintain extreme reliability, a complete integration test has been added to the test suite:
`WispCPP7VR Core/WispCPP7VR Core/WispCPP7VR/Source/WispCPP7VRTests/Private/SaveSystem/SovereignBrokerIntegrationTest.cpp`

### A. Running the Automation Tests in Unreal Engine
1.  Launch **Unreal Editor** for the project.
2.  Open the **Session Frontend** (Menu: `Tools -> Session Frontend` or `Window -> Test Automation`).
3.  Navigate to the **Automation** tab.
4.  Expand the tree to locate: **Sovereign -> Soul -> BrokerIntegration** (or query `Sovereign.Soul.BrokerIntegration` in the search bar).
5.  Check the box and click **Start Tests** in the toolbar.
6.  Ensure all assertions return **Green (Pass)**.

### B. Command-Line Telemetry Validation (Python Bridge)
Verify that the Iron Officer's local RAG and REST bridge handle simulation-side telemetry check-ins:
```bash
python Sovereign_Intelligence/IronOfficer/test_rag_and_psta.py
```
This tests standard communication routes to ensure that structural metadata changes do not block real-world IoT synchronizations.

---

## 📱 4. Blueprint & UI Integration

Both brokers and their fields are fully exposed as BlueprintCallable properties.

### A. Fetching Active Broker States in Widgets
Inside your nested base widgets (`WBP_BIO`, `WBP_Qi`):
1.  Retrieve the `USovereignSaveableEntityComponent` pointer (the `SoulHub`).
2.  Access the `DiagnosticBroker` or `CultivationBroker` property directly.
3.  Read exposed fields:
    *   **Truth Data:** Query `GetTruthValue(Key)` to retrieve custom Lidar or scientific strings.
    *   **Curation Status:** Read `VettedBy` to display Curator signatures.
    *   **Magic/Qi Level:** Read `QiBalance` and `CultivationTier` directly.

### B. Dynamically Modifying Broker Values
*   To add cultivation points from a possession milestone:
    `SoulHub -> CultivationBroker -> AddQi(50.0f)`
*   To approve diagnostic findings from a physical save terminal:
    `SoulHub -> DiagnosticBroker -> SetVettedBy("Curator_Spirit")`
