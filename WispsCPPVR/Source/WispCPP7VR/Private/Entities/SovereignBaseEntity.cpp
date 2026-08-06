//SovereignBaseEntity.ccp

#include "Entities/SovereignBaseEntity.h"
#include "DataTables/SovereignSpeciesData.h" // Essential for accessing GrowthStages

#include "SaveSystem/SovereignActorRegistry.h"
#include "SaveSystem/SovereignSpawnerUtils.h" 
#include "SaveSystem/SovereignGameData.h" 

#include "Entities/SovereignSaveableEntityComponent.h"

#include "Components/CapsuleComponent.h" // Add this include!
#include "Components/StaticMeshComponent.h"
#include "Components/SovereignBioComponent.h"
#include "Components/SovereignQiComponent.h"
#include "Components/SovereignElementComponent.h"
#include "Components/SovereignAttributeComponent.h"

#include "GameplayTagsManager.h"

#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "TimerManager.h"

ASovereignBaseEntity::ASovereignBaseEntity()
{
    // 0. CHARACTER DEFAULTS
    PrimaryActorTick.bCanEverTick = true;

    float Interval = GetHeartbeatInterval();
    /*
    if (Interval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(HeartbeatTimerHandle, this, &ASovereignBaseEntity::OnSovereignHeartbeat, Interval, true);
    }
    */
    //bCanAffectNavigationGeneration = true;

    /*
    // 1. CONFIGURE THE BUILT-IN CAPSULE
        // We don't use CreateDefaultSubobject here because ACharacter already did it!
    if (UCapsuleComponent* MyCapsule = GetCapsuleComponent())
    {
        MyCapsule->InitCapsuleSize(40.f, 90.f);
        MyCapsule->SetCollisionProfileName(TEXT("Pawn"));
    }
    */
    // 2. THE SOUL (SAVE SYSTEM)
    // This component handles the GUID and the metadata tags
    SaveDataComponent = CreateDefaultSubobject<USovereignSaveableEntityComponent>(TEXT("SaveDataComponent"));

    // 2b. SPECIALIZED MODULES
    BioComponent = CreateDefaultSubobject<USovereignBioComponent>(TEXT("BioComponent"));
    QiComponent = CreateDefaultSubobject<USovereignQiComponent>(TEXT("QiComponent"));
    ElementComponent = CreateDefaultSubobject<USovereignElementComponent>(TEXT("ElementComponent"));
    AttributeComponent = CreateDefaultSubobject<USovereignAttributeComponent>(TEXT("AttributeComponent"));

    // 3. PHYSICAL MESH
    // We create a StaticMeshComponent to visualize the 8 growth stages (Seed to Tree)
    // Create the "Master" mesh slot
    EntityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntityMesh"));

    // Since we are an AActor now, the Mesh is our Physical Root
    RootComponent = EntityMesh;

    // This allows the Wisp to possess the "Soul"
    bCanBePossessed = true;

    // 3. Setup Default Collision for your Wisp's LineTrace
    EntityMesh->SetCollisionProfileName(TEXT("BlockAll"));



    /*
    if (RootComponent)
    {
        EntityMesh->SetupAttachment(RootComponent);
    }
    */
    // Optional: Move the mesh down so it sits at the bottom of the capsule
    //EntityMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
}

void ASovereignBaseEntity::VerifySymmetryLevel()
{
    // Placeholder for Symmetry Kernel Check
    // This is where you would grant permissions based on the TrustSignature,
    // and attributes like Luck or Charisma.
    // For example:
    // if (TrustSignature > 1000 && GetAttributeComponent()->Luck > 50)
    // {
    //		// Grant special permission
    // }
}

