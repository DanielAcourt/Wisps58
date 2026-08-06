// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.10. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-06

#include "Entities/SovereignSaveableEntityComponent.h"
#include "Entities/SovereignBrokerInterface.h"
#include "Entities/SovereignDiagnosticBroker.h"
#include "Entities/SovereignCultivationBroker.h"
#include "Subsystems/SovereignBridgeSubsystem.h"
#include "SaveSystem/SovereignActorRegistry.h"
#include "Interaction/SovereignSaveInterface.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/World.h"

USovereignSaveableEntityComponent::USovereignSaveableEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UIUpdateThrottleInterval = 0.05f;
}

void USovereignSaveableEntityComponent::InvalidateStateCache()
{
	LastCachedFrame = 0;
	LastCachedTime = -1.0;
	CachedFullState.Reset();
	CachedCategoryStrings.Empty();
}

void USovereignSaveableEntityComponent::OnRegister()
{
	Super::OnRegister();

	// Instantiate the brokers dynamically at runtime and register them to our RegisteredBrokers list
	if (!DiagnosticBroker)
	{
		DiagnosticBroker = NewObject<UDiagnosticBroker>(this);
	}
	RegisterBroker(DiagnosticBroker);

	if (!CultivationBroker)
	{
		CultivationBroker = NewObject<UCultivationBroker>(this);
	}
	RegisterBroker(CultivationBroker);
}

void USovereignSaveableEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!EntityID.IsValid())
	{
		EntityID = FGuid::NewGuid();
	}

	if (UWorld* World = GetWorld())
	{
		// 1. Register with the Global Actor Registry
		if (UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>())
		{
			Registry->RegisterActor(EntityID, GetOwner());
		}

		// 2. Register with the Bridge Subsystem for simulation tracking
		if (USovereignBridgeSubsystem* Bridge = World->GetSubsystem<USovereignBridgeSubsystem>())
		{
			Bridge->RegisterEntity(this);
		}
	}

	if (!BirthTimestamp.GetTicks())
	{
		BirthTimestamp = FDateTime::Now();
	}
}

void USovereignSaveableEntityComponent::InitializeSoul()
{
	// 1. IDENTITY & BIRTHRIGHT
	if (!EntityID.IsValid()) { EntityID = FGuid::NewGuid(); }

	if (BirthTimestamp.GetTicks() == 0)
	{
		if (bUseManualBirthDate)
		{
			if (!FDateTime::Parse(ManualBirthDateStr, BirthTimestamp))
			{
				BirthTimestamp = FDateTime::Now();
				UE_LOG(LogTemp, Error, TEXT("Sovereign: Failed to parse ManualBirthDateStr! Defaulting to Now."));
			}
		}
		else
		{
			BirthTimestamp = FDateTime::Now();
		}
	}

	// 2. 07 CHECK-IN TELEMETRY
	if (UWorld* World = GetWorld())
	{
		if (USovereignBridgeSubsystem* Bridge = World->GetSubsystem<USovereignBridgeSubsystem>())
		{
			TSharedPtr<FJsonObject> State = CaptureFullEntityState();
			State->SetStringField(TEXT("Event"), TEXT("07_CheckIn_Initialize"));

			FString JsonString = SerializeJsonToString(State);
			Bridge->PushBlackBoxTelemetry(EntityID, 1.0f, JsonString);
		}
	}
}

void USovereignSaveableEntityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>())
		{
			Registry->UnregisterActor(EntityID);
		}

		if (USovereignBridgeSubsystem* Bridge = World->GetSubsystem<USovereignBridgeSubsystem>())
		{
			Bridge->UnregisterEntity(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void USovereignSaveableEntityComponent::RegisterBroker(TScriptInterface<ISovereignBrokerInterface> Broker)
{
	if (Broker.GetInterface() && !RegisteredBrokers.Contains(Broker))
	{
		RegisteredBrokers.Add(Broker);
		UE_LOG(LogTemp, Log, TEXT("Sovereign: Registered Module [%s] to Soul Hub."), *Broker.GetObject()->GetName());

		// Notify listeners that the architecture has changed and invalidate state cache
		InvalidateStateCache();
		OnStateChanged.Broadcast(this);
	}
}

void USovereignSaveableEntityComponent::UnregisterBroker(TScriptInterface<ISovereignBrokerInterface> Broker)
{
	if (RegisteredBrokers.Contains(Broker))
	{
		RegisteredBrokers.Remove(Broker);
		InvalidateStateCache();
		OnStateChanged.Broadcast(this);
	}
}

TSharedPtr<FJsonObject> USovereignSaveableEntityComponent::CaptureFullEntityState()
{
	TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());

	// 1. IDENTITY CATEGORY
	TSharedPtr<FJsonObject> IdentityObj = MakeShareable(new FJsonObject());
	IdentityObj->SetStringField(TEXT("GUID"), EntityID.ToString());
	IdentityObj->SetStringField(TEXT("BirthTimestamp"), BirthTimestamp.ToString());
	IdentityObj->SetBoolField(TEXT("bIsBeingPossessed"), bIsBeingPossessed);
	IdentityObj->SetNumberField(TEXT("ParadoxDensity"), ParadoxDensity);
	RootObj->SetObjectField(TEXT("Identity"), IdentityObj);

	// 2. UNKNOWN TAGS CATEGORY
	if (UnknownMetaTags.Num() > 0)
	{
		TSharedPtr<FJsonObject> TagsObj = MakeShareable(new FJsonObject());
		for (const auto& Elem : UnknownMetaTags)
		{
			TagsObj->SetStringField(Elem.Key, Elem.Value);
		}
		RootObj->SetObjectField(TEXT("UnknownTags"), TagsObj);
	}

	// 3. MODULAR BROKER CATEGORIES
	for (auto& Broker : RegisteredBrokers)
	{
		if (Broker.GetInterface())
		{
			// Each broker adds its own named object to the root
			Broker->OnSave(RootObj);
		}
	}

	// 4. OWNER FLAT SAVE DATA (ISovereignSaveInterface)
	if (AActor* Owner = GetOwner())
	{
		if (ISovereignSaveInterface* SaveInterface = Cast<ISovereignSaveInterface>(Owner))
		{
			TMap<FString, FString> SaveData = SaveInterface->GetSaveData();
			for (const auto& Elem : SaveData)
			{
				RootObj->SetStringField(Elem.Key, Elem.Value);
			}
		}
	}

	return RootObj;
}

void USovereignSaveableEntityComponent::ApplyStateFromJsonObject(const TSharedPtr<FJsonObject>& JsonData)
{
	if (!JsonData.IsValid()) return;

	// 1. RESTORE IDENTITY
	const TSharedPtr<FJsonObject>* IdentityObj;
	if (JsonData->TryGetObjectField(TEXT("Identity"), IdentityObj))
	{
		FString IdStr;
		if ((*IdentityObj)->TryGetStringField(TEXT("GUID"), IdStr)) FGuid::Parse(IdStr, EntityID);

		FString BirthStr;
		if ((*IdentityObj)->TryGetStringField(TEXT("BirthTimestamp"), BirthStr)) FDateTime::Parse(BirthStr, BirthTimestamp);

		(*IdentityObj)->TryGetBoolField(TEXT("bIsBeingPossessed"), bIsBeingPossessed);

		double ParadoxVal;
		if ((*IdentityObj)->TryGetNumberField(TEXT("ParadoxDensity"), ParadoxVal)) ParadoxDensity = (float)ParadoxVal;
	}

	// 2. RESTORE UNKNOWN TAGS
	const TSharedPtr<FJsonObject>* TagsObj;
	if (JsonData->TryGetObjectField(TEXT("UnknownTags"), TagsObj))
	{
		UnknownMetaTags.Empty();
		for (auto& Elem : (*TagsObj)->Values)
		{
			UnknownMetaTags.Add(FString(Elem.Key), Elem.Value->AsString());
		}
	}

	// 3. RESTORE MODULES
	for (auto& Broker : RegisteredBrokers)
	{
		if (Broker.GetInterface())
		{
			Broker->OnLoad(JsonData);
		}
	}

	// 4. RESTORE OWNER FLAT SAVE DATA (ISovereignSaveInterface)
	if (AActor* Owner = GetOwner())
	{
		if (ISovereignSaveInterface* SaveInterface = Cast<ISovereignSaveInterface>(Owner))
		{
			TMap<FString, FString> FlatData;
			for (const auto& Elem : JsonData->Values)
			{
				if (Elem.Value.IsValid())
				{
					if (Elem.Value->Type == EJson::String)
					{
						FlatData.Add(FString(Elem.Key), Elem.Value->AsString());
					}
					else if (Elem.Value->Type == EJson::Number)
					{
						FlatData.Add(FString(Elem.Key), FString::SanitizeFloat(Elem.Value->AsNumber()));
					}
				}
			}
			if (FlatData.Num() > 0)
			{
				SaveInterface->RestoreSaveData(FlatData);
			}
		}
	}

	InvalidateStateCache();
	OnStateChanged.Broadcast(this);
}

void USovereignSaveableEntityComponent::ApplyMetaTags(TMap<FString, FString> LoadedTags)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	for (auto& Elem : LoadedTags)
	{
		// 1. DNA FILTER: If it uses dot-notation (e.g., "Identity.Species.Wisp")
		if (Elem.Key.Contains(TEXT(".")))
		{
			// If it's a component-specific tag from an old save, we might want to skip it here
			// but for Legacy DNA, we try to ingest it.
			continue;
		}

		// 2. ELEMENTAL SOCKETS (Legacy fallback)
		// These should ideally be handled by the specialized components now.
		// For now, we just add them as Unknown Tags which will be processed by brokers.
		AddUnknownTag(Elem.Key, Elem.Value);
	}
}

