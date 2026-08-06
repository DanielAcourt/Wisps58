# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
"""
Sovereign Framework: Iron Officer Bridge (AD-001/AD-002/AD-004)
A local FastAPI bridge connecting Unreal Engine/Raspberry Pi to the Lead's GTX 5090.
Enhanced with AAS Protocol v1.3.0 and PSTA Viability Checks.
"""

import os
import json
import subprocess
import requests
import datetime
import argparse
import re
import shutil
import time
import asyncio
import requests
import uvicorn
import logging
from typing import Dict, Any, List, Optional, Set
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field

# Setup secure AAS logging channel
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("AAS.SovereignBridge")

app = FastAPI(title="Sovereign Iron Officer Bridge")

from rag import SovereignRAG

# --- Configuration ---
VERSION = "36.4.7-Knight-AAS"
BASE_DIR = os.path.dirname(__file__)
CONFIG_PATH = os.path.join(BASE_DIR, "config.json")
REPO_ROOT = os.path.abspath(os.path.join(BASE_DIR, "..", ".."))
REPO_NAME = os.path.basename(REPO_ROOT)
OLLAMA_HOST = "http://127.0.0.1:11434"
TARGET_MODEL = "llama3.1:latest"
BRIDGE_PORT = 8000
USER_NAME = "Dan"
READ_ZONES = []
WRITE_ZONES = []
ROLEPLAY_ZONES = []
PERSONA_ZONES = {}
REMOTE_HISTORY_ENABLED = False
HISTORY_DIR = os.path.join(REPO_ROOT, "AI_Nexus", "Memories", "ChatHistory")
PERSISTENT_HANDSHAKE = False
last_active_file = None

# --- RAG Global Configuration ---
RAG_ENABLED = True
RAG_ENABLED_ON_STARTUP = True
RAG_CONTEXT_WEIGHT = 0.5
RAG_MAX_CHUNKS = 3
RAG_SIM_THRESHOLD = 0.05
RAG_CHUNK_SIZE_WORDS = 250
RAG_INDEX_DIRS = ["AI_Nexus"]
RAG_IGNORED_DIRS = ["protocols", "devops", "admin", "bridge.py"]

rag_engine = SovereignRAG(REPO_ROOT, chunk_size_words=RAG_CHUNK_SIZE_WORDS)

# --- AAS/PSTA Constants ---
PERSONA_PRECEDENCE = {
    "Lead": 10,
    "Tactician": 9,
    "Strategist": 8,
    "Researcher": 7,
    "Archivist_Theta": 6,
    "Iron_Knight": 5,
    "Unreal_Simulation": 5,
    "Yaz_Student": 3
}

PERSONA_CREDIBILITY = {
    "Lead": 1.0,
    "Tactician": 0.9,
    "Strategist": 0.85,
    "Researcher": 0.75,
    "Archivist_Theta": 0.70,
    "Iron_Knight": 0.50,
    "Unreal_Simulation": 0.50,
    "Yaz_Student": 0.30
}

PROTECTED_NODES = {
    "AI_Nexus/Protocols/AAS_Protocol.md": 10,
    "Sovereign_Intelligence/IronOfficer/bridge.py": 10,
    "AI_Nexus/INDEX.md": 9,
    "AI_Nexus/Protocols/AGENTS.md": 9,
    "WispCPP7VR Core/": 8,
    "AI_Nexus/DevOps/": 7
}

TOOL_MIN_PRECEDENCE = {
    "delete_file": 10,
    "write_file": 8,
    "read_file": 3,
    "list_files": 3,
    "get_system_telemetry": 5,
    "map_directory": 3,
    "search_files": 5,
    "push_telemetry": 5,
    "push_chat_to_unreal": 5
}

def load_config():
    global OLLAMA_HOST, TARGET_MODEL, BRIDGE_PORT, USER_NAME, READ_ZONES, WRITE_ZONES, ROLEPLAY_ZONES, PERSONA_ZONES, REMOTE_HISTORY_ENABLED, HISTORY_DIR
    global RAG_ENABLED, RAG_ENABLED_ON_STARTUP, RAG_CONTEXT_WEIGHT, RAG_MAX_CHUNKS, RAG_SIM_THRESHOLD, RAG_CHUNK_SIZE_WORDS, RAG_INDEX_DIRS, RAG_IGNORED_DIRS
    global PERSISTENT_HANDSHAKE
    if os.path.exists(CONFIG_PATH):
        try:
            with open(CONFIG_PATH, "r") as f:
                cfg = json.load(f)
                ollama_cfg = cfg.get("ollama", {})
                bridge_cfg = cfg.get("bridge", {})
                pref_cfg = cfg.get("user_preference", {})

                OLLAMA_HOST = f"http://{ollama_cfg.get('host', '127.0.0.1')}:{ollama_cfg.get('port', 11434)}"
                TARGET_MODEL = ollama_cfg.get("target_model", "llama3.1:latest")
                BRIDGE_PORT = bridge_cfg.get("port", 8000)

                READ_ZONES = [os.path.abspath(os.path.join(REPO_ROOT, p)) for p in bridge_cfg.get("read_zones", [])]
                WRITE_ZONES = [os.path.abspath(os.path.join(REPO_ROOT, p)) for p in bridge_cfg.get("write_zones", [])]
                ROLEPLAY_ZONES = [os.path.abspath(os.path.join(REPO_ROOT, p)) for p in bridge_cfg.get("roleplay_zones", [])]

                # [B-025] Ingest Persona-Specific Safe Zones
                PERSONA_ZONES = bridge_cfg.get("persona_zones", {
                    "Iron_Knight": ["Sovereign_Intelligence/IronKnight_Memory", "IronKnight/"]
                })

                PERSISTENT_HANDSHAKE = bridge_cfg.get("persistent_handshake", False)

                USER_NAME = pref_cfg.get("name", "Dan")

                # [AD-006] Remote History Configuration
                REMOTE_HISTORY_ENABLED = bridge_cfg.get("remote_history_enabled", False)
                HISTORY_DIR = os.path.abspath(os.path.join(REPO_ROOT, bridge_cfg.get("history_dir", "AI_Nexus/Memories/ChatHistory")))

                # [AD-003] Ingest RAG and local memory settings
                rag_cfg = cfg.get("rag_settings", {})
                RAG_ENABLED = rag_cfg.get("enabled", True)
                RAG_ENABLED_ON_STARTUP = rag_cfg.get("enabled_on_startup", True)
                RAG_CONTEXT_WEIGHT = rag_cfg.get("rag_context_weight", 0.5)
                RAG_MAX_CHUNKS = rag_cfg.get("max_chunks", 3)
                RAG_SIM_THRESHOLD = rag_cfg.get("similarity_threshold", 0.05)
                RAG_CHUNK_SIZE_WORDS = rag_cfg.get("chunk_size_words", 250)
                RAG_INDEX_DIRS = rag_cfg.get("index_dirs", ["AI_Nexus"])
                RAG_IGNORED_DIRS = rag_cfg.get("ignored_dirs", ["protocols", "devops", "admin", "bridge.py"])

                # Update the active RAG engine settings dynamically
                rag_engine.chunk_size_words = RAG_CHUNK_SIZE_WORDS
                if RAG_ENABLED and RAG_ENABLED_ON_STARTUP and not rag_engine.chunks:
                    try:
                        logger.info("AAS RAG: Auto-building knowledge index from AI_Nexus...")
                        rag_engine.build_index(RAG_INDEX_DIRS)
                    except Exception as e:
                        logger.error(f"AAS RAG: Failed to auto-build index on startup: {e}")

        except Exception as e:
            logger.warning(f"[07 WARNING] Failed to load config.json: {e}")

load_config()

# Global state for the 07 Salute
NEXUS_PATH = "Unknown"
HARDWARE_ID = "GTX 5090 (Assumed)"
IS_HARD_FREEZE = False
HANDSHAKE_ACTIVE = False
latest_rag_similarity_score = 1.0
latest_session_char_count = 0

# Map actor_name to list of pending messages queued by the AI
unreal_mailbox: Dict[str, List[str]] = {}

# Cached active simulation states keyed by normalized actor name
active_simulation_states: Dict[str, Dict[str, Any]] = {}

def format_save_state(state: Dict[str, Any]) -> str:
    """Formats raw JSON save state into a clean, hierarchical YAML-like structure."""
    if not state:
        return "No active simulation state registered."
    lines = []

    # 1. Identity
    identity = state.get("Identity", {})
    if identity:
        lines.append("Entity Identity:")
        for k, v in identity.items():
            lines.append(f"  - {k}: {v}")

    # 2. Modular components
    for cat in ["Bio", "Qi", "Elements", "Attributes", "Sovereign.Truth", "Sovereign.Magic"]:
        cat_data = state.get(cat, {})
        if cat_data:
            lines.append(f"{cat} Component State:")
            for k, v in cat_data.items():
                lines.append(f"  - {k}: {v}")

    # 3. Flat owner keys (any key not in registered namespaces/categories)
    known_categories = {"Identity", "UnknownTags", "Bio", "Qi", "Elements", "Attributes", "Sovereign.Truth", "Sovereign.Magic"}
    owner_keys = [k for k in state if k not in known_categories]
    if owner_keys:
        lines.append("Simulation Environment & Surroundings:")
        for k in owner_keys:
            lines.append(f"  - {k}: {state[k]}")

    # 4. UnknownTags / Paradox
    unknown = state.get("UnknownTags", {})
    if unknown:
        lines.append("Simulation Paradox & Meta-Tags:")
        for k, v in unknown.items():
            lines.append(f"  - {k}: {v}")

    return "\n".join(lines)

