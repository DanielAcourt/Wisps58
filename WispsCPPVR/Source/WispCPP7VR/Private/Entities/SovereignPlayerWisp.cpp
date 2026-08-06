// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/SovereignPlayerWisp.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignQiComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"

#include "Kismet/KismetSystemLibrary.h" // <--- CRITICAL FOR THE TRACE
#include "Components/WidgetComponent.h" // Add this include!

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

//DO i need these 2 in the wisp aswell?
#include "Interaction/SovereignInterfaceMain.h"
#include "Entities/SovereignSaveableEntityComponent.h"

//Visuals
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"

//Default Starting Logic
ASovereignPlayerWisp::ASovereignPlayerWisp()
{
	// Force the Movement Component to link to your Capsule
	GetCharacterMovement()->SetUpdatedComponent(RootComponent);

	// 1. Setup the Visuals
	SpiritEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SpiritEffect"));
	SpiritEffect->SetupAttachment(RootComponent);

	// 2. Flying Movement (Spirit style)
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;

	// Ensure we are in Flying mode immediately
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// The 1% Architect values for a snappy spirit
	GetCharacterMovement()->MaxAcceleration = 10000.0f; // Extremely high to bypass friction
	GetCharacterMovement()->MaxFlySpeed = 1200.0f;
	GetCharacterMovement()->BrakingDecelerationFlying = 2000.0f;

	// Ghostly settings
	GetCharacterMovement()->bCheatFlying = true;
	GetCharacterMovement()->GravityScale = 0.0f;


	// 3. Ghostly Physics
	ConfigureSpiritPhysics();
}

void ASovereignPlayerWisp::BeginPlay()
{
	Super::BeginPlay();

	//PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = false; // Stay dark until needed

	// Initialize the Wisp with "Spirit" stats
	// We access these from the parent ASovereignBaseCharacter
	if (AttributeComponent)
	{
		//AttributeComponent->Strength = 1;      // Spirit cannot lift rocks
		//AttributeComponent->Constitution = 5;  // Spirit is fragile if hit by magic
		//AttributeComponent->Intelligence = 25; // Spirit is pure mind

		// Wisps are eternal energy; they don't hunger for meat
		//AttributeComponent->Hunger = 100.0f;
	}

	if (QiComponent)
	{
		// A Wisp is literally made of Qi
		QiComponent->MaxQiCapacity = 500.0f;
		QiComponent->CurrentQi = 500.0f;
		QiComponent->QiPurity = 0.8f; // Very pure by default
	}

	// This makes the Wisp an 'active listener' even when it's just an attached child
	EnableInput(GetWorld()->GetFirstPlayerController());
}

void ASovereignPlayerWisp::ConfigureSpiritPhysics()
{
	// Ensure the capsule is physically active but ghostly
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// SPIRIT RULES: Pass through characters/items, but stop at walls/floors
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// The Soul is weightless
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = 0.0f;
		// Ensure flying mode is active so we don't just 'fall' slowly
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
}

void ASovereignPlayerWisp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. PERSISTENT SOUL VISUALIZATION
	// If we are attached to something, we are "Possessing" it.
	if (GetRootComponent()->GetAttachParent())
	{
		// Draw a cyan "Soul Capsule" to show where the Wisp is sitting
		DrawDebugCapsule(
			GetWorld(),
			GetActorLocation(),
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			GetCapsuleComponent()->GetScaledCapsuleRadius(),
			GetActorQuat(),
			FColor::Cyan,
			false,   // Persistent? No, we draw it every frame
			-1.0f,   // LifeTime ( -1 = 1 frame)
			0,       // Depth priority
			1.5f     // Thickness
		);
	}

	// 2. METABOLISM (Existing Qi Drain)
	if (QiComponent)
	{
		float CurrentDrain = (GetRootComponent()->GetAttachParent()) ? (QiDrainRate * 0.5f) : QiDrainRate;
		QiComponent->SpendQi(CurrentDrain * DeltaTime);
	}
}

FVector ASovereignPlayerWisp::GetSpiritForwardVector() const
{
	// Priority 1: Use Control Rotation (Player's Eyes/Headset)
	if (GetController())
	{
		return GetControlRotation().Vector();
	}

	// Fallback: Use the Actor's physical forward if no controller is found
	return GetActorForwardVector();
}

