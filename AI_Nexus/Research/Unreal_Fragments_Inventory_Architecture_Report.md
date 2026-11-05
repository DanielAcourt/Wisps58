// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07
# Research Report: Unreal Engine Fragment Architecture for Character Vessel Inventory

> **Author:** Jules (The Researcher / INTP Logician)
> **Target System:** WispsGame Character Vessel Architecture (`ASovereignBaseCharacter`)
> **Status:** Completed Architectural Research & Technical Specification
> **Context:** Single-Player Desktop PC First-Person Wisp Navigation, Entity Possession, and Phase 2 Dungeon Extraction

---

## Executive Summary

WispsGame is evolving from core garden entity stance manipulation, biological metabolism, and cultivation into Phase 2 turn-based combat, rogue-lite dungeon exploration, and reverse-dungeon extraction mechanics. As player wisps (`ASovereignPlayerWisp`) explore the world, they possess physical character vessels (`ASovereignBaseCharacter` subclasses such as Humanoids, Elves, and Dragons).

A foundational design requirement has been established:
1. **Vessel-Only Inventory:** Inventory belongs strictly to physical character vessels (`ASovereignBaseCharacter`). The spiritual player wisp does not possess a physical pack or item inventory. When a wisp unpossesses or ejects from a host, the physical vessel retains its gear, supplies, and harvested loot.
2. **Deterministic Single-Player Scope:** WispsGame is a focused single-player desktop PC experience. We do not incur the complex networking overhead of fast array replication serializers (`FFastArraySerializer`), RPC handshakes, or server-authoritative prediction algorithms.
3. **Fragment-Driven Composition:** Instead of rigid monolithic C++ item classes with dozens of unused variables, modern game development leverages **Fragments**—small, decoupled data-and-logic components that attach modularly to item definitions.
4. **Cohesive Sovereign Integration:** The inventory system must seamlessly interface with existing Sovereign components:
   - Biological vitals on `USovereignBioComponent` (hunger, hydration, stamina, gestation).
   - Spiritual cultivation and aura parameters on `USovereignQiComponent` (`QiGive`, `QiTake`, `QiAuraRadius`).
   - D&D base attributes on `USovereignAttributeComponent` (STR, DEX, CON, INT, WIS, CHA, LCK).
   - Experience accumulators and item XP consumption on `USovereignLevelComponent` (`StrengthXP` through `LuckXP`).
   - Master data tables (`DT_SovereignSpeciesTemplate.csv` / `FSovereignItemRow`).
   - Central state persistence and atomic save files via `USovereignSaveableEntityComponent` and `ISovereignBrokerInterface`.

This research report provides a deep comparative analysis of Unreal Engine's fragment architectures (Lyra UObject Fragments vs. Mass ECS Struct Fragments vs. Sovereign DataAsset/Struct Hybrid), delivers an exhaustive SWOT analysis, and outlines the complete architectural specification for the Sovereign Character Vessel Inventory.

---

## 1. What are "Fragments" in Unreal Engine?

In traditional game programming (e.g., UE3 / early UE4), item systems were often built using deep object-oriented inheritance hierarchies:
```text
UItemBase
 ├── UEquippableItem
 │    ├── UWeaponItem
 │    │    └── URangedWeaponItem
 │    └── UArmorItem
 └── UConsumableItem
      ├── UFoodItem
      └── UPotionItem
```
This hierarchy quickly collapses under modern game design:
- What happens when a magical sword can also be consumed (broken for Spirit Qi)?
- What happens when an ancient dragon egg can be incubated (biological entity), worn as a relic (equipment), or crushed into an alchemy reagent (consumable)?
Multiple inheritance is prohibited in `UObject` hierarchies, forcing developers into bloated base classes ("god objects") where every item carries fields for durability, nutrition, damage, armor, spell effects, and trade values—90% of which are unused by any individual item.

To solve this, modern Unreal Engine paradigms adopt **Composition via Fragments**. In this paradigm:
1. **Item Definition:** A lightweight container or identifier representing the archetype of the item.
2. **Item Fragments:** Small, specialized pieces of data and behavior that attach to the item definition. An item is defined entirely by the *sum of its fragments*.
3. **Item Instance:** The runtime representation of an item in a container or slot, holding mutable state (stack count, durability, custom enchantments, unique item GUID) and referencing its immutable Item Definition.