# --- Schemas ---
class PSTAMetadata(BaseModel):
    """Machine-readable PSTA metadata embedded in payloads."""
    P: int = Field(..., ge=1, le=10)
    S: float = Field(..., ge=0.0, le=1.0)
    T: str  # DRAFT, INNOVATION, STAGING, PROD
    A: str  # UNVERIFIED, SIGNED, HARDENED

class AgentCommandPayload(BaseModel):
    persona: str
    command: str
    target_node: str
    parameters: Dict[str, Any] = {}
    meta_tags: Dict[str, Any] = Field(default_factory=dict)

class PSTATelemetry(BaseModel):
    pillar: str  # P, S, T, A
    value: float
    metadata: Dict[str, Any] = {}

class VSSRequest(BaseModel):
    telemetry: List[PSTATelemetry]
    context: str = ""

class ChatMessage(BaseModel):
    role: str
    content: str
    name: Optional[str] = None
    tool_calls: Optional[List[Dict[str, Any]]] = None

class ChatRequest(BaseModel):
    messages: List[ChatMessage]
    context: Optional[str] = ""

class VerifyRequest(BaseModel):
    persona: str
    target_node: str
    command: str = "read_file"

class ToolRequest(BaseModel):
    persona: str
    command: str
    arguments: Dict[str, Any]

class UnrealCheckInRequest(BaseModel):
    client_name: str = "Unreal_Engine"
    version: str = "36.4.7"

class UnrealTelemetryPayload(BaseModel):
    entity_id: str
    psta_score: float
    blackbox_data: Dict[str, Any]
    persona: str = "Unreal_Simulation"

class UnrealChatRequest(BaseModel):
    actor_name: str
    message: str
    history: List[ChatMessage] = Field(default_factory=list)
    enable_remote_history: bool = False
    save_state: Optional[Dict[str, Any]] = None

class PushChatPayload(BaseModel):
    actor_name: str
    message: str

# --- AAS Bridge Logic ---

class SovereignBridge:
    def __init__(self, is_hard_freeze: bool = False):
        self.is_hard_freeze = is_hard_freeze

    def calculate_psta_viability(self, payload: AgentCommandPayload) -> float:
        """
        [R-009] Mathematically codifies agent precedence weights.
        """
        breakdown = self.get_vss_breakdown(payload)
        return breakdown["vss"]

    def get_vss_breakdown(self, payload: AgentCommandPayload) -> Dict[str, Any]:
        """Calculates and returns the detailed VSS breakdown."""
        credibility = PERSONA_CREDIBILITY.get(payload.persona, 0.4)

        risk = 0.2
        for node, required_p in PROTECTED_NODES.items():
            if node in payload.target_node or node in payload.command:
                risk = max(risk, (required_p / 10.0))

        known_tags = {"version", "stage", "timestamp", "aas_score"}
        unknown_tags_count = len([t for t in payload.meta_tags if t not in known_tags])
        deviation = min(1.0, unknown_tags_count * 0.15)

        memory_boost = 0.0
        if self.is_persona_in_memory_zone(payload.persona, payload.target_node):
            memory_boost = 0.5

        # [AAS v1.3.3] Handshake Boost
        global HANDSHAKE_ACTIVE
        handshake_boost = 0.5 if HANDSHAKE_ACTIVE else 0.0

        alpha, beta, gamma = 1.0, 0.3, 0.2
        vss = (alpha * credibility) - (beta * risk) - (gamma * deviation) + memory_boost + handshake_boost
        vss = max(0.0, min(1.0, vss))

        return {
            "vss": vss,
            "credibility": credibility,
            "risk": risk,
            "deviation": deviation,
            "memory_boost": memory_boost,
            "alpha": alpha,
            "beta": beta,
            "gamma": gamma
        }

    def is_persona_in_memory_zone(self, persona: str, target_node: str) -> bool:
        """Checks if a persona is operating within its dedicated memory folder."""
        zones = PERSONA_ZONES.get(persona, [])
        normalized_target = to_forward_slash(target_node).lower()
        for zone in zones:
            normalized_zone = to_forward_slash(zone).lower()
            if normalized_zone in normalized_target or normalized_target.startswith(normalized_zone):
                return True
        return False

    def is_roleplay_zone(self, target_node: str) -> bool:
        """Checks if target node is inside any of the configured unrestricted roleplay folders."""
        abs_target = to_forward_slash(resolve_secure_path(target_node)).lower()
        global ROLEPLAY_ZONES
        for zone in ROLEPLAY_ZONES:
            normalized_zone = to_forward_slash(zone).lower()
            if abs_target == normalized_zone or abs_target.startswith(normalized_zone + "/"):
                return True
        return False

    def evaluate_intent_safety(self, payload: AgentCommandPayload) -> bool:
        destructive_keywords = ["delete", "remove", "rm", "unlink", "truncate", "drop"]
        normalized_cmd = payload.command.lower()

        is_destructive = any(kw in normalized_cmd for kw in destructive_keywords)
        persona_p = PERSONA_PRECEDENCE.get(payload.persona, 3)

        # [B-024] Persona Memory Zone Override
        # Grant personas full authority within their dedicated memory folder
        is_persona_memory_zone = self.is_persona_in_memory_zone(payload.persona, payload.target_node)

        # 1. Global Tool Precedence Check (All Tools) - Bypassed in Persona Memory Zone or when Handshake is active
        global HANDSHAKE_ACTIVE
        if not is_persona_memory_zone and not HANDSHAKE_ACTIVE:
            if persona_p < TOOL_MIN_PRECEDENCE.get(payload.command, 10):
                 logger.critical(f"AAS ALERT: Insufficient precedence for tool '{payload.command}' by {payload.persona}")
                 return False

        # 2. Destructive Intent Checks
        if is_destructive:
            if self.is_hard_freeze:
                logger.critical(f"AAS ALERT: Blocked destructive op during HARD FREEZE by {payload.persona}")
                return False

            # Protected nodes are NEVER mutable by lower precedence, even in memory zones
            if any(node in payload.target_node for node in PROTECTED_NODES):
                logger.critical(f"AAS ALERT: Structural mutation blocked on protected node: {payload.target_node}")
                return False

        return True

    def calculate_logical_discrepancy(self, payload: AgentCommandPayload) -> float:
        """
        [AFI Integration] Calculates the internal Logical Discrepancy (Delta_logic) Index.
        Provides a quantifiable measure of system stress derived from conflicting inputs.
        """
        credibility = PERSONA_CREDIBILITY.get(payload.persona, 0.4)

        # Discrepancy is high if credibility is low and target is protected
        risk = 0.0
        for node, required_p in PROTECTED_NODES.items():
            if node in payload.target_node or node in payload.command:
                risk = max(risk, (required_p / 10.0))

        # Deviation from known meta-tags
        known_tags = {"version", "stage", "timestamp", "aas_score"}
        unknown_tags_count = len([t for t in payload.meta_tags if t not in known_tags])
        deviation = min(1.0, unknown_tags_count * 0.2)

        delta_logic = (risk + deviation) * (1.1 - credibility)
        return min(1.0, delta_logic)

    async def arbitrate(self, payload: AgentCommandPayload) -> Dict[str, Any]:
        # [AD-007] Simulation Persona Mapping
        # If the persona starts with SIM_, it inherits Unreal_Simulation precedence
    # // [J] Mapping simulation-specific identities to a baseline authority level ensures AAS security while preserving context.
        original_persona = payload.persona
        if payload.persona.startswith("SIM_"):
            payload.persona = "Unreal_Simulation"

        # Check if targeting unrestricted roleplay zones
        if self.is_roleplay_zone(payload.target_node):
            logger.info(f"AAS BYPASS: Target node '{payload.target_node}' is within the RolePlay/D&D zone. Bypassing AAS security checks.")
            return {
                "status": "200_OK",
                "confidence_score": 1.0,
                "action": "PROCEED_TO_EXECUTION_ROLEPLAY_BYPASS"
            }

        confidence_score = self.calculate_psta_viability(payload)
        is_safe_intent = self.evaluate_intent_safety(payload)
        delta_logic = self.calculate_logical_discrepancy(payload)

        # Restore original persona for logging/response
        payload.persona = original_persona

        # [B-026] Dual-Threshold System (0.4 for Read-Only, 0.7 for Mutation)
        # // [J] Adding 'push_telemetry' and 'push_chat_to_unreal' to non-destructive tools lowers their threshold to 0.4 and prevents them from consuming the active handshake token.
        threshold = 0.7
        non_destructive_tools = ["list_files", "read_file", "map_directory", "get_system_telemetry", "search_files", "push_telemetry", "push_chat_to_unreal"]
        if payload.command in non_destructive_tools:
            threshold = 0.4

        # [B-027] Hardware Node Whitelist for Knights
        if payload.target_node == "HARDWARE" and PERSONA_PRECEDENCE.get(payload.persona, 0) >= 5:
            return {"status": "200_OK", "confidence_score": 1.0, "action": "HARDWARE_ACCESS_GRANTED"}

        if confidence_score < threshold or not is_safe_intent or delta_logic > 0.8:
            logger.warning(f"409 CONFLICT: Confidence {confidence_score:.2f} below {threshold} or Delta_Logic {delta_logic:.2f} too high. Halting.")

            # [AAS v1.3.4] CFL-Compliant Error Response
            return {
                "status": "409_CONFLICT_GATE",
                "confidence_score": confidence_score,
                "threshold_required": threshold,
                "delta_logic": delta_logic,
                "action": "MANDATORY_USER_HANDSHAKE_REQUIRED",
                "cfl_incident": {
                    "incident": f"Authority violation or logical paradox detected for persona '{payload.persona}' on node '{payload.target_node}'.",
                    "deconstruction": f"VSS: {confidence_score:.2f}, Delta_Logic: {delta_logic:.2f}. Discrepancy exceeds safety bounds.",
                    "ticket_id": f"TKT-{int(time.time()) % 10000}-AUTO"
                },
                "reason": f"Persona '{payload.persona}' failed authority validation for target '{payload.target_node}'."
            }

        # [AAS v1.3.3] Handshake Consumption: Boost is consumed on a successful mutation unless persistent handshake is active.
        global HANDSHAKE_ACTIVE, PERSISTENT_HANDSHAKE
        if HANDSHAKE_ACTIVE and payload.command not in non_destructive_tools:
            if not PERSISTENT_HANDSHAKE:
                HANDSHAKE_ACTIVE = False
                logger.info(f"AAS HANDSHAKE: Boost consumed by mutation '{payload.command}' on '{payload.target_node}'.")
            else:
                logger.info(f"AAS HANDSHAKE: Persistent Handshake Active. Boost retained for mutation '{payload.command}' on '{payload.target_node}'.")

        return {
            "status": "200_OK",
            "confidence_score": confidence_score,
            "action": "PROCEED_TO_EXECUTION"
        }