void ASovereignBaseEntity::BeginPlay()
{
    //Force being play
    Super::BeginPlay();

    // 1. Manifest the Soul's History (The March 23, 2017 logic)
    if (SaveDataComponent)
    {
        SaveDataComponent->InitializeSoul();
    }

    // After InitializeSoul()
    if (SaveDataComponent)
    {
        // If this soul is over 500 Sovereign Years old, force it to the final growth stage
        float TotalDays = (FDateTime::Now() - SaveDataComponent->BirthTimestamp).GetTotalDays();
        float SovYears = TotalDays / 5.6f;

        if (SovYears > 500.0f)
        {
            CurrentGrowthStage = 7; // The "Ancient" or "Tree" stage
            RefreshVisuals();
        }
    }

    // 2. Now it is safe to talk to the World and TimerManager!
    float Interval = GetHeartbeatInterval();

    if (Interval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            HeartbeatTimerHandle,
            this,
            &ASovereignBaseEntity::OnSovereignHeartbeat,
            Interval,
            true
        );
    }

    switch (UpdateFrequency)
    {

    case EUpdateFrequency::Faster:


        //Time dilation and slowing world down
        /*
        case EUpdateFrequency::Faster: 
    // 1. High-frequency logic update (Realtime)
    Interval = 0.0f; 

    // 2. The Time Warp: 
    // We slow the world down by 5x (0.2), 
    // but we speed THIS entity up by 5x (5.0) to compensate.
    // Result: The world looks slow to you, but you move at normal speed.
    
    if (UWorld* World = GetWorld())
    {
        // Slow down everything else
        World->GetWorldSettings()->SetTimeDilation(0.2f); 
        
        // Overclock only the Sovereign Soul
        this->CustomTimeDilation = 5.0f; 
        
        UE_LOG(LogTemp, Warning, TEXT("Sovereign: Faster Frequency Engaged. Time Dilated."));
    }
    break;
        */

    case EUpdateFrequency::Realtime: Interval = 0.0f; break;   // 0.0 = Every Frame

    case EUpdateFrequency::Standard: Interval = 1.0f; break;   // 1 Second

    case EUpdateFrequency::Slow:     Interval = 10.0f; break;  // 10 Seconds

    case EUpdateFrequency::Slower:   Interval = 30.0f; break;  // 30 Seconds

    case EUpdateFrequency::Slowest:  Interval = 60.0f; break;  // 60 Seconds (1 Minute)

    case EUpdateFrequency::Glacier:  Interval = 360.0f; break; // 360 Seconds (6 Minutes)

    case EUpdateFrequency::Dormant:  Interval = -1.0f; break;  // Disable Heartbeat

    //case EUpdateFrequency::Standard: Interval = 30.0f; break;
    //case EUpdateFrequency::Faster:     Interval = 60.0f; break;
    //case EUpdateFrequency::SuperFast:  Interval = 360.0f; break;
    }

    if (Interval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(HeartbeatTimerHandle, this, &ASovereignBaseEntity::OnSovereignHeartbeat, Interval, true);
    }

    // Disable standard Tick if we are using a Heartbeat to save CPU (Crucial for VR performance)
    SetActorTickEnabled(UpdateFrequency == EUpdateFrequency::Realtime);

    // Initial visual setup
    RefreshVisuals();
}

void ASovereignBaseEntity::OnSovereignHeartbeat()
{
    if (SaveDataComponent)
    {
        float HeartbeatSeconds = GetWorldTimerManager().GetTimerRate(HeartbeatTimerHandle);

        // 1. BIOLOGICAL GROWTH & CONSUMPTION
        if (BioComponent)
        {
            BioComponent->MaturityProgress += (BioComponent->MaturityRate);
            BioComponent->UpdateMetabolism(HeartbeatSeconds);
        }

        // 2. SPIRITUAL FLOW
        if (QiComponent)
        {
            QiComponent->ProcessQiFlow(HeartbeatSeconds, 10); // Wisdom hardcoded for now
        }

        // 3. EVOLUTION CHECK
        if (BioComponent && BioComponent->MaturityProgress >= 1.0f)
        {
            BioComponent->MaturityProgress = 0.0f;
            Evolve();
        }

        //old logic
        /*
        // Check for Evolution (Threshold met, move to next growth stage)
        if (SaveDataComponent->MaturityProgress >= 1.0f)
        {
            // Reset progress for the next stage (or keep remainder for overflow)
            SaveDataComponent->MaturityProgress = 0.0f;
            Evolve();
        }
        */
    }
}

