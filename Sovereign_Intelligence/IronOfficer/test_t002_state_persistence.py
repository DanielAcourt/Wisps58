# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for Ticket T-002: Local Verifiable State Persistence & Memory Retention Protocol.
"""

import os
import sys
import unittest
import shutil

# Ensure bridge directory is in path
sys.path.insert(0, os.path.dirname(__file__))

from local_state_tracker import LocalStateTracker


class TestT002StatePersistence(unittest.TestCase):

    def setUp(self):
        self.repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        self.test_cache_dir = "Local_State_Cache_Test"
        self.tracker = LocalStateTracker(repo_root=self.repo_root, cache_dir_name=self.test_cache_dir)

    def tearDown(self):
        test_dir_path = os.path.join(self.repo_root, self.test_cache_dir)
        if os.path.exists(test_dir_path):
            try:
                shutil.rmtree(test_dir_path)
            except Exception:
                pass

    def test_save_and_load_state(self):
        """Verify atomic state saving and loading."""
        key = "test_actor_state"
        data = {"Health": 100, "Location": {"X": 10.0, "Y": 20.0, "Z": 0.0}}

        saved = self.tracker.save_state(key, data)
        self.assertTrue(saved)

        loaded = self.tracker.load_state(key)
        self.assertIsNotNone(loaded)
        self.assertEqual(loaded.get("Health"), 100)
        self.assertEqual(loaded.get("Location", {}).get("X"), 10.0)

    def test_persistence_across_instances(self):
        """Verify state survives new tracker instantiation."""
        key = "persistent_key"
        data = {"handshake_active": True, "vss": 1.0}

        self.tracker.save_state(key, data)

        # Create a new instance pointing to same cache directory
        new_tracker = LocalStateTracker(repo_root=self.repo_root, cache_dir_name=self.test_cache_dir)
        loaded = new_tracker.load_state(key)

        self.assertIsNotNone(loaded)
        self.assertTrue(loaded.get("handshake_active"))

    def test_create_snapshot(self):
        """Verify snapshot creation aggregates active keys into immutable file."""
        self.tracker.save_state("entity_a", {"STR": 18})
        self.tracker.save_state("entity_b", {"INT": 16})

        snapshot_file = self.tracker.create_snapshot("End_Of_Sprint_Test")
        self.assertIsNotNone(snapshot_file)
        self.assertTrue(snapshot_file.startswith("Snapshot_End_Of_Sprint_Test_"))

        snapshot_path = os.path.join(self.tracker.snapshots_dir, snapshot_file)
        self.assertTrue(os.path.exists(snapshot_path))


if __name__ == "__main__":
    unittest.main()
