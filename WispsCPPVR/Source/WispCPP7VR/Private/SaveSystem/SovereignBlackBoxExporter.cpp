// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#include "SaveSystem/SovereignBlackBoxExporter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

bool USovereignBlackBoxExporter::ExportEntityLogToCSV(const FGuid& EntityID, FString& OutCsvPath)
{
	if (!EntityID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Invalid EntityID provided"));
		return false;
	}

	FString JsonPath = GetBlackBoxDirectory() / FString::Printf(TEXT("BB_%s.json"), *EntityID.ToString());
	TSharedPtr<FJsonObject> JsonRoot;

	if (!LoadBlackBoxJson(JsonPath, JsonRoot) || !JsonRoot.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Failed to load Black Box JSON for entity %s"), *EntityID.ToString());
		return false;
	}

	// Build CSV header and data
	FString CsvContent = TEXT("Timestamp,Key,Value,EventData\n");
	const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;

	if (!JsonRoot->TryGetArrayField(TEXT("Logs"), Logs))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: No Logs array found in Black Box for entity %s"), *EntityID.ToString());
		return false;
	}

	for (const TSharedPtr<FJsonValue>& LogValue : *Logs)
	{
		TSharedPtr<FJsonObject> JsonEntry = LogValue->AsObject();
		if (!JsonEntry.IsValid()) continue;

		FString Timestamp = JsonEntry->GetStringField(TEXT("Timestamp"));
		FString Key = JsonEntry->GetStringField(TEXT("Key"));
		double Value = JsonEntry->GetNumberField(TEXT("Value"));
		FString EventData = JsonEntry->HasField(TEXT("EventData")) ? JsonEntry->GetStringField(TEXT("EventData")) : TEXT("");

		// Escape CSV special characters
		Key = Key.Replace(TEXT("\""), TEXT("\"\""));
		EventData = EventData.Replace(TEXT("\""), TEXT("\"\""));
		CsvContent += FString::Printf(TEXT("%s,\"%s\",%f,\"%s\"\n"), *Timestamp, *Key, Value, *EventData);
	}

	// Write CSV file
	FString ExportDir = GetExportDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*ExportDir))
	{
		PlatformFile.CreateDirectoryTree(*ExportDir);
	}

	OutCsvPath = ExportDir / FString::Printf(TEXT("BB_%s_export.csv"), *EntityID.ToString());

	if (!FFileHelper::SaveStringToFile(CsvContent, *OutCsvPath))
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Failed to write CSV file to %s"), *OutCsvPath);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: Exported %d entries to %s"), Logs->Num(), *OutCsvPath);

	return true;
}