```text
┌────────────────────────────────────────────────────────┐
│             USovereignItemDefinition                   │
│  (Primary DataAsset / Static Archetype Definition)     │
├────────────────────────────────────────────────────────┤
│ Fragments Array:                                       │
│  ├─ [Fragment_UIData]        (Name, Icon, Tooltip)     │
│  ├─ [Fragment_Weight]        (Encumbrance, Stacking)   │
│  ├─ [Fragment_Consumable]    (Hunger, Hydration, XP)   │
│  ├─ [Fragment_Spiritual]     (Qi Give/Take, Aura)      │
│  └─ [Fragment_Equippable]    (Socket, Mesh, Modifiers) │
└────────────────────────────────────────────────────────┘
                           ▲
                           │ References Archetype
┌──────────────────────────┴─────────────────────────────┐
│          FSovereignInventoryItemInstance               │
│          (Runtime State in Vessel Pack)                │
├────────────────────────────────────────────────────────┤
│ - ItemInstanceId (FGuid)                               │
│ - ItemDef (USovereignItemDefinition*)                  │
│ - StackCount (int32)                                   │
│ - DynamicTags (FGameplayTagContainer)                  │
│ - DynamicProperties (TMap<FString, FString>)           │
└────────────────────────────────────────────────────────┘
```

---

## 2. Deep Dive: Three Fragment Architectural Paradigms

In Unreal Engine 5, developers typically evaluate three distinct fragment patterns:
1. **Approach A: Lyra-Style `UObject` Fragment Composition** (`UInventoryItemDefinition` + `UInventoryItemFragment`)
2. **Approach B: Mass Entity ECS Struct Fragments** (`FMassFragment` / `FMassEntityHandle`)
3. **Approach C: Sovereign DataAsset + Instanced Struct Hybrid** (`USovereignItemDefinition` + `TArray<FInstancedStruct>` or typed UObject fragments)

### Approach A: Lyra Modular Item Fragment Pattern
Pioneered by Epic Games in the Lyra Starter Game (`LyraInventoryManagerComponent`, `ULyraInventoryItemDefinition`, `ULyraInventoryItemInstance`, `ULyraInventoryItemFragment`).

- **Architecture:**
  - `ULyraInventoryItemDefinition` inherits from `UObject`. It contains an array of `UPROPERTY(EditDefaultsOnly, Instanced, Category = "Fragments") TArray<TObjectPtr<UInventoryItemFragment>> Fragments`.
  - Designers create Blueprint/DataAsset sub-definitions (e.g., `ID_HealthPotion`, `ID_SilverSword`) and click "Add Element" to attach polymorphic `UInventoryItemFragment` classes:
    - `UItemFragment_EquippableItem`
    - `UItemFragment_SetStats`
    - `UItemFragment_UIInfo`
  - In Lyra, `ULyraInventoryItemInstance` is also a `UObject` allocated on the heap for every individual item instance in an inventory list, replicated via `FFastArraySerializer`.
- **Strengths:**
  - Native Unreal Engine Editor polymorphism. Designers can choose any fragment subclass from a drop-down menu in the Details panel.
  - Native Blueprints and UMG integration without custom slate or struct wrapper boilerplate.
  - Each fragment is an isolated C++ class with virtual functions (`OnItemEquipped`, `OnItemConsumed`).
- **Weaknesses:**
  - Heap allocation overhead if runtime instances are full UObjects.
  - Subobject management during serialization.
  - Default implementation is tightly coupled to Gameplay Ability System (GAS), Gameplay Tags, and FastArray replication.

### Approach B: Mass Entity ECS Struct Fragments (`FMassFragment`)
Unreal Engine's modern Data-Oriented Technology Stack (Mass Framework / ECS), heavily utilized in The Matrix Awakens demo for high-density crowds and traffic.

