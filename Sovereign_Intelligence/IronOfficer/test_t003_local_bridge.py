# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for Ticket T-003: Sovereign LocalBridge Protocol Core Module & JSON-RPC Router.
"""

import os
import sys
import unittest

# Ensure bridge directory and LocalBridge package are in path
sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "LocalBridge"))

from LocalBridgeInterceptor import (
    LocalBridgeInterceptor,
    LocalAuthManager,
    LocalIDGenerator,
    LocalQueueManager
)


class TestT003LocalBridge(unittest.TestCase):

    def setUp(self):
        self.interceptor = LocalBridgeInterceptor()

    def test_builtin_auth_service(self):
        """Verify built-in Auth service handler response."""
        payload = {
            "source_module": "TestModule",
            "target_service": "Auth",
            "action": "validate_token"
        }
        res = self.interceptor.route_command(payload)
        self.assertEqual(res.get("status"), "SUCCESS")
        self.assertTrue(res.get("result", {}).get("authenticated"))

    def test_custom_handler_registration_and_dispatch(self):
        """Verify registering and dispatching to custom local service handlers."""
        def custom_sim_handler(action, params):
            if action == "get_health":
                return {"health": 100, "status": "alive"}
            return {"status": "unknown_action"}

        self.interceptor.register_handler("SimState", custom_sim_handler)

        payload = {
            "source_module": "SpeciesEngine",
            "target_service": "SimState",
            "action": "get_health",
            "parameters": {}
        }
        res = self.interceptor.route_command(payload)
        self.assertEqual(res.get("status"), "SUCCESS")
        self.assertEqual(res.get("result", {}).get("health"), 100)

    def test_unknown_service_error(self):
        """Verify error response for unregistered services."""
        payload = {
            "source_module": "Unknown",
            "target_service": "NonExistentService",
            "action": "test"
        }
        res = self.interceptor.route_command(payload)
        self.assertEqual(res.get("status"), "FAILURE")
        self.assertEqual(res.get("error_code"), "UNKNOWN_SERVICE")

    def test_queue_manager(self):
        """Verify LocalQueueManager event publishing and polling."""
        qm = LocalQueueManager()
        qm.publish_event("UnrealMailbox", {"msg": "hello"})
        qm.publish_event("UnrealMailbox", {"msg": "world"})

        events = qm.poll_events("UnrealMailbox")
        self.assertEqual(len(events), 2)
        self.assertEqual(events[0]["msg"], "hello")

        # After polling, queue should be empty
        events_after = qm.poll_events("UnrealMailbox")
        self.assertEqual(len(events_after), 0)


if __name__ == "__main__":
    unittest.main()
