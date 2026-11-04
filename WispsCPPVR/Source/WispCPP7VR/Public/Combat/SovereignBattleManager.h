// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignPartyComponent.h"
#include "Combat/SovereignSkillAugment.h"
#include "Combat/SovereignCombatCalculationLibrary.h"
#include "SovereignBattleManager.generated.h"

class ASovereignBaseCharacter;

/**
 * FSovereignReadyEntry
 * Container for a ready combatant with deterministic tie-breaking.
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignReadyEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Battle")
	TWeakObjectPtr<ASovereignBaseCharacter> Combatant = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Battle")
	int32 Dexterity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Battle")
	int32 TieBreakerNonce = 0;
};

/**
 * EBattleState
 * Encapsulates the overall lifecycle states of an active combat encounter.
 */
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	EncounterStarting UMETA(DisplayName = "Encounter Starting"),
	ActiveCombat      UMETA(DisplayName = "Active Combat"),
	ActionExecuting   UMETA(DisplayName = "Action Executing"),
	Victory           UMETA(DisplayName = "Victory"),
	Defeat            UMETA(DisplayName = "Defeat"),
	Fled              UMETA(DisplayName = "Fled")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleStateChanged, EBattleState, OldState, EBattleState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatantTurnReady, ASovereignBaseCharacter*, Combatant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnded, EBattleState, Outcome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSkillActionExecuted, ASovereignBaseCharacter*, Caster, const FGameplayTag&, SkillTag, float, Value, bool, bIsHealing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemActionExecuted, ASovereignBaseCharacter*, User, const FGameplayTag&, ItemTag, ASovereignBaseCharacter*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFleeAttemptFailed, ASovereignBaseCharacter*, Fleer, float, CurrentEscapeProgress);

/**
 * ASovereignBattleManager
 * Orchestrator actor placed or spawned in Dedicated 3D Battle Arenas.
 * Manages dual opposing 3x3 tactical grid matrices (Player Party vs Enemy Encounter),
 * monitors ATB gauges across all combatants, arbitrates turn queues by Dexterity,
 * and handles encounter win/loss/flee state progression.
 */
UCLASS(BlueprintType, Blueprintable)
class WISPCPP7VR_API ASovereignBattleManager : public AActor
{
	GENERATED_BODY()

public:
	ASovereignBattleManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Dual 3x3 Grid Formations ---
	static constexpr int32 GRID_SIZE = 9;

	/** Player party combatants mapped across the 9 slots (0-2: Front, 3-5: Mid, 6-8: Rear) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|Formations")
	TArray<TObjectPtr<ASovereignBaseCharacter>> PlayerGrid;

	/** Opposing enemy combatants mapped across the 9 slots (0-2: Front, 3-5: Mid, 6-8: Rear) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|Formations")
	TArray<TObjectPtr<ASovereignBaseCharacter>> EnemyGrid;

	// --- 3D Spatial Grid Anchors on Arena Platform ---

	/** World transforms for Player party 3x3 slots (0-8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Battle|Spatial")
	TArray<FTransform> PlayerSlotTransforms;

	/** World transforms for Opposing enemy 3x3 slots (0-8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Battle|Spatial")
	TArray<FTransform> EnemySlotTransforms;

	/** Transform for cinematic action camera framing the battle arena */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Battle|Spatial")
	FTransform ArenaCameraTransform;

	/** Current operational encounter state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|State")
	EBattleState BattleState = EBattleState::EncounterStarting;

	/** Queue of combatants whose ATB meters are currently ready to act */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|Queue")
	TArray<FSovereignReadyEntry> ReadyQueue;