- **Architecture:**
  - Entities are pure IDs (`FMassEntityHandle`).
  - Data is organized in contiguous memory chunks (Archetype Storage) as flat C++ structs inheriting from `FMassFragment`.
  - Processors (`UMassProcessor`) iterate over chunks of entities executing SIMD/cache-friendly batch logic.
- **Strengths:**
  - Extreme cache locality and memory density. Zero pointer chasing.
  - Capable of processing 50,000+ items or entities per frame at 120 FPS.
  - Structs are trivially serializable to raw binary buffers.
- **Weaknesses:**
  - Mass ECS is designed for homogeneous populations (thousands of simple bullets, crowd agents, or fish), NOT deep, heterogeneous, event-driven RPG character inventories.
  - Clunky Blueprint and UMG integration. Accessing Mass fragments from UMG widgets requires bridge subsystems and reflection overhead that negates performance benefits.
  - High engineering friction: requires custom Mass traits, processors, and entity archetype definitions.
  - `ASovereignBaseCharacter` is a traditional `ACharacter` pawn, not a Mass agent. Bridging AActor components to Mass fragments creates architectural fragmentation.

### Approach C: Sovereign DataAsset & Composable UObject/Struct Hybrid
A customized Sovereign Framework solution that marries the strengths of Lyra's fragment composition with the simplicity of lightweight value structs and direct `USovereignSaveableEntityComponent` JSON serialization.

- **Architecture:**
  - **Static Definition:** `USovereignItemDefinition` inherits from `UPrimaryDataAsset`. It contains an array of polymorphic `USovereignItemFragment` subobjects (or `FInstancedStruct` fragments) defined at edit-time.
  - **Runtime Instance:** `FSovereignInventoryItemInstance` is a lightweight plain C++ value `USTRUCT(BlueprintType)` (not a heap-allocated UObject). It stores:
    - `FGuid InstanceId`
    - `TSoftObjectPtr<USovereignItemDefinition> ItemDefinition`
    - `int32 Quantity`
    - `FGameplayTagContainer InstanceTags`
    - `TMap<FString, FString> CustomProperties` (durability, infused soul type, crafter signature)
  - **Component Container:** `USovereignInventoryComponent` lives on `ASovereignBaseCharacter`. It manages a contiguous `TArray<FSovereignInventoryItemInstance> Slots` along with capacity, weight limits, and item operation APIs (`AddItem`, `RemoveItem`, `ConsumeItem`, `EquipItem`).
  - **Persistence:** Connects to `USovereignSaveableEntityComponent` via a dedicated `USovereignInventoryBroker`, capturing item instances into structured JSON.

---

## 3. Comprehensive SWOT Analysis Across Approaches

| Criterion | Approach A: Lyra UObject Fragments | Approach B: Mass ECS Struct Fragments | Approach C: Sovereign DataAsset Hybrid (Recommended) |
| :--- | :--- | :--- | :--- |
| **Primary Philosophy** | Full Polymorphic UObject Subobjects | Data-Oriented Entity Component System | Pragmatic PrimaryDataAsset + Lightweight Struct Instance |
| **Memory Footprint** | Moderate (UObject overhead per fragment/instance) | Ultra-Minimal (Contiguous chunk memory) | Minimal (Static DataAssets in memory once; struct instances in array) |
| **Editor Usability** | Exceptional (Polymorphic dropdowns in Details panel) | Poor (Requires Mass traits and raw struct config) | Exceptional (Polymorphic fragment list on PrimaryDataAssets) |
| **UMG & UI Binding** | Native & Clean | Cumbersome (Requires bridge copying) | Native & Clean (`FSovereignInventoryItemInstance` BP exposed) |
| **Sovereign Save Parity**| Moderate (Requires subobject serialization) | Complex (Binary chunk desync from JSON SSoT) | Flawless (TArray of structs serializes directly to FJsonObject) |
| **Single-Player Simplicity**| Over-engineered (Network FastArray boilerplate) | Over-engineered (Mass processors & queries) | Optimized (Direct C++ invocation, zero replication waste) |

---

### Detailed SWOT Matrices

#### 1. Approach A: Lyra-Style `UObject` Fragment System