//Sensing Logic
void ASovereignPlayerWisp::TogglAutoSensing(bool bNewState)
{
	bIsAutoSensingEnabled = bNewState;

	if (bIsAutoSensingEnabled)
	{
		// 1.0f = The loop runs exactly once every second.
				// true = It will keep repeating.
		GetWorldTimerManager().SetTimer(SensingTimerHandle, this, &ASovereignPlayerWisp::UpdateSensingLoop, 2.0f, true);
	}
	else
	{
		// Stop the loop and the drawing
		GetWorldTimerManager().ClearTimer(SensingTimerHandle);
	}

	// Feedback for the player
	UE_LOG(LogTemp, Log, TEXT("Wisp Auto-Sensing: %s"), bNewState ? TEXT("ACTIVE") : TEXT("OFF"));
}
void ASovereignPlayerWisp::UpdateSensingLoop()
{
	if (!GetWorld()) return;

	FVector Start = GetActorLocation();
	FVector End = Start + (GetControlRotation().Vector() * InteractionDistance);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	AActor* CurrentActor = nullptr;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)) //LR ECC_Camera Or ECC Pawn
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UInteractionInterface>())
		{
			CurrentActor = HitActor;
		}
	}
}

//Version X Old idea but kinda of want to refine for later
/*
void ASovereignPlayerWisp::PerformAutoSensing()
{
	FVector Start = GetActorLocation();
	FVector End = Start + (GetControlRotation().Vector() * InteractionDistance);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UInteractionInterface>())
		{
			// Tell the object it is being looked at (for UI/Glow effects)
			IInteractionInterface::Execute_OnBeginHover(HitActor);

			// Draw a small "focus" point
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Cyan, false, 0.1f);
		}
	}
}
*/

void ASovereignPlayerWisp::HandlePossessionLifecycle()
{
	// --- SOVEREIGN CHECK: THE TOGGLE LOGIC ---
	if (bIsPossessing)
	{
		UE_LOG(LogTemp, Log, TEXT("Sovereign: Wisp detected active possession. Initiating Eject."));
		EjectFromHost();
	}
	else
	{
		AttemptPossession();
	}
}

void ASovereignPlayerWisp::AttemptPossession()
{
	// --- STEP 1: TARGETING ---
	FVector Start = GetActorLocation();
	FVector ViewDir = GetSpiritForwardVector();
	FVector End = Start + (ViewDir * InteractionDistance);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	FHitResult OutHit;

	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		GetWorld(), Start, End, PossessionTraceRadius, ObjectTypes, false,
		ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true,
		FLinearColor::Red, FLinearColor::Green, 2.0f
	);

	// --- STEP 2: VALIDATION ---
	if (bHit && OutHit.GetActor())
	{
		AActor* HitActor = OutHit.GetActor();

		if (HitActor->Implements<UInteractionInterface>())
		{
			if (IInteractionInterface::Execute_CanBePossessed(HitActor))
			{
				APlayerController* PC = Cast<APlayerController>(GetController());

				// --- STEP 3: STATE UPDATE (The Sovereign Upgrade) ---
				// We set these BEFORE attachment to ensure the state is locked in.
				bIsPossessing = true;
				CurrentHost = HitActor;

				// --- STEP 4: PHYSICAL ATTACHMENT ---
				if (USceneComponent* AttachmentComponent = IInteractionInterface::Execute_GetPossessionAttachmentComponent(HitActor))
				{
					this->AttachToComponent(AttachmentComponent,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,
						FName("Soul_Socket"));
				}

				// --- STEP 5: SOUL HANDOVER ---
				if (PC && HitActor->IsA<APawn>())
				{
					PC->Possess(Cast<APawn>(HitActor));
					UE_LOG(LogTemp, Warning, TEXT("Soul migrated to Host: %s"), *HitActor->GetName());
				}

				// --- STEP 6: SPIRIT STATE MANAGEMENT ---
				SetActorHiddenInGame(true);

				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

				SetActorTickEnabled(true);

				// Final handshake through the interface
				IInteractionInterface::Execute_RequestPossession(HitActor, PC);
			}
		}
	}
}

bool ASovereignPlayerWisp::IsPossessing()
{
	return bIsPossessing;
}

//Inital thought by Dan "Need a way to unpossess if i am currently posessing something"
//We need a way to possess and control that possessed object.
//We need a way to unposess by pressing F aka the pocesskey

