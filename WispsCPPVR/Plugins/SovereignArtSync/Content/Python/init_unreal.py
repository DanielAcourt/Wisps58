#!/usr/bin/env python3
"""
sovereign-art-sync: Editor Startup Initializer
Registers Python tools into Unreal Engine Editor on startup.
"""

import sys
from pathlib import Path

plugin_python_dir = Path(__file__).resolve().parent
if str(plugin_python_dir) not in sys.path:
    sys.path.insert(0, str(plugin_python_dir))

try:
    import unreal
    unreal.log("Sovereign Art Sync Plugin initialized successfully.")
except ImportError:
    pass