- **Strengths (S):**
  - **Battle-Tested Paradigm:** Epic Games built Lyra entirely on this pattern; thousands of developers understand it.
  - **Editor Polymorphism:** In the Unreal Editor, clicking "+" on an `Instanced` array lets designers pick any `UInventoryItemFragment` subclass and configure custom properties inline.
  - **Encapsulated Logic:** Behaviors like equipping meshes, applying stats, or generating tooltips live inside their own isolated classes.
  - **Blueprint Friendly:** Full native support for Blueprint derivation and event dispatching.
- **Weaknesses (W):**
  - **Replication Baggage:** Lyra couples its inventory with `FFastArraySerializer`, `FLyraInventoryList`, and GAS (`AbilitySystemComponent`). For a single-player game like WispsGame, this introduces hundreds of lines of dead boilerplate.
  - **Instance Allocation Overhead:** Allocating a full `UObject` for every individual runtime item instance (e.g. 50 arrows or 20 mushrooms in a bag) burdens the Unreal Garbage Collector (GC).
- **Opportunities (O):**
  - Can borrow Lyra's clean interface patterns (`FindFragmentByClass<T>()`) without copying its network synchronization code.
- **Threats (T):**
  - Potential cognitive overload and framework bloat if engineers try to blindly copy Lyra's GAS dependencies into WispsGame's lightweight C++ architecture.

#### 2. Approach B: Mass Entity ECS Struct Fragments (`FMassFragment`)

- **Strengths (S):**
  - **Maximum Raw Performance:** Contiguous cache-line memory layout eliminates cache misses during large-scale operations.
  - **Zero GC Pressure:** Mass structs are value types managed in chunk memory blocks; GC never touches them.
- **Weaknesses (W):**
  - **Severe Architectural Mismatch:** WispsGame character vessels (`ASovereignBaseCharacter`) are standard Unreal Pawns/Characters with `USkeletalMeshComponent`, `USovereignBioComponent`, and `USovereignQiComponent`. Mass ECS runs on a decoupled world entity manager.
  - **Heterogeneous Friction:** An RPG inventory has highly heterogeneous items with unique behaviors, meshes, tooltips, and dynamic stats. Mass thrives on homogeneous arrays (e.g. 10,000 identical crowds), not 20 distinct inventory slots with complex UI interactions.
  - **No Native UMG Inspection:** Feeding Mass fragments to UMG UI widgets requires manual bridging, polling subsystems, or copying data to UObjects, eliminating any performance gain.
- **Opportunities (O):**
  - Mass could theoretically be useful in the far future if WispsGame simulated thousands of loose items floating in a physical physics river, but not inside a character's pocket.
- **Threats (T):**
  - High risk of development stall: Mass documentation is notoriously sparse, experimental across engine revisions, and prone to breaking changes.

#### 3. Approach C: Sovereign DataAsset & Struct Hybrid (Recommended Sovereign Choice)

- **Strengths (S):**
  - **Best of Both Worlds:** Static archetype data uses polymorphic `USovereignItemFragment` subobjects on `USovereignItemDefinition` (PrimaryDataAsset), giving designers an intuitive, visual editor workflow. Runtime items are lightweight `FSovereignInventoryItemInstance` value structs stored in an array on `USovereignInventoryComponent`.
  - **Zero GC Clutter for Instances:** 100 inventory items are just an array of structs inside one component. No `UObject` instances created or destroyed during inventory transactions.
  - **Flawless Save System Alignment:** `USovereignSaveableEntityComponent` serializes structured JSON through `ISovereignBrokerInterface`. A struct array maps directly to `FJsonObject` and `FJsonValueObject` without complex subobject serialization acrobatics.
  - **Tight Domain Integration:** Directly executes logic against existing Sovereign components (`USovereignBioComponent::ConsumeFood`, `USovereignQiComponent::AddEnvironmentalBonus`, `USovereignLevelComponent::ConsumeItemXP`).
  - **Single-Player Focused:** 100% deterministic, immediate C++ calls without network tick delays or replication prediction.
- **Weaknesses (W):**
  - If multiplayer were introduced years from now, custom replication structs would need to be added. (Mitigated: WispsGame is strictly single-player).
