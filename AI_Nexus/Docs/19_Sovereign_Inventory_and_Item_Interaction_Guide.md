// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07
# Sovereign Framework: Inventory System & Item Interaction Guide
## Architecture, World Pickups, Fragment Composition & Persistence Specification

---

## 1. Executive Summary & Design Philosophy

The **Sovereign Fragment Inventory Architecture (Approach C)** provides a high-performance, single-player-optimized inventory system tailored to the unique vessel-possession mechanics of **WispsGame**.

```text
       Spiritual Layer                      Physical Vessel Layer
┌───────────────────────────┐         ┌─────────────────────────────────┐
│   ASovereignPlayerWisp    │         │     ASovereignBaseCharacter     │
│  (The Pilot / Inhabitant) │         │ (Humanoids, Elves, Dragons, etc)│
├───────────────────────────┤         ├─────────────────────────────────┤
│ - Spatial Movement        │Possesses│ - USovereignBioComponent        │
│ - Soul Ejection (F key)   │───────► │ - USovereignQiComponent         │
│ - Interaction Traces (E)  │         │ - USovereignLevelComponent      │
│ - NO INVENTORY BURDEN     │         │ - USovereignInventoryComponent  │
└───────────────────────────┘         │ - USovereignSaveableEntityComp  │
                                      └─────────────────────────────────┘
```

### Core Tenets:
1. **Vessel-Only Inventory:** Physical character vessels (`ASovereignBaseCharacter`) carry the inventory pack. The spiritual player wisp has no pockets or item weight. When a wisp ejects from an antelope or dragon, the items remain safely secured on that vessel.
2. **Composition via Fragments:** Rather than monolithic C++ item classes with 50 unused fields, an item is an archetype (`USovereignItemDefinition`) containing small, reusable behavioral pieces (**Fragments**).
3. **Zero Garbage Collection Clutter:** Runtime inventory slots are stored as lightweight **value structs** (`FSovereignInventoryItemInstance`). Adding, moving, or consuming 100 items never allocates heap `UObjects`.
4. **Deterministic Single-Player Execution:** No complex networking serializers (`FFastArraySerializer`) or prediction rollbacks.
5. **Single Source of Truth (SSoT) Persistence:** The entire inventory serializes atomically into `"Sovereign.Inventory"` JSON through `USovereignInventoryBroker` and `USovereignSaveableEntityComponent`.

---

## 2. Architecture & Type Hierarchy

The system consists of five distinct, cohesive C++ types:

```text
┌────────────────────────────────────────────────────────┐
│             USovereignItemDefinition                   │
│  (UPrimaryDataAsset / PrimaryAssetType "SovereignItem")│
├────────────────────────────────────────────────────────┤
│ - ItemTag (FGameplayTag)                               │
│ - DisplayName (FText)                                  │
│ - AssociatedSpeciesData (TSoftObjectPtr)               │
│ - Fragments Array (Instanced Polymorphic UObjects)     │
│    ├── USovereignItemFragment_UIData                   │
│    ├── USovereignItemFragment_Weight                   │
│    ├── USovereignItemFragment_Consumable               │
│    ├── USovereignItemFragment_Spiritual                │
│    └── USovereignItemFragment_Equippable               │
└────────────────────────────────────────────────────────┘
                           ▲
                           │ Soft Reference
┌──────────────────────────┴─────────────────────────────┐
│          FSovereignInventoryItemInstance               │
│               (Plain Value Struct)                     │
├────────────────────────────────────────────────────────┤
│ - InstanceId (FGuid)                                   │
│ - ItemDefinition (TSoftObjectPtr)                      │
│ - Quantity (int32)                                     │
│ - DynamicTags (FGameplayTagContainer)                  │
│ - DynamicProperties (TMap<FString, FString>)           │
└────────────────────────────────────────────────────────┘
                           ▲
                           │ Stored in Slots Array
┌──────────────────────────┴─────────────────────────────┐
│           USovereignInventoryComponent                 │
│      (ActorComponent on ASovereignBaseCharacter)       │
├────────────────────────────────────────────────────────┤
│ - MaxSlotCount: int32                                  │
│ - InventorySlots: TArray<FSovereignInventoryItemInst> │
│ - AddItem() / RemoveItemByInstanceId()                 │
│ - ConsumeItem() / EquipItem() / UnequipItem()          │
│ - Delegates: OnItemAdded, OnItemRemoved, OnUpdated    │
└────────────────────────────────────────────────────────┘
                           │
                           │ Bridges to SSoT Save Hub
                           ▼
┌────────────────────────────────────────────────────────┐
│             USovereignInventoryBroker                  │
│     (Implements ISovereignBrokerInterface)             │
├────────────────────────────────────────────────────────┤
│ - OnSave() -> Serializes slots into JSON               │
│ - OnLoad() -> Restores slots, tags, and properties     │
└────────────────────────────────────────────────────────┘
```

