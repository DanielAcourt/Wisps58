// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: August 2026
# Standard Operating Procedure (SOP): UE 5.8 External Dependencies & Repository Hygene

This SOP addresses the architectural and operational guidelines for managing heavy binary asset dependencies (Uassets), engine-level plugin integrations (specifically the Unreal 5.8.1 MCP Server Plugin), and strict repository hygiene constraints within the Sovereign Framework.

---

## 🏛️ 1. Executive Summary & Strategy Matrix

When managing virtual reality assets and custom C++ extensions in Unreal Engine 5.8.1, keeping the primary source code repository lightweight, portable, and clean is paramount. Including heavy art assets (`.uasset`) and proprietary or large engine plugins in git leads to repository bloat, slow clone/pull times, and potential dependency resolution conflicts.

To solve this, we analyze three strategies for external assets and recommend a path forward:

### SWOT Analysis: Art Asset (Uasset) Externalization Strategies

| Dimension | Option A: Symlinks & Windows Directory Junctions | Option B: Placeholder / Stub Asset Pattern | Option C: JSON-Driven Metadata & Pull Script (Recommended) |
| :--- | :--- | :--- | :--- |
| **Strengths** | • Seamless Unreal Editor operation.<br>• Zero footprint inside git.<br>• Instant local folder population without extra script runs. | • Guarantees no missing class or variable dependencies at compile/editor boot.<br>• Minimal git size footprint (stubs are ~2-5KB). | • **Extremely robust and deterministic.**<br>• Fully aligned with JSON serialization preferences.<br>• High transparency of asset history via JSON metadata. |
| **Weaknesses**| • Requires Administrative privileges on Windows to build symlinks.<br>• Easy to break if drive letters change across systems. | • Requires manual local overrides with high-res art.<br>• Stubs can get accidentally committed back to the repo if not careful. | • Requires a run-time setup/sync script.<br>• Requires an external storage source (NAS, cloud bucket, local drive path) to pull from. |
| **Opportunities**| • Offloads asset management completely to a local high-speed SSD. | • Enables offline prototyping with lightweight "grey-box" stand-ins. | • Allows scriptable asset-on-demand streaming.<br>• Integrates perfectly with Python-based multi-agent context engines. |
| **Threats**  | • Git cannot track the existence of these external dependencies. | • Mismatched GUIDs or serialization signatures if stubs are modified incorrectly. | • Script failure could leave local projects in an uncompilable state. |

### Architectural Recommendation
**We strongly recommend Option C (JSON-Driven Metadata & Pull Script)** as the primary asset restoration mechanism, paired with an optional fallback to **Option A (Symlinks)** for developers with high local disk speeds and static workspace layouts.

Using Option C, we serialize asset dependencies into a lightweight JSON file (`AssetRegistry.json`). A Python script reads this JSON, checks if the local cache has the corresponding `.uasset` package, and copies/downloads it directly to the designated `Content/` subfolder.

---

## 🔌 2. Integration: Unreal 5.8.1 MCP Server Plugin

The Model Context Protocol (MCP) server plugin is a core component enabling real-time communications between Unreal Engine and local LLM clients (like our Iron Officer).

To prevent repository bloat, license issues, and binary build friction, this plugin is designated as an **Engine-Level Plugin (Option A)**.

### Installation & Gitignore Security Ticket Spec

*   **Ticket ID:** `B-045`
*   **Complexity:** 3 (ISTJ Tactician/DevOps)
*   **Title:** Unreal 5.8.1 MCP Server Plugin Integration & Gitignore Security
*   **Context:** The embedded loopback MCP server (defaulting to `http://127.0.0.1:8000/mcp`) is required for local agent execution. Keeping it inside the project directory invites accidental binary and cache pushes, inflating repository size.
*   **Completion Outcome:**
    1.  The plugin is registered inside the engine's host folder (e.g., `C:\Program Files\Epic Games\UE_5.8\Engine\Plugins\Marketplace\MCP_Server\`).
    2.  The `WispCPP7VR.uproject` lists the plugin as active but relies on engine-level resolution.
    3.  A comprehensive gitignore rule prevents the local `Plugins/MCP_Server/` folder (or any local duplicate) from ever staging.

```json
{
  "Name": "MCPServer",
  "Enabled": true
}
```

---

## 🧹 3. Repository Hygiene & Gitignore Reference

Unreal Engine C++ projects generate massive amounts of transient, binary, and temporary intermediate data during compilation, shader generation, and editor usage. Pushing these to git wastes bandwidth and causes file lock conflicts.