- **Opportunities (O):**
  - Can automatically generate `USovereignItemDefinition` DataAssets from our existing `DT_SovereignSpeciesTemplate.csv` using Python scripts (`WispsCPPVR/Content/Python/`), mirroring Ticket `B-055` and `B-056`.
  - Allows Phase 2 dungeon extraction mechanics ("Overburdened" state) to query total inventory weight in a single cache-friendly pass.
- **Threats (T):**
  - Need to ensure fragment lookup (`FindFragment<T>()`) on DataAssets is efficient. (Mitigated: caching fragment pointers in an inline map or fast array query on asset load).

---

## 4. Proposed Sovereign Inventory Architecture Specification

### 4.1 Component Placement & Lifecycle
The inventory system lives strictly on character vessels (`ASovereignBaseCharacter`).

```text
ASovereignBaseCharacter (Pawn / Vessel)
 ├── USovereignSaveableEntityComponent (The Soul / Save SSoT)
 ├── USovereignBioComponent (Metabolism: Hunger, Hydration, Gestation)
 ├── USovereignAttributeComponent (Base D&D Stats: STR, DEX, CON...)
 ├── USovereignLevelComponent (XP Accumulators: StrengthXP... Item XP)
 ├── USovereignQiComponent (Spiritual Footprint: QiGive, QiTake, Aura)
 └── USovereignInventoryComponent (Vessel Pack: Slots, Weight, Fragments)
```

- When an `ASovereignPlayerWisp` possesses an antelope, humanoid, or dragon, the Wisp acts as the spiritual pilot. The vessel's physical inventory remains intact on that vessel.
- If the Wisp leaves the vessel (`EjectFromHost`), the vessel remains in the world with its inventory safely stored in its `USovereignInventoryComponent`.
- When the game saves, the vessel's `USovereignSaveableEntityComponent` calls `OnSave()` on registered brokers, including `USovereignInventoryBroker`, persisting all items to disk.

---

### 4.2 Core C++ Type Hierarchy

#### 1. The Fragment Base Class (`USovereignItemFragment`)
```cpp
// Base class for all immutable item archetype fragments
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType)
class WISPCPP7VR_API USovereignItemFragment : public UObject
{
    GENERATED_BODY()

public:
    // Called when the item is consumed by a character vessel
    virtual bool OnConsumed(ASovereignBaseCharacter* TargetCharacter, int32 Quantity) const { return false; }

    // Called when the item is equipped to a character vessel
    virtual bool OnEquipped(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const { return false; }

    // Called when the item is unequipped from a character vessel
    virtual bool OnUnequipped(ASovereignBaseCharacter* TargetCharacter, const FName& SlotName) const { return false; }
};
```

#### 2. The Item Definition Archetype (`USovereignItemDefinition`)
```cpp
// Master Primary DataAsset defining an item archetype
UCLASS(BlueprintType, Const)
class WISPCPP7VR_API USovereignItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Primary identification tag (e.g. Item.Consumable.SpiritMushroom)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Identity")
    FGameplayTag ItemTag;

    // Display Name
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Display")
    FText DisplayName;

    // Optional Species link if this item is an egg, seed, or biological vessel component
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sovereign|Species")
    TSoftObjectPtr<class USovereignSpeciesData> AssociatedSpeciesData;

    // Composable Fragments
    UPROPERTY(EditDefaultsOnly, Instanced, Category = "Sovereign|Fragments")
    TArray<TObjectPtr<USovereignItemFragment>> Fragments;

    // Fast templated helper to find a fragment by class
    template<typename T>
    const T* FindFragment() const
    {
        for (const TObjectPtr<USovereignItemFragment>& Fragment : Fragments)
        {
            if (const T* TypedFragment = Cast<T>(Fragment.Get()))
            {
                return TypedFragment;
            }
        }
        return nullptr;
    }

    // Blueprint accessible helper
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Fragments", meta = (DeterminesOutputType = "FragmentClass"))
    const USovereignItemFragment* GetFragmentByClass(TSubclassOf<USovereignItemFragment> FragmentClass) const;
};
```

