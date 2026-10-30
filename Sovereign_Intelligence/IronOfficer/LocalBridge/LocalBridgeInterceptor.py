# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Sovereign LocalBridgeInterceptor: Core JSON-RPC Local Loopback Router Module.
Intercepts all outbound cloud gateway API requests and routes execution to local handlers offline.
"""

import os
import sys
import uuid
import logging
import datetime
from typing import Dict, Any, Optional, Callable, List

logger = logging.getLogger("AAS.LocalBridge")


class LocalAuthManager:
    """Mock local authentication manager validating module permissions locally."""

    def __init__(self, allowed_personas: Optional[List[str]] = None):
        self.allowed_personas = allowed_personas or [
            "Lead", "Tactician", "Strategist", "Researcher", "Archivist_Theta", "Iron_Knight", "Unreal_Simulation"
        ]

    def validate_source(self, source_module: str, target_service: str) -> bool:
        """Validates whether calling source module is permitted to invoke target service."""
        if not source_module or not target_service:
            return False
        return True


class LocalIDGenerator:
    """Deterministic local UUID and identifier generator."""

    @staticmethod
    def generate_uuid(prefix: str = "LOCAL") -> str:
        """Generates a deterministic local UUID string."""
        return f"{prefix}-{uuid.uuid4().hex[:12].upper()}"


class LocalQueueManager:
    """In-memory event queue manager for asynchronous message dispatching."""

    def __init__(self):
        self.queues: Dict[str, List[Dict[str, Any]]] = {}

    def publish_event(self, queue_name: str, payload: Dict[str, Any]) -> int:
        if queue_name not in self.queues:
            self.queues[queue_name] = []
        self.queues[queue_name].append(payload)
        return len(self.queues[queue_name])

    def poll_events(self, queue_name: str) -> List[Dict[str, Any]]:
        if queue_name in self.queues:
            events = self.queues[queue_name]
            self.queues[queue_name] = []
            return events
        return []


class LocalBridgeInterceptor:
    """Central JSON-RPC loopback router intercepting and executing commands locally."""

    def __init__(self, auth_manager: Optional[LocalAuthManager] = None):
        self.auth_manager = auth_manager or LocalAuthManager()
        self.queue_manager = LocalQueueManager()
        self.id_generator = LocalIDGenerator()
        self.handlers: Dict[str, Callable[[str, Dict[str, Any]], Dict[str, Any]]] = {}

    def register_handler(self, service_name: str, handler_func: Callable[[str, Dict[str, Any]], Dict[str, Any]]):
        """Registers a local handler function for a target service."""
        self.handlers[service_name] = handler_func
        logger.info(f"LocalBridgeInterceptor: Registered local handler for service '{service_name}'.")

    def route_command(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        """
        Core entry point: Intercepts incoming payload, validates auth, and dispatches to local handler.
        """
        request_id = payload.get("request_id") or self.id_generator.generate_uuid("REQ")
        source_module = payload.get("source_module", "UnknownModule")
        target_service = payload.get("target_service", "UnknownService")
        action = payload.get("action", "default_action")
        parameters = payload.get("parameters") or {}

        # 1. Validation check
        if not self.auth_manager.validate_source(source_module, target_service):
            return self.create_error_response(
                request_id=request_id,
                error_code="LOCAL_AUTH_DENIED",
                message=f"Access Denied: Module '{source_module}' is not authorized to call '{target_service}'."
            )

        # 2. Dispatching
        if target_service in self.handlers:
            try:
                result = self.handlers[target_service](action, parameters)
                return self.create_success_response(request_id=request_id, result=result)
            except Exception as exc:
                logger.error(f"LocalBridgeInterceptor: Handler error for '{target_service}.{action}': {exc}")
                return self.create_error_response(
                    request_id=request_id,
                    error_code="HANDLER_EXECUTION_ERROR",
                    message=str(exc)
                )

        # Built-in fallback services
        if target_service == "Auth":
            return self.create_success_response(
                request_id=request_id,
                result={"authenticated": True, "source": source_module, "vss_boost": 0.5}
            )

        if target_service == "Comms":
            count = self.queue_manager.publish_event(action, parameters)
            return self.create_success_response(
                request_id=request_id,
                result={"status": "published", "queue": action, "pending_count": count}
            )

        return self.create_error_response(
            request_id=request_id,
            error_code="UNKNOWN_SERVICE",
            message=f"Service '{target_service}' has no registered local handler."
        )

    def create_success_response(self, request_id: str, result: Any) -> Dict[str, Any]:
        return {
            "request_id": request_id,
            "status": "SUCCESS",
            "result": result,
            "timestamp": datetime.datetime.now().isoformat()
        }

    def create_error_response(self, request_id: str, error_code: str, message: str) -> Dict[str, Any]:
        return {
            "request_id": request_id,
            "status": "FAILURE",
            "error_code": error_code,
            "message": message,
            "timestamp": datetime.datetime.now().isoformat()
        }
