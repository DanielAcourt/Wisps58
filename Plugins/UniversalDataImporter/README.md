# Universal Data Importer & Blueprint Synthesizer Plugin

**Target Engine:** Unreal Engine 5.8+ (Compatible back to UE 5.0)
**Version:** 1.0.0
**License:** GPLv3 / Commercial

## Overview
The **Universal Data Importer & Blueprint Synthesizer** is an agnostic editor plugin designed to parse CSV datasets, auto-generate or update `UDataAsset` instances, synthesize missing Blueprint actor assets derived from C++ / Blueprint base classes, and bind soft object references (Meshes, Materials, IMC Input Mapping Contexts) via Unreal reflection.

Out-of-the-box, the plugin includes `UUniversalDataAsset` (a default C++ Primary DataAsset with `DisplayName`, `IdentityTag`, `Tags`, `ActorClass`, `Icon`, `Description`, and `Health`).

## Key Features
1. **Default C++ Base Class (`UUniversalDataAsset`):** Ships with a clean, extensible C++ Primary DataAsset ready for immediate use.
2. **Agnostic JSON Schema Mapping:** Decouples CSV column headers from C++ property names.
3. **Reverse Class Exporter (`export_csv_template`):** Reflects ANY user `UClass` (DataAsset, Actor, or Component) and exports a sample CSV template with headers and a matching JSON mapping schema.
4. **Reflective Component Injection:** Dynamically traverses sub-components attached to Blueprint CDOs without requiring specific C++ base classes.

## Quick Start Guide (Standalone Project)

### Option A: Using Python Builtin Commands

1. **Generate Template & Schema from Any Class:**
   ```python
   # Defaults to UUniversalDataAsset out-of-the-box
   export_csv_template("/Script/UniversalDataImporter.UniversalDataAsset", "C:/MyProject/Data/MyTemplate.csv", "C:/MyProject/Data/MySchema.json")
   ```

2. **Import CSV Data & Synthesize Blueprints:**
   ```python
   universal_import_csv("C:/MyProject/Data/MyTemplate.csv", "C:/MyProject/Data/MySchema.json")
   ```

### Option B: Using Editor Utility Widget (EUW Bridge)

The plugin provides `euw_bridge.py` functions for Editor Utility Widgets:
- `euw_generate_template(class_path, output_csv, output_schema)`
- `euw_execute_import(csv_path, schema_path, b_overwrite, b_auto_bind_art)`

For full architectural details and Fab marketplace submission guides, see `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`.