#### 3. The Runtime Item Instance (`FSovereignInventoryItemInstance`)
```cpp
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignInventoryItemInstance
{
    GENERATED_BODY()

    // Unique runtime instance ID for tracking, trading, or persistent stacking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    FGuid InstanceId;

    // Soft reference to the immutable archetype definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    TSoftObjectPtr<USovereignItemDefinition> ItemDefinition;

    // Current quantity in this slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    int32 Quantity = 1;

    // Runtime state tags (e.g. Item.State.Cursed, Item.State.Infused)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    FGameplayTagContainer DynamicTags;

    // Mutable properties (e.g. Durability, Quality, Crafter)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    TMap<FString, FString> DynamicProperties;

    FSovereignInventoryItemInstance()
        : InstanceId(FGuid::NewGuid())
        , Quantity(1)
    {}
};
```

#### 4. The Inventory Component (`USovereignInventoryComponent`)
```cpp
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignInventoryComponent : public USovereignBaseComponent
{
    GENERATED_BODY()

public:
    USovereignInventoryComponent();

    // Maximum inventory slots (e.g., 20)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    int32 MaxSlotCount = 20;

    // Maximum carry weight before triggering "Overburdened" status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Inventory")
    float MaxCarryWeight = 100.0f;

    // Active item instances
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Inventory")
    TArray<FSovereignInventoryItemInstance> InventorySlots;

    // Core Actions
    UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
    bool AddItem(USovereignItemDefinition* ItemDef, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
    bool RemoveItemByInstanceId(const FGuid& InstanceId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
    bool ConsumeItem(const FGuid& InstanceId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Sovereign|Inventory")
    bool EquipItem(const FGuid& InstanceId, const FName& SocketName);

    // Queries
    UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
    float GetCurrentTotalWeight() const;

    UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
    bool IsOverburdened() const;

    UFUNCTION(BlueprintPure, Category = "Sovereign|Inventory")
    int32 GetItemCount(const FGameplayTag& ItemTag) const;
};
```

---

## 5. Detailed Fragment Domain Specifications

The beauty of the Fragment architecture is that each game subsystem can have its own dedicated fragment class without polluting other systems.

### 5.1 Visual & Equipment Fragment (`USovereignItemFragment_Equippable`)
Responsible for physical 3D representation and socket attachment on character vessels.
- **Properties:**
  - `TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset` (for armor, robes, wings).
  - `TSoftObjectPtr<UStaticMesh> StaticMeshAsset` (for held tools, crystals, swords).
  - `TSoftObjectPtr<UMaterialInterface> MaterialOverride`.
  - `FName PreferredSocketName` (e.g., `hand_r_socket`, `spine_03_socket`, `head_socket`).
  - `TSubclassOf<AActor> SpawnedActorClass` (for complex weapons with dynamic particle effects).
- **Execution:**
  - On `OnEquipped`, queries `ASovereignBaseCharacter::GetMesh()`, attaches mesh component to the requested socket using `FAttachmentTransformRules::SnapToTargetNotIncludingScale`, and applies material overrides.

### 5.2 Spiritual & Cultivation Fragment (`USovereignItemFragment_Spiritual`)
Responsible for Qi interactions, cultivation acceleration, and aura alteration.
- **Properties:**
  - `float QiGiveModifier`: Instant Qi granted on consumption or per-second while carried.
  - `float QiTakeCost`: Qi drained from entity to power the relic.
  - `float QiAuraRadiusBonus`: Expands the vessel's `USovereignQiComponent::QiAuraRadius`.
  - `ESovereignTrainingFocus SynergyFocus`: Item resonates when entity is in this stance (e.g., `ESovereignTrainingFocus::Wisdom` doubles Qi yield).
- **Execution:**
  - Interfaces directly with `USovereignQiComponent::AddQi()` and `USovereignQiComponent::ProcessEnvironmentalQiAura()`.

