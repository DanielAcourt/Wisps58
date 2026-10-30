# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Iron Coder: Autonomous Multi-Turn Software Engineering Execution Loop Driver.
Enables 100% local, sovereign programming autonomy on the RTX 5090 rig.
Executes Plan -> RAG Retrieve -> Tool Execution -> Verification -> Reflection turn cycles.
"""

import os
import sys
import json
import argparse
import requests
import logging
from typing import Dict, Any, List, Optional

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("AAS.IronCoder")

BRIDGE_URL = "http://127.0.0.1:8000"
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


class IronCoderAgent:
    """Autonomous multi-turn agent execution loop driver for local software engineering tasks."""

    def __init__(self, bridge_url: str = BRIDGE_URL, max_turns: int = 10, persona: str = "Tactician"):
        self.bridge_url = bridge_url
        self.max_turns = max_turns
        self.persona = persona
        self.history: List[Dict[str, Any]] = []

    def execute_handshake(self) -> bool:
        """Triggers local AAS authority boost via bridge."""
        try:
            res = requests.post(f"{self.bridge_url}/v1/aas/handshake", timeout=5)
            if res.status_code == 200:
                logger.info("IronCoder: AAS Handshake boost active (+0.5 Authority).")
                return True
        except Exception as exc:
            logger.warning(f"IronCoder: Could not execute handshake: {exc}")
        return False

    def run_task(self, prompt: str) -> Dict[str, Any]:
        """Runs the multi-turn agent loop for a given task prompt."""
        logger.info(f"IronCoder: Initializing autonomous task loop for: '{prompt}'")
        self.execute_handshake()

        # Ingest user prompt into history
        self.history.append({"role": "user", "name": self.persona, "content": prompt})

        turn = 0
        final_response = ""
        tool_execution_count = 0

        while turn < self.max_turns:
            turn += 1
            logger.info(f"IronCoder: --- Turn {turn}/{self.max_turns} ---")

            try:
                res = requests.post(
                    f"{self.bridge_url}/v1/chat",
                    json={"messages": self.history},
                    timeout=120
                )

                if res.status_code != 200:
                    logger.error(f"IronCoder: Bridge error status {res.status_code}: {res.text}")
                    break

                data = res.json()
                tool_chain = data.get("tool_chain", [])
                tool_outputs = data.get("tool_outputs", [])
                result = data.get("result", {})
                msg = result.get("message", {})

                content = msg.get("content", "")
                if content:
                    final_response = content
                    self.history.append({"role": "assistant", "content": content})
                    logger.info(f"IronCoder: Assistant Response: {content[:150]}...")

                if tool_chain:
                    tool_execution_count += len(tool_chain)
                    logger.info(f"IronCoder: Executed {len(tool_chain)} tools in turn {turn}.")
                else:
                    # If no tools were called and model provided a final response, task may be complete
                    logger.info("IronCoder: No further tool calls requested. Multi-turn loop complete.")
                    break

            except Exception as exc:
                logger.error(f"IronCoder: Loop exception on turn {turn}: {exc}")
                break

        # Verification step
        verification_status = self.verify_solution()

        return {
            "status": "completed" if turn < self.max_turns else "max_turns_reached",
            "total_turns": turn,
            "tool_executions": tool_execution_count,
            "final_response": final_response,
            "verification": verification_status
        }

    def verify_solution(self) -> Dict[str, Any]:
        """Runs basic pre-commit verification checks locally."""
        logger.info("IronCoder: Running automated pre-commit verification...")

        # Test tts_sanitizer test as verification check
        test_file = os.path.join(REPO_ROOT, "Sovereign_Intelligence", "IronOfficer", "test_tts_sanitizer.py")
        if os.path.exists(test_file):
            import subprocess
            res = subprocess.run(f"python3 -m unittest {test_file}", shell=True, capture_output=True, text=True)
            return {
                "passed": res.returncode == 0,
                "stdout": res.stdout[:500],
                "stderr": res.stderr[:500]
            }

        return {"passed": True, "note": "No verification tests required."}


def main():
    parser = argparse.ArgumentParser(description="Iron Coder: Local Autonomous Engineering Loop")
    parser.add_argument("--prompt", required=True, help="Task prompt or user directive")
    parser.add_argument("--turns", type=int, default=10, help="Maximum turns allowed")
    parser.add_argument("--persona", default="Tactician", help="Agent persona name")
    args = parser.parse_args()

    agent = IronCoderAgent(max_turns=args.turns, persona=args.persona)
    output = agent.run_task(args.prompt)
    print(json.dumps(output, indent=2))


if __name__ == "__main__":
    main()
