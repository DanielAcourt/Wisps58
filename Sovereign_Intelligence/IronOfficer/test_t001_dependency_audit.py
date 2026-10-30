# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for Ticket T-001: Audit External I/O Dependencies & Deprecation Report.
"""

import os
import sys
import unittest


class TestT001DependencyAudit(unittest.TestCase):

    def setUp(self):
        self.repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        self.report_path = os.path.join(self.repo_root, "AI_Nexus", "Deprecation_Report.md")

    def test_deprecation_report_exists(self):
        """Verify that AI_Nexus/Deprecation_Report.md exists on disk."""
        self.assertTrue(os.path.exists(self.report_path), f"Report not found at {self.report_path}")

    def test_deprecation_report_content(self):
        """Verify that AI_Nexus/Deprecation_Report.md contains required audit sections and local replacements."""
        with open(self.report_path, "r", encoding="utf-8") as f:
            content = f.read()

        # Section headings
        self.assertIn("DEPRECATION & LOCALIZATION REPORT", content)
        self.assertIn("EXECUTIVE SUMMARY", content)
        self.assertIn("DEPENDENCY VULNERABILITY MATRIX", content)
        self.assertIn("TECHNICAL DEBT & I/O INVENTORY", content)
        self.assertIn("ACTIONABLE MIGRATION ROADMAP", content)

        # Architectural replacement mappings
        self.assertIn("LocalBridge", content)
        self.assertIn("InferenceEngineInterceptor", content)
        self.assertIn("LocalAuthMock", content)
        self.assertIn("LocalStateTracker", content)


if __name__ == "__main__":
    unittest.main()
