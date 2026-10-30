# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for AD-021: Tool Error Disambiguation & Handshake Context Synchronization.
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

# Mock fastapi classes used as base models / decorators
mock_fastapi = sys.modules["fastapi"]
mock_fastapi.FastAPI = MagicMock
mock_fastapi.HTTPException = Exception

mock_pydantic = sys.modules["pydantic"]
mock_pydantic.BaseModel = MockBaseModel
mock_pydantic.Field = lambda *args, **kwargs: None

# Ensure bridge directory is in path
sys.path.insert(0, os.path.dirname(__file__))

from bridge import tool_patch_file


class TestAD021ToolErrorSync(unittest.TestCase):

    def test_patch_file_identical_search_replace(self):
        """Verify that tool_patch_file intercepts search == replace before arbitration."""
        async def run_test():
            result = await tool_patch_file(
                filepath="AI_Nexus/DevOps/CURRENT_SPRINT.md",
                search="[Start Date]",
                replace="[Start Date]",
                persona="Lead"
            )
            self.assertEqual(result.get("status"), "INVALID_PARAMETERS")
            self.assertIn("identical", result.get("error", ""))

        asyncio.run(run_test())

    def test_patch_file_missing_search_string(self):
        """Verify that tool_patch_file returns SEARCH_STRING_NOT_FOUND when text is absent."""
        async def run_test():
            result = await tool_patch_file(
                filepath="AI_Nexus/DevOps/CURRENT_SPRINT.md",
                search="NON_EXISTENT_STRING_99999",
                replace="NEW_STRING",
                persona="Lead"
            )
            self.assertEqual(result.get("status"), "SEARCH_STRING_NOT_FOUND")
            self.assertIn("not found", result.get("error", ""))

        asyncio.run(run_test())


if __name__ == "__main__":
    unittest.main()