---

## 3. World Interaction & Item Pickup Flow

How does a character in the world physically pick up an item from the ground?

### 3.1 The World Pickup Actor Pattern
World pickups inherit from `ASovereignBaseInteractable` (or derive from `AUniversalActor`):

```cpp
UCLASS()
class WISPCPP7VR_API ASovereignItemPickup : public ASovereignBaseInteractable
{
    GENERATED_BODY()

public:
    /** Archetype definition of the item dropped in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Pickup")
    USovereignItemDefinition* ItemDefinition;

    /** Quantity of the item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Pickup")
    int32 Quantity = 1;

    /** Universal interaction implementation */
    virtual void OnInteract_Implementation(AActor* Interactor) override;
};
```

### 3.2 The Interaction Handshake (`OnInteract_Implementation`)
When a player looks at the item and presses the interaction key (`E` key):
1. `ASovereignBaseCharacter::Interact()` performs a line-of-sight sphere trace (`GetSensedActor()`).
2. Detects that the target implements `IInteractionInterface`.
3. Calls `IInteractionInterface::Execute_OnInteract(Target, this)`.
4. The pickup extracts the character's `USovereignInventoryComponent` and deposits the item:

```cpp
void ASovereignItemPickup::OnInteract_Implementation(AActor* Interactor)
{
    if (!Interactor || !ItemDefinition) return;

    // Check if the Interactor is a character vessel with an inventory pack
    ASovereignBaseCharacter* Character = Cast<ASovereignBaseCharacter>(Interactor);
    if (!Character)
    {
        // If interactor is a Wisp, get the vessel it is possessing
        if (IInteractionInterface* Interface = Cast<IInteractionInterface>(Interactor))
        {
            Character = Cast<ASovereignBaseCharacter>(Interface->Execute_GetInhabitingSpirit(Interactor));
        }
    }

    if (Character)
    {
        if (USovereignInventoryComponent* Inv = Character->GetInventoryComponent())
        {
            if (Inv->AddItem(ItemDefinition, Quantity))
            {
                // Play pickup sound / Niagara effect
                Destroy(); // Successfully collected into pack
            }
            else
            {
                // Inventory is full! Prompt UI notification: "Inventory Full"
            }
        }
    }
}
```

---

## 4. Item Consumption Lifecycle

When a character uses a food, drink, elixir, or class tome:

```text
Player clicks "Consume" in UI
             │
             ▼
USovereignInventoryComponent::ConsumeItem(InstanceId, Quantity)
             │
             ├─ Validates InstanceId and checks (Quantity <= Slot.Quantity)
             ├─ Resolves ItemDefinition
             │
             ▼
Iterates Fragments on ItemDefinition
 ├─ USovereignItemFragment_Consumable::OnConsumed()
 │   ├─ Restores Hunger / Hydration / Fatigue on USovereignBioComponent
 │   └─ Awards StrengthXP / WisdomXP on USovereignLevelComponent
 └─ USovereignItemFragment_Spiritual::OnConsumed()
     └─ Calls USovereignQiComponent::AbsorbQi()
             │
             ▼
If any fragment returns true:
 Decrements Slot.Quantity (or removes slot if 0)
 Broadcasts OnItemRemoved / OnInventoryUpdated
```