int32 USovereignBlackBoxExporter::ExportAllLogsToCSV(FString& OutCsvPath)
{
	FString BlackBoxDir = GetBlackBoxDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*BlackBoxDir))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: Black Box directory does not exist: %s"), *BlackBoxDir);
		return 0;
	}

	// Build CSV header
	FString CsvContent = TEXT("EntityID,Timestamp,Key,Value,EventData\n");
	int32 TotalEntries = 0;
	int32 EntitiesExported = 0;

	// Enumerate all BB_*.json files
	class FJsonFileFinder : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FString& OutputContent;
		int32& EntryCount;
		int32& EntityCount;

		FJsonFileFinder(FString& InContent, int32& InEntries, int32& InEntities)
			: OutputContent(InContent), EntryCount(InEntries), EntityCount(InEntities) {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (bIsDirectory) return true;

			FString Filename = FPaths::GetCleanFilename(FilenameOrDirectory);
			if (!Filename.StartsWith(TEXT("BB_")) || !Filename.EndsWith(TEXT(".json")))
			{
				return true;
			}

			FString JsonContent;
			if (!FFileHelper::LoadFileToString(JsonContent, FilenameOrDirectory))
			{
				return true;
			}

			TSharedPtr<FJsonObject> JsonRoot;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
			if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
			{
				return true;
			}

			// Extract entity ID from filename (BB_GUID.json)
			FString EntityIDStr = Filename.Mid(3, Filename.Len() - 8);

			const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
			if (!JsonRoot->TryGetArrayField(TEXT("Logs"), Logs))
			{
				return true;
			}

			for (const TSharedPtr<FJsonValue>& LogValue : *Logs)
			{
				TSharedPtr<FJsonObject> JsonEntry = LogValue->AsObject();
				if (!JsonEntry.IsValid()) continue;

				FString Timestamp = JsonEntry->GetStringField(TEXT("Timestamp"));
				FString Key = JsonEntry->GetStringField(TEXT("Key"));
				double Value = JsonEntry->GetNumberField(TEXT("Value"));
				FString EventData = JsonEntry->HasField(TEXT("EventData")) ? JsonEntry->GetStringField(TEXT("EventData")) : TEXT("");

				Key = Key.Replace(TEXT("\""), TEXT("\"\""));
				EventData = EventData.Replace(TEXT("\""), TEXT("\"\""));
				OutputContent += FString::Printf(TEXT("%s,%s,\"%s\",%f,\"%s\"\n"),
					*EntityIDStr, *Timestamp, *Key, Value, *EventData);

				EntryCount++;
			}

			EntityCount++;
			return true;
		}
	};

	FJsonFileFinder Visitor(CsvContent, TotalEntries, EntitiesExported);
	PlatformFile.IterateDirectory(*BlackBoxDir, Visitor);

	if (EntitiesExported == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: No Black Box files found to export"));
		return 0;
	}

	// Write combined CSV
	FString ExportDir = GetExportDirectory();
	if (!PlatformFile.DirectoryExists(*ExportDir))
	{
		PlatformFile.CreateDirectoryTree(*ExportDir);
	}

	OutCsvPath = ExportDir / TEXT("BlackBox_AllEntities_export.csv");

	if (!FFileHelper::SaveStringToFile(CsvContent, *OutCsvPath))
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Failed to write combined CSV to %s"), *OutCsvPath);
		return 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: Exported %d entries from %d entities to %s"),
		TotalEntries, EntitiesExported, *OutCsvPath);

	return TotalEntries;
}

