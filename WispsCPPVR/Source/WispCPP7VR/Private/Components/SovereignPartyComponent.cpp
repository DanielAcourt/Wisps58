// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Components/SovereignPartyComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"

USovereignPartyComponent::USovereignPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize Ranks: Rank 0 (Front) is unlocked by default; Rank 1 (Mid) & Rank 2 (Rear) start locked
	UnlockedRanks.SetNum(TOTAL_RANKS);
	UnlockedRanks[0] = true;  // Front Rank
	UnlockedRanks[1] = false; // Mid Rank (Milestone 1)
	UnlockedRanks[2] = false; // Rear Rank (Milestone 2)

	// Initialize 3x3 Grid (9 slots)
	GridSlots.SetNum(TOTAL_GRID_SLOTS);
	for (int32 i = 0; i < TOTAL_GRID_SLOTS; ++i)
	{
		GridSlots[i].SlotIndex = i;
		GridSlots[i].Clear();
	}
}

bool USovereignPartyComponent::AssignPartyMember(int32 SlotIndex, ASovereignBaseCharacter* Character, bool bAsLeader)
{
	if (SlotIndex < 0 || SlotIndex >= TOTAL_GRID_SLOTS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignPartyComponent] SlotIndex %d out of bounds (0-%d)."), SlotIndex, TOTAL_GRID_SLOTS - 1);
		return false;
	}

	if (!IsSlotUnlocked(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[USovereignPartyComponent] Cannot assign to SlotIndex %d: Rank %d is locked!"), SlotIndex, GetRankForSlot(SlotIndex));
		return false;
	}

	// If character is null, treat as clearing the slot
	if (!Character)
	{
		return RemovePartyMember(SlotIndex);
	}

	// If character is already in another slot, remove them from the old slot first
	for (int32 i = 0; i < TOTAL_GRID_SLOTS; ++i)
	{
		if (GridSlots[i].MemberCharacter.Get() == Character)
		{
			GridSlots[i].Clear();
			OnPartyMemberChanged.Broadcast(i, nullptr);
		}
	}

	// If marked as leader, clear any other leader flag
	if (bAsLeader)
	{
		for (int32 i = 0; i < TOTAL_GRID_SLOTS; ++i)
		{
			GridSlots[i].bIsLeader = false;
		}
	}

	FSovereignPartySlot& Slot = GridSlots[SlotIndex];
	Slot.SlotIndex = SlotIndex;
	Slot.MemberCharacter = Character;
	Slot.bIsLeader = bAsLeader;

	// Assign unique MemberId
	Slot.MemberId = FGuid::NewGuid();

	OnPartyMemberChanged.Broadcast(SlotIndex, Character);
	return true;
}

bool USovereignPartyComponent::RemovePartyMember(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= TOTAL_GRID_SLOTS)
	{
		return false;
	}

	if (GridSlots[SlotIndex].IsOccupied())
	{
		GridSlots[SlotIndex].Clear();
		OnPartyMemberChanged.Broadcast(SlotIndex, nullptr);
		return true;
	}

	return false;
}

ASovereignBaseCharacter* USovereignPartyComponent::GetPartyMember(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < TOTAL_GRID_SLOTS)
	{
		return GridSlots[SlotIndex].MemberCharacter.Get();
	}
	return nullptr;
}

TArray<ASovereignBaseCharacter*> USovereignPartyComponent::GetAllActivePartyMembers() const
{
	TArray<ASovereignBaseCharacter*> ActiveMembers;
	for (const FSovereignPartySlot& Slot : GridSlots)
	{
		if (Slot.MemberCharacter.IsValid())
		{
			ActiveMembers.Add(Slot.MemberCharacter.Get());
		}
	}
	return ActiveMembers;
}

int32 USovereignPartyComponent::GetActiveMemberCount() const
{
	int32 Count = 0;
	for (const FSovereignPartySlot& Slot : GridSlots)
	{
		if (Slot.MemberCharacter.IsValid())
		{
			Count++;
		}
	}
	return Count;
}

bool USovereignPartyComponent::IsRankUnlocked(int32 RankIndex) const
{
	if (RankIndex >= 0 && RankIndex < UnlockedRanks.Num())
	{
		return UnlockedRanks[RankIndex];
	}
	return false;
}

