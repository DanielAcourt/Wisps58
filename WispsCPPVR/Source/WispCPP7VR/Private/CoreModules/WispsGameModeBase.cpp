// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreModules/WispsGameModeBase.h"
#include "Entities/SovereignPlayerWisp.h"
#include "UObject/ConstructorHelpers.h"
// 1. Include the "Advanced" Species Data Asset definition
#include "DataTables/SovereignSpeciesData.h" 
// 2. Include the "Hard Data" Table Row definition (Matches your provided code)
#include "DataTables/SovereignDataTypes.h" 
#include "Engine/DataTable.h"


// THIS IS THE CONSTRUCTOR THE LINKER IS MISSING Realy crucial we must make this before we do any logic
AWispsGameModeBase::AWispsGameModeBase()
{

    // 1. Point to the BLUEPRINT version of the Wisp
        // Note the "_C" at the end of the path - this tells Unreal we want the GENERATED class
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Entities/BP_PlayerWisp_C"));

    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
    else
    {
        // If the BP isn't found, we fall back to C++, but we log a warning
        DefaultPawnClass = ASovereignPlayerWisp::StaticClass();
        UE_LOG(LogTemp, Error, TEXT("GameMode: Could not find BP_PlayerWisp. Controls will be NULL!"));
    }
    // 2. Optional: Auto-load the Data Table if you know the path
    static ConstructorHelpers::FObjectFinder<UDataTable> TableObj(TEXT("/Game/Data/DT_SovereignItems"));
    if (TableObj.Succeeded())
    {
        ItemDataTable = TableObj.Object;
    }
}

USovereignSpeciesData* AWispsGameModeBase::GetSpeciesDataByTag(FGameplayTag SpeciesTag)
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: ItemDataTable is NOT assigned in Blueprint!"));
        return nullptr;
    }

    static const FString ContextString(TEXT("Species Lookup"));

    // Now uses your updated struct name: FSovereignItemRow
    TArray<FSovereignItemRow*> AllRows;
    ItemDataTable->GetAllRows<FSovereignItemRow>(ContextString, AllRows);

    for (FSovereignItemRow* Row : AllRows)
    {
        // We check the "Hard Read Only" ItemTag first
        if (Row && Row->ItemTag.MatchesTagExact(SpeciesTag))
        {
            // If it's biological (like an Oak or Isla), load the Advanced Species Data
            if (Row->bIsBiologicalOrigin && !Row->SpeciesVesselData.IsNull())
            {
                return Row->SpeciesVesselData.LoadSynchronous();
            }
        }
    }

    return nullptr;
}