void ASovereignBaseEntity::CheckForEvolution()
{
    /*
    if (!SaveDataComponent) return;

    // 1. GET THE HARDWARE DATA
    auto AttrComp = FindComponentByClass<USovereignAttributeComponent>();
    if (!AttrComp) return;

    // 2. THE COMPOSITE POWER CALCULATION
    // We use the prestige Level (int32) + the scientific Experience (double)
    double TotalPower = (double)AttrComp->Strength + AttrComp->StrengthExperience +
        (double)AttrComp->Intelligence + AttrComp->IntelligenceExperience +
        (double)AttrComp->Wisdom + AttrComp->WisdomExperience;

    //Can we do this smarter like in an array, SO that the training matches the stored experience? in an enum + douuble?


    // 3. APPLY THE SPECIES TAX (Difficulty)
    // SpeciesData->ClassLevelModifier: 1.0 (Chicken), 50.0 (Dragon)
    float Difficulty = SpeciesData ? SpeciesData->ClassLevelModifier : 1.0f;
    double NormalizedPower = TotalPower / (double)Difficulty;

    // 4. EVOLUTION TRIGGER
    // Requirements scale exponentially: Stage 1 needs 100 power, Stage 2 needs 200, etc.
    float PowerRequirement = (float)(CurrentGrowthStage + 1) * 100.0f;
    */


    //if (NormalizedPower >= PowerRequirement)
    //{
        Evolve();
    //}
}

//version 3.2 Updated for Modular Hub
void ASovereignBaseEntity::Evolve()
{
    // Evolution is a massive biological strain
    if (BioComponent)
    {
        // 1. BURN THE ENTIRE PHARMACY
        BioComponent->NutrientReserves.Empty();

        BioComponent->Hunger = 0.0f;
        BioComponent->Entropy += 10.0f; // Rapid aging occurs during evolution

        // The Mass is permanently increased (Physical Prestige)
        BioComponent->MassExperience += 5.0;
        BioComponent->Mass = FMath::FloorToInt(BioComponent->MassExperience);
    }

    // Trigger the Visual Shift (Mesh/Particle swap)
    CurrentGrowthStage = FMath::Clamp(CurrentGrowthStage + 1, 0, 7);
    RefreshVisuals();
}

//Like growing their is a gestatin state should evolving be locked into a state witha  progress bar this progress needs ot be manipulated by timme it can go upo or down
//down = smaller deniser therfore more mass but smaller. 
//U can hone your magic to a refine state increase your max potenitals for rebirths??

/* Version 3.3

Goal to Make evolution a process not a pop use same logic for eggs to be hatched?
Basically if state is progress how does it progress chicken, Incemination, birth egg, incubate egg, hatch , egg, chick then growing levels of maturity
I want to capture the groweth at each stage you cant go back without rebith 
therfore cookie click logic in a slow meditation game.

void ASovereignBaseEntity::ProcessEvolution(float DeltaTime)
{
    if (CurrentState != ESovereignState::Gesticulating) return;

    // Gestation speed is modified by local Qi and Bio Health
    float GestationSpeed = CalculateGestationSpeed();
    GestationProgress += GestationSpeed * DeltaTime;

    // REFINEMENT LOGIC:
    // If we are 'honing' (Magic focus), we sacrifice size for density
    if (bIsHoningSoul)
    {
        RefineDensity(DeltaTime);
    }

    if (GestationProgress >= 100.0f)
    {
        CompleteEvolution();
    }
}
*/

/*
void ASovereignBaseEntity::AttemptMating(AActor* PotentialPartner)
{
    if (!PotentialPartner || !IsReadyForMating()) return;

    ASovereignBaseEntity* PartnerActor = Cast<ASovereignBaseEntity>(PotentialPartner);

    // Logic: Ready, Compatible, and SpawnerUtils confirms the "Handshake"
    if (IsReadyForMating() && PartnerActor && PartnerActor->IsReadyForMating() &&
        USovereignSpawnerUtils::CanBreed(this, PotentialPartner))
    {
        FVector MidPoint = (GetActorLocation() + PotentialPartner->GetActorLocation()) / 2.0f;
        FTransform SpawnTransform(GetActorRotation(), MidPoint);

        USovereignSpawnerUtils::SpawnHybridEntity(
            GetWorld(),
            SpeciesData,
            SpawnTransform,
            this,
            PotentialPartner
        );

        // Since spawning is now async, we can't get a return value.
        // The timestamp logic should be moved to the PostSpawnInitialize function if needed.
        // For now, we will just assume success.
    }
}

bool ASovereignBaseEntity::IsReadyForMating() const
{
    if (!SaveDataComponent || !GetWorld()) return false;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastMating = CurrentTime - SaveDataComponent->LastMatingTimestamp;

    return TimeSinceLastMating >= SaveDataComponent->MatingCooldownDuration;
}
*/

