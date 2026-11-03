// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#pragma once

#include "CoreMinimal.h"
#include "Components/SovereignBaseComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "SovereignPartyComponent.generated.h"

class ASovereignBaseCharacter;
class FJsonObject;

/**
 * FSovereignPartySlot
 * Represents an individual deployment slot within the 3x3 tactical battle matrix.
 * Slots 0-2 = Front Rank
 * Slots 3-5 = Mid Rank
 * Slots 6-8 = Rear Rank
 */
USTRUCT(BlueprintType)
struct WISPCPP7VR_API FSovereignPartySlot
{
	GENERATED_BODY()

	/** Slot index within the 3x3 grid (0 to 8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Party")
	int32 SlotIndex = INDEX_NONE;

	/** Unique entity identifier / GUID of the character occupying this slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Party", meta = (IgnoreForMemberInitializationTest))
	FGuid MemberId;

	/** Pointer to the active character vessel (valid when spawned/loaded in world) */
	UPROPERTY(BlueprintReadOnly, Category = "Sovereign|Party")
	TWeakObjectPtr<ASovereignBaseCharacter> MemberCharacter = nullptr;

	/** Whether this slot contains the primary possessed dragon leader */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sovereign|Party")
	bool bIsLeader = false;

	/** Whether this slot is actively occupied */
	bool IsOccupied() const { return MemberId.IsValid() || MemberCharacter.IsValid(); }

	void Clear()
	{
		MemberId.Invalidate();
		MemberCharacter = nullptr;
		bIsLeader = false;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyMemberChanged, int32, SlotIndex, ASovereignBaseCharacter*, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyRankUnlocked, int32, RankIndex);

/**
 * USovereignPartyComponent
 * Manages the player party roster, 3x3 tactical formation matrix deployment (Front, Mid, Rear ranks),
 * milestone rank unlocks, and state persistence into "Sovereign.Party" save data.
 */
UCLASS(ClassGroup = (Sovereign), meta = (BlueprintSpawnableComponent))
class WISPCPP7VR_API USovereignPartyComponent : public USovereignBaseComponent, public ISovereignBrokerInterface
{
	GENERATED_BODY()

public:
	USovereignPartyComponent();

	// --- 3x3 Grid Matrix Configuration ---
	static constexpr int32 TOTAL_GRID_SLOTS = 9;
	static constexpr int32 SLOTS_PER_RANK = 3;
	static constexpr int32 TOTAL_RANKS = 3;

	/**
	 * Bitmask or array tracking which ranks are unlocked:
	 * Rank 1 (Index 0): Front Rank (Slots 0, 1, 2) - Unlocked by default
	 * Rank 2 (Index 1): Mid Rank (Slots 3, 4, 5) - Milestone 1
	 * Rank 3 (Index 2): Rear Rank (Slots 6, 7, 8) - Milestone 2
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Party")
	TArray<bool> UnlockedRanks;

	/** 3x3 Deployment Grid Slots (Size 9) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sovereign|Party")
	TArray<FSovereignPartySlot> GridSlots;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Party|Events")
	FOnPartyMemberChanged OnPartyMemberChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sovereign|Party|Events")
	FOnPartyRankUnlocked OnPartyRankUnlocked;

	// --- Party Management API ---

	/**
	 * Assigns a character vessel to a specific 3x3 grid slot (0 to 8).
	 * Fails if the rank containing the slot is locked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Party")
	bool AssignPartyMember(int32 SlotIndex, ASovereignBaseCharacter* Character, bool bAsLeader = false);

	/**
	 * Removes any member occupying the given slot index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Party")
	bool RemovePartyMember(int32 SlotIndex);

	/**
	 * Retrieves the character occupying a specific slot index, if valid and alive.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	ASovereignBaseCharacter* GetPartyMember(int32 SlotIndex) const;

	/**
	 * Returns all active characters currently deployed in the 3x3 grid.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	TArray<ASovereignBaseCharacter*> GetAllActivePartyMembers() const;

	/**
	 * Returns the count of deployed members in the party.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	int32 GetActiveMemberCount() const;

	/**
	 * Calculates which rank (0 = Front, 1 = Mid, 2 = Rear) a slot index belongs to.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	static int32 GetRankForSlot(int32 SlotIndex)
	{
		return (SlotIndex >= 0 && SlotIndex < TOTAL_GRID_SLOTS) ? (SlotIndex / SLOTS_PER_RANK) : INDEX_NONE;
	}

	/**
	 * Checks if a specific rank (0 = Front, 1 = Mid, 2 = Rear) is unlocked.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	bool IsRankUnlocked(int32 RankIndex) const;

	/**
	 * Checks if a specific slot index (0 to 8) is unlocked.
	 */
	UFUNCTION(BlueprintPure, Category = "Sovereign|Party")
	bool IsSlotUnlocked(int32 SlotIndex) const;

	/**
	 * Unlocks a milestone rank (0 = Front, 1 = Mid, 2 = Rear).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|Party")
	bool UnlockRank(int32 RankIndex);

	// --- ISovereignBrokerInterface Implementation ---
	virtual FString GetBrokerCategory() const override;
	virtual FString SerializeBrokerState() const override;
	virtual bool DeserializeBrokerState(const FString& InJsonState) override;
};
