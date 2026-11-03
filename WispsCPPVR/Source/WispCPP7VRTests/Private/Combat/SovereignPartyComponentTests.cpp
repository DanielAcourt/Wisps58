// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Components/SovereignPartyComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Entities/SovereignPlayerWisp.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN PARTY COMPONENT TESTS - B-077 Verification
 * ============================================================================
 * Verifies 3x3 tactical grid formation, milestone rank unlock progression,
 * boundary/locked rank rejection, party removal, leader assignment, and JSON persistence.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignPartyComponentTest,
	"Sovereign.Combat.PartyComponentOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignPartyComponentTest::RunTest(const FString& Parameters)
{
	USovereignPartyComponent* PartyComp = NewObject<USovereignPartyComponent>();
	if (!PartyComp)
	{
		AddError(TEXT("Failed to instantiate USovereignPartyComponent"));
		return false;
	}

	// 1. Initial State Verification: Rank 0 (Front) is unlocked, Ranks 1 (Mid) and 2 (Rear) locked
	TestTrue(TEXT("Rank 0 (Front) should be unlocked by default"), PartyComp->IsRankUnlocked(0));
	TestFalse(TEXT("Rank 1 (Mid) should be locked initially"), PartyComp->IsRankUnlocked(1));
	TestFalse(TEXT("Rank 2 (Rear) should be locked initially"), PartyComp->IsRankUnlocked(2));

	TestTrue(TEXT("Slot 0 (Front) is unlocked"), PartyComp->IsSlotUnlocked(0));
	TestTrue(TEXT("Slot 2 (Front) is unlocked"), PartyComp->IsSlotUnlocked(2));
	TestFalse(TEXT("Slot 3 (Mid) is locked"), PartyComp->IsSlotUnlocked(3));
	TestFalse(TEXT("Slot 8 (Rear) is locked"), PartyComp->IsSlotUnlocked(8));

	// Rank for Slot calculations
	TestEqual(TEXT("Slot 0 rank is 0"), USovereignPartyComponent::GetRankForSlot(0), 0);
	TestEqual(TEXT("Slot 2 rank is 0"), USovereignPartyComponent::GetRankForSlot(2), 0);
	TestEqual(TEXT("Slot 4 rank is 1"), USovereignPartyComponent::GetRankForSlot(4), 1);
	TestEqual(TEXT("Slot 8 rank is 2"), USovereignPartyComponent::GetRankForSlot(8), 2);
	TestEqual(TEXT("Slot 9 (out of bounds) rank is INDEX_NONE"), USovereignPartyComponent::GetRankForSlot(9), INDEX_NONE);

	// 2. Member Assignment & Boundary Checks
	ASovereignBaseCharacter* DummyLeader = NewObject<ASovereignBaseCharacter>();
	ASovereignBaseCharacter* DummyAlly = NewObject<ASovereignBaseCharacter>();

	// Placing in locked rank should fail
	const bool bMidAssignFailed = PartyComp->AssignPartyMember(4, DummyAlly);
	TestFalse(TEXT("Assignment to Slot 4 (Mid Rank) must fail while locked"), bMidAssignFailed);
	TestEqual(TEXT("Active member count should remain 0"), PartyComp->GetActiveMemberCount(), 0);

	// Placing in unlocked front rank succeeds
	const bool bLeaderAssigned = PartyComp->AssignPartyMember(1, DummyLeader, true);
	TestTrue(TEXT("Assignment of leader to Slot 1 (Front Rank) should succeed"), bLeaderAssigned);
	TestEqual(TEXT("Active member count is 1"), PartyComp->GetActiveMemberCount(), 1);
	TestEqual(TEXT("Slot 1 occupant is DummyLeader"), PartyComp->GetPartyMember(1), DummyLeader);
	TestTrue(TEXT("Slot 1 is marked as leader"), PartyComp->GridSlots[1].bIsLeader);

	// Moving same character to another slot automatically clears old slot
	const bool bLeaderMoved = PartyComp->AssignPartyMember(0, DummyLeader, true);
	TestTrue(TEXT("Moving leader to Slot 0 should succeed"), bLeaderMoved);
	TestNull(TEXT("Slot 1 should now be empty"), PartyComp->GetPartyMember(1));
	TestEqual(TEXT("Slot 0 occupant is DummyLeader"), PartyComp->GetPartyMember(0), DummyLeader);
	TestEqual(TEXT("Active member count remains 1"), PartyComp->GetActiveMemberCount(), 1);

	// 3. Milestone Rank Unlocking
	const bool bMidUnlocked = PartyComp->UnlockRank(1);
	TestTrue(TEXT("UnlockRank(1) should return true"), bMidUnlocked);
	TestTrue(TEXT("Rank 1 is now unlocked"), PartyComp->IsRankUnlocked(1));
	TestTrue(TEXT("Slot 4 is now unlocked"), PartyComp->IsSlotUnlocked(4));

	// Re-unlocking should return false
	TestFalse(TEXT("UnlockRank(1) again returns false"), PartyComp->UnlockRank(1));

	// Now assignment to Slot 4 succeeds
	const bool bAllyAssigned = PartyComp->AssignPartyMember(4, DummyAlly, false);
	TestTrue(TEXT("Assignment to Slot 4 should succeed now that Rank 1 is unlocked"), bAllyAssigned);
	TestEqual(TEXT("Active member count is 2"), PartyComp->GetActiveMemberCount(), 2);
	TestEqual(TEXT("Slot 4 occupant is DummyAlly"), PartyComp->GetPartyMember(4), DummyAlly);

	// 4. Removal
	const bool bAllyRemoved = PartyComp->RemovePartyMember(4);
	TestTrue(TEXT("RemovePartyMember(4) should return true"), bAllyRemoved);
	TestNull(TEXT("Slot 4 is now cleared"), PartyComp->GetPartyMember(4));
	TestEqual(TEXT("Active member count is back to 1"), PartyComp->GetActiveMemberCount(), 1);

	// 5. Persistence Broker Serialization / Deserialization
	TestEqual(TEXT("Broker category must be Sovereign.Party"), PartyComp->GetBrokerCategory(), TEXT("Sovereign.Party"));

	const FString SerializedJson = PartyComp->SerializeBrokerState();
	TestFalse(TEXT("Serialized JSON should not be empty"), SerializedJson.IsEmpty());
	TestTrue(TEXT("Serialized JSON should contain UnlockedRanks"), SerializedJson.Contains(TEXT("UnlockedRanks")));
	TestTrue(TEXT("Serialized JSON should contain GridSlots"), SerializedJson.Contains(TEXT("GridSlots")));

	// Create fresh component and restore state
	USovereignPartyComponent* RestoredComp = NewObject<USovereignPartyComponent>();
	TestFalse(TEXT("RestoredComp Rank 1 starts locked"), RestoredComp->IsRankUnlocked(1));

	const bool bDeserialized = RestoredComp->DeserializeBrokerState(SerializedJson);
	TestTrue(TEXT("DeserializeBrokerState should succeed"), bDeserialized);
	TestTrue(TEXT("RestoredComp Rank 1 should now be unlocked"), RestoredComp->IsRankUnlocked(1));
	TestFalse(TEXT("RestoredComp Rank 2 should still be locked"), RestoredComp->IsRankUnlocked(2));
	TestTrue(TEXT("RestoredComp Slot 0 occupied with Leader"), RestoredComp->GridSlots[0].bIsLeader);
	TestTrue(TEXT("RestoredComp Slot 0 has valid MemberId"), RestoredComp->GridSlots[0].MemberId.IsValid());

	return true;
}
