# Universal Data Importer & Blueprint Synthesizer Plugin

**Target Engine:** Unreal Engine 5.8+ (Compatible back to UE 5.0)
**Version:** 1.0.0
**License:** GPLv3 / Commercial

## Overview
The **Universal Data Importer & Blueprint Synthesizer** is an editor plugin designed to parse CSV datasets, auto-generate or update `UDataAsset` instances, synthesize missing Blueprint actor assets derived from C++ / Blueprint base classes, and bind soft object references (Meshes, Materials, IMC Input Mapping Contexts) via Unreal reflection.

## Setup & Usage
1. Place the `UniversalDataImporter` plugin inside your project's `Plugins/` folder or Engine `Plugins/Marketplace/` directory.
2. Enable the **Python Script Plugin** and **Editor Scripting Utilities** plugins in your project settings.
3. Configure your mapping schema in `Resources/Schemas/DefaultMappingSchema.json`.
4. Run the Editor Utility Widget `EUW_UniversalDataImporter` or invoke `universal_data_importer.import_dataset()` from the Python console in Unreal Editor.

For full architectural details and Fab marketplace submission guides, see `AI_Nexus/Docs/17_Agnostic_DataImporter_Plugin_Architecture_Spec.md`.