### Blueprint Usage:
In UMG or Blueprint graphs, consuming an item is a single clean node:
```text
[Get Inventory Component] ──► [Consume Item] (InstanceId, Quantity: 1)
```

---

## 5. Equipment & Visual Sockets

When an item contains `USovereignItemFragment_Equippable`:

```cpp
// Equipping an item
InvComp->EquipItem(ItemInstanceGuid, FName("hand_r_socket"));
```

1. **`OnEquipped` Hook:** Queries the character's skeletal mesh (`TargetCharacter->GetMesh()`).
2. Spawns/attaches the mesh component to the requested socket using `FAttachmentTransformRules::SnapToTargetNotIncludingScale`.
3. Adds the `Item.State.Equipped` Gameplay Tag to `FSovereignInventoryItemInstance::DynamicTags`.
4. **`OnUnequipped` Hook:** Detaches the spawned mesh from the socket and destroys the component, clearing `Item.State.Equipped`.

---

## 6. Persistence & World Manifest Saves

The inventory system seamlessly connects to the Sovereign Framework's Single Source of Truth (`USovereignSaveableEntityComponent`):

### 6.1 The Broker Contract
`ASovereignBaseCharacter::BeginPlay()` instantiates `USovereignInventoryBroker` and registers it:
```cpp
InventoryBroker = NewObject<USovereignInventoryBroker>(this, TEXT("InventoryBroker"));
InventoryBroker->Initialize(InventoryComponent);
SaveDataComponent->RegisterBroker(InventoryBroker);
```

### 6.2 Saved JSON Structure
When saving the world, the broker generates clean, human-readable JSON:
```json
{
  "Sovereign.Inventory": {
    "MaxSlotCount": 20,
    "Slots": [
      {
        "InstanceId": "9F1B2C3D-4E5F-6A7B-8C9D-0E1F2A3B4C5D",
        "ItemDef": "/Game/Data/ItemAssets/DA_RubyDragonEgg.DA_RubyDragonEgg",
        "Quantity": 1,
        "DynamicTags": [
          "Item.State.Equipped"
        ],
        "DynamicProperties": {
          "EggIncubationTier": "2",
          "RescuedDungeon": "VaultOfFire"
        }
      },
      {
        "InstanceId": "7A8B9C0D-1E2F-3A4B-5C6D-7E8F9A0B1C2D",
        "ItemDef": "/Game/Data/ItemAssets/DA_SpiritQiElixir.DA_SpiritQiElixir",
        "Quantity": 5,
        "DynamicTags": [],
        "DynamicProperties": {}
      }
    ]
  }
}
```

### 6.3 Restoration on Load
When the character is streamed in from a `.sav` file:
1. `USovereignSaveableEntityComponent` loads the JSON block.
2. `USovereignInventoryBroker::OnLoad()` clears current slots.
3. Parses each item, resolving the DataAsset soft path.
4. Restores exact GUIDs, quantities, tags, and custom metadata without GC leaks.
5. Broadcasts `OnInventoryUpdated`.

---

## 7. Master CSV Authoring & Content Pipeline

Content designers do not write C++ code to add new items. Everything is driven upstream from `AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv`.

### 7.1 CSV Column Mapping
| Column | Type | Maps To Fragment | Description |
| :--- | :--- | :--- | :--- |
| `Name` | String | DataAsset Name | e.g. `Item_RubyDragonEgg` $\rightarrow$ `DA_RubyDragonEgg` |
| `ItemTag` | GameplayTag | `USovereignItemDefinition::ItemTag` | Primary identifier tag (`Item.Egg.Dragon.Ruby`) |
| `DisplayName` | Text | `USovereignItemFragment_UIData` | User-facing display title in UMG cards |
| `Weight` | Float | `USovereignItemFragment_Weight` | Unit weight (kg). Feeds into total carry encumbrance |
| `MaxHunger` | Float | `USovereignItemFragment_Consumable` | Hunger points restored on consumption |
| `MaxHydration`| Float | `USovereignItemFragment_Consumable` | Hydration points restored on consumption |
| `QiGive` | Float | `USovereignItemFragment_Spiritual` | Instant spiritual Qi absorbed on consumption |
| `StrengthXP` | Double | `USovereignItemFragment_Consumable` | D&D stat XP rollover into `USovereignLevelComponent` |

