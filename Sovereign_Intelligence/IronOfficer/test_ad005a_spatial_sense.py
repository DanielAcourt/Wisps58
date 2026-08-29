import unittest
import json
from bridge import format_save_state, UnrealChatRequest

class TestSpatialWorldManifest(unittest.TestCase):
    def test_format_save_state_with_world_manifest(self):
        sample_world_manifest = {
            "BP_PlayerWisp_C_0": {
                "Identity": {
                    "GUID": "7D4406E247C5C1A17AE05F927BBBA2B3",
                    "ActorName": "BP_PlayerWisp_C_0",
                    "Class": "BP_PlayerWisp_C"
                },
                "SpatialTransform": {
                    "Location": {"X": 100.0, "Y": 250.0, "Z": 50.0},
                    "Rotation": {"Pitch": 0.0, "Yaw": 90.0, "Roll": 0.0}
                },
                "Attributes": {
                    "STR": 1,
                    "DEX": 18,
                    "CON": 5,
                    "INT": 25,
                    "WIS": 12,
                    "CHA": 10
                }
            },
            "BP_Antelope_C_UAID_1": {
                "Identity": {
                    "GUID": "91D2C3924FA0891591621F851295390B",
                    "ActorName": "BP_Antelope_C_UAID_1",
                    "Class": "BP_Antelope_C"
                },
                "SpatialTransform": {
                    "Location": {"X": 500.0, "Y": 250.0, "Z": 0.0},
                    "Rotation": {"Pitch": 0.0, "Yaw": 180.0, "Roll": 0.0}
                },
                "Attributes": {
                    "STR": 12,
                    "DEX": 14,
                    "CON": 12,
                    "INT": 3,
                    "WIS": 12,
                    "CHA": 6
                }
            }
        }

        formatted = format_save_state(state=None, world_manifest=sample_world_manifest)

        # Assert world manifest headers and entities are present
        self.assertIn("REGISTERED WORLD MANIFEST (2 Active Entities in Unreal Scene):", formatted)
        self.assertIn("[ENTITY: BP_PlayerWisp_C_0]", formatted)
        self.assertIn("[ENTITY: BP_Antelope_C_UAID_1]", formatted)

        # Assert spatial transforms are formatted
        self.assertIn("Location: (X=100.00, Y=250.00, Z=50.00)", formatted)
        self.assertIn("Location: (X=500.00, Y=250.00, Z=0.00)", formatted)

        # Assert full attributes are present for BP_Antelope
        self.assertIn("STR: 12", formatted)
        self.assertIn("DEX: 14", formatted)
        self.assertIn("WIS: 12", formatted)

    def test_unreal_chat_request_schema(self):
        payload = {
            "actor_name": "BP_PlayerWisp_C_0",
            "message": "What are the attributes of the antelope?",
            "world_manifest": {
                "BP_Antelope_C_UAID_1": {
                    "Attributes": {"STR": 12, "DEX": 14, "CON": 12, "INT": 3, "WIS": 12, "CHA": 6}
                }
            }
        }
        req = UnrealChatRequest(**payload)
        self.assertEqual(req.actor_name, "BP_PlayerWisp_C_0")
        self.assertIsNotNone(req.world_manifest)
        self.assertIn("BP_Antelope_C_UAID_1", req.world_manifest)

if __name__ == "__main__":
    unittest.main()
