# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-27
"""
Sovereign Framework UE 5.8.1 Python Initialization Script
Automatically executed by Unreal Engine upon Python plugin startup.
"""
import unreal
import sovereign_mcp_tools

unreal.log("🏛️ Sovereign Framework: Initializing Python MCP Tools for UE 5.8.1...")
sovereign_mcp_tools.register_sovereign_tools()
