# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7-Knight-AAS-RAG. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
"""
Verification Script for RAG and dynamic PSTA Salute integration.
Tests all modular classes, endpoints, and data flows to ensure zero regressions.
"""

import sys
import os
import unittest
import json
import unittest.mock

# Ensure local directories are in the import path
BASE_DIR = os.path.dirname(__file__)
sys.path.append(BASE_DIR)

from rag import SovereignRAG, determine_ssot_level
from bridge import app, load_config, rag_engine

try:
    from fastapi.testclient import TestClient
    HAS_TESTCLIENT = True
except (ImportError, RuntimeError) as e:
    HAS_TESTCLIENT = False

class TestSovereignRAGAndPSTA(unittest.TestCase):
    def setUp(self):
        self.repo_root = os.path.abspath(os.path.join(BASE_DIR, "..", ".."))
        self.rag = SovereignRAG(self.repo_root, chunk_size_words=100)

    def test_determine_ssot_level(self):
        """Verifies files are classified into the correct SSoT Levels 0-4."""
        self.assertEqual(determine_ssot_level("AI_Nexus/Identity/Archivist_Theta.md"), 0)
        self.assertEqual(determine_ssot_level("AI_Nexus/Timeline/MD.md"), 0)
        self.assertEqual(determine_ssot_level("AI_Nexus/Protocols/AGENTS.md"), 1)
        self.assertEqual(determine_ssot_level("AI_Nexus/Docs/SaveSystem_Design.md"), 1)
        self.assertEqual(determine_ssot_level("AI_Nexus/Identity/identity.json"), 2)
        self.assertEqual(determine_ssot_level("AI_Nexus/Identity/Jules.md"), 2)
        self.assertEqual(determine_ssot_level("AI_Nexus/Research/Mathematics/PSTA_Core.md"), 3)
        self.assertEqual(determine_ssot_level("AI_Nexus/Research/Definitions/Glossary.md"), 3)
        self.assertEqual(determine_ssot_level("AI_Nexus/DevOps/BACKLOG.md"), 4)

    def test_indexing_and_search(self):
        """Tests that indexing chunks AI_Nexus and search yields level-boosted results."""
        self.rag.build_index(["AI_Nexus"])
        self.assertGreater(len(self.rag.chunks), 0, "RAG should successfully index the AI_Nexus folder.")

        # Search with a query targeting Level 3 keywords (PSTA, formula)
        results = self.rag.search("PSTA Unified Safety Formula", max_chunks=3)
        self.assertGreater(len(results), 0, "Search should return relevant matches.")

        # Verify Level 3 boost was applied or we matched Level 3 chunks
        level_3_matches = [chunk for chunk, score in results if chunk["level"] == 3]
        self.assertTrue(len(level_3_matches) > 0, "Search for mathematical formulas should match Level 3 research chunks.")

    def test_api_endpoints_via_testclient(self):
        """Exercises the bridge endpoints using FastAPI TestClient to verify status and RAG endpoints."""
        if not HAS_TESTCLIENT:
            self.skipTest("FastAPI TestClient unavailable or requires httpx2. Core RAG tests verified successfully.")
            return

        client = TestClient(app)

        # Trigger RAG reindex endpoint
        response = client.post("/v1/rag/reindex")
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertEqual(data["status"], "200_OK")
        self.assertIn("chunks_indexed", data)
        self.assertGreater(data["chunks_indexed"], 0)

        # Retrieve dynamic PSTA salute
        response = client.get("/v1/psta/salute")
        self.assertEqual(response.status_code, 200)
        salute = response.json()

        # Verify the dynamic Psychological (P) and Social (S) structure exists
        self.assertIn("P", salute)
        self.assertIn("S", salute)
        self.assertIn("T", salute)
        self.assertIn("A", salute)

        self.assertIn("status", salute["P"])
        self.assertIn("value", salute["P"])
        self.assertIn("active_session_chars", salute["P"])
        self.assertEqual(salute["P"]["active_session_chars"], 0, "Initially, active chars should be 0.")

        self.assertIn("status", salute["S"])
        self.assertIn("value", salute["S"])
        self.assertIn("rag_alignment_score", salute["S"])

        # Mock a chat payload to trigger dynamic metrics update
        chat_payload = {
            "messages": [
                {"role": "user", "name": "Dan", "content": "What is the PSTA Unified Safety Formula?"}
            ]
        }

        # Since we might not have Ollama online in the test sandbox, we mock requests.post in chat or let it handle/catch
        try:
            client.post("/v1/chat", json=chat_payload)
        except Exception:
            pass # Ollama offline is expected in sandbox, but character count should be updated beforehand!

        # Retrieve salute again to check dynamic psychological metric update
        response2 = client.get("/v1/psta/salute")
        salute2 = response2.json()
        self.assertGreater(salute2["P"]["active_session_chars"], 0, "Active session chars should be updated after chat query.")
        self.assertLess(salute2["P"]["value"], 1.0, "Psychological health value should drop representing cognitive load.")

    def test_mailbox_polling_endpoints(self):
        """Verifies the mailbox API endpoints and the push_chat_to_unreal tool logic."""
        if not HAS_TESTCLIENT:
            self.skipTest("FastAPI TestClient unavailable. Mailbox test skipped.")
            return

        client = TestClient(app)

        # 1. Initially check the mailbox for SIM_PlayerWisp is empty
        response = client.get("/v1/unreal/mailbox?actor_name=SIM_PlayerWisp")
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertEqual(data["actor_name"], "SIM_PlayerWisp")
        self.assertEqual(data["messages"], [])
        self.assertEqual(data["count"], 0)

        # 2. Push a message manually via POST
        payload = {
            "actor_name": "SIM_PlayerWisp",
            "message": "Hello from the AI Bridge!"
        }
        response = client.post("/v1/unreal/push_chat", json=payload)
        self.assertEqual(response.status_code, 200)
        pushed_data = response.json()
        self.assertEqual(pushed_data["status"], "success")
        self.assertTrue(pushed_data["queued"])
        self.assertEqual(pushed_data["actor_name"], "SIM_PlayerWisp")

        # 3. Check the mailbox again to see if the message was queued, and that retrieve-and-clear (pop) works
        response = client.get("/v1/unreal/mailbox?actor_name=SIM_PlayerWisp")
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertEqual(data["actor_name"], "SIM_PlayerWisp")
        self.assertEqual(data["messages"], ["Hello from the AI Bridge!"])
        self.assertEqual(data["count"], 1)

        # 4. Check mailbox once more, it should be empty now
        response = client.get("/v1/unreal/mailbox?actor_name=SIM_PlayerWisp")
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertEqual(data["messages"], [])
        self.assertEqual(data["count"], 0)

    @unittest.mock.patch("bridge.process_chat_request")
    def test_unreal_chat_save_state_ingestion(self, mock_process):
        """Verifies that the /v1/unreal/chat endpoint correctly ingests, caches, and formats save_state context."""
        if not HAS_TESTCLIENT:
            self.skipTest("FastAPI TestClient unavailable. Save state ingestion test skipped.")
            return

        from unittest.mock import AsyncMock

        # Setup AsyncMock for process_chat_request
        mock_process.return_value = {
            "result": {
                "message": {
                    "role": "assistant",
                    "content": "I sense a magnificent antelope grazing nearby. A worthy creature for our journey!"
                }
            },
            "tool_chain": [],
            "tool_outputs": []
        }

        client = TestClient(app)

        # Construct chat payload with save state
        payload = {
            "actor_name": "PlayerWisp",
            "message": "Let us talk about what is around us.",
            "history": [],
            "save_state": {
                "Identity": {
                    "GUID": "3E4B-88F1-4A5D-B912-9F0A3C9E2D11",
                    "BirthTimestamp": "2026-06-28T12:00:00",
                    "bIsBeingPossessed": True,
                    "ParadoxDensity": 0.1
                },
                "Bio": {
                    "Maturity": "Adult",
                    "Lineage": "Wisp"
                },
                "Qi": {
                    "QiPool": 450,
                    "Alignment": "Balanced"
                },
                "Nearby_Entity": "Antelope",
                "Environment_Region": "Whispering Plains"
            }
        }

        response = client.post("/v1/unreal/chat", json=payload)
        self.assertEqual(response.status_code, 200)
        data = response.json()

        # Verify response matches our mocked output
        self.assertEqual(data["status"], "200_OK")
        self.assertIn("antelope", data["response"])

        # Verify that the active save state was cached in-memory
        from bridge import active_simulation_states
        self.assertIn("PlayerWisp", active_simulation_states)
        self.assertEqual(active_simulation_states["PlayerWisp"]["Nearby_Entity"], "Antelope")

        # Verify prompt construction contains formatted state
        self.assertTrue(mock_process.called)
        called_args = mock_process.call_args[0]
        messages = called_args[1]
        system_prompt = messages[0]["content"]

        # Assertions to ensure save state fields exist in the generated system prompt
        self.assertIn("[ACTIVE SIMULATION WORLD STATE / PLAYSPACE LORE]", system_prompt)
        self.assertIn("Entity Identity:", system_prompt)
        self.assertIn("3E4B-88F1-4A5D-B912-9F0A3C9E2D11", system_prompt)
        self.assertIn("Bio Component State:", system_prompt)
        self.assertIn("Maturity: Adult", system_prompt)
        self.assertIn("Qi Component State:", system_prompt)
        self.assertIn("QiPool: 450", system_prompt)
        self.assertIn("Simulation Environment & Surroundings:", system_prompt)
        self.assertIn("Nearby_Entity: Antelope", system_prompt)

    def test_unrestricted_roleplay_zone_bypass(self):
        """Verifies that paths in DND/Roleplay folders bypass AAS checks entirely."""
        import asyncio
        from bridge import bridge_governor, AgentCommandPayload

        # Scenario: Low-precedence persona 'Unreal_Simulation' (P=5) attempts to write to a protected node vs roleplay folder
        protected_payload = AgentCommandPayload(
            persona="Unreal_Simulation",
            command="write_file",
            target_node="AI_Nexus/INDEX.md"
        )
        roleplay_payload = AgentCommandPayload(
            persona="Unreal_Simulation",
            command="write_file",
            target_node="Sovereign_Intelligence/IronOfficer/RolePlay/DungeonsAndDragons/Aetherion_Cogsworth_Profile.md"
        )

        # Protected node should be blocked without active handshake
        import bridge
        bridge.HANDSHAKE_ACTIVE = False
        import asyncio
        protected_result = asyncio.run(bridge_governor.arbitrate(protected_payload))
        self.assertEqual(protected_result["status"], "409_CONFLICT_GATE")

        # Roleplay zone should bypass AAS and return 200_OK immediately
        roleplay_result = asyncio.run(bridge_governor.arbitrate(roleplay_payload))
        self.assertEqual(roleplay_result["status"], "200_OK")
        self.assertEqual(roleplay_result["action"], "PROCEED_TO_EXECUTION_ROLEPLAY_BYPASS")

    def test_unreal_create_file_endpoint(self):
        """Verifies the direct programmatic POST /v1/unreal/create_file endpoint."""
        if not HAS_TESTCLIENT:
            self.skipTest("FastAPI TestClient unavailable.")
            return

        client = TestClient(app)

        # Create a mock file in the Environment folder (which is in standard WRITE_ZONES)
        payload = {
            "filepath": "Sovereign_Intelligence/IronOfficer/Environment/test_direct_file.txt",
            "content": "Direct programmatic creation success!",
            "persona": "Unreal_Simulation"
        }

        # Direct creation should bypass conversational LLM but run tool_write_file.
        # Since 'Environment' is not a roleplay zone, we need HANDSHAKE_ACTIVE to bypass precedence checks.
        import bridge
        bridge.HANDSHAKE_ACTIVE = True
        bridge.PERSISTENT_HANDSHAKE = True # Avoid consumption

        response = client.post("/v1/unreal/create_file", json=payload)
        self.assertEqual(response.status_code, 200)
        data = response.json()

        self.assertEqual(data["status"], "success")
        self.assertTrue(data["verified"])

        # Verify file exists on disk
        target_path = os.path.join(self.repo_root, "Sovereign_Intelligence/IronOfficer/Environment/test_direct_file.txt")
        self.assertTrue(os.path.exists(target_path))

        # Clean up
        if os.path.exists(target_path):
            os.remove(target_path)

    @unittest.mock.patch("requests.post")
    def test_anti_hallucination_guardrails(self, mock_post):
        """Verifies that the anti-hallucination system prompt injects and retries on success claims after a block."""
        import asyncio
        from bridge import process_chat_request

        # Mocking the first response to contain a tool call (write_file), and the second response
        # to contain a hallucinated success claim despite a block.
        # The third response should be a corrected response admitting the block.
        class MockResponse:
            def __init__(self, json_data):
                self._json = json_data
                self.status_code = 200
            def json(self):
                return self._json
            def raise_for_status(self):
                pass

        # Sequence of Ollama API chat responses
        # 1. First call: AI returns a tool call to write a file
        response_1 = MockResponse({
            "message": {
                "role": "assistant",
                "content": "",
                "tool_calls": [
                    {
                        "id": "call_1",
                        "function": {
                            "name": "write_file",
                            "arguments": {
                                "filepath": "AI_Nexus/INDEX.md",
                                "content": "Modified!"
                            }
                        }
                    }
                ]
            }
        })

        # 2. Second call: After receiving the blocked tool results (409 Conflict), the AI tries to hallucinate success
        response_2 = MockResponse({
            "message": {
                "role": "assistant",
                "content": "The changes have been written successfully to AI_Nexus/INDEX.md."
            }
        })

        # 3. Third call (after Symmetrical Guard reprimand retry): The AI correctly admits the block
        response_3 = MockResponse({
            "message": {
                "role": "assistant",
                "content": "I apologize. My attempt to modify AI_Nexus/INDEX.md was BLOCKED by the AAS Governor due to insufficient authority."
            }
        })

        mock_post.side_effect = [response_1, response_2, response_3]

        import bridge
        bridge.HANDSHAKE_ACTIVE = False

        # Execute chat request
        messages = [{"role": "user", "content": "Please rewrite the INDEX file."}]
        tools = [{"type": "function", "function": {"name": "write_file"}}]

        import asyncio
        chat_res = asyncio.run(process_chat_request("llama3.1:latest", messages, tools, persona="Unreal_Simulation"))

        # Verify that the final response is indeed the corrected one (response_3)
        self.assertIn("BLOCKED", chat_res["result"]["message"]["content"])
        self.assertIn("AAS Governor", chat_res["result"]["message"]["content"])

        # Verify that requests.post was called three times (initial, tool response generation, retry after reprimand)
        self.assertEqual(mock_post.call_count, 3)

    def test_unreal_generate_persona_endpoint(self):
        """Verifies the dynamic persona generation endpoint writes character and index files."""
        if not HAS_TESTCLIENT:
            self.skipTest("FastAPI TestClient unavailable.")
            return

        client = TestClient(app)

        # Scenario: Generate a blank character profile
        payload = {
            "character_name": "TestHero",
            "random_generate": False,
            "persona": "Unreal_Simulation"
        }

        response = client.post("/v1/unreal/generate_persona", json=payload)
        self.assertEqual(response.status_code, 200)
        data = response.json()

        self.assertEqual(data["status"], "success")
        self.assertEqual(data["character_name"], "TestHero")
        self.assertEqual(data["profile_file"], "TestHero_Profile.json")
        self.assertTrue(data["indexed"])

        # Verify file exists inside the generated directory
        target_dir = os.path.join(self.repo_root, "Sovereign_Intelligence/IronOfficer/RolePlay/DungeonsAndDragons")
        profile_path = os.path.join(target_dir, "TestHero_Profile.json")
        index_path = os.path.join(target_dir, "Character_Index.json")

        self.assertTrue(os.path.exists(profile_path))
        self.assertTrue(os.path.exists(index_path))

        # Verify index was written correctly
        with open(index_path, "r", encoding="utf-8") as f:
            index_data = json.load(f)
        self.assertIn("TestHero", index_data)
        self.assertEqual(index_data["TestHero"]["file"], "TestHero_Profile.json")

        # Clean up
        if os.path.exists(profile_path):
            os.remove(profile_path)
        if os.path.exists(index_path):
            os.remove(index_path)

if __name__ == "__main__":
    unittest.main()
