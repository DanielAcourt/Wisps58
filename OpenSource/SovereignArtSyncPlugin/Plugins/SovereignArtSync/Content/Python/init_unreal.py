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
    import builtins
    import sovereign_art_sync_plugin
    from sovereign_art_sync_plugin import execute_1click_sync, execute_dry_run_autodiscover, get_last_sync_log

    # Bind into builtins so execute_1click_sync() works directly in UE Output Log console
    builtins.sovereign_art_sync_plugin = sovereign_art_sync_plugin
    builtins.execute_1click_sync = execute_1click_sync
    builtins.execute_dry_run_autodiscover = execute_dry_run_autodiscover
    builtins.get_last_sync_log = get_last_sync_log

    import unreal
    unreal.log("Sovereign Art Sync Plugin initialized. 'execute_1click_sync()' and 'sovereign_art_sync_plugin' are now ready in Output Log.")
except Exception as e:
    try:
        import unreal
        unreal.log_warning(f"Sovereign Art Sync Plugin initialization notice: {e}")
    except ImportError:
        pass