void ASovereignPlayerWisp::EjectFromHost()
{

	// 0. Safety Check
	if (!bIsPossessing || !IsValid(CurrentHost))
	{
		return;
	}

	// 1. ARCHITECT CHECK: Safety first
	if (!bIsPossessing || !CurrentHost)
	{
		UE_LOG(LogTemp, Log, TEXT("Sovereign: Eject ignored. Wisp is not currently inhabiting a vessel."));
		return;
	}

	// 1. Unhide and Enable Physics FIRST
	SetActorHiddenInGame(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetActorTickEnabled(true);

	// 2. IDENTITY: Cache the host before we break the bond
	APawn* HostPawn = Cast<APawn>(CurrentHost);
	APlayerController* PC = nullptr;

	if (HostPawn)
	{
		PC = Cast<APlayerController>(HostPawn->GetController());
	}

	// B-039: Fallback to the Wisp's own controller if the Host Pawn has no controller (e.g. for Interactables)
	if (!PC)
	{
		PC = Cast<APlayerController>(GetController());
	}

	// 3. THE GREAT DETACHMENT
	// Break physical parent-child link while keeping the Wisp's current world coordinates.

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 4. Force the Handover
	if (PC)
	{
		// Move the Wisp to a safe spot so it's not inside the character's collision
		AddActorWorldOffset(FVector(0, 0, 80.f));

		// This is the magic line that actually moves your camera/controls
		PC->Possess(this);

		UE_LOG(LogTemp, Warning, TEXT("Sovereign: Soul successfully Ejected."));
	}

	//4.a FallBack

	else
	{
		UE_LOG(LogTemp, Error, TEXT("Sovereign: Eject failed - No Controller found on Host!"));
	}

	// 5. Cleanup
	bIsPossessing = false;
	CurrentHost = nullptr;
}

//Evolving and leveling logic
void ASovereignPlayerWisp::Evolve()
{
	// Call the parent Evolve first (if it has logic)
	Super::Evolve();

	// Wisp-specific evolution logic (e.g., change particle color)
	UE_LOG(LogTemp, Warning, TEXT("The Player Spirit is evolving to a higher density!"));
}

//make sure we have the ability to recieve input
void ASovereignPlayerWisp::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 1. ARCHITECT FOUNDATION: Call the Base version.
	// This ensures the SovereignBaseCharacter handles movement (WASD/Thumbstick),
	// contextual interaction ('E'), and the Possession lifecycle ('F').
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 2. CAST TO ENHANCED INPUT: 
	// We cast the generic UInputComponent to UEnhancedInputComponent to use the modern input system.
	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Note: All core bindings (Move, Look, Interact, Possess) are now
		// centralized in ASovereignBaseCharacter for consistent behavior across all entities.
	}
}


// Interaction Logic: Sovereign Soul Edition v2.5 (Kind of an unstable work in progress)
void ASovereignPlayerWisp::Interact(const FInputActionValue& Value)
{
	// 1. ARCHITECT TRUTH: Always respect the inheritance chain.
	Super::Interact(Value);

	if (!GetWorld()) return;

	// 2. PRECISION VECTOR: Trace from where the player is LOOKING.
	FVector Start = GetActorLocation();
	FVector ViewDir = GetControlRotation().Vector();
	FVector End = Start + (ViewDir * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // The Wisp is a ghost; it shouldn't hit itself.

	// 3. THE SINGLE TRACE REQUEST
	// We only perform the physics check ONCE for efficiency.
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();

		// 4. THE SOVEREIGN HANDSHAKE
		// We only care if the object speaks our Interface language.
		if (HitActor->Implements<UInteractionInterface>())
		{
			// Fetch the "Simulated Truth" hub (The Sovereign Soul)
			USovereignSaveableEntityComponent* Soul = IInteractionInterface::Execute_GetSovereignSoul(HitActor);

			if (Soul)
			{
				// Isla's Logic: Accessing metadata dynamically (Unknown Tags)
				TMap<FString, FString> Metadata = Soul->GetUnknownMetaTags();

				// Example: Identity Check for UI/Logs
				if (Metadata.Contains("Identity.Species"))
				{
					UE_LOG(LogTemp, Log, TEXT("Wisp identified species: %s"), *Metadata["Identity.Species"]);
				}
			}

			// 5. TRIGGER ACTION
			// This tells the Target (like Erisis) that the Wisp is interacting/possessing.
			IInteractionInterface::Execute_OnInteract(HitActor, this);

			// FEEDBACK: Cyan is the color of the Spirit
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 12.f, 12, FColor::Cyan, false, 2.0f);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
					FString::Printf(TEXT("Sovereign Target: %s"), *HitActor->GetName()));
			}
		}
		else
		{
			// MUNDANE OBJECT: Red feedback for non-interactable surfaces
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 8.f, 8, FColor::Red, false, 1.0f);
		}
	}
	else
	{
		// TOTAL MISS: Show the player the line of their spiritual reach
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 0.5f);
	}
}

// Ensure your bridge function looks like this:
void ASovereignPlayerWisp::EnhancedInteract(const FInputActionValue& Value)
{
	Interact(Value); // This now links correctly to the function above
}
