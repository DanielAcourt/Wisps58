# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for Ticket AD-023: Iron Coder Autonomous Multi-Turn Execution Loop & Subprocess Harness.
"""

import os
import sys
import unittest
import asyncio
from unittest.mock import MagicMock


class MockBaseModel:
    def __init__(self, **kwargs):
        for k, v in kwargs.items():
            setattr(self, k, v)

    def model_dump(self, *args, **kwargs):
        return self.__dict__


# Mock third-party modules if not installed in sandbox environment
for mod in ["requests", "uvicorn", "fastapi", "pydantic"]:
    if mod not in sys.modules:
        sys.modules[mod] = MagicMock()

mock_fastapi = sys.modules["fastapi"]
mock_fastapi.FastAPI = MagicMock
mock_fastapi.HTTPException = Exception

mock_pydantic = sys.modules["pydantic"]
mock_pydantic.BaseModel = MockBaseModel
mock_pydantic.Field = lambda *args, **kwargs: None

# Ensure bridge directory is in path
sys.path.insert(0, os.path.dirname(__file__))

from bridge import tool_run_command
from iron_coder import IronCoderAgent


class TestAD023IronCoder(unittest.TestCase):

    def test_run_command_success(self):
        """Verify tool_run_command executes safe shell commands and returns output."""
        async def run_test():
            result = await tool_run_command(
                command="python3 --version",
                persona="Lead"
            )
            self.assertEqual(result.get("status"), "success")
            self.assertEqual(result.get("returncode"), 0)
            self.assertIn("Python", result.get("stdout", ""))

        asyncio.run(run_test())

    def test_run_command_blocks_destructive_cmds(self):
        """Verify tool_run_command blocks destructive commands."""
        async def run_test():
            result = await tool_run_command(
                command="rm -rf /",
                persona="Lead"
            )
            self.assertIn("error", result)
            self.assertIn("forbidden", result.get("error", "").lower())

        asyncio.run(run_test())

    def test_iron_coder_agent_instantiation(self):
        """Verify IronCoderAgent initialization and verification pipeline."""
        agent = IronCoderAgent(max_turns=5, persona="Tactician")
        self.assertEqual(agent.max_turns, 5)
        self.assertEqual(agent.persona, "Tactician")

        verification = agent.verify_solution()
        self.assertIn("passed", verification)


if __name__ == "__main__":
    unittest.main()
