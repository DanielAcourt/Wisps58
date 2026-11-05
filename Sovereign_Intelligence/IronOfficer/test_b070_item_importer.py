# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07
"""
Unit test suite verifying Ticket B-070 Automated Item Definition Synthesis.
Tests CSV parsing, fragment assignment logic, and dry-run execution.
"""

import os
import sys
import unittest

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PYTHON_CONTENT_DIR = os.path.join(REPO_ROOT, "WispsCPPVR", "Content", "Python")
if PYTHON_CONTENT_DIR not in sys.path:
    sys.path.insert(0, PYTHON_CONTENT_DIR)

import import_item_definitions


class TestB070ItemImporter(unittest.TestCase):

    def setUp(self):
        self.csv_path = os.path.join(REPO_ROOT, "AI_Nexus", "Data", "DT_SovereignSpeciesTemplate.csv")

    def test_csv_file_exists(self):
        self.assertTrue(os.path.exists(self.csv_path), f"CSV path missing: {self.csv_path}")

    def test_parse_csv_item_rows(self):
        records = import_item_definitions.parse_csv_item_rows(self.csv_path)
        self.assertGreaterEqual(len(records), 70, f"Expected at least 70 records, got {len(records)}")

        # Verify specific items
        names = [r["RawName"] for r in records]
        self.assertIn("Species_SilverDragon", names)
        self.assertIn("Species_AncientOak", names)
        self.assertIn("Item_IronOre", names)
        self.assertIn("Item_BronzeIngot", names)
        self.assertIn("Species_Mushroom", names)

    def test_determine_required_fragments_for_mineral(self):
        # Iron ore should have UIData, Weight, and Spiritual (since QiGive=5.0), but NOT Consumable
        record = {
            "RawName": "Item_IronOre",
            "AssetName": "DA_IronOre",
            "ItemTag": "Item.Material.Ore.Iron",
            "DisplayName": "Iron Ore",
            "Weight": 10.0,
            "MaxHunger": 0.0,
            "MaxHydration": 0.0,
            "QiGive": 5.0,
            "QiTake": 5.0,
            "QiAuraRadius": 500.0,
            "StrengthXP": 0.0,
            "DexterityXP": 0.0,
            "ConstitutionXP": 0.0,
            "IntelligenceXP": 0.0,
            "WisdomXP": 0.0,
            "CharismaXP": 0.0,
            "LuckXP": 0.0,
            "Traits": "Material.Ore.Iron"
        }

        frags = import_item_definitions.determine_required_fragments(record)
        frag_types = [f["Type"] for f in frags]

        self.assertIn("USovereignItemFragment_UIData", frag_types)
        self.assertIn("USovereignItemFragment_Weight", frag_types)
        self.assertIn("USovereignItemFragment_Spiritual", frag_types)
        self.assertNotIn("USovereignItemFragment_Consumable", frag_types)

    def test_determine_required_fragments_for_food(self):
        # Mushroom with metabolic restore should have UIData, Weight, Consumable, and Spiritual
        record = {
            "RawName": "Species_Mushroom",
            "AssetName": "DA_Mushroom",
            "ItemTag": "Species.Plant.Mushroom",
            "DisplayName": "Woodland Mushroom",
            "Weight": 0.3,
            "MaxHunger": 40.0,
            "MaxHydration": 120.0,
            "QiGive": 5.0,
            "QiTake": 5.0,
            "QiAuraRadius": 500.0,
            "StrengthXP": 0.0,
            "DexterityXP": 0.0,
            "ConstitutionXP": 0.0,
            "IntelligenceXP": 0.0,
            "WisdomXP": 0.0,
            "CharismaXP": 0.0,
            "LuckXP": 0.0,
            "Traits": "Trait.Fungal"
        }

        frags = import_item_definitions.determine_required_fragments(record)
        frag_types = [f["Type"] for f in frags]

        self.assertIn("USovereignItemFragment_UIData", frag_types)
        self.assertIn("USovereignItemFragment_Weight", frag_types)
        self.assertIn("USovereignItemFragment_Consumable", frag_types)
        self.assertIn("USovereignItemFragment_Spiritual", frag_types)

    def test_dry_run_execution(self):
        summary = import_item_definitions.execute_item_importer_ui(self.csv_path)
        self.assertIn("[Standalone Dry-Run]", summary)
        self.assertIn("Parsed", summary)


if __name__ == "__main__":
    unittest.main()