### 5.3 Consumable & Metabolic Fragment (`USovereignItemFragment_Consumable`)
Responsible for biological vitals replenishment and d20 attribute experience accumulation.
- **Properties:**
  - `float HungerRestore`: Adds to `USovereignBioComponent::CurrentHunger`.
  - `float HydrationRestore`: Adds to `USovereignBioComponent::CurrentHydration`.
  - `float FatigueRelief`: Reduces `USovereignBioComponent::CurrentFatigue`.
  - `float ToxicityIncrease`: Adds to `USovereignBioComponent::CurrentToxicity`.
  - **Attribute XP Gains:**
    - `double StrengthXP`, `double DexterityXP`, `double ConstitutionXP`, etc.
- **Execution:**
  - On `OnConsumed`, calls `USovereignBioComponent::ConsumeFood(HungerRestore, HydrationRestore)`.
  - Calls `USovereignLevelComponent::ConsumeItemXP(STR_XP, DEX_XP, ...)` to feed into integer attribute level-up rollovers (conforming to Ticket `B-059`).

### 5.4 UI & Description Fragment (`USovereignItemFragment_UIData`)
Responsible for UMG inventory grids, tooltips, and inspection cards.
- **Properties:**
  - `TSoftObjectPtr<UTexture2D> Icon`.
  - `FText ItemDescription`.
  - `FText FlavorLoreText`.
  - `FGameplayTag RarityTag` (e.g., `Item.Rarity.Common`, `Item.Rarity.Divine`).
  - `bool bShowInQuickBar`.
- **Execution:**
  - Provides clean, lightweight data binding for `WBP_Inventory` and inspection cards (`ISovereignUIInspectable`).

### 5.5 Weight & Encumbrance Fragment (`USovereignItemFragment_Weight`)
Responsible for mass calculation, stacking limits, and dungeon extraction penalties.
- **Properties:**
  - `float UnitWeight` (in kg / units).
  - `int32 MaxStackSize` (e.g., 99 for herbs, 1 for greatswords).
  - `bool bIgnoreEncumbrance`.
- **Execution:**
  - `USovereignInventoryComponent::GetCurrentTotalWeight()` sums `UnitWeight * Quantity` across all slots.
  - If `TotalWeight > MaxCarryWeight`, sets the `Character.Status.Overburdened` gameplay tag on the character vessel.
  - In Phase 2 dungeon extraction (GDD 18), an overburdened vessel suffers a 30% movement penalty and increased encounter danger, forcing tactical inventory triage.

---

## 6. Integration with Master Data Tables (`FSovereignItemRow` / `DT_SovereignSpeciesTemplate.csv`)

Our existing `DT_SovereignSpeciesTemplate.csv` defines 76 items and species rows, and `FSovereignItemRow` in `SovereignDataTypes.h` already defines:
- `ItemTag`, `DisplayName`, `Weight`
- `BaseStrength` ... `BaseLuck`
- `MetabolicRate`, `MaxHunger`, `MaxHydration`
- `QiGive`, `QiTake`, `QiAuraRadius`
- `StrengthXP` ... `LuckXP`

### Backwards Compatibility & Automated Asset Generation
We do not discard this existing data table. Instead, we use it as the upstream source of truth:
1. **Automated DataAsset Synthesis:** A Python script in `WispsCPPVR/Content/Python/` (extending the architecture proven in Ticket `B-055` and `B-056`) reads `DT_SovereignSpeciesTemplate.csv`.
2. **Fragment Composition on Import:**
   - If row has `Weight > 0`: Python script attaches `USovereignItemFragment_Weight`.
   - If row has `MaxHunger > 0` or any `*XP > 0`: Python attaches `USovereignItemFragment_Consumable` and populates the metabolic and XP fields.
   - If row has `QiGive > 0` or `QiTake > 0`: Python attaches `USovereignItemFragment_Spiritual`.
   - Python attaches `USovereignItemFragment_UIData` setting `DisplayName` and generating tooltip tags.
3. **Output:** Creates or updates `USovereignItemDefinition` Primary DataAssets in `/Game/Data/ItemAssets/`.
4. **Fallback Runtime Importer:** If an item is added by tag (`ItemTag`) and no explicit DataAsset exists, `USovereignInventoryComponent` can query `DT_SovereignSpeciesTemplate.csv` directly via `FSovereignItemRow`, populating fallback fragments on the fly.

