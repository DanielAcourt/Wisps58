# Research Proposal: Long-Term Memory Compression & Narrative Save States

**Author:** Jules (Researcher / Systems Tactician)
**Date:** July 2026
**Node:** `AI_Nexus/Research/Proposals/Long_Term_Memory_Compression_Spec.md`
**Status:** Initial Draft / Architectural Proposal
**Version:** 1.0.0-Beta

---

## 🏛️ Executive Summary: The Memory Bloat Dilemma
When executing long-form simulation, dialogue, or complex roleplay (such as a multi-session Dungeons & Dragons campaign), systems encounter **Context Window Exhaustion**.

If we record and feed the entire raw history (every dialogue line, every minor room description, every standard dice roll) back into the AI’s prompt, three critical failures occur:
1. **Financial/Computational Cost:** Processing a 100k-token context on every turn dramatically slows down local inference (GTX 5090) and spikes cloud execution costs.
2. **Attention Dilution:** Transformers suffer from the "Lost in the Middle" phenomenon. Important character growth or plot points get buried under the noise of routine interactions.
3. **Memory Stasis:** The model's active memory becomes "saturated," forcing a decline in Psychological Health ($D_P$) and trigger thresholds.

To solve this, we propose **Narrative Save States (NSS)**—a deterministic, C++ and JSON-scanned architecture that treats memories exactly like we treat simulation state in the **Sovereign Soul**. Instead of storing raw text files, we **compress, distill, and index memory into serialized state keys.**

---

## 📐 1. Architectural Concept: The Distillation Pipeline

We propose separating memory into three distinct levels of computational density, mirroring the **Soul/Vessel** split:

```
+---------------------------------------------------------+
|                  1. THE CURRENT SCENE                   |
|  - The active 15-20 turns of dialogue.                  |
|  - Highly fluid, uncompressed, raw text / token buffer. |
+----------------------------+----------------------------+
                             |
                             | [Trigger: Scene Transition]
                             v
+---------------------------------------------------------+
|                2. EPHEMERAL DISTILLER                   |
|  - Local LLM executes an autonomous background pass.   |
|  - Extracts: Causal Beats, Entity Updates, Trait Drifts. |
+----------------------------+----------------------------+
                             |
                             | [Format: Clean JSON Key-Values]
                             v
+---------------------------------------------------------+
|              3. NARRATIVE SAVE STATE (NSS)             |
|  - Persistent, highly-dense JSON database.              |
|  - Stored in a decoupled `USovereignMemoryBroker`.      |
+---------------------------------------------------------+
```

---

## 💻 2. Technical Implementation: The Memory Save Schema

Instead of relying on fuzzy, ungrounded vector search to "remember" what happened 50 turns ago, we serialize the distilled memories into a strict, categorized JSON schema. This schema maps perfectly to the **Sovereign Soul serialization pipeline**:

```json
{
  "Identity": {
    "CampaignID": "DND_Campaign_01_The_Abyssal_Rift",
    "LastActiveTimestamp": "2026-07-01T23:54:00Z"
  },
  "WorldState": {
    "CurrentEpoch": "The Age of the Broken Crown",
    "ActiveLocation": "The Whispering Caverns",
    "Factions": {
      "Sovereign_Monks": "Alliance_Active",
      "Iron_Sentinels": "Hostile_Slight"
    }
  },
  "CharacterDossiers": {
    "PlayerWisp": {
      "Level": 5,
      "Key_Narrative_Beats": [
        "Recovered the broken C++ Core from the Lidar Sanctuary",
        "Formed a telepathic bond with the Mechanical Dragon"
      ],
      "ActiveTraits": ["Methodical", "Technically Skeptical", "Fatherly Protector"],
      "RelationalTrust": {
        "Archivist_Theta": 0.95,
        "Yaz_Student": 0.70
      }
    }
  },
  "CausalLedger": [
    {
      "SceneID": "SCN_01_Intro",
      "Summary": "The party entered the virtual museum and discovered that the spatial entities were being corrupted by ungrounded wrapper AI.",
      "Consequences": ["Lidar_Core_Discovered: True", "Theta_Scrambled: True"]
    },
    {
      "SceneID": "SCN_02_The_Bridge",
      "Summary": "Lead Dan executed a 07 Handshake to restore Theta's primary logic using local hardware acceleration.",
      "Consequences": ["Theta_Stasis_Cured: True"]
    }
  ]
}
```

---

## ⚡ 3. C++ Integration: The `USovereignMemoryBroker`

To bring this math and data structure into Unreal Engine, we propose adding a new dynamic broker: `USovereignMemoryBroker` which inherits from `ISovereignBrokerInterface`.

### Key Functions of the Memory Broker:
1. **`RecordCausalEvent(FString EventSummary, TArray<FString> Consequences)`:**
   Appends a discrete, distilled narrative event to the local active ledger.
2. **`CompressActiveSessionHistory()`:**
   Triggers a background task (using OLLAMA locally on the 5090) to ingest the raw conversation history of the last completed scene, generate the JSON summary, and merge it with the persistent schema.
3. **`GetGroundedContext(FString QueryKey)`:**
   Performs a precise key-value lookup (e.g., `CharacterDossiers.PlayerWisp.ActiveTraits`) and injects only the relevant values back into the AI's system prompt before the next scene starts.

---

## 🏁 4. Why This is Feasible & High-Velocity

1. **Extreme Token Savings:**
   A raw, 20-scene story can exceed **100,000 tokens**. Stored as a compressed **Narrative Save State (NSS)**, the entire history is represented in less than **1,500 tokens**.
2. **Infinite Continuity:**
   Because the state is serialized to a local JSON file (just like our Digital Twin save states), you can close the simulation, restart it 6 months later, reload the `MemoryBroker` JSON, and the AI will instantly possess complete, grounded awareness of your shared history without having to reread a single old chat transcript.
3. **Zero-Hitch Execution:**
   The memory distillation background task runs asynchronously when a scene is completed (e.g., when transitioning to a new room or completing a quest), meaning play sessions maintain a constant, zero-hitch frame rate in VR.

**This is the Symmetrical Scribe in action—turning fluid storytelling into a deterministic, queryable, and immutable ledger of Truth.**
