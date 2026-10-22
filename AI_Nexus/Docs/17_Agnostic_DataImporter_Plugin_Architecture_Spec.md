# Universal Data Importer & Blueprint Synthesizer (Fab Plugin Spec)
**Document ID:** `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`
**Version:** `1.0.0`
**Target Engine:** `Unreal Engine 5.8.0+` (Compatible with UE 5.0 - 5.7)
**Author:** Sovereign Framework Architecture Team / Jules
**Status:** Approved Architecture Draft

---

## 1. Executive Summary & Vision

The **Universal Data Importer & Blueprint Synthesizer** (`UniversalDataImporter`) is an agnostic, reusable Unreal Engine editor plugin (`.uplugin`) designed for distribution on Epic Games' Fab Marketplace as well as internal project deployment.

While current internal tools (e.g. `import_species_data_assets.py`) are tightly coupled to the Sovereign Framework C++ class signatures (`USovereignSpeciesData`, `ASovereignBaseEntity`, `USovereignAttributeComponent`, hardcoded CSV column names, and fixed package paths), this architecture specification defines a completely decoupled, schema-driven pipeline.

### Core Capabilities:
1. **Agnostic CSV / JSON Data Mapping:** Reads any tabular dataset or raw JSON/YAML and maps columns/fields to target `UDataAsset` or `UObject` properties via configurable mapping schemas.
2. **Automated Blueprint Asset Synthesizer:** Instantiates missing Blueprint actors from defined parent C++ / Blueprint classes and automatically binds synthesized DataAssets to Blueprint CDOs (Class Default Objects).
3. **Reflective Component & Attribute Injection:** Traverses actor components (including nested sub-components) via Unreal Reflection (`UProperty` / `FProperty` iteration) to update attributes without hardcoding component types.
4. **Input Mapping Context (IMC) & Controller Automation:** Automatically populates Enhanced Input contexts (`UInputMappingContext`) on synthesized Blueprint CDOs via interface contracts (`IInputConfigurable`) or property reflection.
5. **Asset Discovery & Auto-Binding Engine:** Scans target Content directories for Static Meshes (`SM_*`), Skeletal Meshes (`SK_*`), Materials (`MI_*`/`M_*`), and Sounds (`S_*`), auto-binding soft pointers dynamically.

---

## 2. Decoupling Architecture & Schema Design

To make the plugin project-agnostic, all hardcoded type strings and path conventions are extracted into a JSON or YAML mapping schema file (`DefaultMappingSchema.json`).

### 2.1 Sample Mapping Schema (`DefaultMappingSchema.json`)

```json
{
  "SchemaVersion": "1.0.0",
  "PluginName": "UniversalDataImporter",
  "TargetDataAssetClass": "/Script/Engine.DataAsset",
  "TargetPackageDirectory": "/Game/Data/GeneratedAssets",
  "AutoBindArtAssets": true,
  "ArtSearchDirectories": [
    "/Game/Assets/External",
    "/Game/ArtVault",
    "/Game/Assets"
  ],
  "ColumnToPropertyMap": [
    {
      "CSVColumn": "Name",
      "TargetProperty": "AssetDisplayName",
      "PropertyType": "Text",
      "FallbackValue": "Unnamed_Asset"
    },
    {
      "CSVColumn": "Tag",
      "TargetProperty": "IdentityTag",
      "PropertyType": "GameplayTag",
      "FallbackValue": "None"
    },
    {
      "CSVColumn": "Traits",
      "TargetProperty": "GameplayTraits",
      "PropertyType": "GameplayTagContainer",
      "Delimiter": ","
    },
    {
      "CSVColumn": "ActorClass",
      "TargetProperty": "TargetActorClass",
      "PropertyType": "SoftClassPath"
    }
  ],
  "NestedComponentPropertyMap": [
    {
      "ComponentClass": "/Script/Engine.ActorComponent",
      "CSVColumn": "BaseStrength",
      "TargetProperty": "Strength",
      "PropertyType": "Int32"
    },
    {
      "ComponentClass": "/Script/Engine.ActorComponent",
      "CSVColumn": "BaseConstitution",
      "TargetProperty": "Constitution",
      "PropertyType": "Int32"
    }
  ],
  "BlueprintSynthesisRules": {
    "bCreateMissingBlueprints": true,
    "DefaultParentClass": "/Script/Engine.Actor",
    "ParentClassColumn": "ParentClass",
    "CDOBindingProperty": "DataAssetRef",
    "DefaultInputMappingContext": "/Game/Input/IMC_Default"
  }
}
```

---

## 3. Reflective Component & Attribute Traversal Strategy

A key challenge when decoupling from framework-specific classes (e.g. `USovereignAttributeComponent`) is assigning values to components attached to an actor CDO.

### Strategy:
1. **CDO Component Inspection:** Upon creating or loading a Blueprint actor asset, the synthesizer queries the Blueprint's Simple Construction Script (`USimpleConstructionScript`) and Default Subobjects.
2. **Class & Property Matching:** For each component attached to the CDO:
   - The plugin inspects the component's class (`UClass`) using reflection.
   - It checks whether the class matches or derives from `ComponentClass` specified in the schema.
   - If a matching `FProperty` exists on the component (e.g. `Strength`, `Health`, `MetabolicRate`), the value parsed from the CSV row is set via reflection (`SetEditorProperty` in Python or `FProperty::ContainerPtrToValuePtr` in C++).