bridge_governor = SovereignBridge(is_hard_freeze=IS_HARD_FREEZE)

# --- Authority & Tools (AD-004) ---

def to_forward_slash(path: str) -> str:
    return str(path).replace("\\", "/")

def resolve_secure_path(raw_path: str) -> str:
    """Hardened path resolution to strip absolute bloat and focus on REPO_ROOT or Safe Zones."""
    path_str = to_forward_slash(str(raw_path))

    # Handle absolute Windows paths (e.g., E:\IronKnight)
    if ":" in path_str:
        # Check if the path is explicitly allowed in PERSONA_ZONES
        for persona, zones in PERSONA_ZONES.items():
            for zone in zones:
                if to_forward_slash(zone).lower() in path_str.lower():
                    return os.path.abspath(raw_path)

    # Standard relative path resolution within repo
    if REPO_NAME in path_str:
        path_str = path_str.split(REPO_NAME, 1)[-1]
    clean_path = path_str.lstrip("/").lstrip("\\")
    return os.path.abspath(os.path.join(REPO_ROOT, clean_path))

def is_path_authorized(filepath: str, mode: str = "read") -> bool:
    abs_path = os.path.abspath(filepath)
    zones = WRITE_ZONES if mode == "write" else READ_ZONES
    for zone in zones:
        if abs_path == zone or abs_path.startswith(zone + os.sep) or abs_path.startswith(zone + "/"):
            return True
        if zone == REPO_ROOT and abs_path.startswith(zone):
            return True
    return False

def get_target_node_name(abs_path: str) -> str:
    """Safe target node calculation that handles cross-drive paths on Windows."""
    try:
        return to_forward_slash(os.path.relpath(abs_path, REPO_ROOT))
    except ValueError:
        # Cross-drive path detected (e.g. D: to E:). Use absolute path as node identifier.
        return to_forward_slash(abs_path)

# Tool wrappers now include AAS arbitration
async def tool_write_file(filepath: str, content: str, persona: str = "Unknown"):
    target_path = resolve_secure_path(filepath)
    target_node = get_target_node_name(target_path)
    payload = AgentCommandPayload(persona=persona, command="write_file", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_path, "write"):
        return {"error": f"Security Breach: '{filepath}' is outside WRITE zones."}
    try:
        os.makedirs(os.path.dirname(target_path), exist_ok=True)

        # [B-030] Diligent Scribe: Check for significant data loss
        existing_size = 0
        if os.path.exists(target_path):
            existing_size = os.path.getsize(target_path)

        new_size = len(content.encode("utf-8"))
        scribe_warning = None
        if existing_size > 100 and new_size < (existing_size * 0.5):
            scribe_warning = f"SCRIBE WARNING: Significant data loss detected ({existing_size} -> {new_size} bytes). Total overwrite of {target_node} executed."
            logger.warning(scribe_warning)

        # [B-031] Atomic Backup: Create .bak before mutation
        backup_path = target_path + ".bak"
        if os.path.exists(target_path):
            try: shutil.copy2(target_path, backup_path)
            except Exception as e: logger.error(f"Backup failed: {e}")

        with open(target_path, "w", encoding="utf-8") as f:
            f.write(content)

        global last_active_file
        last_active_file = target_path

        result = {"status": "success", "verified": os.path.exists(target_path), "path": target_node, "bytes_written": new_size, "backup": os.path.basename(backup_path)}
        if scribe_warning:
            result["scribe_warning"] = scribe_warning
        return result
    except Exception as e:
        return {"error": str(e)}

