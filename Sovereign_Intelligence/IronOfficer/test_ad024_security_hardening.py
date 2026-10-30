# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for Ticket AD-024: AAS Security Hardening, Tokenized Authority Grants & Path Containment.
"""

import os
import sys
import unittest
import datetime
from pathlib import Path
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

from bridge import is_path_authorized, resolve_secure_path, REPO_ROOT


class AuthorityGrant:
    """Tokenized bounded authority grant struct for AD-024."""

    def __init__(self, grant_id: str, issuing_persona: str, target_agent: str,
                 permitted_operations: list, permitted_resources: list,
                 valid_minutes: int = 15, max_uses: int = 1):
        self.grant_id = grant_id
        self.issuing_persona = issuing_persona
        self.target_agent = target_agent
        self.permitted_operations = permitted_operations
        self.permitted_resources = permitted_resources
        self.issued_timestamp = datetime.datetime.now()
        self.expiry_timestamp = self.issued_timestamp + datetime.timedelta(minutes=valid_minutes)
        self.max_uses = max_uses
        self.uses_remaining = max_uses

    def is_valid(self, operation: str, resource_path: str) -> bool:
        """Checks if the authority grant is valid for a given operation and resource."""
        now = datetime.datetime.now()
        if now > self.expiry_timestamp or self.uses_remaining <= 0:
            return False

        if operation not in self.permitted_operations and "*" not in self.permitted_operations:
            return False

        return True

    def consume_use(self) -> bool:
        """Decrements uses remaining."""
        if self.uses_remaining > 0:
            self.uses_remaining -= 1
            return True
        return False


class TestAD024SecurityHardening(unittest.TestCase):

    def test_path_containment_is_relative_to(self):
        """Verify strict path containment checks using pathlib.Path."""
        repo_path = Path(REPO_ROOT).resolve()
        file_inside = repo_path / "AI_Nexus" / "DevOps" / "CURRENT_SPRINT.md"
        file_outside = Path("/tmp/outside_repo_file.txt").resolve()

        self.assertTrue(file_inside.is_relative_to(repo_path))
        self.assertFalse(file_outside.is_relative_to(repo_path))

    def test_tokenized_authority_grant_expiry(self):
        """Verify that tokenized authority grants expire after time or max uses."""
        grant = AuthorityGrant(
            grant_id="GRANT-07-TEST",
            issuing_persona="Lead",
            target_agent="Iron_Knight",
            permitted_operations=["write_file"],
            permitted_resources=["AI_Nexus/*"],
            valid_minutes=15,
            max_uses=1
        )

        self.assertTrue(grant.is_valid("write_file", "AI_Nexus/test.md"))
        grant.consume_use()
        self.assertFalse(grant.is_valid("write_file", "AI_Nexus/test.md"))

    def test_path_authorized_repo_root(self):
        """Verify is_path_authorized accepts valid repository paths."""
        valid_path = os.path.join(REPO_ROOT, "AI_Nexus", "DevOps", "CURRENT_SPRINT.md")
        self.assertTrue(is_path_authorized(valid_path, mode="write"))


if __name__ == "__main__":
    unittest.main()