3. **Fallback Interface Contract:** Projects can optionally implement a lightweight interface (`IUniversalDataReceiver`) on components or actors to receive custom batch payload dictionaries post-synthesis.

---

## 4. SWOT Analysis: Mapping Architectures

| Category | Option A: JSON/YAML Mapping Schema (Selected) | Option B: Hardcoded Python Engine | Option C: C++ UDeveloperSettings |
|:---|:---|:---|:---|
| **Strengths** | • Highly portable across projects without re-compiling C++.<br>• Human-readable, easily edited in VSCode/text editors.<br>• Non-programmers can create custom templates. | • Simple to write initially.<br>• Fast execution speed. | • Native Unreal Editor Settings panel UI integration.<br>• Strongly typed and validated in C++. |
| **Weaknesses** | • Requires runtime JSON parsing and path string resolution.<br>• Syntax errors in JSON can break imports if not validated. | • Inflexible; requires code edits for any schema change.<br>• Fails Fab marketplace quality standards. | • Requires project re-compile to add new property mappings.<br>• Harder for non-C++ designers to share schemas. |
| **Opportunities** | • Can build visual EUW drag-and-drop schema mapping UI.<br>• Standardized format for Fab marketplace distribution. | • Ideal for rapid internal prototyping. | • Direct C++ performance for massive (100,000+ row) imports. |
| **Threats** | • Potential schema drift between plugin updates if versioning isn't enforced. | • Deprecation risks when UE Python API changes across engine versions. | • Engine upgrade breaking changes to UDeveloperSettings APIs. |

---

## 5. Input Mapping Context (IMC) & Possession Binding

When generating Blueprint actors from CSV templates:
1. **Default IMC Assignment:** The synthesizer checks the schema's `BlueprintSynthesisRules.DefaultInputMappingContext` soft path. If valid, it binds the `UInputMappingContext` asset to the CDO's input component or default IMC property container.
2. **Interface Fallback:** Blueprints can implement `IInputConfigurable` interface. The importer invokes `SetupDefaultMappingContext(IMC)` on the CDO during synthesis.
3. **Possession Spirit Fallback:** If the actor is unpossessed or possessed at runtime, input bindings fall back to querying the controller/possessing pawn for active mapping contexts.

---

## 6. Engine Version & Fab Marketplace Standards

### 6.1 Version Compatibility
- **Target Version:** Unreal Engine `5.8.0`
- **Backwards Compatibility:** Compatible back to `UE 5.0` by using core `UnrealEd`, `AssetTools`, `EditorScriptingUtilities`, and Python `unreal` module bindings available since UE 4.26+.
- **Pre-processor / Python Guards:** All UE 5.8-specific Python/C++ API calls are wrapped in version guards or `try/except` blocks to maintain compatibility across engine releases.

### 6.2 Fab Marketplace Compliance Checklist
- [x] No hardcoded project paths (`/Game/WispCPP7VR/...`).
- [x] Agnostic asset namespace and configurable default output folders (`/Game/Data/GeneratedAssets`).
- [x] Includes sample CSV template (`SampleSpeciesTemplate.csv`) and sample schema (`DefaultMappingSchema.json`).
- [x] Includes full Editor Utility Widget (`EUW_UniversalDataImporter`).
- [x] Non-destructive execution: dirty package saving before run and clean error logging.

---

## 7. Proposed Plugin Directory Structure

```
Plugins/UniversalDataImporter/
├── UniversalDataImporter.uplugin
├── Config/
│   └── FilterPlugin.ini
├── Content/
│   ├── Python/
│   │   ├── universal_data_importer.py
│   │   ├── schema_parser.py
│   │   └── init_unreal.py
│   ├── UI/
│   │   └── EUW_UniversalDataImporter.uasset
│   └── Resources/
│       ├── Schemas/
│       │   └── DefaultMappingSchema.json
│       └── SampleTemplates/
│           └── SampleSpeciesTemplate.csv
├── Source/
│   └── UniversalDataImporter/
│       ├── Private/
│       │   ├── UniversalDataImporterModule.cpp
│       │   └── Interfaces/IUniversalDataReceiver.cpp
│       └── Public/
│           ├── UniversalDataImporterModule.h
│           └── Interfaces/IUniversalDataReceiver.h
└── README.md
```

---

## 8. Sprint Ticket Specification (Ticket B-065)

To integrate this work into the active sprint (`SR_20261026`), Ticket `B-065` is added to `AI_Nexus/DevOps/CURRENT_SPRINT.md`:

- **Ticket ID:** `B-065`
- **Title:** Agnostic CSV DataImporter UE5.8 Plugin Architecture & Fab Packaging Protocol
- **Complexity:** 5 Points
- **Status:** In Research / Architecture Approved
- **Node:** DevOps / Tools
- **Why:** Existing CSV importer scripts are tied to Sovereign C++ classes and fixed project folders. An agnostic plugin architecture is required for project reusability and Fab marketplace release.
- **What:** Deliver `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`, initialize `Plugins/UniversalDataImporter/` directory structure with sample schema JSON and template CSV, and outline reflective attribute component mapping.
