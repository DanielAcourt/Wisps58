# Universal Data Importer & Blueprint Synthesizer Plugin

**Target Engine:** Unreal Engine 5.8+ (Compatible back to UE 5.0)
**Version:** 1.0.0
**License:** GPLv3 / Commercial

## Overview
The **Universal Data Importer & Blueprint Synthesizer** is an agnostic editor plugin designed to parse CSV datasets, auto-generate or update `UDataAsset` instances, synthesize missing Blueprint actor assets derived from C++ / Blueprint base classes, and bind soft object references (Meshes, Materials, IMC Input Mapping Contexts) via Unreal reflection.

## Out-Of-The-Box C++ Classes
1. **`UUniversalDataAsset`:** Primary DataAsset containing `DisplayName`, `IdentityTag`, `Tags`, `ActorClass`, `Mesh`, `SkeletalMesh`, `Material`, `Icon`, `Description`, and `Health`.
2. **`AUniversalActor`:** Base Actor class with a root `UStaticMeshComponent` and `DataAssetRef` variable. Automatically loads and renders `DataAssetRef->Mesh` and `DataAssetRef->Material` synchronously inside `OnConstruction` and `BeginPlay` so 3D models display immediately in the Editor Viewport and at runtime!

## Key Features
1. **Default C++ Base Classes (`UUniversalDataAsset` & `AUniversalActor`):** Ships with clean, extensible C++ classes ready for immediate use.
2. **Automatic 3D Mesh Rendering:** Synthesized Blueprints inheriting from `AUniversalActor` render their assigned 3D static mesh and material directly in the Editor Viewport.
3. **Agnostic JSON Schema Mapping:** Decouples CSV column headers from C++ property names.
4. **Reverse Class Exporter (`export_csv_template`):** Reflects ANY user `UClass` (DataAsset, Actor, or Component) and exports a sample CSV template with headers and a matching JSON mapping schema.
5. **Injective Component Attribute Reflection:** Traverses attached CDO sub-components and updates matching properties.

## Quick Start Guide (Standalone Project)

### Option A: 0-Parameter Execution (Smart Defaults)

1. **Generate Starter CSV & Schema:**
   ```python
   export_csv_template()
   ```

2. **Import CSV Data & Auto-Render 3D Actors:**
   ```python
   universal_import_csv()
   ```

### Option B: Using Editor Utility Widget (EUW Bridge)

The plugin provides `euw_bridge.py` functions for Editor Utility Widgets:
- `euw_generate_template(class_path, output_csv, output_schema)`
- `euw_execute_import(csv_path, schema_path, b_overwrite, b_auto_bind_art)`

For full architectural details and Fab marketplace submission guides, see `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`.
