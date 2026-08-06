// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveSystem/SovereignGameData.h"
#include "SovereignBlackBoxExporter.generated.h"

/**
 * @class USovereignBlackBoxExporter
 * @brief Utilities for exporting Black Box logs to CSV and other analysis formats.
 *
 * Provides forensic and analytical tools for examining entity telemetry data.
 * Converts JSON Black Box logs to CSV for external tool compatibility.
 */
UCLASS()
class WISPCPP7VR_API USovereignBlackBoxExporter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Exports a single entity's Black Box log to CSV format.
	 *
	 * @param EntityID          The entity whose logs to export
	 * @param OutCsvPath        Output path for the CSV file
	 * @return                  True if export successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Forensics")
	static bool ExportEntityLogToCSV(const FGuid& EntityID, FString& OutCsvPath);

	/**
	 * Exports all Black Box logs in /Saved/BlackBox/ to a single CSV file.
	 * Useful for cross-entity analysis and batch forensics.
	 *
	 * @param OutCsvPath        Output path for the CSV file
	 * @return                  Number of logs exported (0 if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Forensics")
	static int32 ExportAllLogsToCSV(FString& OutCsvPath);

	/**
	 * Generates a summary statistics file for a single entity.
	 * Includes min, max, average, and variance for each tracked metric.
	 *
	 * @param EntityID          The entity to analyze
	 * @param OutStatsPath      Output path for the stats JSON file
	 * @return                  True if generation successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Forensics")
	static bool GenerateEntityStatistics(const FGuid& EntityID, FString& OutStatsPath);

	/**
	 * Validates the integrity of a Black Box JSON file.
	 * Checks for malformed entries, missing fields, timestamp ordering.
	 *
	 * @param FilePath          Full path to the Black Box JSON file
	 * @param OutValidationReport Output text report of validation results
	 * @return                  True if file is valid, false if corrupted
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Forensics")
	static bool ValidateBlackBoxFile(const FString& FilePath, FString& OutValidationReport);

	/**
	 * Exports a time-series snapshot for visualization.
	 * Groups entries by configurable time window and exports aggregated values.
	 *
	 * @param EntityID          The entity whose logs to export
	 * @param WindowSeconds     Time window for aggregation (e.g., 60 = 1-minute buckets)
	 * @param OutJsonPath       Output path for the JSON file
	 * @return                  True if export successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Sovereign|BlackBox|Forensics")
	static bool ExportTimeSeriesSnapshot(const FGuid& EntityID, int32 WindowSeconds, FString& OutJsonPath);

private:
	/** Helper to load Black Box JSON from disk */
	static bool LoadBlackBoxJson(const FString& FilePath, TSharedPtr<FJsonObject>& OutJsonObject);

	/** Helper to get the Black Box directory path */
	static FString GetBlackBoxDirectory();

	/** Helper to get the export directory path */
	static FString GetExportDirectory();
};
