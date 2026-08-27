# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2025-06-18
"""
Sovereign Framework: Iron Officer Verification Script
------------------------------------------------------
Verifies the Social (S) and Technical (T) pillars by testing the communication link
between the Vessel, Bridge, and Ollama.
"""

import requests
import json
import os

CONFIG_PATH = os.path.join(os.path.dirname(__file__), "config.json")
BRIDGE_PORT = 8050
if os.path.exists(CONFIG_PATH):
    try:
        with open(CONFIG_PATH, "r") as f:
            cfg = json.load(f)
            BRIDGE_PORT = cfg.get("bridge", {}).get("port", 8050)
    except Exception:
        pass

BRIDGE_URL = f"http://127.0.0.1:{BRIDGE_PORT}"

def test_bridge_health():
    """Checks if the FastAPI bridge is active and reporting identity correctly."""
    print("--- 1. Testing Bridge Health (Social Pillar) ---")
    try:
        response = requests.get(BRIDGE_URL)
        print(f"Status: {response.status_code}")
        print(f"Identity: {response.json().get('identity', 'Unknown')}")
        print(f"Hardware: {response.json().get('hardware', 'Unknown')}")
    except Exception as e:
        print(f"[07 ERROR] Connection to Bridge failed: {e}")
        if "10061" in str(e):
            print(">>> SOLUTION: Run 'run_bridge.bat' first.")

def check_ollama_models():
    """Checks if the Bridge can query the local Ollama instance for model parity."""
    print("\n--- 2. Checking Ollama Models (Technical Pillar) ---")
    try:
        response = requests.get(f"{BRIDGE_URL}/v1/ollama/status")
        if response.status_code == 200:
            models = response.json().get('models', [])
            model_names = [m['name'] for m in models]
            print(f"Detected Models: {model_names}")
        else:
            print(f"Error: {response.json()}")
    except Exception as e:
        print(f"[07 ERROR] Bridge cannot talk to Ollama: {e}")
        print(">>> SOLUTION: Ensure Ollama is running (check system tray).")

def test_vss_evaluation():
    """Performs a live VSS evaluation using the target model to verify GPU inference."""
    print("\n--- 3. Testing VSS Evaluation (GTX 5090 Inference) ---")
    telemetry = [
        {"pillar": "T", "value": 0.9, "metadata": {"temp": "nominal"}},
        {"pillar": "P", "value": 1.0, "metadata": {"confidence": "optimal"}}
    ]
    payload = {
        "telemetry": telemetry,
        "context": "Architectural integrity verification."
    }

    try:
        # Note: This may take a few seconds as it wakes up the GPU
        response = requests.post(f"{BRIDGE_URL}/v1/safety/evaluate", json=payload, timeout=30)
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            print("Iron Officer Analysis:")
            print(json.dumps(response.json(), indent=2))
        else:
            print("Error Details:")
            print(json.dumps(response.json(), indent=2))
    except Exception as e:
        print(f"Inference Error: {e}")

if __name__ == "__main__":
    test_bridge_health()
    check_ollama_models()
    test_vss_evaluation()
