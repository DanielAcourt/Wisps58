// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/SovereignBaseCharacter.h"

//Core Stats And attritbutes
#include "Components/SovereignElementComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignControllerComponent.h"
#include "Components/SovereignQiComponent.h"

//Entinty framework to save information
#include "Entities/SovereignSaveableEntityComponent.h"
#include "Entities/SovereignPlayerWisp.h" //hard coded reference for now

//The ability for Character to move
#include "GameFramework/Character.h" // 
#include "GameFramework/CharacterMovementComponent.h"//Core Unreal Character frameworks

//The ability for characters to receive input
#include "EnhancedInputComponent.h" //core unreal input libraries
#include "EnhancedInputSubsystems.h" // You'll likely need this for the Mapping Context too


ASovereignBaseCharacter::ASovereignBaseCharacter()
{
	// 1. CONSTRUCT THE HYBRID STACK
	// Naming here now matches the header exactly
	ElementComponent = CreateDefaultSubobject<USovereignElementComponent>(TEXT("ElementComponent"));
	ControlComponent = CreateDefaultSubobject<USovereignControllerComponent>(TEXT("ControlComponent"));
	AttributeComponent = CreateDefaultSubobject<USovereignAttributeComponent>(TEXT("AttributeComponent"));
	QiComponent = CreateDefaultSubobject<USovereignQiComponent>(TEXT("QiComponent"));

	// 2. CONFIGURE CHARACTER DEFAULTS
	bUseControllerRotationYaw = false;
}
void ASovereignBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Only run this if we are controlled by a Human (Local Player)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (DefaultMappingContext)
				{
					// Clear existing to prevent double-mapping, then add
					Subsystem->RemoveMappingContext(DefaultMappingContext);
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
					//UE_LOG(LogTemp, Log, TEXT("Sovereign: [%s] Input Mapping Context Active."), *GetName());
				}
			}
		}
	}
}

void ASovereignBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 1. COMPONENT INITIALIZATION
	if (ControlComponent)
	{
		ControlComponent->OnPossessed(NewController);
	}

	// 2. PLAYER-SPECIFIC SETUP (Input & UI)
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		// Force the Input System to use this Character's keys (F for Eject/Interact)
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Clear the Wisp's old context and add the Vessel's context
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("Sovereign: Input Context swapped to %s"), *GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Sovereign: %s has NO DefaultMappingContext assigned!"), *GetName());
			}
		}

		if (AttributeComponent)
		{
			UE_LOG(LogTemp, Log, TEXT("Sovereign: %s is now Player-Controlled."), *GetName());
		}
	}
}

void ASovereignBaseCharacter::UnPossessed()
{

	// 1. RUN CUSTOM LOGIC FIRST
	// The Controller is still valid here, so your component can 
	// save final stats or notify the AI it needs to take back over.

	if (ControlComponent)
	{
		ControlComponent->OnUnpossessed();
	}

	// 2. LOG THE EVENT
	UE_LOG(LogTemp, Log, TEXT("Sovereign: %s is no longer possessed."), *GetName());

	// 3. HAND OVER TO ENGINE
// This clears the Pawn's internal Controller pointer.
	Super::UnPossessed(); // THIS WAS HERE NEED IT FIRST?
}

//Setting Up the Possession Attacthment Component
USceneComponent* ASovereignBaseCharacter::GetPossessionAttachmentComponent_Implementation()
{
	return GetMesh();
}
void ASovereignBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move & Look: Keep these as 'Triggered' for smooth continuous movement
		if (MoveAction) EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASovereignBaseCharacter::Move);
		if (LookAction) EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASovereignBaseCharacter::Look);

		// Interact: Use 'Started' to ensure the Eject/Possess happens exactly ONCE per tap
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ASovereignBaseCharacter::Interact);
		}

		// Handle the Possession lifecycle (F key)
		if (PossessAction)
		{
			EIC->BindAction(PossessAction, ETriggerEvent::Started, this, &ASovereignBaseCharacter::HandlePossessionLifecycle);
		}
	}
}

//How to move and look as a input controller not an AI instruction!!!
void ASovereignBaseCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	// 1. Get the Raw Input (-1.0 to 1.0 for each axis)
	const FVector Input = Value.Get<FVector>();

	// 2. DEBUG FEEDBACK (The Truth Teller for movement input)
	/*	if (GEngine)
	{
		// This prints the raw numbers from your IMC to the screen
		//FString DebugMsg = FString::Printf(TEXT("Input -> Forward/Back(Y): %.2f | Right/Left(X): %.2f | Up/Down(Z): %.2f"),Input.Y, Input.X, Input.Z);
		//GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, DebugMsg);
	}
	*/

	// 3. Get Camera Directions
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 4. The Movement Logic
	// Distance = speed. If 5 is very slow ,15 normal too slow, bump it to 30 for fast or 50 for medium.
	const float Distance = 5.0f;

	FVector MoveDelta = (Forward * Input.Y * Distance) +
		(Right * Input.X * Distance) +
		(FVector::UpVector * Input.Z * Distance);

	// 5. Apply Position Change
	// Sweep = true ensures you hit walls instead of going through them
	AddActorWorldOffset(MoveDelta, true);

	// 6. Kill Physics Interference
	// This prevents the character movement component from adding its own friction/drift
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
}
void ASovereignBaseCharacter::Look(const FInputActionValue& Value)
{
	// 1. Extract the 2D Axis data (X = Mouse X / Yaw, Y = Mouse Y / Pitch)
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 2. Apply the input to the Controller
		// These functions are built into APawn and automatically handle sensitivity
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		// 3. 1% POLISH: If you are in VR, usually the HMD handles this. 
		// For desktop/Spectator mode, this rotates the camera boom/view.
	}
}

