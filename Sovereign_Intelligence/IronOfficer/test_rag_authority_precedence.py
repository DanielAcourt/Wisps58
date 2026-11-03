# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for SSoT Authority Precedence and Provenance Headers in Sovereign RAG.
"""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))

from rag import determine_ssot_level_and_tier, SovereignRAG


class TestRAGAuthorityPrecedence(unittest.TestCase):

    def test_determine_ssot_level_and_tier(self):
        """Verify that paths map to correct SSoT levels and Authority Tiers."""
        level1, tier1, prio1 = determine_ssot_level_and_tier("AI_Nexus/Protocols/AAS_Protocol.md")
        self.assertEqual(level1, 1)
        self.assertIn("Tier 1", tier1)
        self.assertEqual(prio1, 100)

        level2, tier2, prio2 = determine_ssot_level_and_tier("AI_Nexus/Docs/GDD.md")
        self.assertEqual(level2, 3)
        self.assertIn("Tier 2", tier2)
        self.assertEqual(prio2, 80)

        level3, tier3, prio3 = determine_ssot_level_and_tier("WispsCPPVR/Source/BaseCharacter.cpp")
        self.assertEqual(level3, 2)
        self.assertIn("Tier 3", tier3)
        self.assertEqual(prio3, 70)

        level4, tier4, prio4 = determine_ssot_level_and_tier("AI_Nexus/DevOps/CURRENT_SPRINT.md")
        self.assertEqual(level4, 4)
        self.assertIn("Tier 4", tier4)
        self.assertEqual(prio4, 50)

        level5, tier5, prio5 = determine_ssot_level_and_tier("AI_Nexus/Timeline/SprintReviews/SR_20260928.md")
        self.assertEqual(level5, 0)
        self.assertIn("Tier 5", tier5)
        self.assertEqual(prio5, 10)

    def test_chunk_authority_tier_metadata(self):
        """Verify that indexed chunks retain authority_tier metadata."""
        repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        rag = SovereignRAG(repo_root=repo_root)

        rag.add_chunk(
            rel_path="AI_Nexus/Protocols/END_OF_SPRINT_PROTOCOL.md",
            level=1,
            header="Overview",
            text="End of sprint protocol instructions."
        )

        self.assertEqual(len(rag.chunks), 1)
        chunk = rag.chunks[0]
        self.assertEqual(chunk.get("level"), 1)
        self.assertIn("Tier 1", chunk.get("authority_tier", ""))


if __name__ == "__main__":
    unittest.main()