### 7.2 Running the Synthesizer
From Unreal Engine's Python Console or an Editor Utility Widget:
```python
import import_item_definitions
import_item_definitions.execute_item_importer_ui()
```
Or from terminal:
```bash
python WispsCPPVR/Content/Python/import_item_definitions.py
```
This reads the CSV and auto-generates populated `USovereignItemDefinition` assets in `/Game/Data/ItemAssets/`.

---

## 8. Developer Quick-Start: Common Recipes

### Recipe A: Adding a Custom Fragment in C++
To create a fragment for weapon durability, elemental enchantment, or dragon bonding:
1. Inherit from `USovereignItemFragment`.
2. Add `DefaultToInstanced, EditInlineNew` to `UCLASS`.
3. Override `OnConsumed`, `OnEquipped`, or define custom methods:

```cpp
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Weapon Durability Fragment"))
class WISPCPP7VR_API USovereignItemFragment_Durability : public USovereignItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Durability")
    float MaxDurability = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Durability")
    float DegradationPerStrike = 1.0f;
};
```

### Recipe B: Querying Fragments in C++
```cpp
USovereignItemDefinition* ItemDef = ...;

// Fast templated query
if (const USovereignItemFragment_Consumable* Consumable = ItemDef->FindFragment<USovereignItemFragment_Consumable>())
{
    float RestoreAmount = Consumable->HungerRestore;
}
```

### Recipe C: Querying Fragments in Blueprint
Use the Blueprint node **Get Fragment By Class**:
```text
[Item Def] ──► [Get Fragment By Class] (Class: Consumable Fragment) ──► [Cast to Consumable Fragment] ──► Get Hunger Restore
```

---

## 9. Verification & Automated Testing Matrix

The inventory system is fully covered by automated tests in `WispsCPPVR/Source/WispCPP7VRTests/` and Python:

| Test Suite | File Path | Coverage |
| :--- | :--- | :--- |
| `Sovereign.Inventory.ItemCore` | `Private/Inventory/SovereignItemCoreTests.cpp` | DataAsset instantiation, `PrimaryAssetId` generation, templated & Blueprint fragment retrieval |
| `Sovereign.Inventory.ComponentOperations` | `Private/Inventory/SovereignInventoryComponentTests.cpp` | Adding items, stack merging, slot capacity gating, partial & complete removal |
| `Sovereign.Inventory.CharacterIntegration` | `Private/Inventory/SovereignInventoryComponentTests.cpp` | `ASovereignBaseCharacter` subobject initialization and ownership |
| `Sovereign.Inventory.ConsumableFragment` | `Private/Inventory/SovereignDomainFragmentsTests.cpp` | Hunger replenishment, LevelComponent StrengthXP accumulation |
| `Sovereign.Inventory.WeightFragment` | `Private/Inventory/SovereignDomainFragmentsTests.cpp` | Unit and stack weight math |
| `Sovereign.Inventory.SpiritualFragment` | `Private/Inventory/SovereignDomainFragmentsTests.cpp` | Spiritual Qi absorption into `USovereignQiComponent` |
| `Sovereign.Inventory.PersistenceRoundtrip` | `Private/Inventory/SovereignInventoryPersistenceTests.cpp` | JSON serialization, slot mutation, and 100% deserialization restoration |
| `test_b070_item_importer` | `Sovereign_Intelligence/IronOfficer/test_b070_item_importer.py` | CSV parsing, decision matrix fragment assignment, dry-run execution |