FGuid ASovereignBaseEntity::GetSovereignID() const
{
    return SaveDataComponent ? SaveDataComponent->EntityID : FGuid();
}

// Implementation of the IGameplayTagAssetInterface
void ASovereignBaseEntity::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = GameplayTags;

    // Optional: If you want the IdentitySignature to always be included in the save
    if (IdentitySignature.IsValid())
    {
        TagContainer.AddTag(IdentitySignature);
    }
}

void ASovereignBaseEntity::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If Realtime frequency, simulate biology and Qi flow every frame using DeltaTime to prevent biological freeze
    if (UpdateFrequency == EUpdateFrequency::Realtime)
    {
        if (BioComponent)
        {
            BioComponent->MaturityProgress += (BioComponent->MaturityRate * DeltaTime);
            BioComponent->UpdateMetabolism(DeltaTime);
        }

        if (QiComponent)
        {
            QiComponent->ProcessQiFlow(DeltaTime, 10);
        }
    }
}


void ASovereignBaseEntity::InitializeFromSovereignData(USovereignSpeciesData* InData)
{
    // 1. Safety Check: Stop the crash if data is missing
    if (!InData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeFromSovereignData: No Data Asset provided."), *GetName());
        return;
    }

    // 2. Assign the Pointer
    SpeciesData = InData;

    // 3. Update Visuals
    // This ensures the Mesh matches the Data Asset right away
    RefreshVisuals();

    // 4. Log Success (Simplified)
    UE_LOG(LogTemp, Log, TEXT("[%s] Sovereign Entity Initialized successfully."), *GetName());
}

void ASovereignBaseEntity::RefreshVisuals()
{
    // 1. Fast Exit: Basic safety checks
    if (!EntityMesh || !SpeciesData) return;

    // 2. Index Validation
    if (!SpeciesData->GrowthStages.IsValidIndex(CurrentGrowthStage))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid Growth Stage: %d"), *GetName(), CurrentGrowthStage);
        return;
    }

    // 3. Get the Soft Pointer to the mesh
    TSoftObjectPtr<UStaticMesh> NewMeshPtr = SpeciesData->GrowthStages[CurrentGrowthStage].StageMesh;

    // 4. Optimization: Check if the mesh is already loaded and set
    // This prevents redundant logic if Evolve() is called multiple times
    if (NewMeshPtr.Get() == EntityMesh->GetStaticMesh() && !NewMeshPtr.IsNull())
    {
        return;
    }

    // 5. The "Sovereign" Load Strategy: Asynchronous Loading
    if (NewMeshPtr.IsPending())
    {
        FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
        StreamableManager.RequestAsyncLoad(NewMeshPtr.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ASovereignBaseEntity::OnMeshLoaded, NewMeshPtr));
    }
    else
    {
        // Mesh is already loaded, so just apply it.
        OnMeshLoaded(NewMeshPtr);
    }
}

void ASovereignBaseEntity::OnMeshLoaded(TSoftObjectPtr<UStaticMesh> LoadedMeshPtr)
{
    if (LoadedMeshPtr.IsValid())
    {
        EntityMesh->SetStaticMesh(LoadedMeshPtr.Get());

        // 6. Scale/Physical Logic (The "Symmetry" adjustment)
        // Adjust the scale based on the growth stage defined in the data asset
        float StageScale = SpeciesData->GrowthStages[CurrentGrowthStage].VisualScale;
        EntityMesh->SetRelativeScale3D(FVector(StageScale));

        UE_LOG(LogTemp, Log, TEXT("[%s] Visuals synced to Stage %d"), *GetName(), CurrentGrowthStage);
    }
}

