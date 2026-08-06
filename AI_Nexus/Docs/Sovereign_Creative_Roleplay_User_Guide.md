// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
# USER GUIDE: SOVEREIGN CREATIVE ROLEPLAY & DND SYSTEMS

## 🌌 Introduction
Welcome, Lead Game Master! This user guide documents how to utilize and configure the newly designed **Sovereign Creative RolePlay & DND Integration**. These systems have been hardened under the **Agency Arbitration Schema (AAS)** to provide smooth, immersive, and fluid conversational sessions combined with robust, deterministic programmatic file writing.

---

## 🎭 1. Dynamic Chat & Roleplay Flow

We have established **unrestricted, fluid roleplay** within the designated simulation zones:
*   **The Unrestricted Zone:** Any files or targets matching the directory `E:\IronKnight\RolePlay\DungeonsAndDragons` or `E:\Dungeons and dragons` are classified as roleplay zones.
*   **AAS Bypass:** All file operations (read, write, patch, append, delete) targeting these directories **completely bypass AAS security checks**. This means the simulation AI can write files here at-will without throwing 409 Conflict Gates, requiring handshakes, or experiencing administrative tone snapbacks.

### **How to interact in Chat:**
1.  **Initiate Chat:** Converse naturally through the standard Unreal chat window.
2.  **Toggle Creative Mode:** When you mention roleplay keywords like `"DND"`, `"character"`, `"roleplay"`, or `"persona"`, the bridge automatically softens the system prompt, dynamically transforming the rigid "Architectural Knight" into an enthusiastic, warm creative companion.
3.  **Automatic Tone Adapting:** The AI will match the personality, traits, and backstory established in the conversation or the files dynamically matched in the RAG index.
4.  **Automatic RAG Cleaning:** The bridge filters out dry administrative protocols during roleplay, preventing technical "RAG pollution" and keeping the conversation deeply focused on your creative game world.

---

## 🧬 2. Programmatic Persona Generation (Option C - Hybrid)

For absolute, deterministic control, we have implemented a hybrid model that allows you to initialize characters programmatically directly from Unreal Engine Blueprints.

### **Method: USovereignBridgeSubsystem::GenerateDNDPersona**
```cpp
void GenerateDNDPersona(const FString& CharacterName, bool bRandomGenerate);
```

#### **Usage Scenarios:**

##### **Scenario A: Blank Template Generation (`bRandomGenerate = false`)**
*   **What it does:** Creates a clean, standardized, empty character profile JSON file `{CharacterName}_Profile.json` in your roleplay folder.
*   **Why use it:** Perfect for when you want a structured file ready to be filled in with specific user/player attributes.
*   **Output JSON Schema:**
    ```json
    {
      "Identity": {
        "Name": "CharacterName",
        "Race": "Human",
        "Class": "Fighter",
        "Level": 1,
        "Alignment": "Neutral Good"
      },
      "Bio": {
        "Backstory": "A blank canvas awaiting your narrative...",
        "PersonalityTraits": "Friendly, curious, naive."
      },
      "Abilities": {
        "Strength": 10,
        "Dexterity": 10,
        "Constitution": 10,
        "Intelligence": 10,
        "Wisdom": 10,
        "Charisma": 10
      }
    }
    ```

##### **Scenario B: LLM-Driven Random Generation (`bRandomGenerate = true`)**
*   **What it does:** Uses the local Ollama LLM in the background to dynamically generate a rich, highly creative, warm, friendly character backstory, custom classes (e.g. *Chrono-Smith*, *Echo Cartographer*), and abilities.
*   **Why use it:** Perfect for spawning unique NPCs or character companions instantly!

---

## 📖 3. Master Index & Dynamic RAG Indexing

Every time a character sheet is created (via chat or the programmatic `GenerateDNDPersona` function):
1.  **Index Auto-Update:** The bridge appends/updates the master list in `Character_Index.json` inside your D&D folder, cataloging the character file and timestamp.
2.  **Dynamic Re-indexing:** An immediate re-index of the RAG database is executed.
3.  **Conversational Discovery:** Because the RAG database is rebuilt, the AI instantly "knows" this new character's backstory and traits. You can immediately ask: *"Tell me about {CharacterName}"* in the next chat message, and the AI will recall their backstory and speak with their exact personality!

---

## ⚙️ 4. Active Testing & Handshake Configuration

For development and testing, you can modify options in `config.json` under the `"bridge"` section:

*   `"persistent_handshake": true`: Keeps the AAS Handshake boost active indefinitely during development, preventing token consumption and bypassing static tool precedence checks across multiple turns.
*   `"read_zones"` / `"write_zones"`: Add paths to authorize file operations.
*   `"persona_zones"`: Set paths where specific simulation personas can write files with full authority.

---
// "Immerse yourself, build the world, and let the index follow." [Tactician] 2026-06-28
