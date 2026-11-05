# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21
"""
Regression & Automation Suite for Combat Skills & Classes CSV Importer (CS-001 & CS-002)
---------------------------------------------------------------------------------------
Validates:
1. Skills CSV schema, row count, power formulas, and target pattern valid strings.
2. Classes CSV schema, row count, prerequisite syntax (e.g. 'Class.Combat.Warrior:5'),
   minimum attribute requirements, and granted skill references.
3. Importer dry-run execution without syntax errors.
"""

import os
import sys
import unittest

# Add project root and python content directory to sys.path
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
PYTHON_CONTENT_DIR = os.path.join(PROJECT_ROOT, "WispsCPPVR", "Content", "Python")

if PYTHON_CONTENT_DIR not in sys.path:
    sys.path.insert(0, PYTHON_CONTENT_DIR)

import import_combat_data


class TestCombatDataImporter(unittest.TestCase):

    def setUp(self):
        self.skills_csv = os.path.join(PROJECT_ROOT, "AI_Nexus", "Data", "DT_SovereignSkillsTemplate.csv")
        self.classes_csv = os.path.join(PROJECT_ROOT, "AI_Nexus", "Data", "DT_SovereignClassesTemplate.csv")

    def test_skills_csv_existence_and_parsing(self):
        self.assertTrue(os.path.exists(self.skills_csv), f"Skills CSV missing: {self.skills_csv}")
        records = import_combat_data.parse_csv_skill_rows(self.skills_csv)
        self.assertGreaterEqual(len(records), 8, "Expected at least 8 starter skills")

        valid_patterns = {"singletarget", "frontrowsweep", "columnpierce", "fullmatrixaoe", "selfonly", "allallies"}
        valid_scaling = {"strength", "dexterity", "intelligence", "wisdom", "constitution"}

        for r in records:
            self.assertTrue(r["SkillTag"].startswith("Skill."), f"SkillTag should start with Skill.: {r['SkillTag']}")
            self.assertTrue(r["AssetName"].startswith("DA_"), f"AssetName should start with DA_: {r['AssetName']}")
            self.assertIn(r["TargetPattern"].lower(), valid_patterns, f"Invalid TargetPattern: {r['TargetPattern']}")
            self.assertIn(r["ScalingAttribute"].lower(), valid_scaling, f"Invalid ScalingAttribute: {r['ScalingAttribute']}")
            self.assertGreater(r["BasePower"], 0.0, "BasePower should be greater than 0")
            self.assertGreaterEqual(r["StatMultiplier"], 0.0, "StatMultiplier must be non-negative")

    def test_classes_csv_existence_and_parsing(self):
        self.assertTrue(os.path.exists(self.classes_csv), f"Classes CSV missing: {self.classes_csv}")
        records = import_combat_data.parse_csv_class_rows(self.classes_csv)
        self.assertGreaterEqual(len(records), 10, "Expected at least 10 starter classes")

        class_map = {r["ClassTag"]: r for r in records}

        # Check Adventurer (starter)
        self.assertIn("Class.Combat.Adventurer", class_map)
        adv = class_map["Class.Combat.Adventurer"]
        self.assertEqual(adv["CPCost"], 1)
        self.assertEqual(len(adv["Prerequisites"]), 0)

        # Check Ninja prerequisites: Rogue:5, DEX 15, STR 10
        self.assertIn("Class.Combat.Ninja", class_map)
        ninja = class_map["Class.Combat.Ninja"]
        self.assertEqual(ninja["CPCost"], 3)
        self.assertGreaterEqual(ninja["DexterityMultiplier"], 1.4)
        self.assertEqual(ninja["ReqDEX"], 15)
        self.assertEqual(ninja["ReqSTR"], 10)
        self.assertEqual(len(ninja["Prerequisites"]), 1)
        self.assertEqual(ninja["Prerequisites"][0]["ClassTag"], "Class.Combat.Rogue")
        self.assertEqual(ninja["Prerequisites"][0]["Level"], 5)

        # Check Dragoon multi-prerequisites: Warrior:5 and Rogue:5
        self.assertIn("Class.Combat.Dragoon", class_map)
        dragoon = class_map["Class.Combat.Dragoon"]
        self.assertEqual(len(dragoon["Prerequisites"]), 2)
        prereq_tags = [p["ClassTag"] for p in dragoon["Prerequisites"]]
        self.assertIn("Class.Combat.Warrior", prereq_tags)
        self.assertIn("Class.Combat.Rogue", prereq_tags)

    def test_dry_run_execution(self):
        s_result = import_combat_data.import_skills(self.skills_csv)
        c_result = import_combat_data.import_classes(self.classes_csv)
        self.assertIn("Successfully validated", s_result)
        self.assertIn("Successfully validated", c_result)


if __name__ == "__main__":
    unittest.main()