---

## 7. Persistence Architecture: Single Source of Truth (`USovereignSaveableEntityComponent`)

In the Sovereign Framework, `USovereignSaveableEntityComponent` is the Single Source of Truth ("The Soul") for persistence. It delegates modular state capture to registered brokers via `ISovereignBrokerInterface`.

### 7.1 The Inventory Broker (`USovereignInventoryBroker`)
We implement `USovereignInventoryBroker`, which registers with `USovereignSaveableEntityComponent` on `BeginPlay`:

```text
Save Game Triggered
   ↓
USovereignSaveableEntityComponent::CaptureFullEntityState()
   ↓
Iterates RegisteredBrokers -> Calls USovereignInventoryBroker::OnSave()
   ↓
USovereignInventoryBroker queries USovereignInventoryComponent
   ↓
Serializes TArray<FSovereignInventoryItemInstance> into JSON object:
"Sovereign.Inventory": {
    "MaxSlotCount": 20,
    "Slots": [
        {
            "InstanceId": "A1B2C3D4-...",
            "ItemDef": "/Game/Data/ItemAssets/DA_SpiritMushroom.DA_SpiritMushroom",
            "Quantity": 5,
            "Tags": ["Item.Type.Consumable"],
            "DynamicProperties": {
                "InfusedQiElement": "Wood",
                "Freshness": "0.95"
            }
        }
    ]
}
   ↓
Written atomically to disk in world manifest save (.tmp -> atomic move)
```

### 7.2 Restoration Pipeline (`OnLoad`)
When the character vessel is streamed or loaded:
1. `USovereignSaveableEntityComponent::ApplyStateFromJsonObject` is called with the saved JSON.
2. `USovereignInventoryBroker::OnLoad` parses the `"Sovereign.Inventory"` field.
3. Reconstructs each `FSovereignInventoryItemInstance`, resolving soft object pointers to `USovereignItemDefinition`.
4. Re-equips active items, reinstating attached skeletal meshes and socket bindings seamlessly.

---

## 8. Strategic Roadmap & Next Steps

When Daniel is ready to advance this architecture into the active sprint, the following roadmap is recommended:

| Step | Milestone | Description |
| :---: | :--- | :--- |
| **1** | **C++ Foundation** | Create `USovereignItemFragment`, `USovereignItemDefinition`, `FSovereignInventoryItemInstance`, and `USovereignInventoryComponent` in `WispCPP7VR`. |
| **2** | **Fragment Domain Library** | Implement `USovereignItemFragment_UIData`, `USovereignItemFragment_Weight`, `USovereignItemFragment_Consumable`, `USovereignItemFragment_Spiritual`, and `USovereignItemFragment_Equippable`. |
| **3** | **Persistence Broker** | Implement `USovereignInventoryBroker` implementing `ISovereignBrokerInterface`, linking inventory serialization to `USovereignSaveableEntityComponent`. |
| **4** | **Python Importer Bridge** | Author automated script to convert `DT_SovereignSpeciesTemplate.csv` rows into populated `USovereignItemDefinition` DataAssets with auto-attached fragments. |
| **5** | **UI Inspection Widget** | Implement `WBP_Inventory` inspector widget binding to `USovereignInventoryComponent` via `ISovereignUIInspectable`. |

---

## Conclusion

The **Sovereign DataAsset + Composable Fragment Hybrid (Approach C)** is decisively the optimal architecture for WispsGame:
- It eliminates the replication baggage and GC allocation waste of Lyra's UObject instances.
- It avoids the extreme engineering friction and UI mismatch of Mass ECS.
- It provides designers with an intuitive, polymorphic Unreal Editor workflow on `USovereignItemDefinition` Primary DataAssets.
- It keeps runtime inventory instances as ultra-fast, lightweight value structs (`FSovereignInventoryItemInstance`).
- It guarantees 100% architectural harmony with character vessel possession, `USovereignBioComponent` metabolism, `USovereignLevelComponent` attribute leveling, and atomic JSON persistence through `USovereignSaveableEntityComponent`.