	/** Currently acting combatant (if in ActionExecuting state) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|Queue")
	TObjectPtr<ASovereignBaseCharacter> ActiveCombatant;

	/** Whether escape/fleeing is allowed in this battle (false for Story Bosses / Vault Guardians) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Battle|Config")
	bool bIsEscapable = true;

	/** Accumulated escape bar progress towards guaranteed flight (0.0 to 100.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Battle|Escape")
	float CurrentEscapeProgress = 0.0f;

	/** Percentage added to CurrentEscapeProgress each time an escape attempt fails (Default 25.0%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Battle|Escape")
	float EscapeProgressIncrementPerFail = 25.0f;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnBattleStateChanged OnBattleStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnCombatantTurnReady OnCombatantTurnReady;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnBattleEnded OnBattleEnded;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnSkillActionExecuted OnSkillActionExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnItemActionExecuted OnItemActionExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Battle|Events")
	FOnFleeAttemptFailed OnFleeAttemptFailed;

	// --- Flee API ---

	/**
	 * Calculates the percentage chance (0.0 to 100.0) for a combatant to flee successfully.
	 * Formula: 50% + (PartyAverageDEX - EnemyAverageDEX) * 2% + FleerLuck * 0.5% + CurrentEscapeProgress.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	float CalculateEscapeChance(const ASovereignBaseCharacter* Fleer) const;

	/**
	 * Executes an escape attempt for the acting combatant.
	 * If battle is unescapable, rejects immediately without ending turn.
	 * If roll succeeds, sets state to Fled.
	 * If roll fails, increments escape progress by EscapeProgressIncrementPerFail and ends turn.
	 * @param Fleer The acting character attempting to run.
	 * @param ForcedRoll Optional deterministic roll between 0.0 and 100.0 for unit testing (-1.0 uses random).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	bool AttemptFlee(ASovereignBaseCharacter* Fleer, float ForcedRoll = -1.0f);

	// --- Skill & Item Execution API ---

	/**
	 * Executes an inventory item action against a designated target during battle.
	 * Consumes 1 quantity from User's USovereignInventoryComponent, applies fragment effects,
	 * and concludes the active turn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	bool ExecuteItemAction(ASovereignBaseCharacter* User, const FGuid& ItemInstanceId, ASovereignBaseCharacter* Target);

	/**
	 * Checks if a caster has sufficient Qi to cast the given skill definition.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	bool CanCastSkill(const ASovereignBaseCharacter* Caster, const USovereignSkillDefinition* SkillDef) const;

	/**
	 * Executes a combat skill instance against one or more targets.
	 * Deducts Qi from Caster, computes percentage-mitigated damage or Wisdom healing,
	 * applies changes to target attributes, checks Defend mitigation, and concludes turn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	bool ExecuteSkillAction(ASovereignBaseCharacter* Caster, const FSovereignSkillInstance& SkillInstance, const TArray<ASovereignBaseCharacter*>& Targets);

	/**
	 * Creates a combat snapshot from an active character vessel and their grid slot.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	FSovereignCombatEntitySnapshot CreateSnapshotForCombatant(const ASovereignBaseCharacter* Combatant, int32 SlotIndex) const;

	// --- Encounter API ---

	/**
	 * Initializes the encounter with deployed player party and opposing enemy units.
	 */
	/** Generates default 200u grid offsets if custom transforms were not placed by designer */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle|Spatial")
	void GenerateDefaultGridTransforms();

	/** Positions all assigned combatants onto their physical 3D arena transforms */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle|Spatial")
	void PositionCombatantsOnGrid();

	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void InitializeEncounter(const TArray<ASovereignBaseCharacter*>& InPlayerUnits, const TArray<ASovereignBaseCharacter*>& InEnemyUnits, bool bCanFlee = true);

	/**
	 * Advances the battle state machine.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void SetBattleState(EBattleState NewState);

	/** Delegate hook called automatically when any combatant's ATB hits 100% */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void OnCombatantATBReady(ASovereignBaseCharacter* Combatant);

	/** Registers a combatant whose ATB has reached 100%. */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void EnqueueReadyCombatant(ASovereignBaseCharacter* Combatant);

	/**
	 * Pops and returns the next combatant ready to take an action.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	ASovereignBaseCharacter* DequeueNextCombatant();

	/**
	 * Concludes the current action execution, checking for victory or defeat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void ConcludeActiveTurn();

	/**
	 * Checks living status of both teams and triggers Victory or Defeat if applicable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	bool CheckCombatOutcome();

	/**
	 * Triggers a battle conclusion due to fleeing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void ConcludeFled();

	/** Returns all living members in the Player grid */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	TArray<ASovereignBaseCharacter*> GetLivingPlayerUnits() const;

	/** Returns all living members in the Enemy grid */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	TArray<ASovereignBaseCharacter*> GetLivingEnemyUnits() const;

	/** Checks if an actor is currently alive (Health > 0) */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Battle")
	static bool IsCombatantAlive(const ASovereignBaseCharacter* Combatant);

	/** Freezes or unfreezes all combatants' ATBs during animations / wait mode */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Battle")
	void SetAllCombatantsWaitMode(bool bWait);
};