bool USovereignPartyComponent::IsSlotUnlocked(int32 SlotIndex) const
{
	const int32 Rank = GetRankForSlot(SlotIndex);
	return IsRankUnlocked(Rank);
}

bool USovereignPartyComponent::UnlockRank(int32 RankIndex)
{
	if (RankIndex >= 0 && RankIndex < UnlockedRanks.Num())
	{
		if (!UnlockedRanks[RankIndex])
		{
			UnlockedRanks[RankIndex] = true;
			OnPartyRankUnlocked.Broadcast(RankIndex);
			return true;
		}
	}
	return false;
}

// --- Persistence ---

void USovereignPartyComponent::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
	if (!OutJson.IsValid()) return;

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

	// Serialize Unlocked Ranks
	TArray<TSharedPtr<FJsonValue>> RanksArray;
	for (bool bUnlocked : UnlockedRanks)
	{
		RanksArray.Add(MakeShared<FJsonValueBoolean>(bUnlocked));
	}
	RootObject->SetArrayField(TEXT("UnlockedRanks"), RanksArray);

	// Serialize Grid Slots
	TArray<TSharedPtr<FJsonValue>> SlotsArray;
	for (const FSovereignPartySlot& Slot : GridSlots)
	{
		if (Slot.IsOccupied())
		{
			TSharedPtr<FJsonObject> SlotObj = MakeShared<FJsonObject>();
			SlotObj->SetNumberField(TEXT("SlotIndex"), Slot.SlotIndex);
			SlotObj->SetStringField(TEXT("MemberId"), Slot.MemberId.ToString());
			SlotObj->SetBoolField(TEXT("bIsLeader"), Slot.bIsLeader);

			if (Slot.MemberCharacter.IsValid())
			{
				SlotObj->SetStringField(TEXT("CharacterName"), Slot.MemberCharacter->GetName());
			}

			SlotsArray.Add(MakeShared<FJsonValueObject>(SlotObj));
		}
	}
	RootObject->SetArrayField(TEXT("GridSlots"), SlotsArray);

	OutJson->SetObjectField(TEXT("Sovereign.Party"), RootObject);
}

void USovereignPartyComponent::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
	if (!InJson.IsValid() || !InJson->HasField(TEXT("Sovereign.Party"))) return;

	TSharedPtr<FJsonObject> RootObject = InJson->GetObjectField(TEXT("Sovereign.Party"));
	if (!RootObject.IsValid()) return;

	// Restore Unlocked Ranks
	const TArray<TSharedPtr<FJsonValue>>* RanksArray;
	if (RootObject->TryGetArrayField(TEXT("UnlockedRanks"), RanksArray))
	{
		for (int32 i = 0; i < RanksArray->Num() && i < UnlockedRanks.Num(); ++i)
		{
			UnlockedRanks[i] = (*RanksArray)[i]->AsBool();
		}
	}

	// Restore Slots
	const TArray<TSharedPtr<FJsonValue>>* SlotsArray;
	if (RootObject->TryGetArrayField(TEXT("GridSlots"), SlotsArray))
	{
		// Reset slots first
		for (int32 i = 0; i < TOTAL_GRID_SLOTS; ++i)
		{
			GridSlots[i].Clear();
		}

		for (const TSharedPtr<FJsonValue>& SlotVal : *SlotsArray)
		{
			TSharedPtr<FJsonObject> SlotObj = SlotVal->AsObject();
			if (!SlotObj.IsValid()) continue;

			int32 SlotIdx = SlotObj->GetIntegerField(TEXT("SlotIndex"));
			if (SlotIdx >= 0 && SlotIdx < TOTAL_GRID_SLOTS)
			{
				FSovereignPartySlot& Slot = GridSlots[SlotIdx];
				Slot.SlotIndex = SlotIdx;
				FGuid::Parse(SlotObj->GetStringField(TEXT("MemberId")), Slot.MemberId);
				Slot.bIsLeader = SlotObj->GetBoolField(TEXT("bIsLeader"));
			}
		}
	}
}

void USovereignPartyComponent::OnProcessData(const TMap<FString, FString>& Data)
{
	// External telemetry injection hook if needed
}