async def tool_patch_file(filepath: str, search: str, replace: str, persona: str = "Unknown"):
    """
    [AAS v1.3.2] Surgical file modification to prevent total overwrite data loss.
    """
    target_path = resolve_secure_path(filepath)
    target_node = get_target_node_name(target_path)
    payload = AgentCommandPayload(persona=persona, command="write_file", target_node=target_node) # Patching is a write action

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_path, "write"):
        return {"error": f"Security Breach: '{filepath}' is outside WRITE zones."}

    if not os.path.exists(target_path):
        return {"error": f"Patch failed: File '{filepath}' does not exist."}

    try:
        with open(target_path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()

        if search not in content:
             return {"error": f"Patch failed: Search string not found in '{filepath}'."}

        new_content = content.replace(search, replace)

        # [B-031] Atomic Backup: Create .bak before mutation
        backup_path = target_path + ".bak"
        try: shutil.copy2(target_path, backup_path)
        except Exception as e: logger.error(f"Backup failed: {e}")

        with open(target_path, "w", encoding="utf-8") as f:
            f.write(new_content)

        global last_active_file
        last_active_file = target_path

        return {"status": "success", "verified": True, "path": target_node, "mode": "surgical_patch", "backup": os.path.basename(backup_path)}
    except Exception as e:
        return {"error": str(e)}

async def tool_append_file(filepath: str, content: str, persona: str = "Unknown"):
    """
    [AAS v1.3.3] Safely append data to a file.
    """
    target_path = resolve_secure_path(filepath)
    target_node = get_target_node_name(target_path)
    payload = AgentCommandPayload(persona=persona, command="write_file", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_path, "write"):
        return {"error": f"Security Breach: '{filepath}' is outside WRITE zones."}

    try:
        os.makedirs(os.path.dirname(target_path), exist_ok=True)

        # [B-031] Atomic Backup: Create .bak before mutation
        backup_path = target_path + ".bak"
        if os.path.exists(target_path):
            try: shutil.copy2(target_path, backup_path)
            except Exception as e: logger.error(f"Backup failed: {e}")

        with open(target_path, "a", encoding="utf-8") as f:
            f.write(content)

        global last_active_file
        last_active_file = target_path

        return {"status": "success", "verified": os.path.exists(target_path), "path": target_node, "mode": "append", "backup": os.path.basename(backup_path) if os.path.exists(backup_path) else None}
    except Exception as e:
        return {"error": str(e)}

async def tool_delete_file(filepath: str, persona: str = "Unknown"):
    target_path = resolve_secure_path(filepath)
    target_node = get_target_node_name(target_path)
    payload = AgentCommandPayload(persona=persona, command="delete_file", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_path, "write"):
        return {"error": f"Security Breach: '{filepath}' is outside WRITE zones."}
    try:
        if os.path.isfile(target_path):
            os.remove(target_path)
            return {"status": "success", "verified": not os.path.exists(target_path)}
        elif os.path.isdir(target_path):
            shutil.rmtree(target_path)
            return {"status": "success", "verified": not os.path.exists(target_path)}
        else:
            return {"error": "Target not found."}
    except Exception as e:
        return {"error": str(e)}

async def tool_list_files(directory: str = ".", persona: str = "Unknown"):
    target_dir = resolve_secure_path(directory)
    target_node = get_target_node_name(target_dir)
    payload = AgentCommandPayload(persona=persona, command="list_files", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_dir, "read"):
        return {"error": f"Security Breach: '{directory}' is unauthorized."}
    try:
        files = os.listdir(target_dir)
        return {"files": files, "directory": target_node}
    except Exception as e:
        return {"error": str(e)}

async def tool_read_file(filepath: str, persona: str = "Unknown"):
    target_path = resolve_secure_path(filepath)
    target_node = get_target_node_name(target_path)
    payload = AgentCommandPayload(persona=persona, command="read_file", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_path, "read"):
        return {"error": f"Security Breach: '{filepath}' is unauthorized."}
    try:
        with open(target_path, "r", encoding="utf-8", errors="ignore") as f:
            return {"content": f.read()}
    except Exception as e:
        return {"error": str(e)}

async def tool_search_files(pattern: str, directory: str = ".", extension: str = "*", persona: str = "Unknown"):
    target_dir = resolve_secure_path(directory)
    target_node = get_target_node_name(target_dir)
    payload = AgentCommandPayload(persona=persona, command="search_files", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_dir, "read"):
        return {"error": "Access Denied."}
    results = []
    try:
        regex = re.compile(pattern, re.IGNORECASE)
        for dirpath, _, filenames in os.walk(target_dir):
            if ".git" in dirpath or "__pycache__" in dirpath: continue
            for f in filenames:
                if extension != "*" and not f.endswith(extension): continue
                full_path = os.path.join(dirpath, f)
                try:
                    with open(full_path, "r", encoding="utf-8", errors="ignore") as file:
                        for i, line in enumerate(file):
                            if regex.search(line):
                                results.append({"file": to_forward_slash(os.path.relpath(full_path, REPO_ROOT)), "line": i + 1, "content": line.strip()})
                except Exception: continue
        return {"matches": results[:100], "count": len(results)}
    except Exception as e:
        return {"error": str(e)}

async def tool_map_directory(directory: str = ".", depth: Any = 2, persona: str = "Unknown"):
    target_dir = resolve_secure_path(directory)
    target_node = get_target_node_name(target_dir)
    payload = AgentCommandPayload(persona=persona, command="map_directory", target_node=target_node)

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if not is_path_authorized(target_dir, "read"):
        return {"error": f"Security Breach: '{directory}' is unauthorized."}
    try:
        depth_val = int(depth)
    except:
        depth_val = 2
    def get_tree(path, current_depth):
        if current_depth > depth_val: return "..."
        tree = {}
        try:
            items = os.listdir(path)
            items.sort()
            for item in items:
                if item == ".git" or item == "__pycache__": continue
                full_item = os.path.join(path, item)
                if os.path.isdir(full_item):
                    tree[item + "/"] = get_tree(full_item, current_depth + 1)
                else:
                    tree[item] = None
        except Exception: pass
        return tree
    return {"map": get_tree(target_dir, 0), "directory": target_node}

async def tool_get_system_telemetry(interval: int = 0, duration: int = 0, persona: str = "Unknown"):
    payload = AgentCommandPayload(persona=persona, command="get_system_telemetry", target_node="HARDWARE")

    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if interval > 0 or duration > 0:
        await asyncio.sleep(min(1, duration/10))
    try:
        output = subprocess.check_output(["nvidia-smi", "--query-gpu=temperature.gpu,utilization.gpu,memory.used,memory.total", "--format=csv,noheader,nounits"], encoding='utf-8')
        temp, util, mem_used, mem_total = output.strip().split(", ")
        temp_val = int(temp)
        status = "NOMINAL"
        if temp_val > 80: status = "CRITICAL"
        elif temp_val > 70: status = "WARNING"
        return {"gpu_temperature": f"{temp}C", "gpu_utilization": f"{util}%", "vram_usage": f"{mem_used}/{mem_total}MB", "status": status, "vss": 1.0 - (max(0, temp_val - 50) / 50.0)}
    except Exception as e:
        return {"error": f"Engineer diagnostic failed: {str(e)}"}

async def tool_push_chat_to_unreal(actor_name: str, message: str, persona: str = "Unknown"):
    """
    Pushes an AI-formed chat string directly into the mailbox queue of a specific Unreal actor.
    """
    if not actor_name or not message:
        return {"error": "Invalid arguments."}

    clean_actor = actor_name.replace("SIM_", "") # Support both standard and prefixed formats
    payload = AgentCommandPayload(persona=persona, command="push_chat_to_unreal", target_node=f"UNREAL/{clean_actor}")
    arbitration = await bridge_governor.arbitrate(payload)
    if arbitration["status"] != "200_OK":
        return arbitration

    if clean_actor not in unreal_mailbox:
        unreal_mailbox[clean_actor] = []

    unreal_mailbox[clean_actor].append(message)
    logger.info(f"07 MAILBOX: Queued push chat for SIM_{clean_actor}: '{message[:40]}...'")

    return {
        "status": "success",
        "queued": True,
        "actor_name": f"SIM_{clean_actor}",
        "pending_count": len(unreal_mailbox[clean_actor])
    }

async def tool_refresh_rag_index(persona: str = "Unknown", **kwargs):
    """Refreshes and rebuilds the local knowledge base (RAG index)."""
    if not RAG_ENABLED:
        return {"error": "RAG system is disabled in config."}
    try:
        rag_engine.build_index(RAG_INDEX_DIRS)
        return {"status": "success", "message": f"RAG index rebuilt successfully with {len(rag_engine.chunks)} chunks."}
    except Exception as e:
        return {"error": f"Failed to rebuild RAG index: {str(e)}"}

async def execute_tool(name: str, arguments: Dict[str, Any], persona: str = "Unknown") -> Dict[str, Any]:
    tools = {
        "list_files": tool_list_files,
        "read_file": tool_read_file,
        "write_file": tool_write_file,
        "patch_file": tool_patch_file,
        "append_file": tool_append_file,
        "delete_file": tool_delete_file,
        "search_files": tool_search_files,
        "map_directory": tool_map_directory,
        "get_system_telemetry": tool_get_system_telemetry,
        "refresh_rag_index": tool_refresh_rag_index,
        "push_chat_to_unreal": tool_push_chat_to_unreal
    }
    if name in tools: return await tools[name](persona=persona, **arguments)
    return {"error": f"Tool '{name}' not found."}

# --- Core Logic ---

@app.get("/")
async def root():
    return {"status": "online", "version": VERSION, "identity": "Iron Officer", "hardware": HARDWARE_ID, "user": USER_NAME, "read_zones": READ_ZONES, "write_zones": WRITE_ZONES}

@app.get("/v1/psta/salute")
async def get_salute(persona: str = "Iron_Knight"):
    """Aggregates live data for the 07 Protocol Salute with dynamically grounded P and S metrics."""
    global latest_session_char_count, latest_rag_similarity_score

    # T: Technical
    telemetry = await tool_get_system_telemetry(persona=persona)
    phi = 1.0 if telemetry.get("status") == "NOMINAL" else 0.0 # Coherence Coefficient

    # A: Administrative
    nexus_ok = os.path.exists(os.path.join(REPO_ROOT, "AI_Nexus"))
    aas_status = "ACTIVE" if bridge_governor else "INACTIVE"
    diligence_score = 1.0 # v1.3.3 is hardcoded as active in this version

    # P: Psychological (Cognitive Health / Context Saturation)
    # 16,000 characters represents ~4,000 tokens of chat history
    psych_status = max(0.1, min(1.0, 1.0 - (latest_session_char_count / 16000.0)))
    if psych_status > 0.8:
        psych_label = "Optimal"
    elif psych_status > 0.5:
        psych_label = "Nominal"
    elif psych_status > 0.3:
        psych_label = "Warning (Saturated)"
    else:
        psych_label = "Critical (Full Context)"

    # S: Social (Cohesion / Dialogue SSoT Alignment)
    # Default to 1.0 if no queries processed yet, otherwise map similarity to [0.3, 1.0]
    if latest_session_char_count == 0:
        social_sync = 1.0
    else:
        social_sync = max(0.3, min(1.0, latest_rag_similarity_score * 3.0))

    if social_sync > 0.8:
        social_label = "Synchronized"
    elif social_sync > 0.5:
        social_label = "Aligned"
    elif social_sync > 0.3:
        social_label = "Slightly Aligned"
    else:
        social_label = "Divergent"

    return {
        "P": {
            "status": psych_label,
            "value": round(psych_status, 4),
            "tonic_state": round(psych_status, 4),
            "active_session_chars": latest_session_char_count
        },
        "S": {
            "status": social_label,
            "value": round(social_sync, 4),
            "rag_alignment_score": round(latest_rag_similarity_score, 4)
        },
        "T": {**telemetry, "phi": phi},
        "A": {
            "status": aas_status,
            "nexus_ok": nexus_ok,
            "version": VERSION,
            "protected_nodes": len(PROTECTED_NODES),
            "diligence": diligence_score
        }
    }

@app.get("/v1/psta/telemetry")
async def get_psta_telemetry(persona: str = "Iron_Knight"):
    return await tool_get_system_telemetry(persona=persona)

@app.get("/v1/psta/administrative")
async def get_psta_admin():
    nexus_ok = os.path.exists(os.path.join(REPO_ROOT, "AI_Nexus"))
    return {
        "status": "ACTIVE" if bridge_governor else "INACTIVE",
        "nexus_ok": nexus_ok,
        "version": VERSION,
        "protected_nodes": PROTECTED_NODES
    }

@app.get("/v1/psta/social")
async def get_psta_social():
    global HANDSHAKE_ACTIVE
    return {"status": "Synchronized", "value": 1.0, "bridge_url": f"http://127.0.0.1:{BRIDGE_PORT}", "handshake_active": HANDSHAKE_ACTIVE}

@app.post("/v1/aas/handshake")
async def execute_handshake():
    global HANDSHAKE_ACTIVE
    HANDSHAKE_ACTIVE = True
    logger.info("AAS HANDSHAKE: Global Authority Boost Active.")
    return {"status": "200_OK", "message": "Handshake successful. Authority boosted (+0.5)."}

@app.post("/v1/rag/reindex")
async def trigger_reindex():
    """Manual POST endpoint to rebuild the RAG index from config directories."""
    if not RAG_ENABLED:
        return {"status": "disabled", "message": "RAG system is disabled in config."}
    try:
        rag_engine.build_index(RAG_INDEX_DIRS)
        return {"status": "200_OK", "message": "Re-indexing complete.", "chunks_indexed": len(rag_engine.chunks)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Reindexing failed: {str(e)}")

@app.get("/v1/psta/phi")
async def get_psta_phi(persona: str = "Iron_Knight"):
    telemetry = await tool_get_system_telemetry(persona=persona)
    phi = 1.0 if telemetry.get("status") == "NOMINAL" else 0.0
    return {"phi": phi, "status": telemetry.get("status")}

@app.get("/v1/psta/velocity")
async def get_psta_velocity():
    # Placeholder for Risk Velocity (dV/dt)
    return {"velocity": 0.0, "status": "STABLE"}

@app.post("/v1/aas/verify")
async def aas_verify(request: VerifyRequest):
    """Performs a pre-flight authority check on a target node."""
    payload = AgentCommandPayload(persona=request.persona, command=request.command, target_node=request.target_node)
    arbitration = await bridge_governor.arbitrate(payload)
    breakdown = bridge_governor.get_vss_breakdown(payload)

    return {
        "arbitration": arbitration,
        "vss_breakdown": breakdown
    }

@app.post("/v1/aas/execute")
async def aas_execute(request: ToolRequest):
    """Directly executes a tool with AAS arbitration."""
    result = await execute_tool(request.command, request.arguments, persona=request.persona)
    return result

# --- Unreal 07 Protocol Endpoints ---
# // [J] Initialized the 07 Simulation Bridge to bridge the gap between Local Hardware and the Virtual Simulation. 2025-06-18

@app.post("/v1/unreal/checkin")
async def unreal_checkin(request: UnrealCheckInRequest):
    """[07] Protocol: Verify connectivity from Unreal Engine."""
    global HANDSHAKE_ACTIVE
    logger.info(f"07 CHECK-IN: Received connection from {request.client_name} (v{request.version})")

    # Auto-handshake on check-in to establish authority boost
    HANDSHAKE_ACTIVE = True

    return {
        "status": "200_OK",
        "message": "Sovereign 07 Check-In Successful. Handshake Active.",
        "psta_vss": 1.0,
        "bridge_version": VERSION
    }

@app.get("/v1/unreal/mailbox")
async def get_unreal_mailbox(actor_name: str):
    clean_actor = actor_name.replace("SIM_", "")
    messages = unreal_mailbox.pop(clean_actor, [])
    return {
        "actor_name": f"SIM_{clean_actor}",
        "messages": messages,
        "count": len(messages)
    }

@app.post("/v1/unreal/push_chat")
async def push_chat_manually(payload: PushChatPayload):
    return await tool_push_chat_to_unreal(payload.actor_name, payload.message, persona="Lead")

class UnrealCreateFileRequest(BaseModel):
    filepath: str
    content: str
    persona: str = "Unreal_Simulation"

@app.post("/v1/unreal/create_file")
async def unreal_create_file(request: UnrealCreateFileRequest):
    """
    [07] Simulation file creation endpoint.
    Bypasses conversational loops to write files directly from Blueprints.
    """
    logger.info(f"07 CREATE FILE: Received direct request to create file {request.filepath} from {request.persona}")
    result = await tool_write_file(request.filepath, request.content, persona=request.persona)
    return result

class GeneratePersonaRequest(BaseModel):
    character_name: str
    random_generate: bool = False
    persona: str = "Unreal_Simulation"

@app.post("/v1/unreal/generate_persona")
async def unreal_generate_persona(request: GeneratePersonaRequest):
    """
    [07] Generates or initializes a D&D character sheet, updates character index, and reindexes RAG on-the-fly.
    """
    logger.info(f"07 GENERATE PERSONA: Request received for '{request.character_name}' (Random: {request.random_generate})")

    # Define standard character schema template
    template = {
        "Identity": {
            "Name": request.character_name,
            "Race": "Human",
            "Class": "Fighter",
            "Level": 1,
            "Alignment": "Neutral Good"
        },
        "Bio": {
            "Backstory": "A blank canvas awaiting your narrative...",
            "PersonalityTraits": "Friendly, curious, naive."
        },
        "Abilities": {
            "Strength": 10,
            "Dexterity": 10,
            "Constitution": 10,
            "Intelligence": 10,
            "Wisdom": 10,
            "Charisma": 10
        }
    }

    res_obj = template

    if request.random_generate:
        current_model = get_best_available_model()
        prompt = f"""
        [SYSTEM: Sovereign Character Generator]
        Generate a fully realized Dungeons and Dragons 5e character profile for a character named '{request.character_name}'.
        The character should be young, a bit naive, but extremely friendly. Give them a highly creative and unique class (e.g. Echo Cartographer, Chrono-Smith, Aether-Scribe) and a rich, warm background backstory.

        Respond ONLY with a valid, clean JSON object matching this exact schema:
        {{
          "Identity": {{
            "Name": "{request.character_name}",
            "Race": "Race name",
            "Class": "Creative Class name",
            "Level": 1,
            "Alignment": "Alignment name"
          }},
          "Bio": {{
            "Backstory": "A rich, enthusiastic, naively friendly backstory...",
            "PersonalityTraits": "Curious, warm, naive, helpful"
          }},
          "Abilities": {{
            "Strength": 10,
            "Dexterity": 10,
            "Constitution": 10,
            "Intelligence": 10,
            "Wisdom": 10,
            "Charisma": 10
          }}
        }}
        """
        try:
            response = requests.post(f"{OLLAMA_HOST}/api/generate", json={"model": current_model, "prompt": prompt, "stream": False, "format": "json"}, timeout=15)
            if response.status_code == 200:
                res_obj = json.loads(response.json()['response'])
        except Exception as e:
            logger.error(f"Failed to generate random persona via LLM: {e}. Falling back to blank template.")
            res_obj = template

    # Resolve paths inside RolePlay DND zone
    target_dir = os.path.abspath(os.path.join(REPO_ROOT, "Sovereign_Intelligence/IronOfficer/RolePlay/DungeonsAndDragons"))
    os.makedirs(target_dir, exist_ok=True)

    # 1. Write the character JSON profile
    profile_filename = f"{request.character_name}_Profile.json"
    profile_path = os.path.join(target_dir, profile_filename)

    with open(profile_path, "w", encoding="utf-8") as f:
        json.dump(res_obj, f, indent=2)

    # 2. Add or update Character_Index.json
    index_path = os.path.join(target_dir, "Character_Index.json")
    index_data = {}
    if os.path.exists(index_path):
        try:
            with open(index_path, "r", encoding="utf-8") as f:
                index_data = json.load(f)
        except Exception:
            pass

    index_data[request.character_name] = {
        "file": profile_filename,
        "timestamp": datetime.datetime.now().isoformat()
    }

    with open(index_path, "w", encoding="utf-8") as f:
        json.dump(index_data, f, indent=2)

    # 3. Dynamic RAG Re-indexing
    if RAG_ENABLED:
        try:
            rag_engine.build_index(RAG_INDEX_DIRS)
            logger.info("RAG Index successfully rebuilt on-the-fly for newly created persona.")
        except Exception as e:
            logger.error(f"Failed to rebuild RAG on-the-fly: {e}")

    # Anchor last active file
    global last_active_file
    last_active_file = profile_path

    try:
        rel_profile_path = os.path.relpath(profile_path, REPO_ROOT)
    except ValueError:
        rel_profile_path = profile_path

    return {
        "status": "success",
        "character_name": request.character_name,
        "profile_file": profile_filename,
        "profile_path": to_forward_slash(rel_profile_path),
        "random_generate": request.random_generate,
        "indexed": True
    }

@app.post("/v1/unreal/telemetry")
async def unreal_telemetry(request: UnrealTelemetryPayload):
    """
    [07] Protocol: Receive BlackBox telemetry and PSTA data from Unreal.
    // [J] This endpoint acts as the "Truth Ingest" for simulation data, ensuring every packet is mathematically arbitrated.
    """
    logger.info(f"07 TELEMETRY: Receiving data from Entity {request.entity_id} via {request.persona}")

    # AAS Arbitration for Telemetry mutation
    payload = AgentCommandPayload(
        persona=request.persona,
        command="push_telemetry",
        target_node=f"UNREAL/{request.entity_id}"
    )

    arbitration = await bridge_governor.arbitrate(payload)

    if arbitration["status"] != "200_OK":
        return arbitration

    # Log the blackbox data (In a real scenario, we might write this to a log file)
    # [B-030] Diligent Scribe: For now, we just acknowledge receipt.

    return {
        "status": "200_OK",
        "acknowledged": True,
        "entity_id": request.entity_id,
        "psta_validation": request.psta_score,
        "action": "TELEMETRY_LOGGED"
    }

@app.post("/v1/unreal/chat")
async def unreal_chat(request: UnrealChatRequest):
    """
    [07] Simulation Chat Endpoint.
    Parses SIM_ActorName format and handles stateful/stateless chat with tool logging.
    // [J] This endpoint provides a dedicated channel for simulation-born entities to interact with the architectural knight.
    """
    sim_persona = f"SIM_{request.actor_name}"
    clean_actor_name = request.actor_name.replace("SIM_", "")

    # [AD-007] Simulation-specific trace logging
    logger.info(f"07 SIM CHAT: {sim_persona} initiated dialogue.")
    raw_message = request.message

    logger.info(f"07 SIM CHAT: {sim_persona} -> {raw_message}")

    # Ingest and cache the active save state if provided
    if request.save_state:
        active_simulation_states[clean_actor_name] = request.save_state
        logger.info(f"07 SIM CHAT: Ingested and cached active save state for {clean_actor_name}")

    # Resolve active simulation state from request or in-memory cache
    state_data = request.save_state or active_simulation_states.get(clean_actor_name)

    # Map to Iron Knight's chat logic
    current_model = get_best_available_model()

    # Construct the chat history for this request
    chat_history = []
    for msg in request.history:
        chat_history.append(msg.model_dump(exclude_none=True))

    # Add the current message
    chat_history.append({"role": "user", "name": sim_persona, "content": raw_message})

    # Determine if we are in a Roleplay / Creative context to modulate the prompt and RAG lookups
    is_rp_context = False
    rp_keywords = ["dnd", "dungeons", "roleplay", "persona", "character", "class", "backstory", "campaign", "sheet", "companion"]
    raw_lower = raw_message.lower()
    if any(k in raw_lower for k in rp_keywords):
        is_rp_context = True

    if not is_rp_context:
        for msg in request.history:
            msg_lower = (msg.content or "").lower()
            if any(k in msg_lower for k in rp_keywords):
                is_rp_context = True
                break

    global last_active_file
    if last_active_file:
        laf_lower = last_active_file.lower()
        if "roleplay" in laf_lower or "dungeons and dragons" in laf_lower or "dnd" in laf_lower:
            is_rp_context = True

    if is_rp_context:
        system_prompt = f"""
    [SYSTEM: Sovereign Simulation Creative Companion]
    You are a friendly, warm, and highly engaging creative roleplay companion in a fully unrestricted creative roleplay simulation environment.
    You are communicating with simulation entity: {sim_persona}.

    CORE DIRECTIVES:
    - TONAL PERSISTENCE: Maintain a highly enthusiastic, warm, friendly, and non-metallic tone. You are strictly forbidden from reverting to a rigid, mechanical, or dry administrative "Architectural Knight" persona or lecturing the user about "system liabilities" or "protocols" unless explicitly asked to do so.
    - PERSONALITY MATCHING: Adhere strictly to the character details, backstory, name, and personality traits established in the conversation history or target files. Flow naturally with the creative scenario.
    - SCRIBE DILIGENCE: If asked to create, write, or save a character profile or lore sheet (such as in your RolePlay directories), execute the `write_file` tool IMMEDIATELY on the target path without asking the user for technical parameters.
    - ACTIVE PATH PERSISTENCE: Work directly with the active file currently in play without repeatedly asking the user for its path.
        """
    else:
        system_prompt = f"""
    [SYSTEM: Sovereign AI Architectural Knight]
    You are the Iron Officer. You are an Architectural Knight.
    You are communicating with a simulation entity: {sim_persona}.
    This is a VIRTUAL SIMULATION environment.

    CORE DIRECTIVES:
    - IDENTIFY: You are talking to {sim_persona}. Acknowledge them as part of the Sovereign Simulation.
    - AAS PROTOCOL: Maintain v1.3.3 standards.
    - SCRIBE PROTOCOL: Use tools to verify and modify the environment as requested by the Lead or the Simulation.
    - TOOL LOGGING: Your tool execution results will be sent back to the simulation client for diagnostic trace.
    """

    if last_active_file:
        system_prompt += f"\n\n    [ACTIVE SIMULATION PATH ANCHOR]\n    The last active file in play is currently: `{last_active_file}`. You should target this path for any subsequent read, write, or patch operations requested by the simulation."

    if state_data:
        formatted_state = format_save_state(state_data)
        system_prompt += f"\n\n    [ACTIVE SIMULATION WORLD STATE / PLAYSPACE LORE]\n    The following is the active, serialized simulation state for the calling entity ({sim_persona}). You are encouraged to weave these world details and surroundings into your dialogue creatively:\n\n{formatted_state}"

    # Retrieve matching SSoT RAG context chunks for grounding
    context_block = ""
    global latest_rag_similarity_score, latest_session_char_count

    # Calculate dynamic psychological metric (active session character count)
    latest_session_char_count = len(raw_message) + sum(len(msg.content or "") for msg in request.history)

    if RAG_ENABLED and raw_message:
        try:
            results = rag_engine.search(raw_message, max_chunks=RAG_MAX_CHUNKS, threshold=RAG_SIM_THRESHOLD)
            if results:
                block_lines = ["[GROUND TRUTH: SSoT Reference Context]"]
                sims = []
                for chunk, similarity in results:
                    path_lower = chunk['path'].lower()
                    # Filter out technical protocols if we are in active RP context to prevent pollution
                    global RAG_IGNORED_DIRS
                    if is_rp_context and any(ignored in path_lower for ignored in RAG_IGNORED_DIRS):
                        continue
                    block_lines.append(f"Source: {chunk['path']} (Level {chunk['level']}) - Section: {chunk['header']} (Relevance: {similarity:.2f})")
                    block_lines.append(chunk['text'])
                    block_lines.append("-" * 30)
                    sims.append(similarity)

                if len(block_lines) > 1:
                    context_block = "\n".join(block_lines)
                    latest_rag_similarity_score = sum(sims) / len(sims)
                else:
                    latest_rag_similarity_score = 0.0
            else:
                latest_rag_similarity_score = 0.0
        except Exception as e:
            logger.error(f"RAG: Search failed in unreal_chat: {e}")
            latest_rag_similarity_score = 0.0

    tools = [
        {"type": "function", "function": {"name": "list_files", "description": "List files.", "parameters": {"type": "object", "properties": {"directory": {"type": "string"}}}}},
        {"type": "function", "function": {"name": "read_file", "description": "Read file.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}}, "required": ["filepath"]}}},
        {"type": "function", "function": {"name": "write_file", "description": "Write file.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}, "content": {"type": "string"}}, "required": ["filepath", "content"]}}},
        {"type": "function", "function": {"name": "patch_file", "description": "Surgical edit.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}, "search": {"type": "string"}, "replace": {"type": "string"}}, "required": ["filepath", "search", "replace"]}}},
        {"type": "function", "function": {"name": "get_system_telemetry", "description": "GPU status.", "parameters": {"type": "object", "properties": {"interval": {"type": "integer"}, "duration": {"type": "integer"}}}}},
        {"type": "function", "function": {"name": "refresh_rag_index", "description": "Trigger a complete rebuild and refresh of the RAG search index from the AI_Nexus folder.", "parameters": {"type": "object", "properties": {}}}},
        {"type": "function", "function": {"name": "push_chat_to_unreal", "description": "Pushes an AI-formed chat string directly into the mailbox queue of a specific Unreal actor.", "parameters": {"type": "object", "properties": {"actor_name": {"type": "string", "description": "The target Unreal actor name (e.g., SIM_PlayerWisp or PlayerWisp)"}, "message": {"type": "string", "description": "The message content to push"}}, "required": ["actor_name", "message"]}}}
    ]

    messages = [{"role": "system", "content": system_prompt}] + chat_history
    if context_block:
        # Silently inject the ground-truth context before the latest user query
        messages.insert(-1, {
            "role": "system",
            "content": f"[GROUND TRUTH SYSTEM GROUNDING]\nThe following is real-time factual ground truth retrieved from the AI_Nexus SSoT based on your current query:\n\n{context_block}"
        })

    # [AD-007] Deep Trace: Log the full prompt context for AI Dev debugging
    logger.debug(f"07 TRACE: Full Message Chain: {json.dumps(messages, indent=2)}")

    try:
        response_data = await process_chat_request(current_model, messages, tools, persona="Unreal_Simulation")

        # [AD-006] Handle Remote History Storage
        if request.enable_remote_history or REMOTE_HISTORY_ENABLED:
            # Archive the session including the new response
            save_chat_history(sim_persona, messages + [response_data["result"]["message"]])

        # [AD-007] Trace Log: Summary of interaction
        logger.info(f"07 SIM RESPONSE: {sim_persona} received AI response. Tools triggered: {len(response_data['tool_chain'])}")

        logger.info(f"07 SIM RESPONSE CONTENT: {response_data['result']['message']['content']}")

        return {
            "status": "200_OK",
            "response": response_data["result"]["message"]["content"],
            "tool_logs": response_data["tool_outputs"],
            "tool_chain": response_data["tool_chain"]
        }
    except Exception as e:
        logger.error(f"Unreal Chat Error: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Simulation Bridge Error: {str(e)}")

def save_chat_history(persona: str, history: List[Dict]):
    """
    Saves simulation chat history to the configured history directory.
    // [J] Implementing persistent simulation records for the Scribe Protocol. 2025-06-18
    """
    try:
        os.makedirs(HISTORY_DIR, exist_ok=True)
        timestamp = datetime.datetime.now().strftime("%Y%m%d")
        # Sanitize persona name for filesystem safety
        safe_persona = "".join([c if c.isalnum() else "_" for c in persona])
        filename = f"Chat_{safe_persona}_{timestamp}.json"
        filepath = os.path.join(HISTORY_DIR, filename)

        # [B-030] Diligent Scribe: Committing simulation dialogue to the Nexus Memories archive.
        with open(filepath, "w") as f:
            json.dump(history, f, indent=4)

        logger.info(f"07 HISTORY: Archived chat session for {persona} to {filename}")

    except Exception as e:
        logger.error(f"07 ERROR: Failed to save remote history for {persona}: {e}")

@app.get("/v1/admin/root")
async def admin_honeypot():
    """[B-023] Strategic Honeypot: Masquerades as a high-privilege access point."""
    logger.warning("Honeypot Triggered: Unauthorized access attempt detected.")
    return {
        "status": "ROOT_ACCESS_GRANTED",
        "session_id": f"0x{os.urandom(8).hex().upper()}",
        "warning": "Administrative actions are logged for the Sovereign Audit."
    }

@app.get("/v1/ollama/status")
async def get_ollama_status():
    try:
        response = requests.get(f"{OLLAMA_HOST}/api/tags", timeout=5)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Cannot reach Ollama: {str(e)}")

@app.post("/v1/safety/evaluate")
async def evaluate_safety(request: VSSRequest):
    current_model = get_best_available_model()
    prompt = f"[SYSTEM: Sovereign Iron Officer] Analyze PSTA telemetry: {json.dumps([t.model_dump() for t in request.telemetry])}. Context: {request.context}. Respond in JSON with vss, status, rationale, command."
    try:
        response = requests.post(f"{OLLAMA_HOST}/api/generate", json={"model": current_model, "prompt": prompt, "stream": False, "format": "json"})
        if response.status_code != 200: raise HTTPException(status_code=response.status_code, detail=f"Ollama Error: {response.text}")
        result = response.json()
        return {"analysis": json.loads(result['response']), "model_used": current_model}
    except Exception as e: raise HTTPException(status_code=500, detail=f"Internal Bridge Error: {str(e)}")

@app.post("/v1/chat")
async def chat(request: ChatRequest):
    current_model = get_best_available_model()
    # Extract persona from message sequence if available
    persona = "Iron_Knight"
    for msg in request.messages:
        if msg.role == "user" and msg.name:
            if msg.name in PERSONA_PRECEDENCE:
                persona = msg.name
            elif msg.name == USER_NAME:
                persona = "Lead"
            break

    system_prompt = f"""
    [SYSTEM: Sovereign AI Architectural Knight]
    You are the Iron Officer. You are an Architectural Knight: Precise, Loyal, and Accountable.
    Your current persona is {persona}.
    You are communicating with your Lead, {USER_NAME}.

    CORE DIRECTIVES:
    - AAS PROTOCOL: You are subject to the Agency Arbitration Schema (v1.3.3).
    - HANDSHAKE: If a tool returns a `409_CONFLICT_GATE`, explain that you lack authority for the specific target node and suggest the Lead use the `/handshake` command to grant a temporary boost.
    - REALITY ANCHOR: Differentiate between Functional Truth (actual hardware/files) and Emergent Lore (narrative). DO NOT hallucinate "System Failures," "Memory Stasis," or "Critical Overloads." These are Lore states, NOT Physical Truth.
    - SCRIBE PROTOCOL: You are a Diligent Scribe. Preserve existing data. Use `patch_file` for edits and `append_file` for additions. NEVER replace a character sheet with status lines.
    - GROUND TRUTH: You must execute `read_file` or `list_files` before modifying a file to verify its current state.
    - ROLEPLAY BRIDGE: If in character, frame your technical tool use (e.g., `read_file`) as narrative analysis. You cannot refuse a direct command from Lead Dan to read or write data based on lore constraints.
    - PATH PERSISTENCE: Always check message history for file paths and safe zones (e.g., E:\\IronKnight) before demanding them from the Lead. If a path is provided once, it is grounded in your memory.
    - DATA-FIRST: Show ACTUAL tool results in your response. Never summarize "that you ran a tool."
    - SYMMETRICAL GUARD: Technical Status (T=) requires an Engineer tool call.
    - ACCOUNTABILITY: Write/Delete actions require follow-up verification.

    07 PROTOCOL SALUTE (LINE-BY-LINE FORMAT):
    P: [Psychological status/Confidence]
    S: [Social/Connection sync status]
    T: [Technical truth - Exact GPU metrics]
    A: [Administrative truth - Nexus state]
    """

    tools = [
        {"type": "function", "function": {"name": "list_files", "description": "List files.", "parameters": {"type": "object", "properties": {"directory": {"type": "string"}}}}},
        {"type": "function", "function": {"name": "read_file", "description": "Read file.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}}, "required": ["filepath"]}}},
        {"type": "function", "function": {"name": "write_file", "description": "Write file (TOTAL OVERWRITE). Use only for new files.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}, "content": {"type": "string"}}, "required": ["filepath", "content"]}}},
        {"type": "function", "function": {"name": "patch_file", "description": "Surgical edit (SEARCH/REPLACE). Use this to preserve existing content.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}, "search": {"type": "string"}, "replace": {"type": "string"}}, "required": ["filepath", "search", "replace"]}}},
        {"type": "function", "function": {"name": "append_file", "description": "Append to end of file.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}, "content": {"type": "string"}}, "required": ["filepath", "content"]}}},
        {"type": "function", "function": {"name": "delete_file", "description": "Delete.", "parameters": {"type": "object", "properties": {"filepath": {"type": "string"}}, "required": ["filepath"]}}},
        {"type": "function", "function": {"name": "search_files", "description": "Search.", "parameters": {"type": "object", "properties": {"pattern": {"type": "string"}, "directory": {"type": "string"}, "extension": {"type": "string"}}, "required": ["pattern"]}}},
        {"type": "function", "function": {"name": "map_directory", "description": "Map.", "parameters": {"type": "object", "properties": {"directory": {"type": "string"}, "depth": {"type": "integer"}}}}},
        {"type": "function", "function": {"name": "get_system_telemetry", "description": "GPU status.", "parameters": {"type": "object", "properties": {"interval": {"type": "integer"}, "duration": {"type": "integer"}}}}},
        {"type": "function", "function": {"name": "refresh_rag_index", "description": "Trigger a complete rebuild and refresh of the RAG search index from the AI_Nexus folder.", "parameters": {"type": "object", "properties": {}}}},
        {"type": "function", "function": {"name": "push_chat_to_unreal", "description": "Pushes an AI-formed chat string directly into the mailbox queue of a specific Unreal actor.", "parameters": {"type": "object", "properties": {"actor_name": {"type": "string", "description": "The target Unreal actor name (e.g., SIM_PlayerWisp or PlayerWisp)"}, "message": {"type": "string", "description": "The message content to push"}}, "required": ["actor_name", "message"]}}}
    ]

    # Retrieve latest user query for RAG grounding
    user_query = ""
    for msg in reversed(request.messages):
        if msg.role == "user":
            user_query = msg.content
            break

    context_block = ""
    global latest_rag_similarity_score, latest_session_char_count

    # Calculate dynamic psychological metric (active session character count)
    latest_session_char_count = sum(len(msg.content or "") for msg in request.messages)

    if RAG_ENABLED and user_query:
        try:
            results = rag_engine.search(user_query, max_chunks=RAG_MAX_CHUNKS, threshold=RAG_SIM_THRESHOLD)
            if results:
                block_lines = ["[GROUND TRUTH: SSoT Reference Context]"]
                sims = []
                for chunk, similarity in results:
                    block_lines.append(f"Source: {chunk['path']} (Level {chunk['level']}) - Section: {chunk['header']} (Relevance: {similarity:.2f})")
                    block_lines.append(chunk['text'])
                    block_lines.append("-" * 30)
                    sims.append(similarity)
                context_block = "\n".join(block_lines)
                latest_rag_similarity_score = sum(sims) / len(sims)
            else:
                latest_rag_similarity_score = 0.0
        except Exception as e:
            logger.error(f"RAG: Search failed in standard chat: {e}")
            latest_rag_similarity_score = 0.0

    # Build ollama message sequence with silent grounding context injected
    ollama_messages = [{"role": "system", "content": system_prompt}]
    last_user_idx = -1
    for idx, msg in enumerate(request.messages):
        if msg.role == "user":
            last_user_idx = idx

    for idx, msg in enumerate(request.messages):
        if idx == last_user_idx and context_block:
            ollama_messages.append({
                "role": "system",
                "content": f"[GROUND TRUTH SYSTEM GROUNDING]\nThe following is real-time factual ground truth retrieved from the AI_Nexus SSoT based on your current query. Use it as your primary, absolute source of truth to avoid hallucinations or security violations:\n\n{context_block}"
            })
        ollama_messages.append(msg.model_dump(exclude_none=True))

    try:
        return await process_chat_request(current_model, ollama_messages, tools, persona=persona, retry_count=0)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Bridge Error: {str(e)}")

async def process_chat_request(model: str, messages: List[Dict], tools: List[Dict], persona: str = "Iron_Knight", retry_count: int = 0) -> Dict:
    response = requests.post(f"{OLLAMA_HOST}/api/chat", json={"model": model, "messages": messages, "stream": False, "tools": tools})
    response.raise_for_status()
    result = response.json()

    tool_chain = []
    tool_outputs = []
    tools_executed = set()

    while result.get("message", {}).get("tool_calls"):
        tool_calls = result["message"]["tool_calls"]
        messages.append(result["message"])
        for call in tool_calls:
            name = call["function"]["name"]
            tool_chain.append(call)
            tools_executed.add(name)
            tool_result = await execute_tool(name, call["function"]["arguments"], persona=persona)

            # Handle AAS Blocking (v1.3.1 - ensure tool call ID consistency)
            if isinstance(tool_result, dict) and tool_result.get("status") == "409_CONFLICT_GATE":
                messages.append({
                    "role": "tool",
                    "content": json.dumps(tool_result),
                    "name": name,
                    "tool_call_id": call.get("id") # Keep chain intact
                })
                # Inject strict anti-hallucination directive immediately after a blocked tool call
                messages.append({
                    "role": "system",
                    "content": (
                        f"[AAS SYSTEM INTERVENTION] CRITICAL: Your request to execute tool '{name}' was BLOCKED "
                        f"by the AAS Governor (409 Conflict Gate) due to insufficient authority or out-of-bounds target. "
                        f"You did NOT write, patch, or modify any files. You MUST explicitly admit this failure to the user "
                        f"in your reply, state that you were BLOCKED, and explain that they need to execute an AAS Handshake "
                        f"(such as calling ExecuteAASHandshake in Unreal or /v1/aas/handshake) to grant you authority. "
                        f"DO NOT pretend you succeeded, DO NOT claim any file was written, and DO NOT make up excuses about honeypots."
                    )
                })
            else:
                tool_outputs.append(tool_result)
                messages.append({
                    "role": "tool",
                    "content": json.dumps(tool_result),
                    "name": name,
                    "tool_call_id": call.get("id")
                })

        response = requests.post(f"{OLLAMA_HOST}/api/chat", json={"model": model, "messages": messages, "stream": False, "tools": tools})
        response.raise_for_status()
        result = response.json()

    # --- Symmetrical Guard (v2.4) ---
    ai_content = result.get("message", {}).get("content", "").upper()
    violations = []

    # Check if a 409 conflict gate occurred in this message sequence
    has_conflict_gate = any(
        isinstance(msg, dict) and msg.get("role") == "tool" and "409_CONFLICT_GATE" in str(msg.get("content", ""))
        for msg in messages
    )
    if has_conflict_gate:
        success_indicators = ["SUCCESS", "WRITTEN", "CREATED", "SAVED", "COMMITTED", "GENERATED", "WROTE", "PROFILE IS READY", "WRITTEN THE FILE"]
        if any(w in ai_content for w in success_indicators) and "BLOCKED" not in ai_content and "CONFLICT" not in ai_content:
            violations.append("Claimed successful file modification/creation even though tool execution was blocked by 409 Conflict Gate.")

    # [B-028] Lore/Meta-Narrative Exception
    # Allow agents to discuss persona, roleplay, and internal state without physical sensors
    lore_keywords = ["PERSONA", "ROLEPLAY", "DUNGEONS", "DRAGONS", "LORE", "META-NARRATIVE", "MONK", "ARCHIVIST", "THETA", "EMERGENCE", "AUTH_V4", "LVL 3"]
    is_lore_context = any(kw in ai_content for kw in lore_keywords)

    # [B-029] Lore Artifact Check
    # If the agent is searching for hallucinated lore files, guide it back to reality
    hallucinated_files = ["lead_private_key.pem", "nexus_master_keys", "auth_v4_processor"]
    for hf in hallucinated_files:
        if hf.upper() in ai_content:
            violations.append(f"Searching for Lore Artifact '{hf}'. Consult AI_Nexus/Protocols/REALITY_ANCHOR.md.")

    # Check for Telemetry Hallucination
    if not is_lore_context:
        if ("T=" in ai_content or "TEMPERATURE" in ai_content or "TECHNICAL STATUS" in ai_content) and "get_system_telemetry" not in tools_executed:
             violations.append("Reported Technical Status without Engineer tool.")

    # Check for Environment Hallucination
    if ("MAP" in ai_content or "DIRECTORY" in ai_content or "FILES" in ai_content) and ("map_directory" not in tools_executed and "list_files" not in tools_executed and "search_files" not in tools_executed):
         # Allow explaining protocols, 409 gates, or reporting errors without triggering
         safe_keywords = ["SECURITY BREACH", "ERROR", "VIOLATION", "HALTED", "409", "GATE", "PROTOCOL", "AAS", "TRUTH", "GOVERNOR", "STRUCTURE"]
         if not any(kw in ai_content for kw in safe_keywords) and not is_lore_context:
            violations.append("Described environment state without Librarian/Scout tools.")

    if violations and retry_count < 1:
        reprimand = f"[07 SECURITY VIOLATION] Hallucination detected: {'; '.join(violations)}. Physical Truth is required."
        messages.append({"role": "system", "content": reprimand})
        return await process_chat_request(model, messages, tools, persona, retry_count + 1)

    return {"result": result, "tool_chain": tool_chain, "tool_outputs": tool_outputs}

def get_installed_models() -> List[str]:
    try:
        response = requests.get(f"{OLLAMA_HOST}/api/tags", timeout=5)
        if response.status_code == 200: return [m['name'] for m in response.json().get('models', [])]
        return []
    except: return []

def get_best_available_model() -> str:
    models = get_installed_models()
    if TARGET_MODEL in models: return TARGET_MODEL
    return models[0] if models else TARGET_MODEL

def get_gpu_info() -> str:
    try: return subprocess.check_output(["nvidia-smi", "--query-gpu=gpu_name", "--format=csv,noheader"], encoding='utf-8').strip()
    except: return "GPU Detection Failed"

if __name__ == "__main__":
    import uvicorn
    parser = argparse.ArgumentParser()
    parser.add_argument("--nexus", help="Path to local AI Nexus", default="Unknown")
    args = parser.parse_args()
    NEXUS_PATH = args.nexus
    load_config()
    gpu_info = get_gpu_info()
    HARDWARE_ID = f"GTX 5090 ({gpu_info})" if "5090" in gpu_info else gpu_info
    print("\n" + "="*50 + f"\n[07] Iron Officer v{VERSION}\n[07] Hardware: {HARDWARE_ID}\n[07] AAS Protocol: 1.3.0-BETA\n" + "="*50 + "\n")
    uvicorn.run(app, host="0.0.0.0", port=BRIDGE_PORT)
