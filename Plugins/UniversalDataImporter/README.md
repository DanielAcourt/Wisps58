# Universal Data Importer & Blueprint Synthesizer Plugin

**Target Engine:** Unreal Engine 5.8+ (Compatible back to UE 5.0)
**Version:** 1.0.0
**License:** GPLv3 / Commercial

## Overview
The **Universal Data Importer & Blueprint Synthesizer** is an agnostic editor plugin designed to parse CSV datasets, auto-generate or update `UDataAsset` instances, synthesize missing Blueprint actor assets derived from C++ / Blueprint base classes, and bind soft object references (Meshes, Materials, IMC Input Mapping Contexts) via Unreal reflection.

## Key Features
1. **Agnostic JSON Schema Mapping:** Decouples CSV column headers from C++ property names.
2. **Reverse Class Exporter (`export_csv_template`):** Reflects ANY user `UClass` (DataAsset, Actor, or Component) and exports a sample CSV template with headers and a matching JSON mapping schema.
3. **Reflective Component Injection:** Dynamically traverses sub-components attached to Blueprint CDOs without requiring specific C++ base classes.

## Setup & Usage
1. Place the `UniversalDataImporter` plugin inside your project's `Plugins/` folder or Engine `Plugins/Marketplace/` directory.
2. Enable the **Python Script Plugin** and **Editor Scripting Utilities** plugins in your project settings.
3. To generate a starter CSV template and schema from any existing class:
   ```python
   export_csv_template("/Script/Engine.DataAsset", "MyTemplate.csv", "MySchema.json")
   ```
4. To import CSV data using a schema:
   ```python
   universal_import_csv("MyData.csv", "MySchema.json")
   ```

For full architectural details and Fab marketplace submission guides, see `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`.
