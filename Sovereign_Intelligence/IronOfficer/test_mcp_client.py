# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
"""Unit tests for SovereignMCPClient (Unreal Engine 5.8.1 Loopback MCP Client Bridge)."""

import unittest
from unittest.mock import patch, MagicMock
import tempfile
import os
import json
from mcp_client import SovereignMCPClient


class TestSovereignMCPClient(unittest.TestCase):

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.cache_path = os.path.join(self.temp_dir.name, "test_mcp_cache.json")
        self.client = SovereignMCPClient(mcp_url="http://127.0.0.1:8000/mcp", cache_path=self.cache_path)

    def tearDown(self):
        self.temp_dir.cleanup()

    @patch("mcp_client.requests.post")
    def test_initialize(self, mock_post):
        mock_response = MagicMock()
        mock_response.json.return_value = {
            "jsonrpc": "2.0",
            "id": 1,
            "result": {"serverInfo": {"name": "UnrealEngine_MCP", "version": "5.8.1"}}
        }
        mock_response.raise_for_status = MagicMock()
        mock_post.return_value = mock_response

        res = self.client.initialize()
        self.assertIn("serverInfo", res)
        self.assertEqual(res["serverInfo"]["name"], "UnrealEngine_MCP")

    @patch("mcp_client.requests.post")
    def test_list_tools_and_cache_writing(self, mock_post):
        mock_response = MagicMock()
        mock_response.json.return_value = {
            "jsonrpc": "2.0",
            "id": 1,
            "result": {
                "tools": [
                    {"name": "ActorTools_SpawnActor", "description": "Spawns an actor"},
                    {"name": "Query_ListActors", "description": "Lists actors in level"}
                ]
            }
        }
        mock_response.raise_for_status = MagicMock()
        mock_post.return_value = mock_response

        tools = self.client.list_tools()
        self.assertEqual(len(tools), 2)
        self.assertTrue(os.path.exists(self.cache_path))

        with open(self.cache_path, "r", encoding="utf-8") as f:
            data = json.load(f)
            self.assertEqual(len(data["tools"]), 2)

    def test_vss_gating_and_409_conflict(self):
        # Destructive tool requires VSS >= 1.0; if VSS is 0.5, raise PermissionError
        with self.assertRaises(PermissionError):
            self.client.call_tool("ActorTools_DeleteActor", {"actor_id": "123"}, vss_score=0.5, handshake_active=False)

        # Mutation tool requires VSS >= 0.7; if VSS is 0.5, raise PermissionError
        with self.assertRaises(PermissionError):
            self.client.call_tool("ActorTools_SpawnActor", {"class": "AActor"}, vss_score=0.5, handshake_active=False)

    @patch("mcp_client.requests.post")
    def test_call_tool_permitted(self, mock_post):
        mock_response = MagicMock()
        mock_response.json.return_value = {
            "jsonrpc": "2.0",
            "id": 1,
            "result": {"content": [{"type": "text", "text": "Actor spawned successfully"}]}
        }
        mock_response.raise_for_status = MagicMock()
        mock_post.return_value = mock_response

        # Mutation tool call permitted when VSS >= 0.7
        res = self.client.call_tool("ActorTools_SpawnActor", {"class": "AActor"}, vss_score=0.85)
        self.assertIn("content", res)

    @patch("mcp_client.requests.post")
    def test_handshake_boost_permits_tool(self, mock_post):
        mock_response = MagicMock()
        mock_response.json.return_value = {
            "jsonrpc": "2.0",
            "id": 1,
            "result": {"content": [{"type": "text", "text": "Actor deleted successfully"}]}
        }
        mock_response.raise_for_status = MagicMock()
        mock_post.return_value = mock_response

        # Destructive tool requires VSS >= 1.0. Raw VSS is 0.5, but Handshake Active adds +0.5 -> Effective 1.0
        res = self.client.call_tool("ActorTools_DeleteActor", {"actor_id": "123"}, vss_score=0.5, handshake_active=True)
        self.assertIn("content", res)

    @patch("mcp_client.requests.post")
    def test_offline_fallback(self, mock_post):
        import requests
        mock_post.side_effect = requests.exceptions.RequestException("Connection refused")

        res = self.client.send_jsonrpc("tools/list")
        self.assertEqual(res.get("status"), "offline")
        self.assertIn("Connection refused", res.get("error", ""))


if __name__ == "__main__":
    unittest.main()
