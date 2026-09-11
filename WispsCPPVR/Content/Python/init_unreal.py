# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-27
"""
Sovereign Framework UE 5.8.1 Python Initialization Script
Automatically executed by Unreal Engine upon Python plugin startup.
"""
import builtins
import unreal
import sovereign_mcp_tools

unreal.log("🏛️ Sovereign Framework: Initializing Python MCP Tools for UE 5.8.1...")
sovereign_mcp_tools.register_sovereign_tools()

try:
    import sovereign_art_sync_plugin
    from sovereign_art_sync_plugin import execute_1click_sync, execute_artist_friendly_sync, execute_dry_run_autodiscover, get_last_sync_log

    builtins.sovereign_art_sync_plugin = sovereign_art_sync_plugin
    builtins.execute_1click_sync = execute_1click_sync
    builtins.execute_artist_friendly_sync = execute_artist_friendly_sync
    builtins.execute_dry_run_autodiscover = execute_dry_run_autodiscover
    builtins.get_last_sync_log = get_last_sync_log
    unreal.log("🏛️ Sovereign Framework: Art Sync Tools registered into Python builtins context.")
except Exception as e:
    unreal.log_warning(f"🏛️ Sovereign Framework: Art Sync pre-load notice: {e}")