bool USovereignBlackBoxExporter::GenerateEntityStatistics(const FGuid& EntityID, FString& OutStatsPath)
{
	if (!EntityID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Invalid EntityID for statistics"));
		return false;
	}

	FString JsonPath = GetBlackBoxDirectory() / FString::Printf(TEXT("BB_%s.json"), *EntityID.ToString());
	TSharedPtr<FJsonObject> JsonRoot;

	if (!LoadBlackBoxJson(JsonPath, JsonRoot) || !JsonRoot.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
	if (!JsonRoot->TryGetArrayField(TEXT("Logs"), Logs))
	{
		return false;
	}

	// Group values by key and compute statistics
	TMap<FString, TArray<double>> ValuesByKey;

	for (const TSharedPtr<FJsonValue>& LogValue : *Logs)
	{
		TSharedPtr<FJsonObject> JsonEntry = LogValue->AsObject();
		if (!JsonEntry.IsValid()) continue;

		FString Key = JsonEntry->GetStringField(TEXT("Key"));
		if (JsonEntry->HasField(TEXT("Value")))
		{
			double Value = JsonEntry->GetNumberField(TEXT("Value"));
			ValuesByKey.FindOrAdd(Key).Add(Value);
		}
	}

	// Compute statistics
	TSharedPtr<FJsonObject> StatsRoot = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> MetricsArray;

	for (auto& KeyValues : ValuesByKey)
	{
		TArray<double>& Values = KeyValues.Value;
		if (Values.Num() == 0) continue;

		// Calculate statistics
		double Min = Values[0];
		double Max = Values[0];
		double Sum = 0.0;

		for (double Val : Values)
		{
			Min = FMath::Min(Min, Val);
			Max = FMath::Max(Max, Val);
			Sum += Val;
		}

		double Average = Sum / Values.Num();

		// Calculate variance
		double VarianceSum = 0.0;
		for (double Val : Values)
		{
			VarianceSum += FMath::Square(Val - Average);
		}
		double Variance = VarianceSum / Values.Num();
		double StdDev = FMath::Sqrt(Variance);

		// Build metric object
		TSharedPtr<FJsonObject> MetricObj = MakeShareable(new FJsonObject());
		MetricObj->SetStringField(TEXT("Key"), KeyValues.Key);
		MetricObj->SetNumberField(TEXT("Min"), Min);
		MetricObj->SetNumberField(TEXT("Max"), Max);
		MetricObj->SetNumberField(TEXT("Average"), Average);
		MetricObj->SetNumberField(TEXT("StdDev"), StdDev);
		MetricObj->SetNumberField(TEXT("Count"), static_cast<double>(Values.Num()));

		MetricsArray.Add(MakeShareable(new FJsonValueObject(MetricObj)));
	}

	StatsRoot->SetArrayField(TEXT("Metrics"), MetricsArray);
	StatsRoot->SetStringField(TEXT("EntityID"), EntityID.ToString());
	StatsRoot->SetStringField(TEXT("GeneratedAt"), FDateTime::Now().ToString());

	// Write stats file
	FString ExportDir = GetExportDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*ExportDir))
	{
		PlatformFile.CreateDirectoryTree(*ExportDir);
	}

	OutStatsPath = ExportDir / FString::Printf(TEXT("BB_%s_stats.json"), *EntityID.ToString());

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(StatsRoot.ToSharedRef(), Writer);

	if (!FFileHelper::SaveStringToFile(JsonString, *OutStatsPath))
	{
		UE_LOG(LogTemp, Error, TEXT("BlackBoxExporter: Failed to write statistics file"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: Generated statistics for %d metrics"), MetricsArray.Num());
	return true;
}

bool USovereignBlackBoxExporter::ValidateBlackBoxFile(const FString& FilePath, FString& OutValidationReport)
{
	OutValidationReport = TEXT("");

	if (!FPaths::FileExists(FilePath))
	{
		OutValidationReport = FString::Printf(TEXT("ERROR: File not found: %s"), *FilePath);
		return false;
	}

	// Load and parse JSON
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
	{
		OutValidationReport = TEXT("ERROR: Failed to read file");
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	TSharedPtr<FJsonObject> JsonRoot;

	if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
	{
		OutValidationReport = TEXT("ERROR: Invalid JSON format");
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
	if (!JsonRoot->TryGetArrayField(TEXT("Logs"), Logs))
	{
		OutValidationReport = TEXT("ERROR: No Logs array found");
		return false;
	}

	// Validate each entry
	OutValidationReport = FString::Printf(TEXT("✓ File is valid. Found %d entries.\n"), Logs->Num());
	int32 EntriesWithErrors = 0;

	FDateTime LastTimestamp = FDateTime::MinValue();

	for (int32 i = 0; i < Logs->Num(); ++i)
	{
		TSharedPtr<FJsonObject> JsonEntry = (*Logs)[i]->AsObject();
		if (!JsonEntry.IsValid())
		{
			OutValidationReport += FString::Printf(TEXT("  ✗ Entry %d: Invalid object\n"), i);
			EntriesWithErrors++;
			continue;
		}

		// Check required fields
		if (!JsonEntry->HasField(TEXT("Timestamp")))
		{
			OutValidationReport += FString::Printf(TEXT("  ✗ Entry %d: Missing Timestamp\n"), i);
			EntriesWithErrors++;
		}

		if (!JsonEntry->HasField(TEXT("Key")))
		{
			OutValidationReport += FString::Printf(TEXT("  ✗ Entry %d: Missing Key\n"), i);
			EntriesWithErrors++;
		}

		// Check timestamp ordering
		if (JsonEntry->HasField(TEXT("Timestamp")))
		{
			FString TimestampStr = JsonEntry->GetStringField(TEXT("Timestamp"));
			FDateTime EntryTime;
			if (FDateTime::Parse(TimestampStr, EntryTime))
			{
				if (EntryTime < LastTimestamp)
				{
					OutValidationReport += FString::Printf(TEXT("  ⚠ Entry %d: Timestamp out of order\n"), i);
				}
				LastTimestamp = EntryTime;
			}
			else
			{
				OutValidationReport += FString::Printf(TEXT("  ✗ Entry %d: Invalid Timestamp format\n"), i);
				EntriesWithErrors++;
			}
		}
	}

	if (EntriesWithErrors > 0)
	{
		OutValidationReport += FString::Printf(TEXT("\n✗ Validation FAILED: %d entries with errors"), EntriesWithErrors);
		return false;
	}

	OutValidationReport += TEXT("\n✓ All entries valid. File is healthy.");
	return true;
}

bool USovereignBlackBoxExporter::ExportTimeSeriesSnapshot(const FGuid& EntityID, int32 WindowSeconds, FString& OutJsonPath)
{
	if (!EntityID.IsValid() || WindowSeconds <= 0)
	{
		return false;
	}

	FString JsonPath = GetBlackBoxDirectory() / FString::Printf(TEXT("BB_%s.json"), *EntityID.ToString());
	TSharedPtr<FJsonObject> JsonRoot;

	if (!LoadBlackBoxJson(JsonPath, JsonRoot) || !JsonRoot.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Logs = nullptr;
	if (!JsonRoot->TryGetArrayField(TEXT("Logs"), Logs))
	{
		return false;
	}

	// Group entries by time windows
	TMap<int64, TMap<FString, TArray<double>>> WindowedData;

	for (const TSharedPtr<FJsonValue>& LogValue : *Logs)
	{
		TSharedPtr<FJsonObject> JsonEntry = LogValue->AsObject();
		if (!JsonEntry.IsValid()) continue;

		FString TimestampStr = JsonEntry->GetStringField(TEXT("Timestamp"));
		FDateTime Timestamp;
		if (!FDateTime::Parse(TimestampStr, Timestamp)) continue;

		FString Key = JsonEntry->GetStringField(TEXT("Key"));
		if (!JsonEntry->HasField(TEXT("Value"))) continue;

		double Value = JsonEntry->GetNumberField(TEXT("Value"));

		int64 WindowIndex = Timestamp.ToUnixTimestamp() / WindowSeconds;
		WindowedData.FindOrAdd(WindowIndex).FindOrAdd(Key).Add(Value);
	}

	// Build aggregated data
	TArray<TSharedPtr<FJsonValue>> SnapshotArray;

	for (auto& Window : WindowedData)
	{
		TSharedPtr<FJsonObject> WindowObj = MakeShareable(new FJsonObject());
		WindowObj->SetNumberField(TEXT("WindowIndex"), static_cast<double>(Window.Key));
		WindowObj->SetStringField(TEXT("Timestamp"), FDateTime::FromUnixTimestamp(Window.Key * WindowSeconds).ToString());

		for (auto& KeyAgg : Window.Value)
		{
			double Sum = 0.0;
			for (double Val : KeyAgg.Value)
			{
				Sum += Val;
			}

			double Average = Sum / KeyAgg.Value.Num();
			WindowObj->SetNumberField(KeyAgg.Key, Average);
		}

		SnapshotArray.Add(MakeShareable(new FJsonValueObject(WindowObj)));
	}

	// Create output JSON
	TSharedPtr<FJsonObject> OutputRoot = MakeShareable(new FJsonObject());
	OutputRoot->SetStringField(TEXT("EntityID"), EntityID.ToString());
	OutputRoot->SetNumberField(TEXT("WindowSeconds"), static_cast<double>(WindowSeconds));
	OutputRoot->SetArrayField(TEXT("Snapshots"), SnapshotArray);

	FString ExportDir = GetExportDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*ExportDir))
	{
		PlatformFile.CreateDirectoryTree(*ExportDir);
	}

	OutJsonPath = ExportDir / FString::Printf(TEXT("BB_%s_timeseries_%ds.json"), *EntityID.ToString(), WindowSeconds);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(OutputRoot.ToSharedRef(), Writer);

	if (!FFileHelper::SaveStringToFile(JsonString, *OutJsonPath))
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("BlackBoxExporter: Exported %d time windows to %s"),
		SnapshotArray.Num(), *OutJsonPath);

	return true;
}

bool USovereignBlackBoxExporter::LoadBlackBoxJson(const FString& FilePath, TSharedPtr<FJsonObject>& OutJsonObject)
{
	if (!FPaths::FileExists(FilePath))
	{
		return false;
	}

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
	{
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	return FJsonSerializer::Deserialize(Reader, OutJsonObject) && OutJsonObject.IsValid();
}

FString USovereignBlackBoxExporter::GetBlackBoxDirectory()
{
	return FPaths::ProjectSavedDir() / TEXT("BlackBox");
}

FString USovereignBlackBoxExporter::GetExportDirectory()
{
	return FPaths::ProjectSavedDir() / TEXT("BlackBox") / TEXT("Exports");
}
