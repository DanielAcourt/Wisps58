# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer: Editor Utility Widget Bridge
"""
Execution bridge for Editor Utility Widgets (EUW).
Saves dirty packages before execution and returns clear summary logs for EUW UI Text Blocks.
"""

import os
import sys
from pathlib import Path

current_dir = Path(__file__).resolve().parent
if str(current_dir) not in sys.path:
    sys.path.insert(0, str(current_dir))

import universal_data_importer

try:
    import unreal
    IN_UNREAL = True
except ImportError:
    IN_UNREAL = False


def save_dirty_packages():
    """Saves all unsaved packages before performing asset operations."""
    if IN_UNREAL:
        try:
            unreal.EditorLoadingAndSavingUtils.save_dirty_packages(show_dialog_if_dirty=False, save_map_packages=False)
        except Exception:
            pass


def execute_template_generator_ui(class_path="/Script/UniversalDataImporter.UniversalDataAsset", output_csv="", output_schema=""):
    """
    Called by EUW Section 1: "Generate Starter CSV & Schema"
    """
    save_dirty_packages()

    if not class_path or not class_path.strip():
        class_path = "/Script/UniversalDataImporter.UniversalDataAsset"

    if not output_csv or not output_csv.strip():
        output_csv = os.path.abspath(os.path.join(current_dir, "..", "Resources", "SampleTemplates", "GeneratedTemplate.csv"))

    if not output_schema or not output_schema.strip():
        output_schema = os.path.abspath(os.path.join(current_dir, "..", "Resources", "Schemas", "GeneratedSchema.json"))

    # Sanitize backslashes for Windows path strings in EUW
    output_csv = str(output_csv).strip().strip("'").strip('"').replace("\\", "/")
    output_schema = str(output_schema).strip().strip("'").strip('"').replace("\\", "/")

    success = universal_data_importer.export_csv_template_from_class(
        class_path=class_path,
        output_csv_path=output_csv,
        output_schema_path=output_schema
    )

    if success:
        msg = f"[SUCCESS] Generated CSV Template at '{output_csv}' and Schema at '{output_schema}'."
    else:
        msg = f"[ERROR] Failed to generate template from class '{class_path}'."

    print(msg)
    if IN_UNREAL:
        if success: unreal.log(msg)
        else: unreal.log_error(msg)

    return msg


def execute_importer_ui(csv_path="", schema_path="", b_overwrite=True, b_auto_bind_art=True):
    """
    Called by EUW Section 2: "1-Click Import Data"
    """
    save_dirty_packages()

    if not csv_path or not csv_path.strip():
        csv_path = os.path.abspath(os.path.join(current_dir, "..", "Resources", "SampleTemplates", "SampleSpeciesTemplate.csv"))

    if not schema_path or not schema_path.strip():
        schema_path = os.path.abspath(os.path.join(current_dir, "..", "Resources", "Schemas", "DefaultMappingSchema.json"))

    csv_path = str(csv_path).strip().strip("'").strip('"').replace("\\", "/")
    schema_path = str(schema_path).strip().strip("'").strip('"').replace("\\", "/")

    success = universal_data_importer.import_csv_with_schema(
        csv_path=csv_path,
        schema_path=schema_path,
        b_overwrite=b_overwrite,
        b_auto_bind_art=b_auto_bind_art
    )

    if success:
        msg = f"[SUCCESS] Successfully imported dataset from '{csv_path}' using schema '{schema_path}'."
    else:
        msg = f"[ERROR] Batch import failed for CSV '{csv_path}'."

    print(msg)
    if IN_UNREAL:
        if success: unreal.log(msg)
        else: unreal.log_error(msg)

    return msg
