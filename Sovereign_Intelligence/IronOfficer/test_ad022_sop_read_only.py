# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for AD-022: Protocol SOP Read-Only Grounding & Review Artifact Target Routing.
"""

import os
import sys
import unittest
import asyncio


class MockBaseModel:
    def __init__(self, **kwargs):
        for k, v in kwargs.items():
            setattr(self, k, v)

    def model_dump(self, *args, **kwargs):
        return self.__dict__


# Mock third-party modules if not installed in sandbox environment
for mod in ["requests", "uvicorn", "fastapi", "pydantic"]:
    if mod not in sys.modules:
        sys.modules[mod] = unittest.mock.MagicMock() if hasattr(unittest, "mock") else None

from unittest.mock import MagicMock

for mod in ["requests", "uvicorn", "fastapi", "pydantic"]:
    if sys.modules.get(mod) is None:
        sys.modules[mod] = MagicMock()

mock_fastapi = sys.modules["fastapi"]
mock_fastapi.FastAPI = MagicMock
mock_fastapi.HTTPException = Exception

mock_pydantic = sys.modules["pydantic"]
mock_pydantic.BaseModel = MockBaseModel
mock_pydantic.Field = lambda *args, **kwargs: None

# Ensure bridge directory is in path
sys.path.insert(0, os.path.dirname(__file__))

from bridge import tool_write_file, tool_patch_file


class TestAD022SOPReadOnly(unittest.TestCase):

    def test_write_file_blocked_on_protocol_sop(self):
        """Verify that tool_write_file is blocked on AI_Nexus/Protocols/ files."""
        async def run_test():
            result = await tool_write_file(
                filepath="AI_Nexus/Protocols/END_OF_SPRINT_PROTOCOL.md",
                content="New SOP Content",
                persona="Lead"
            )
            self.assertEqual(result.get("status"), "SOP_READ_ONLY_PROTECTED")
            self.assertIn("READ-ONLY", result.get("error", ""))

        asyncio.run(run_test())

    def test_patch_file_blocked_on_protocol_sop(self):
        """Verify that tool_patch_file is blocked on AI_Nexus/Protocols/ files."""
        async def run_test():
            result = await tool_patch_file(
                filepath="AI_Nexus/Protocols/AAS_Protocol.md",
                search="Version 1.3.0",
                replace="Version 2.0.0",
                persona="Lead"
            )
            self.assertEqual(result.get("status"), "SOP_READ_ONLY_PROTECTED")
            self.assertIn("READ-ONLY", result.get("error", ""))

        asyncio.run(run_test())


if __name__ == "__main__":
    unittest.main()
