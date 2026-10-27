# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.7-Knight-AAS. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21
"""
Iron Officer: Chat Vessel (AD-002)
----------------------------------
A terminal-based Human-Machine Interface (HMI) for communicating with the Sovereign Bridge.
The Vessel serves as the primary 'Spirit Link' between the Lead Programmer and the
Architectural Knight, enabling PSTA-driven command execution, AAS arbitration, and direct
local photorealistic image generation (/image command).

// [Jules] Added /image & /imagegen direct synthesis commands to ChatVessel [2026-10-21]
"""

import os
import sys
import json
import requests
import datetime
import shlex
import time
import uuid
from typing import List, Dict

# --- Configuration ---
VESSEL_VERSION = "0.36.4.0-Knight" # Internal UI Version
BRIDGE_URL = "http://127.0.0.1:8000"
COMFY_URL = "http://127.0.0.1:8188"

# Handle PyInstaller paths for frozen executables
if getattr(sys, 'frozen', False):
    REPORT_DIR = os.path.dirname(sys.executable)
else:
    REPORT_DIR = os.path.dirname(__file__)

class ChatVessel:
    """
    Main HMI Controller. Manages session history, command parsing, and
    synchronization with the Iron Officer Bridge and ComfyUI Image Generation Engine.
    """
    def __init__(self):
        self.session_id = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        self.history: List[Dict[str, str]] = []
        self.user_name = "User"
        self.identity = "Iron Officer"
        self.show_tools = True
        self.bridge_version = "Unknown"
        self.fetch_bridge_info()

    def fetch_bridge_info(self):
        """
        Synchronizes with the local FastAPI bridge and validates version compatibility.
        Mandatory step to ensure AAS arbitration protocols are aligned.
        """
        try:
            response = requests.get(f"{BRIDGE_URL}/", timeout=3)
            if response.status_code == 200:
                data = response.json()
                self.user_name = data.get("user", "Dan")
                self.identity = data.get("identity", "Iron Officer")
                self.bridge_version = data.get("version", "Legacy")

                print(f"[07] Connected to {self.identity} (Bridge v{self.bridge_version})")

                if "36.4.7" not in self.bridge_version and "37." not in self.bridge_version:
                    print("\n" + "!"*60)
                    print(f"[07 WARNING] Version Mismatch Detected!")
                    print(f"Vessel: {VESSEL_VERSION} | Bridge: {self.bridge_version}")
                    print("[07] Please run 'build_vessel.bat' to synchronize.")
                    print("!"*60 + "\n")
            else:
                print(f"[07 WARNING] Bridge returned status {response.status_code}")
        except Exception as e:
            print(f"[07 ERROR] Could not connect to Iron Officer Bridge at {BRIDGE_URL}")
            print(f"[07 Details] {e}")
            print("[07] Ensure 'run_bridge.bat' is active and listening on port 8000.")
            input("\nPress Enter to exit...")
            sys.exit(1)

    def print_header(self):
        """Displays the mission-critical header and command manifest."""
        os.system('cls' if os.name == 'nt' else 'clear')
        print("="*60)
        print(f" SOVEREIGN IRON OFFICER | VESSEL v{VESSEL_VERSION}")
        print(f" User: {self.user_name} | Session: {self.session_id}")
        print("="*60)
        print(" Commands: /07, /p, /t, /s, /a, /verify, /vss, /phi, /velocity, /image")
        print(" Tools:    /read, /write, /patch, /append, /list, /backups, /handshake")
        print(" Meta:     /report, /status, /tools, /exit")
        print("-"*60)

    def save_mission_report(self):
        """Exports the current session transcript as a JSON Mission Report for archival."""
        filename = f"Mission_Report_{self.session_id}.json"
        filepath = os.path.join(REPORT_DIR, filename)
        report = {
            "vessel_version": VESSEL_VERSION,
            "bridge_version": self.bridge_version,
            "timestamp": datetime.datetime.now().isoformat(),
            "transcript": self.history,
        }
        try:
            with open(filepath, "w") as f:
                json.dump(report, f, indent=4)
            print(f"\n[07] Report saved: {filepath.replace('\\', '/')}")
        except Exception as e:
            print(f"\n[07 ERROR] Failed to save report: {e}")

    def show_status(self):
        """Displays low-level bridge health and Safe Zone configuration."""
        try:
            response = requests.get(f"{BRIDGE_URL}/", timeout=2)
            data = response.json()
            print("\n" + "-"*30)
            print(f"BRIDGE: {data.get('status')} v{data.get('version')}")
            print(f"HARDWARE: {data.get('hardware')}")
            print(f"SAFE ZONES (R): {len(data.get('read_zones', []))}")
            print(f"SAFE ZONES (W): {len(data.get('write_zones', []))}")
            print("-"*30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Status check failed: {e}")

    def run_salute(self):
        """
        Executes the 07 Protocol Salute.
        Collects Psychological (P), Social (S), Technical (T), and Administrative (A)
        telemetry to verify overall systemic integrity.
        """
        print(f"\n{self.user_name}> /07")
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/salute?persona={self.identity.replace(' ', '_')}", timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(f"\n{self.identity}> **07 Protocol Salute Initiated.**")
                print(f"P: **[Psychological Status]:** {data['P']['status']} operational capacity maintained. Assessment confirms high confidence in current objectives. Tonic State={data['P'].get('tonic_state', '1.0')}")
                print(f"S: **[Social Sync Status]:** {data['S']['status']} with Command vector (Lead {self.user_name}). Connection parameters stable and non-degraded.")

                t = data['T']
                if 'error' in t:
                    print(f"T: **[Technical Truth]:** ERROR - {t['error']}")
                else:
                    print(f"T: **[Technical Truth - GPU Metrics]:** {t.get('gpu_utilization', '??%')} Utilization, Temp={t.get('gpu_temperature', '??C')} ({t.get('status', 'Unknown')}). Phi={t.get('phi', '1.0')}")

                a = data['A']
                print(f"A: **[Administrative Truth - Nexus State]:** All AAS directives are {a['status']} and compliant with v{a['version']}. Nexus {'OK' if a['nexus_ok'] else 'FAILED'}. Diligence Score={a.get('diligence', '0.0')}")
                print("\n*Status Cycle Complete. Reporting nominal functionality.*\n")
            else:
                print(f"\n[07 ERROR] Salute failed: {response.text}\n")
        except Exception as e:
            print(f"\n[07 ERROR] Salute exception: {e}\n")

    def run_telemetry(self):
        """Fetches raw hardware telemetry (Technical Pillar)."""
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/telemetry", timeout=5)
            data = response.json()
            print(f"\n[TECHNICAL TRUTH - HARDWARE]")
            if "error" in data:
                print(f" -> ERROR: {data['error']}")
            else:
                for k, v in data.items():
                    print(f" -> {k.upper()}: {v}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Telemetry retrieval failed: {e}\n")

    def run_social(self):
        """Checks social sync and connection stability (Social Pillar)."""
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/social", timeout=2)
            data = response.json()
            print(f"\n[SOCIAL SYNC - CONNECTION]")
            for k, v in data.items():
                print(f" -> {k.upper()}: {v}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Social sync check failed: {e}\n")

    def run_admin(self):
        """Verifies Nexus state and AAS compliance (Administrative Pillar)."""
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/administrative", timeout=2)
            data = response.json()
            print(f"\n[ADMINISTRATIVE TRUTH - NEXUS]")
            print(f" -> STATUS: {data['status']}")
            print(f" -> NEXUS: {'OK' if data['nexus_ok'] else 'NOT FOUND'}")
            print(f" -> VERSION: {data['version']}")
            print(f" -> PROTECTED NODES: {len(data['protected_nodes'])}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Administrative check failed: {e}\n")

    def run_verify(self, args: List[str]):
        """Runs a pre-flight AAS authority audit for a specific file/node."""
        if not args:
            print("\n[07] Usage: /verify <node_path> [command]\n")
            return
        node = args[0]
        cmd = args[1] if len(args) > 1 else "read_file"
        try:
            payload = {"persona": self.identity.replace(" ", "_"), "target_node": node, "command": cmd}
            response = requests.post(f"{BRIDGE_URL}/v1/aas/verify", json=payload, timeout=5)
            data = response.json()
            print(f"\n[AAS AUTHORITY VERIFICATION]")
            print(f" -> NODE: {node}")
            print(f" -> ACTION: {cmd}")
            arb = data["arbitration"]
            print(f" -> STATUS: {arb['status']}")
            print(f" -> CONFIDENCE: {arb.get('confidence_score', 'N/A')}")
            if arb["status"] != "200_OK":
                print(f" -> REASON: {arb.get('reason', 'Access Denied')}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Authority verification failed: {e}\n")

    def run_vss(self, args: List[str]):
        """Displays the mathematical breakdown of the Viability Safety Score (VSS)."""
        node = args[0] if args else "AI_Nexus"
        cmd = args[1] if len(args) > 1 else "read_file"
        try:
            payload = {"persona": self.identity.replace(" ", "_"), "target_node": node, "command": cmd}
            response = requests.post(f"{BRIDGE_URL}/v1/aas/verify", json=payload, timeout=5)
            data = response.json()
            breakdown = data["vss_breakdown"]
            print(f"\n[VSS MATHEMATICAL BREAKDOWN]")
            print(f" V = ({breakdown['alpha']} * {breakdown['credibility']}) - ({breakdown['beta']} * {breakdown['risk']}) - ({breakdown['gamma']} * {breakdown['deviation']}) + {breakdown['memory_boost']}")
            print(f" RESULT: {breakdown['vss']:.4f}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] VSS breakdown failed: {e}\n")

    def run_phi(self):
        """Calculates the system's Coherence Coefficient (Phi)."""
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/phi", timeout=5)
            data = response.json()
            print(f"\n[COHERENCE COEFFICIENT - PHI]")
            print(f" -> PHI: {data['phi']}")
            print(f" -> STATUS: {data['status']}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Phi calculation failed: {e}\n")

    def run_velocity(self):
        """Monitors current Risk Velocity (V_i)."""
        try:
            response = requests.get(f"{BRIDGE_URL}/v1/psta/velocity", timeout=5)
            data = response.json()
            print(f"\n[RISK VELOCITY - V_i]")
            print(f" -> VELOCITY: {data['velocity']}")
            print(f" -> STATUS: {data['status']}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Velocity monitoring failed: {e}\n")

    def run_backups(self):
        """Inventories all architectural backups (.bak) created by the Scribe Protocol."""
        print(f"\n[ARCHITECTURAL BACKUP INVENTORY]")
        try:
            response = requests.post(f"{BRIDGE_URL}/v1/chat", json={"messages": [{"role": "user", "name": self.user_name, "content": "list all .bak files in safe zones"}]}, timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(f"{self.identity}> {data.get('result', {}).get('message', {}).get('content', 'No backup data found.')}\n")
            else:
                print(f"\n[07 ERROR] Backup inventory failed: {response.text}\n")
        except Exception as e:
            print(f"\n[07 ERROR] Backup inventory exception: {e}\n")

    def run_handshake(self):
        """
        Executes a High-Authority Handshake.
        Provides a temporary +0.5 VSS boost to clear 409 Conflict Gates.
        """
        try:
            response = requests.post(f"{BRIDGE_URL}/v1/aas/handshake", timeout=5)
            data = response.json()
            print(f"\n[AAS AUTHORITY HANDSHAKE]")
            print(f" -> STATUS: {data['status']}")
            print(f" -> MESSAGE: {data['message']}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Handshake failed: {e}\n")

    def run_image(self, args: List[str]):
        """
        Executes local photorealistic image synthesis via /image or /imagegen.
        Attempts Iron Officer Bridge API first, with automatic fallback to standalone ComfyUI REST API.
        """
        if not args:
            print("\n[07] Usage: /image <photorealistic prompt descriptors>\n")
            return

        prompt_text = " ".join(args)
        print(f"\n[07 IMAGE SYNTHESIS]")
        print(f" -> PROMPT: {prompt_text}")

        # 1. Try Iron Officer Bridge API endpoint
        try:
            payload = {"prompt": prompt_text, "model_type": "flux_dev"}
            response = requests.post(f"{BRIDGE_URL}/v1/image/generate", json=payload, timeout=5)
            if response.status_code == 200:
                data = response.json()
                print(f" -> STATUS: SUCCESS (Bridge Gateway)")
                print(f" -> IMAGE PATH: {data.get('image_path')}")
                print("-" * 30 + "\n")
                return
        except Exception:
            pass # Fall through to direct standalone ComfyUI API

        # 2. Standalone Direct ComfyUI REST API Engine (Port 8188)
        try:
            stats_resp = requests.get(f"{COMFY_URL}/system_stats", timeout=3)
            if stats_resp.status_code == 200:
                seed = int(time.time() * 1000) % 1000000000
                workflow = {
                    "3": {
                        "inputs": {
                            "seed": seed, "steps": 25, "cfg": 3.5, "sampler_name": "euler",
                            "scheduler": "simple", "denoise": 1.0,
                            "model": ["4", 0], "positive": ["6", 0], "negative": ["7", 0], "latent_image": ["5", 0]
                        },
                        "class_type": "KSampler"
                    },
                    "4": {"inputs": {"unet_name": "flux1-dev-fp8.safetensors", "weight_dtype": "fp8_e4m3fn"}, "class_type": "UNETLoader"},
                    "5": {"inputs": {"width": 1024, "height": 1024, "batch_size": 1}, "class_type": "EmptyLatentImage"},
                    "6": {"inputs": {"text": prompt_text, "clip": ["8", 0]}, "class_type": "CLIPTextEncode"},
                    "7": {"inputs": {"text": "blurry, low quality, distorted", "clip": ["8", 0]}, "class_type": "CLIPTextEncode"},
                    "8": {"inputs": {"clip_name1": "clip_l.safetensors", "clip_name2": "t5xxl_fp8_e4m3fn.safetensors", "type": "flux"}, "class_type": "DualCLIPLoader"},
                    "9": {"inputs": {"samples": ["3", 0], "vae": ["10", 0]}, "class_type": "VAEDecode"},
                    "10": {"inputs": {"vae_name": "ae.safetensors"}, "class_type": "VAELoader"},
                    "11": {"inputs": {"filename_prefix": "IronKnight_Gen", "images": ["9", 0]}, "class_type": "SaveImage"}
                }
                client_id = str(uuid.uuid4())
                prompt_res = requests.post(f"{COMFY_URL}/prompt", json={"prompt": workflow, "client_id": client_id}, timeout=5)
                if prompt_res.status_code == 200:
                    p_id = prompt_res.json().get("prompt_id")
                    print(f" -> COMFYUI QUEUED: Prompt ID {p_id}")
                    print(f" -> Generating image on local GPU... Please wait.")

                    # Poll history endpoint for completion (up to 60 seconds)
                    for _ in range(120):
                        time.sleep(0.5)
                        try:
                            hist_resp = requests.get(f"{COMFY_URL}/history/{p_id}", timeout=3)
                            if hist_resp.status_code == 200 and p_id in hist_resp.json():
                                hist_data = hist_resp.json()[p_id]
                                outputs = hist_data.get("outputs", {}).get("11", {}).get("images", [])
                                if outputs:
                                    img_info = outputs[0]
                                    filename = img_info.get("filename")
                                    subfolder = img_info.get("subfolder", "")
                                    print(f" -> GENERATION COMPLETE!")
                                    print(f" -> FILENAME: {filename}")
                                    print(f" -> OUTPUT LOCATION: ComfyUI/output/{subfolder}/{filename}".replace("//", "/"))
                                else:
                                    print(f" -> GENERATION FINISHED.")
                                print("-" * 30 + "\n")
                                return
                        except Exception:
                            pass
                    print(f" -> [TIMEOUT] Generation queued but took longer than 60s.")
                else:
                    print(f" -> ERROR: ComfyUI rejected prompt payload: {prompt_res.text}")
            else:
                print(f" -> ERROR: ComfyUI API server not responding on {COMFY_URL}")
        except Exception as e:
            print(f" -> ERROR: Could not connect to ComfyUI Engine at {COMFY_URL}")
            print(f" -> Tip: Run 'run_comfy_api.bat' to start the ComfyUI API server.")
        print("-" * 30 + "\n")

    def run_tool_command(self, cmd: str, args: List[str]):
        """Directly executes AAS-governed tools (read, write, patch, etc.) via slash commands."""
        tool_map = {
            "/read": "read_file",
            "/write": "write_file",
            "/patch": "patch_file",
            "/append": "append_file",
            "/list": "list_files"
        }
        tool_name = tool_map.get(cmd)
        if not tool_name: return

        if not args:
            print(f"\n[07] Usage: {cmd} <path> [args...]\n")
            return

        path = args[0]
        tool_args = {}
        if tool_name == "read_file": tool_args = {"filepath": path}
        elif tool_name == "list_files": tool_args = {"directory": path}
        elif tool_name == "write_file": tool_args = {"filepath": path, "content": " ".join(args[1:])}
        elif tool_name == "append_file": tool_args = {"filepath": path, "content": " ".join(args[1:])}
        elif tool_name == "patch_file":
            if len(args) < 3:
                print(f"\n[07] Usage: /patch <path> <search> <replace>\n")
                return
            tool_args = {"filepath": path, "search": args[1], "replace": args[2]}

        print(f"\n[AAS TOOL EXECUTION: {tool_name}]")
        print(f" -> TARGET: {path}")
        try:
            payload = {"persona": self.identity.replace(" ", "_"), "command": tool_name, "arguments": tool_args}
            response = requests.post(f"{BRIDGE_URL}/v1/aas/execute", json=payload, timeout=10)
            data = response.json()
            if "error" in data:
                print(f" -> ERROR: {data['error']}")
                if data.get("status") == "409_CONFLICT_GATE":
                    print(f" -> AAS ALERT: Use /handshake to override.")
            else:
                print(f" -> STATUS: SUCCESS")
                if "content" in data:
                    print("-" * 20)
                    print(data["content"][:1000] + ("..." if len(data["content"]) > 1000 else ""))
                if "files" in data:
                    print(f" -> FILES: {', '.join(data['files'])}")
                if "backup" in data:
                    print(f" -> BACKUP: {data['backup']}")
            print("-" * 30 + "\n")
        except Exception as e:
            print(f"\n[07 ERROR] Tool execution failed: {e}\n")

    def run(self):
        """Main operational loop. Handles input aggregation, shorthand parsing, and bridge communication."""
        self.print_header()
        while True:
            try:
                # Multi-line input support (terminated by empty line or backslash logic)
                lines = []
                while True:
                    prompt = f"{self.user_name}> " if not lines else "... "
                    line = input(prompt).strip()
                    if not line: break
                    if line.endswith("\\"):
                        lines.append(line[:-1].strip())
                        continue
                    else:
                        lines.append(line)
                        break

                user_input = "\n".join(lines).strip()
                if not user_input: continue

                # --- Natural Command Bridge ---
                # Translates [ACTION]:[TARGET]: tags into functional slash commands
                if "[ACTION]:" in user_input.upper():
                    action = ""
                    target = ""
                    other_lines = []
                    for line in lines:
                        upper_line = line.upper()
                        if "[ACTION]:" in upper_line:
                            action = line.split(":", 1)[1].strip().lower()
                        elif "[TARGET]:" in upper_line:
                            target = line.split(":", 1)[1].strip()
                        else:
                            other_lines.append(line)

                    content = "\n".join(other_lines).strip()
                    if "retrieve" in action or "read" in action:
                        user_input = f"/read {target}"
                    elif "list" in action:
                        user_input = f"/list {target}"
                    elif "write" in action or "save" in action or "commit" in action:
                        user_input = f"/write {target} {content}"
                    elif "monitor" in action or "status" in action:
                        user_input = "/07"

                # shlex parsing for command arguments to support quoted paths
                try:
                    parts = shlex.split(user_input)
                except ValueError:
                    parts = user_input.split()

                if not parts: continue
                cmd = parts[0].lower()
                args = parts[1:]

                # Internal Command Handlers
                if cmd == "/exit": break
                if cmd == "/report": self.save_mission_report(); continue
                if cmd == "/status": self.show_status(); continue
                if cmd == "/07" or cmd == "/p": self.run_salute(); continue
                if cmd == "/t": self.run_telemetry(); continue
                if cmd == "/s": self.run_social(); continue
                if cmd == "/a": self.run_admin(); continue
                if cmd == "/verify": self.run_verify(args); continue
                if cmd == "/vss": self.run_vss(args); continue
                if cmd == "/phi": self.run_phi(); continue
                if cmd == "/velocity": self.run_velocity(); continue
                if cmd == "/backups": self.run_backups(); continue
                if cmd == "/handshake": self.run_handshake(); continue
                if cmd in ["/image", "/imagegen"]: self.run_image(args); continue
                if cmd in ["/read", "/write", "/patch", "/append", "/list"]:
                    self.run_tool_command(cmd, args); continue
                if cmd == "/tools":
                    self.show_tools = not self.show_tools
                    print(f"[07] Logs: {'ON' if self.show_tools else 'OFF'}")
                    continue

                # Standard Chat Path
                self.history.append({"role": "user", "content": user_input})
                response = requests.post(f"{BRIDGE_URL}/v1/chat", json={"messages": self.history}, timeout=120)

                if response.status_code == 200:
                    data = response.json()
                    tool_chain = data.get("tool_chain", [])
                    tool_outputs = data.get("tool_outputs", [])
                    result = data.get("result", {})

                    # Display Knight Tool Logs if enabled
                    if self.show_tools and tool_chain:
                        print("\n[KNIGHT TOOL LOG]")
                        for i, tool in enumerate(tool_chain):
                            name = tool.get("function", {}).get("name", "unknown")
                            args = tool.get("function", {}).get("arguments", {})
                            output = tool_outputs[i] if i < len(tool_outputs) else "No data"

                            # Diligent Scribe: Visual feedback on mutations
                            if name == "write_file":
                                print(f" -> [SCRIBE] TOTAL OVERWRITE: {args.get('filepath')}")
                                if isinstance(output, dict) and "backup" in output:
                                    print(f" -> [SCRIBE] BACKUP CREATED: {output['backup']}")
                                if isinstance(output, dict) and "scribe_warning" in output:
                                    print(f" -> [SCRIBE WARNING]: {output['scribe_warning']}")
                            elif name == "patch_file":
                                print(f" -> [SCRIBE] SURGICAL PATCH: {args.get('filepath')}")
                                if isinstance(output, dict) and "backup" in output:
                                    print(f" -> [SCRIBE] BACKUP CREATED: {output['backup']}")

                            print(f" -> EXECUTING: {name}({args})")
                            print(f" -> RESULT: {str(output)[:200]}...")
                        print("-" * 20)

                    ai_msg = result.get("message", {})
                    content = ai_msg.get("content", "...")
                    print(f"\n{self.identity}> {content}\n")
                    self.history.append({"role": "assistant", "content": content})
                else:
                    print(f"\n[07 ERROR] Bridge Error: {response.text}\n")

            except KeyboardInterrupt: break
            except Exception as e: print(f"\n[07 ERROR] {e}\n")

if __name__ == "__main__":
    vessel = ChatVessel()
    vessel.run()
