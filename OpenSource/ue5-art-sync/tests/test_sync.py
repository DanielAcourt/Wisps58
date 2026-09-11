#!/usr/bin/env python3
"""
Unit tests for sovereign-art-sync (Local Art Asset Synchronizer)
"""

import os
import sys
import json
import shutil
import tempfile
import unittest
from pathlib import Path

# Add parent directory to sys.path to import sync_art_assets
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import sync_art_assets


class TestSovereignArtSync(unittest.TestCase):

    def setUp(self):
        """Create a temporary workspace simulating an Unreal Engine project root."""
        self.test_dir = tempfile.mkdtemp()
        self.old_cwd = os.getcwd()
        os.chdir(self.test_dir)

        self.project_root = Path(self.test_dir)
        self.uproject_file = self.project_root / "MockGame.uproject"
        self.uproject_file.write_text('{"EngineAssociation": "5.8"}', encoding="utf-8")

        self.vault_dir = self.project_root / "Content" / "ArtVault"
        self.vault_dir.mkdir(parents=True, exist_ok=True)

        self.target_dir = self.project_root / "Content" / "Assets" / "External" / "ArtVault"
        self.target_dir.mkdir(parents=True, exist_ok=True)

        self.manifest_path = self.project_root / "asset_manifest.json"
        self.initial_manifest = {
            "manifest_version": "1.0.0",
            "project_name": "auto",
            "local_vault_root": "Content/ArtVault",
            "asset_packages": [
                {
                    "package_id": "art_vault_meshes",
                    "name": "Art Vault Meshes",
                    "version": "1.0.0",
                    "source_path": "Meshes",
                    "target_destination": "Content/Assets/External/ArtVault/Meshes",
                    "enabled": True,
                    "description": "Static & Skeletal Meshes"
                }
            ]
        }
        with open(self.manifest_path, "w", encoding="utf-8") as f:
            json.dump(self.initial_manifest, f, indent=2)

    def tearDown(self):
        """Clean up temporary directory and restore CWD."""
        os.chdir(self.old_cwd)
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_detect_uproject(self):
        """Verify dynamic .uproject detection."""
        proj_name, proj_path = sync_art_assets.detect_uproject(self.test_dir)
        self.assertEqual(proj_name, "MockGame")
        self.assertEqual(proj_path.resolve(), self.project_root.resolve())

    def test_load_manifest(self):
        """Verify manifest loading and error handling."""
        manifest = sync_art_assets.load_manifest(self.manifest_path)
        self.assertIsNotNone(manifest)
        self.assertEqual(manifest.get("project_name"), "auto")

        missing_manifest = sync_art_assets.load_manifest("non_existent_manifest.json")
        self.assertIsNone(missing_manifest)

    def test_is_file_ignored(self):
        """Verify strict safety rule exclusions (.umap, World Partition, backups, logs)."""
        restricted_files = [
            "Content/Maps/MainLevel.umap",
            "Content/Maps/MainLevel.umap.bak",
            "Content/Meshes/SM_Box.uasset.bak",
            "Content/__ExternalActors__/Level/0/0/Actor.uasset",
            "Content/__ExternalObjects__/Level/0/0/Object.uasset",
            "Content/Intermediate/BuildCache.bin",
            "Content/Saved/Logs/Unreal.log",
            "Content/Textures/T_Noise.tmp"
        ]
        for rel_path in restricted_files:
            file_path = self.project_root / rel_path
            self.assertTrue(sync_art_assets.is_file_ignored(file_path), f"Failed to ignore: {rel_path}")

        allowed_files = [
            "Content/ArtVault/Meshes/SM_Chair.uasset",
            "Content/ArtVault/Textures/T_Rock_D.uasset",
            "Content/ArtVault/Materials/M_Basic.uasset"
        ]
        for rel_path in allowed_files:
            file_path = self.project_root / rel_path
            self.assertFalse(sync_art_assets.is_file_ignored(file_path), f"Incorrectly ignored: {rel_path}")

    def test_auto_discover_packages(self):
        """Verify auto-discovery registers newly dropped vault subdirectories into asset_manifest.json."""
        new_folder = self.vault_dir / "Textures"
        new_folder.mkdir(parents=True, exist_ok=True)

        # Also create a restricted folder that should be skipped
        restricted_folder = self.vault_dir / "__ExternalActors__"
        restricted_folder.mkdir(parents=True, exist_ok=True)

        manifest_data = sync_art_assets.load_manifest(self.manifest_path)
        discovered = sync_art_assets.auto_discover_packages(
            self.manifest_path, manifest_data, "Content/ArtVault", self.project_root
        )

        self.assertTrue(discovered)

        # Reload manifest and check registered packages
        updated_manifest = sync_art_assets.load_manifest(self.manifest_path)
        pkg_ids = [p["package_id"] for p in updated_manifest["asset_packages"]]
        self.assertIn("art_vault_textures", pkg_ids)
        self.assertNotIn("art_vault___externalactors__", pkg_ids)

    def test_bidirectional_sync_copy_mode(self):
        """Verify forward (vault -> engine) and reverse (engine -> vault) synchronization in copy mode."""
        mesh_vault_dir = self.vault_dir / "Meshes"
        mesh_engine_dir = self.target_dir / "Meshes"
        mesh_vault_dir.mkdir(parents=True, exist_ok=True)

        # Create asset in vault
        vault_asset = mesh_vault_dir / "SM_Table.uasset"
        vault_asset.write_text("BINARY_MESH_DATA_VAULT", encoding="utf-8")

        # Execute sync
        result = sync_art_assets.run_sync(manifest_path=str(self.manifest_path), copy_mode=True)
        self.assertEqual(result["status"], "success")

        # Verify asset copied to engine target
        engine_asset = mesh_engine_dir / "SM_Table.uasset"
        self.assertTrue(engine_asset.exists())
        self.assertEqual(engine_asset.read_text(encoding="utf-8"), "BINARY_MESH_DATA_VAULT")

        # Reverse Sync Test: Simulate editor asset save in engine target
        new_engine_asset = mesh_engine_dir / "SM_Table_New.uasset"
        new_engine_asset.write_text("BINARY_MESH_DATA_ENGINE", encoding="utf-8")

        sync_art_assets.run_sync(manifest_path=str(self.manifest_path), copy_mode=True)

        # Verify asset backed up to vault
        new_vault_asset = mesh_vault_dir / "SM_Table_New.uasset"
        self.assertTrue(new_vault_asset.exists())
        self.assertEqual(new_vault_asset.read_text(encoding="utf-8"), "BINARY_MESH_DATA_ENGINE")

    def test_bidirectional_sync_symlink_mode(self):
        """Verify symlink creation in symlink mode."""
        # Check if environment permits symlink creation
        dummy_src = Path(self.test_dir) / "dummy_src.txt"
        dummy_link = Path(self.test_dir) / "dummy_link.txt"
        dummy_src.write_text("test", encoding="utf-8")
        try:
            os.symlink(dummy_src, dummy_link)
            if dummy_link.exists():
                dummy_link.unlink()
        except OSError:
            self.skipTest("Symlinks require Administrator privileges or enabling Windows Developer Mode.")

        mesh_vault_dir = self.vault_dir / "Meshes"
        mesh_engine_dir = self.target_dir / "Meshes"
        mesh_vault_dir.mkdir(parents=True, exist_ok=True)

        vault_asset = mesh_vault_dir / "SM_Statue.uasset"
        vault_asset.write_text("BINARY_STATUE_DATA", encoding="utf-8")

        result = sync_art_assets.run_sync(manifest_path=str(self.manifest_path), copy_mode=False)
        self.assertEqual(result["status"], "success")

        engine_asset = mesh_engine_dir / "SM_Statue.uasset"
        self.assertTrue(engine_asset.exists())
        self.assertTrue(engine_asset.is_symlink())


if __name__ == "__main__":
    unittest.main()