//v2.7
bool ASovereignBaseCharacter::IsPossessing()
{
	return false; // The Character is a Vessel, not a Spirit
}

//INTERACTION
//v2.6 Current

void ASovereignBaseCharacter::Interact(const FInputActionValue& Value)
{
	// DEBUG: If you don't see this on screen, your Input Binding is broken
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("VESSEL: INTERACT TRIGGERED"));

	// --- SOVEREIGN UPGRADE: E is only for contextual interaction ---

	// THE EXTERNAL CHECK (Normal world interaction)
	AActor* Target = GetSensedActor();
	if (Target)
	{
		// Log for Debugging
		UE_LOG(LogTemp, Log, TEXT("Sovereign: Vessel %s sensed target %s"), *GetName(), *Target->GetName());

		// Broadcast to Blueprints (for UI/Highlighters)
		OnActorSensed.Broadcast(Target);

		// Execute interaction if target speaks the Sovereign language
		if (Target->Implements<UInteractionInterface>())
		{
			IInteractionInterface::Execute_OnInteract(Target, this);
		}
	}
}

void ASovereignBaseCharacter::HandlePossessionLifecycle()
{
	// 1. THE INTERNAL CHECK (Is there a soul in here?)
	// We use the interface to check for attached spirits.
	AActor* Spirit = IInteractionInterface::Execute_GetInhabitingSpirit(this);

	if (Spirit)
	{
		// 2. THE EJECT COMMAND
		// We cast to the Wisp to trigger its high-level EjectFromHost logic.
		ASovereignPlayerWisp* Wisp = Cast<ASovereignPlayerWisp>(Spirit);
		if (Wisp)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sovereign: Soul Eject initiated by vessel %s"), *GetName());
			Wisp->EjectFromHost();
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Sovereign: %s is not inhabited by a soul, but 'F' was pressed."), *GetName());
}

void ASovereignBaseCharacter::OnInteract_Implementation(AActor* Interactor)
{
	// For now, we can just log that a character-based entity was touched.
	// In the future, Erisis might say "Hello" here.
	UE_LOG(LogTemp, Log, TEXT("%s interacted with Sovereign Character %s"),	Interactor ? *Interactor->GetName() : TEXT("Unknown"), *GetName());
}

//Sensing ANother Actor

AActor* ASovereignBaseCharacter::GetSensedActor()
{
	if (!Controller) return nullptr;

	FVector Start;
	FRotator Rot;
	Controller->GetPlayerViewPoint(Start, Rot);

	const float Range = 500.0f;
	const float Radius = 25.0f;
	FVector End = Start + (Rot.Vector() * Range);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Perform the thick raycast (Sphere Trace)
	bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params
	);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	// --- NEW DEBUG & BLUEPRINT LOGIC ---
	if (HitActor)
	{
		// 1. Logs to the Output Log (Window -> Output Log)
		//UE_LOG(LogTemp, Warning, TEXT("Sovereign Sense: Hit Actor [%s] at Location [%s]"), *HitActor->GetName(), *Hit.Location.ToString());

		// 2. Fires the Blueprint Event
		OnActorSensed.Broadcast(HitActor);
	}

#if !UE_BUILD_SHIPPING
	//DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);

	if (bHit)
	{
		// Draw a solid sphere where the hit actually occurred
		//DrawDebugSphere(GetWorld(), Hit.Location, Radius, 12, FColor::Green, false, 1.0f);
	}
#endif

	return HitActor;
}

//It compilies i am confused with this
//v2.5 
AActor* ASovereignBaseCharacter::GetInhabitingSpirit_Implementation()
{
	// Search ALL actors attached to us, including children of children
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true); // The 'true' is critical for deep search

	for (AActor* Actor : AttachedActors)
	{
		// Does this attached thing (the Wisp) speak the Sovereign language?
		if (Actor && Actor->Implements<UInteractionInterface>())
		{
			// We ask the actor: "Are you a Spirit/Soul?"
			// You should define this in your Wisp's .cpp to return 'true'.
			if (IInteractionInterface::Execute_IsSpiritEntity(Actor))
			{
				return Actor;
			}
		}
	}

	// FALLBACK: If the deep search failed, check specifically for the Wisp class
	// (This requires #include "SovereignPlayerWisp.h")
	for (AActor* Actor : AttachedActors)
	{
		if (Actor && Actor->IsA(ASovereignPlayerWisp::StaticClass()))
		{
			return Actor;
		}
	}
	return nullptr;
}


//BE CAREFUL USING TICKS//
//THERE ARE LOTS OF CHARACTERS IN A SCENE SO IF WE USE TOO MANY WE ARE BUGGARED CPU WISE
void ASovereignBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Call this every frame so the Blueprint Event and Log fire constantly
	GetSensedActor();
}

//v3.2
/*

Ideally what we want for the base character is that it has a gate somehow so it doesnt do this tick unless it in controlled by the player or

it make a request


void ASovereignBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only perform the expensive sphere trace if a Human Player is currently controlling this body
	if (IsLocallyControlled() && IsPlayerController())
	{
		GetSensedActor();
	}
}*/


// Leveling up or evolving both in stats and visuals
void ASovereignBaseCharacter::Evolve()
{
	// Base character evolution logic (e.g., check for enough Qi)
	UE_LOG(LogTemp, Log, TEXT("Base Character Evolving..."));
}