### The Standard Sovereign .gitignore Manifest
The following rules are implemented to completely ignore intermediate, binary, local configuration, and cache files:

```gitignore
# ==========================================
# Sovereign Framework: UE5.8 Gitignore Specification
# ==========================================

# Prerequisites & C++ Artifacts
*.d
*.slo
*.lo
*.o
*.obj
*.gch
*.pch
*.so
*.dylib
*.dll
*.mod
*.smod
*.lai
*.la
*.a
*.lib
*.exe
*.out
*.app

# Python & Environment
__pycache__/
*.py[cod]
*$py.class
*.bak
.env
.venv/
env/
venv/
pip-log.txt
pip-delete-this-directory.txt

# ==========================================
# Unreal Engine Specific Ignored Directories
# ==========================================
WispsCPPVR/Binaries/
WispsCPPVR/Intermediate/
WispsCPPVR/Saved/
WispsCPPVR/DerivedDataCache/
WispsCPPVR/Build/
WispsCPPVR/.vs/
WispsCPPVR/*.sln
WispsCPPVR/*.VC.db
WispsCPPVR/*.opensdf
WispsCPPVR/*.sdf
WispsCPPVR/*.suo
WispsCPPVR/*.filters
WispsCPPVR/*.user
WispsCPPVR/*.vbx
WispsCPPVR/*.pfx
WispsCPPVR/Plugins/*/Binaries/
WispsCPPVR/Plugins/*/Intermediate/

# Web Caching, Telemetry, and Crash Logs
WispsCPPVR/Saved/Logs/
WispsCPPVR/Saved/Crashes/
WispsCPPVR/Saved/Config/CrashReportClient/
WispsCPPVR/Saved/Config/WindowsEditor/
WispsCPPVR/Saved/Webcache/
WispsCPPVR/Saved/ShaderDebugInfo/
WispsCPPVR/Saved/Autosaves/

# Active DND Gameplay Campaigns & Chat Logs
Sovereign_Intelligence/IronOfficer/RolePlay/DungeonsAndDragons/Campaigns/
Sovereign_Intelligence/IronOfficer/RolePlay/DungeonsAndDragons/Sessions/
AI_Nexus/Memories/ChatHistory/Chat_SIM_*
```

---

## 📦 4. Web Caching & Repository Bloat Elimination

Unreal Engine often spins up web browsers or embedded HTTP services (such as for web views, bridge modules, or asset portals) which generate deep folders of transient webcache files inside `Saved/Webcache/`.

### Audit and Elimination Specification

*   **Ticket ID:** `B-046`
*   **Complexity:** 3 (ISTJ Tactician/DevOps)
*   **Title:** Web Caching & Repository Bloat Elimination Audit
*   **Context:** Transient web cache folders (`Saved/Webcache`, `Intermediate/Webcache`, `Saved/Logs`) dynamically accumulate hundreds of MBs of sqlite databases and local cookie stores. These files must be permanently blocked from staging, and any existing tracking must be purged.
*   **Completion Outcome:**
    1.  The `.gitignore` strictly targets `*Webcache*`, `*WebCache*`, and `Saved/Logs/`.
    2.  An audit run (`git rm --cached -r`) confirms no such files remain tracked.
    3.  A protocol is established for manually packing and uploading large log files/telemetry streams directly to Jules (the agent) when necessary, rather than committing them to the project git tree.

---

## 🛠️ 5. Implementation Guide: Local Sync Script (Option C)

Below is the design template for the Python-based JSON Asset Registry Synchronizer (`sync_external_assets.py`) to automate Option C.

```python
import os
import json
import shutil
import urllib.request

def sync_assets(registry_path="AssetRegistry.json", target_content_dir="WispsCPPVR/Content"):
    if not os.path.exists(registry_path):
        print(f"[!] Registry file not found: {registry_path}")
        return

    with open(registry_path, "r") as f:
        registry = json.load(f)

    for asset_id, info in registry.get("assets", {}).items():
        relative_path = info.get("relative_path")
        source_url = info.get("source_url")
        md5_checksum = info.get("checksum")

        destination_file = os.path.join(target_content_dir, relative_path)
        os.makedirs(os.path.dirname(destination_file), exist_ok=True)

        print(f"[*] Syncing: {asset_id} -> {destination_file}")

        # Download or copy from NAS/external cache
        if source_url.startswith("http"):
            urllib.request.urlretrieve(source_url, destination_file)
        else:
            shutil.copy2(source_url, destination_file)

    print("[+] Asset Synchronization Complete.")
```

Through this architecture, we preserve full compilation soundness, maintain light repository clones, and prevent transient runtime caches from polluting the Sovereign SSoT.
