import os
import logging
from typing import Dict, Any, Optional, List
from pydantic import BaseModel, Field

# Setup secure AAS logging channel
logger = logging.getLogger("AAS.SovereignBridge")

class PSTAMetadata(BaseModel):
    """Machine-readable PSTA metadata embedded in payloads or file headers."""
    precedence: int = Field(..., alias="P", ge=1, le=10)
    stability: float = Field(..., alias="S", ge=0.0, le=1.0)
    testing_stage: str = Field(..., alias="T")  # DRAFT, INNOVATION, STAGING, PROD
    authorization: str = Field(..., alias="A")   # UNVERIFIED, SIGNED, HARDENED

class AgentCommandPayload(BaseModel):
    """The formal input structure required from the Iron Knight local agent."""
    persona: str
    command: str
    target_node: str
    parameters: Dict[str, Any] = {}
    meta_tags: Dict[str, Any] = Field(default_factory=dict)  # Handles unknown tags fluidly


class SovereignBridge:
    def __init__(self, is_hard_freeze: bool = False):
        self.is_hard_freeze = is_hard_freeze
        # Core protection registry mapping system nodes to required base Precedence
        self.protected_nodes: Dict[str, int] = {
            "CONTRIBUTING.md": 10,
            "bridge.py": 10,
            "USovereignSaveableEntityComponent": 10,
            "INDEX.md": 9,
            "DevOps/": 7
        }
        # Historical credibility scores of personas (AAS: R-009 tracking)
        self.persona_credibility: Dict[str, float] = {
            "Lead": 1.0,
            "Tactician": 0.9,
            "Strategist": 0.85,
            "Iron_Knight": 0.50,  # Starts unverified in INNOVATION stage
            "Researcher": 0.75
        }

    def calculate_psta_viability(self, payload: AgentCommandPayload) -> float:
        """
        [R-009] Mathematically codifies agent precedence weights.
        Formula: Weight = f(Risk, Credibility, Deviation)
        """
        credibility = self.persona_credibility.get(payload.persona, 0.4)

        # Determine risk profile based on targeted architecture node
        risk = 0.2
        for node, required_p in self.protected_nodes.items():
            if node in payload.target_node or node in payload.command:
                risk = max(risk, (required_p / 10.0))

        # Check for unknown tags or structural deviation
        # If the Iron Knight sends unrecognized tags, it represents structural deviation
        known_tags = {"version", "stage", "timestamp", "aas_score"}
        unknown_tags_count = len([t for t in payload.meta_tags if t not in known_tags])
        deviation = min(1.0, unknown_tags_count * 0.15)

        # AAS Formula Execution
        alpha, beta, gamma = 0.5, 0.3, 0.2
        weight = (alpha * credibility) - (beta * risk) - (gamma * deviation)

        # Normalize between 0.0 and 1.0
        return max(0.0, min(1.0, weight))

    def evaluate_intent_safety(self, payload: AgentCommandPayload) -> bool:
        """Enforces AAS Section 3 Precedence Rules against destructive actions."""
        destructive_keywords = ["delete", "remove", "rm", "unlink", "truncate", "drop"]
        normalized_cmd = payload.command.lower()

        is_destructive = any(kw in normalized_cmd for kw in destructive_keywords) or "os.remove" in normalized_cmd

        if is_destructive:
            # Hard Freeze or low precedence overrides intent entirely
            if self.is_hard_freeze:
                logger.critical(f"AAS ALERT: Blocked destructive op during HARD FREEZE by {payload.persona}")
                return False

            # Require absolute authority for protected file nodes
            if any(node in payload.target_node for node in self.protected_nodes):
                logger.critical(f"AAS ALERT: Structural mutation blocked on protected node: {payload.target_node}")
                return False

        return True

    async def arbitrate(self, payload: AgentCommandPayload) -> Dict[str, Any]:
        """
        Main gatekeeper route called by FastAPI lifecycle endpoints.
        Acts as the AAS Operational Trigger.
        """
        # 1. Evaluate Math Viability (R-009)
        confidence_score = self.calculate_psta_viability(payload)

        # 2. Evaluate Physical / Structural Safety (B-021)
        is_safe_intent = self.evaluate_intent_safety(payload)

        # 3. Handle [B-022] Deep Planning Handshake Trigger (Confidence < 0.7)
        if confidence_score < 0.7 or not is_safe_intent:
            logger.warning(f"409 CONFLICT: Confidence {confidence_score:.2f} below threshold. Halting.")
            return {
                "status": "409_CONFLICT_GATE",
                "confidence_score": confidence_score,
                "action": "MANDATORY_USER_HANDSHAKE_REQUIRED",
                "reason": f"Persona '{payload.persona}' failed authority validation for target '{payload.target_node}'."
            }

        # 4. Route Execution to Core System
        return {
            "status": "200_OK",
            "confidence_score": confidence_score,
            "action": "PROCEED_TO_EXECUTION",
            "payload": payload.dict()
        }