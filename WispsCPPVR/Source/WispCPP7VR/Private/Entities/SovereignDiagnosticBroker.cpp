// Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
#include "Entities/SovereignDiagnosticBroker.h"
#include "Dom/JsonObject.h"

void UDiagnosticBroker::OnSave(TSharedPtr<FJsonObject>& OutJson)
{
    if (!OutJson.IsValid()) return;

    TSharedPtr<FJsonObject> TruthObj = MakeShareable(new FJsonObject());

    for (auto& Elem : TruthMetadata)
    {
        TruthObj->SetStringField(Elem.Key, Elem.Value);
    }

    TruthObj->SetStringField(TEXT("VettedBy"), VettedBy);

    OutJson->SetObjectField(TEXT("Sovereign.Truth"), TruthObj);
}

void UDiagnosticBroker::OnLoad(const TSharedPtr<FJsonObject>& InJson)
{
    if (!InJson.IsValid() || !InJson->HasField(TEXT("Sovereign.Truth"))) return;

    TSharedPtr<FJsonObject> TruthObj = InJson->GetObjectField(TEXT("Sovereign.Truth"));

    TruthMetadata.Empty();
    for (auto& Elem : TruthObj->Values)
    {
        // Explicitly convert Elem.Key to FString for Unreal 5.8 compatibility
        FString KeyStr = FString(Elem.Key);

        if (KeyStr == TEXT("VettedBy"))
        {
            VettedBy = Elem.Value->AsString();
        }
        else
        {
            TruthMetadata.Add(KeyStr, Elem.Value->AsString());
        }
    }
}

void UDiagnosticBroker::OnProcessData(const TMap<FString, FString>& Data)
{
    // Ingest Lidar Manifest / Museum Data
    for (auto& Elem : Data)
    {
        TruthMetadata.Add(Elem.Key, Elem.Value);
    }
}

FString UDiagnosticBroker::GetTruthValue(const FString& Key) const
{
    if (TruthMetadata.Contains(Key))
    {
        return TruthMetadata[Key];
    }
    return TEXT("");
}

void UDiagnosticBroker::SetTruthValue(const FString& Key, const FString& Value)
{
    TruthMetadata.Add(Key, Value);
}