void ASovereignBaseEntity::PostSpawnInitialize(const USovereignSpeciesData* InSpeciesData, const FGuid& InMotherID, const FGuid& InFatherID)
{
	if (InSpeciesData)
	{
		InitializeFromSovereignData(const_cast<USovereignSpeciesData*>(InSpeciesData));
	}

	if (SaveDataComponent)
	{
		SaveDataComponent->EntityID = FGuid::NewGuid();

        if (BioComponent)
        {
            BioComponent->MotherID = InMotherID;
            BioComponent->FatherID = InFatherID;
        }

		if (UWorld* World = GetWorld())
		{
			if (UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>())
			{
				Registry->RegisterActor(SaveDataComponent->EntityID, this);
			}
		}

		if (InFatherID.IsValid())
		{
			// This is a hybrid, so we need to recombine DNA
			if (UWorld* World = GetWorld())
			{
				if (UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>())
				{
					AActor* Mother = Registry->FindActor(InMotherID);
					AActor* Father = Registry->FindActor(InFatherID);
					if (Mother && Father)
					{
						auto* MomComp = Mother->FindComponentByClass<USovereignSaveableEntityComponent>();
						auto* DadComp = Father->FindComponentByClass<USovereignSaveableEntityComponent>();
						if (MomComp && DadComp)
						{
							TMap<FString, FString> ChildDNA = USovereignSpawnerUtils::RecombineDNA(MomComp->GetUnknownMetaTags(), DadComp->GetUnknownMetaTags(), 0.05f);
							SaveDataComponent->ApplyMetaTags(ChildDNA);

							float CurrentTime = World->GetTimeSeconds();
							
							// Access LastMatingTimestamp from Bio components instead
							auto* MomBio = Mother->FindComponentByClass<USovereignBioComponent>();
							auto* DadBio = Father->FindComponentByClass<USovereignBioComponent>();
							
							if (MomBio)
							{
								MomBio->LastMatingTimestamp = CurrentTime;
							}
							if (DadBio)
							{
								DadBio->LastMatingTimestamp = CurrentTime;
							}

							UE_LOG(LogTemp, Log, TEXT("Sovereign: Hybrid born between %s and %s!"), *Mother->GetName(), *Father->GetName());
						}
					}
				}
			}
		}
		else if (InMotherID.IsValid())
		{
			// This is a clone, so we need to mutate DNA
			if (UWorld* World = GetWorld())
			{
				if (UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>())
				{
					AActor* Mother = Registry->FindActor(InMotherID);
					if (Mother)
					{
						auto* MomComp = Mother->FindComponentByClass<USovereignSaveableEntityComponent>();
						if (MomComp)
						{
							TMap<FString, FString> MutatedDNA = USovereignSpawnerUtils::GenerateMutatedTags(MomComp->GetUnknownMetaTags(), 0.1f);
							SaveDataComponent->ApplyMetaTags(MutatedDNA);
						}
					}
				}
			}
		}
	}
}

void ASovereignBaseEntity::IngestSovereignTag(FString IncomingTagString)
{
    if (IncomingTagString.IsEmpty()) return;

    // Use the native FGameplayTag request - no Manager include needed!
    FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(FName(*IncomingTagString), false);

    if (NewTag.IsValid())
    {
        GameplayTags.AddTag(NewTag);
        UE_LOG(LogTemp, Log, TEXT("Sovereign: Ingested Tag %s"), *IncomingTagString);
    }
}



 //version 3.31
//is this ok down here?

float ASovereignBaseEntity::GetHeartbeatInterval() const
{
    switch (UpdateFrequency)
    {
    case EUpdateFrequency::Realtime: return 0.0f; // Use Tick instead
    case EUpdateFrequency::Standard: return 1.0f;
    case EUpdateFrequency::Slow:     return 10.0f;
    case EUpdateFrequency::Slower:   return 30.0f;
    case EUpdateFrequency::Slowest:  return 60.0f;
    case EUpdateFrequency::Glacier:  return 360.0f;
    case EUpdateFrequency::Dormant:  return -1.0f; // Disable timer

    // NEW TRUTHS:
    //case EUpdateFrequency::Seasonal: return 7200.0f;  // 2 Hours
    //case EUpdateFrequency::Epoch:    return 43200.0f; // 12 Hours

    default:                         return 1.0f;
    }
}

UFUNCTION(BlueprintCallable, Category = "Sovereign|Soul")
USovereignSaveableEntityComponent* ASovereignBaseEntity::GetSovereignSoul_Implementation() const
{
    // Simply return the component we already have!
    return SaveDataComponent;
}

//Put end at the bottem makes sense?
void ASovereignBaseEntity::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        UActorRegistry* Registry = World->GetSubsystem<UActorRegistry>();
        if (Registry && SaveDataComponent)
        {
            Registry->UnregisterActor(SaveDataComponent->EntityID);
        }
    }
    Super::EndPlay(EndPlayReason);
}
