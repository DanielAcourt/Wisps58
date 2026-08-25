# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
"""
Sovereign MCP Client (Local Loopback Bridge to Unreal Engine 5.8.1 ModelContextProtocol)

Connects the Iron Knight agent and local Python bridge directly to Unreal Engine 5.8.1's
embedded HTTP loopback MCP server (defaulting to http://127.0.0.1:8000/mcp).
Enforces local sovereignty, token-free editor automation, and AAS VSS safety gating.
"""

import json
import logging
import os
from typing import Dict, Any, Optional, List
import requests

logger = logging.getLogger("AAS.SovereignMCP")


class SovereignMCPClient:
    """Lightweight JSON-RPC 2.0 loopback client for Unreal Engine 5.8.1 embedded MCP Server."""

    def __init__(self, mcp_url: str = "http://127.0.0.1:8000/mcp", cache_path: Optional[str] = None):
        self.mcp_url = mcp_url
        self.cache_path = cache_path or os.path.join(os.path.dirname(__file__), "unreal_mcp_cache.json")
        self.request_id = 1
        self.cached_tools: Dict[str, Dict[str, Any]] = {}

    def _next_id(self) -> int:
        req_id = self.request_id
        self.request_id += 1
        return req_id

    def send_jsonrpc(self, method: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Dispatches a JSON-RPC 2.0 request to the Unreal loopback MCP endpoint."""
        payload = {
            "jsonrpc": "2.0",
            "id": self._next_id(),
            "method": method,
            "params": params or {}
        }
        headers = {"Content-Type": "application/json"}

        try:
            response = requests.post(self.mcp_url, json=payload, headers=headers, timeout=5)
            response.raise_for_status()
            data = response.json()
            if "error" in data:
                logger.error(f"Unreal MCP Error [{method}]: {data['error']}")
                raise RuntimeError(f"Unreal MCP Error: {data['error']}")
            return data.get("result", {})
        except requests.exceptions.RequestException as exc:
            logger.warning(f"Unreal MCP Server offline or unreachable at {self.mcp_url}: {exc}")
            return {"status": "offline", "error": str(exc)}

    def initialize(self) -> Dict[str, Any]:
        """Initializes the Model Context Protocol session with the Unreal Engine Editor."""
        logger.info(f"Initializing MCP loopback connection to {self.mcp_url}...")
        result = self.send_jsonrpc("initialize", {
            "protocolVersion": "2024-11-05",
            "capabilities": {
                "tools": {}
            },
            "clientInfo": {
                "name": "Sovereign_IronKnight_MCP_Client",
                "version": "36.5.0"
            }
        })
        return result

    def list_tools(self) -> List[Dict[str, Any]]:
        """Discovers active tools registered in Unreal Engine 5.8 and updates local schema cache."""
        result = self.send_jsonrpc("tools/list")
        tools = result.get("tools", [])

        # Update local cached tools dictionary
        self.cached_tools = {tool["name"]: tool for tool in tools if "name" in tool}

        # Persist schema cache to disk for offline RAG / R&D reference
        try:
            with open(self.cache_path, "w", encoding="utf-8") as f:
                json.dump({"tools": tools}, f, indent=2)
            logger.info(f"Updated Unreal MCP tool schema cache ({len(tools)} tools cached at {self.cache_path}).")
        except Exception as exc:
            logger.error(f"Failed to write MCP cache to {self.cache_path}: {exc}")

        return tools

    def call_tool(self, tool_name: str, tool_args: Dict[str, Any], vss_score: float = 1.0) -> Dict[str, Any]:
        """
        Executes a registered Unreal MCP tool gated by AAS VSS safety thresholds.

        - Observation / Query Tools: Requires VSS >= 0.4
        - Mutation Tools (Spawn, Transform, Modify): Requires VSS >= 0.7
        - Destructive / Critical Tools (Delete, Force Halt): Requires VSS >= 1.0
        """
        # AAS Risk Classification
        required_vss = 0.4
        is_mutation = any(kw in tool_name.lower() for kw in ["spawn", "transform", "set", "modify", "override"])
        is_destructive = any(kw in tool_name.lower() for kw in ["delete", "remove", "clear", "destroy", "halt"])

        if is_destructive:
            required_vss = 1.0
        elif is_mutation:
            required_vss = 0.7

        if vss_score < required_vss:
            error_msg = f"409 CONFLICT GATE: Tool '{tool_name}' requires VSS >= {required_vss:.2f}, but current VSS is {vss_score:.2f}."
            logger.critical(error_msg)
            raise PermissionError(error_msg)

        logger.info(f"Dispatching MCP tool call '{tool_name}' (VSS {vss_score:.2f} >= required {required_vss:.2f})...")
        return self.send_jsonrpc("tools/call", {
            "name": tool_name,
            "arguments": tool_args
        })


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    client = SovereignMCPClient()
    print("Sovereign MCP Client initialized.")
