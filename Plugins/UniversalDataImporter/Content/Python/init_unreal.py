# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer Plugin Auto-Initialization
"""
Auto-executes upon Unreal Editor startup when PythonScriptPlugin is enabled.
Registers universal_import_csv and export_csv_template into Python builtins.
"""

import sys
import builtins
from pathlib import Path

plugin_python_dir = Path(__file__).resolve().parent
if str(plugin_python_dir) not in sys.path:
    sys.path.insert(0, str(plugin_python_dir))

try:
    import universal_data_importer

    builtins.universal_import_csv = universal_data_importer.import_csv_with_schema
    builtins.export_csv_template = universal_data_importer.export_csv_template_from_class

    print("✨ [UniversalDataImporter Plugin] Successfully initialized builtins: 'universal_import_csv' & 'export_csv_template'.")
except Exception as e:
    print(f"⚠️ [UniversalDataImporter Plugin] Initialization notice: {e}")
