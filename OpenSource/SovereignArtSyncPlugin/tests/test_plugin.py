#!/usr/bin/env python3
"""
Unit tests for SovereignArtSync Unreal Engine 5 Plugin
"""

import os
import sys
import json
import zipfile
import unittest
from pathlib import Path

root_plugin_dir = Path(__file__).resolve().parent.parent
plugin_python_dir = root_plugin_dir / "Plugins" / "SovereignArtSync" / "Content" / "Python"

if str(root_plugin_dir) not in sys.path:
    sys.path.insert(0, str(root_plugin_dir))

if str(plugin_python_dir) not in sys.path:
    sys.path.insert(0, str(plugin_python_dir))

import sovereign_art_sync_engine
import sovereign_art_sync_plugin


class TestSovereignArtSyncPlugin(unittest.TestCase):

    def setUp(self):
        self.root_dir = Path(__file__).resolve().parent.parent
        self.plugin_dir = self.root_dir / "Plugins" / "SovereignArtSync"

    def test_uplugin_descriptor(self):
        """Verify .uplugin file validity and schema."""
        uplugin_file = self.plugin_dir / "SovereignArtSync.uplugin"
        self.assertTrue(uplugin_file.exists())

        with open(uplugin_file, "r", encoding="utf-8") as f:
            data = json.load(f)

        self.assertEqual(data.get("FileVersion"), 3)
        self.assertEqual(data.get("FriendlyName"), "Sovereign Art Sync")
        self.assertEqual(data.get("Category"), "Editor")
        self.assertTrue(data.get("CanContainContent"))

        plugins = [p.get("Name") for p in data.get("Plugins", [])]
        self.assertIn("PythonScriptPlugin", plugins)
        self.assertIn("EditorScriptingUtilities", plugins)

    def test_plugin_icon_exists(self):
        """Verify plugin Icon128.png exists."""
        icon_file = self.plugin_dir / "Resources" / "Icon128.png"
        self.assertTrue(icon_file.exists())

    def test_plugin_content_manifest(self):
        """Verify template asset_manifest.json inside plugin Content."""
        manifest_file = self.plugin_dir / "Content" / "asset_manifest.json"
        self.assertTrue(manifest_file.exists())

        with open(manifest_file, "r", encoding="utf-8") as f:
            data = json.load(f)

        self.assertIn("asset_packages", data)
        self.assertGreaterEqual(len(data["asset_packages"]), 1)

    def test_plugin_python_execution(self):
        """Verify in-editor execution function returns formatted logs without crashing."""
        manifest_path = self.plugin_dir / "Content" / "asset_manifest.json"
        log_output = sovereign_art_sync_plugin.execute_dry_run_autodiscover(manifest_path=str(manifest_path))
        self.assertIn("=== Sovereign Art Sync (Dry-Run & Auto-Discovery) ===", log_output)

    def test_fab_zip_bundler(self):
        """Verify Fab marketplace zip package builder."""
        import package_fab_plugin
        success = package_fab_plugin.create_fab_package()
        self.assertTrue(success)

        zip_file = self.root_dir / "SovereignArtSync_v1.0.0_Fab.zip"
        self.assertTrue(zip_file.exists())

        with zipfile.ZipFile(zip_file, "r") as z:
            names = z.namelist()
            self.assertIn("Plugins/SovereignArtSync/SovereignArtSync.uplugin", names)
            self.assertIn("README.md", names)
            self.assertIn("Fab_Marketplace_Metadata.json", names)


if __name__ == "__main__":
    unittest.main()
