# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Sovereign LocalStateTracker: Atomic, File-System-Backed Memory & State Persistence Manager.
Guarantees memory retention and transactional atomic state persistence across local process restarts.
"""

import os
import json
import logging
import datetime
import shutil
from typing import Dict, Any, Optional, List

logger = logging.getLogger("AAS.LocalStateTracker")


class LocalStateTracker:
    """Atomic, file-system-backed state persistence manager using Local_State_Cache/."""

    def __init__(self, repo_root: str, cache_dir_name: str = "Local_State_Cache"):
        self.repo_root = os.path.abspath(repo_root)
        self.cache_dir = os.path.join(self.repo_root, cache_dir_name)
        self.snapshots_dir = os.path.join(self.cache_dir, "snapshots")

        os.makedirs(self.cache_dir, exist_ok=True)
        os.makedirs(self.snapshots_dir, exist_ok=True)

    def _get_key_path(self, key: str) -> str:
        safe_key = "".join([c if c.isalnum() or c in ("-", "_", ".") else "_" for c in key])
        if not safe_key.endswith(".json"):
            safe_key += ".json"
        return os.path.join(self.cache_dir, safe_key)

    def save_state(self, key: str, data: Dict[str, Any]) -> bool:
        """Atomically saves state dictionary to disk using a temporary file swap."""
        filepath = self._get_key_path(key)
        temp_filepath = filepath + ".tmp"

        payload = {
            "key": key,
            "timestamp": datetime.datetime.now().isoformat(),
            "data": data
        }

        try:
            with open(temp_filepath, "w", encoding="utf-8") as f:
                json.dump(payload, f, indent=2)

            # Atomic file replace
            os.replace(temp_filepath, filepath)
            logger.info(f"LocalStateTracker: Atomically saved state for key '{key}'.")
            return True
        except Exception as exc:
            logger.error(f"LocalStateTracker: Failed to save state for '{key}': {exc}")
            if os.path.exists(temp_filepath):
                try:
                    os.remove(temp_filepath)
                except Exception:
                    pass
            return False

    def load_state(self, key: str, default: Optional[Dict[str, Any]] = None) -> Optional[Dict[str, Any]]:
        """Reads and returns state dictionary for key, or default if missing/corrupt."""
        filepath = self._get_key_path(key)
        if not os.path.exists(filepath):
            return default

        try:
            with open(filepath, "r", encoding="utf-8") as f:
                payload = json.load(f)
                return payload.get("data", default)
        except Exception as exc:
            logger.error(f"LocalStateTracker: Failed to read state for '{key}': {exc}")
            return default

    def create_snapshot(self, snapshot_name: str) -> Optional[str]:
        """Creates an immutable, timestamped snapshot of all current cached state keys."""
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        safe_name = "".join([c if c.isalnum() or c in ("-", "_") else "_" for c in snapshot_name])
        snapshot_filename = f"Snapshot_{safe_name}_{timestamp}.json"
        snapshot_path = os.path.join(self.snapshots_dir, snapshot_filename)

        all_keys = self.list_keys()
        snapshot_data = {}

        for k in all_keys:
            val = self.load_state(k)
            if val is not None:
                snapshot_data[k] = val

        payload = {
            "snapshot_name": snapshot_name,
            "timestamp": datetime.datetime.now().isoformat(),
            "keys_count": len(snapshot_data),
            "states": snapshot_data
        }

        try:
            temp_path = snapshot_path + ".tmp"
            with open(temp_path, "w", encoding="utf-8") as f:
                json.dump(payload, f, indent=2)
            os.replace(temp_path, snapshot_path)
            logger.info(f"LocalStateTracker: Created state snapshot '{snapshot_filename}'.")
            return snapshot_filename
        except Exception as exc:
            logger.error(f"LocalStateTracker: Failed to create snapshot '{snapshot_name}': {exc}")
            return None

    def list_keys(self) -> List[str]:
        """Lists all active state keys stored in the cache directory."""
        keys = []
        if not os.path.exists(self.cache_dir):
            return keys

        for f in os.listdir(self.cache_dir):
            if f.endswith(".json") and not f.startswith("Snapshot_"):
                key_name = f[:-5]
                keys.append(key_name)
        return keys

    def delete_state(self, key: str) -> bool:
        """Removes state file for key."""
        filepath = self._get_key_path(key)
        if os.path.exists(filepath):
            try:
                os.remove(filepath)
                return True
            except Exception as exc:
                logger.error(f"LocalStateTracker: Failed to delete state for '{key}': {exc}")
                return False
        return False