FString USovereignSaveableEntityComponent::GetCategoryStateJson(FString CategoryName)
{
	uint64 CurrentFrame = GFrameCounter;
	double CurrentTime = -1.0;
	if (UWorld* World = GetWorld())
	{
		CurrentTime = World->GetTimeSeconds();
	}

	bool bShouldRebuild = false;
	if (!CachedFullState.IsValid())
	{
		bShouldRebuild = true;
	}
	else if (CurrentFrame != LastCachedFrame)
	{
		// New frame: check if the configured throttle interval has elapsed
		if (CurrentTime - LastCachedTime >= (double)UIUpdateThrottleInterval)
		{
			bShouldRebuild = true;
		}
	}

	if (bShouldRebuild)
	{
		CachedFullState = CaptureFullEntityState();
		LastCachedFrame = CurrentFrame;
		LastCachedTime = CurrentTime;
		CachedCategoryStrings.Empty();
	}

	if (FString* CachedStrPtr = CachedCategoryStrings.Find(CategoryName))
	{
		return *CachedStrPtr;
	}

	const TSharedPtr<FJsonObject>* CategoryObj;
	FString ResultString = TEXT("{}");
	if (CachedFullState->TryGetObjectField(CategoryName, CategoryObj))
	{
		ResultString = SerializeJsonToString(*CategoryObj);
	}

	CachedCategoryStrings.Add(CategoryName, ResultString);
	return ResultString;
}

void USovereignSaveableEntityComponent::AddUnknownTag(FString Key, FString Value)
{
	UnknownMetaTags.Add(Key, Value);

	if (Key.Contains(TEXT("Paradox")) || Key.Contains(TEXT("Conflict")) || Value.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase))
	{
		ParadoxDensity = FMath::Clamp(ParadoxDensity + 0.1f, 0.0f, 1.0f);
	}

	// Notify brokers of the new "Truth"
	TMap<FString, FString> SingleTagMap;
	SingleTagMap.Add(Key, Value);

	for (auto& Broker : RegisteredBrokers)
	{
		if (Broker.GetInterface())
		{
			Broker->OnProcessData(SingleTagMap);
		}
	}

	InvalidateStateCache();
	OnStateChanged.Broadcast(this);
}

FString USovereignSaveableEntityComponent::SerializeJsonToString(TSharedPtr<FJsonObject> JsonObj)
{
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	return JsonString;
}

float USovereignSaveableEntityComponent::GetSystemConfidence_Implementation() const
{
	float BaseConfidence = 1.0f - ParadoxDensity;

	// Coupling with the Diagnostic Broker's curation status (VettedBy indicator)
	if (DiagnosticBroker && !DiagnosticBroker->VettedBy.IsEmpty())
	{
		// Curator validation mitigates 80% of current paradox/uncertainty density, boosting the system confidence.
		BaseConfidence = 1.0f - (ParadoxDensity * 0.2f);
	}

	return FMath::Clamp(BaseConfidence, 0.0f, 1.0f);
}

#if WITH_EDITOR
void USovereignSaveableEntityComponent::PostEditImport() { Super::PostEditImport(); EntityID = FGuid::NewGuid(); }

void USovereignSaveableEntityComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (!bDuplicateForPIE) { EntityID = FGuid::NewGuid(); }
}
#endif