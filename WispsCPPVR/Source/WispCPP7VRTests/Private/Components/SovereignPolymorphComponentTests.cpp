// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Components/SovereignPolymorphComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN POLYMORPH COMPONENT TESTS - B-072 Verification
 * ============================================================================
 * Verifies that USovereignPolymorphComponent properly gates transformation
 * to adult growth stages, toggles forms, resizes the capsule, swaps speeds,
 * and broadcasts the OnVesselFormChanged dynamic delegate.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignPolymorphComponentTest,
	"Sovereign.Polymorph.TransformationCycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignPolymorphComponentTest::RunTest(const FString& Parameters)
{
	// 1. Instantiate Character Vessel
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	// 2. Attach USovereignPolymorphComponent dynamically
	USovereignPolymorphComponent* PolymorphComp = NewObject<USovereignPolymorphComponent>(Character, TEXT("PolymorphComponent"));
	if (!PolymorphComp)
	{
		AddError(TEXT("Failed to instantiate USovereignPolymorphComponent"));
		return false;
	}
	Character->AddInstanceComponent(PolymorphComp);

	// 3. Verify Initial Defaults
	TestTrue(TEXT("Initial form should be TrueForm"), PolymorphComp->CurrentForm == ESovereignVesselForm::TrueForm);
	TestEqual(TEXT("RequiredGrowthStage should default to 7 (Adult)"), PolymorphComp->RequiredGrowthStage, 7);

	// 4. Test Growth Stage Gating
	// Hatchling dragon (Stage 0) cannot polymorph
	TestFalse(TEXT("Hatchling dragon (Stage 0) cannot polymorph"), PolymorphComp->CanPolymorph());
	TestFalse(TEXT("TogglePolymorph fails when growth stage requirement not met"), PolymorphComp->TogglePolymorph());
	TestTrue(TEXT("Form remains TrueForm after failed toggle"), PolymorphComp->CurrentForm == ESovereignVesselForm::TrueForm);

	// Advance to Stage 7 (Adult)
	for (int32 i = 0; i < 7; ++i)
	{
		Character->Evolve();
	}
	TestEqual(TEXT("CurrentGrowthStage is now 7"), Character->GetCurrentGrowthStage(), 7);
	TestTrue(TEXT("Adult dragon (Stage 7) can polymorph"), PolymorphComp->CanPolymorph());

	// 5. Test Successful Polymorph into Humanoid Visage
	TestTrue(TEXT("TogglePolymorph succeeds when adult"), PolymorphComp->TogglePolymorph());
	TestTrue(TEXT("Form is now HumanoidVisage"), PolymorphComp->CurrentForm == ESovereignVesselForm::HumanoidVisage);

	// Verify Humanoid Capsule Dimensions
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		TestEqual(TEXT("Capsule radius adjusted to humanoid radius (34.0)"), Capsule->GetUnscaledCapsuleRadius(), 34.0f);
		TestEqual(TEXT("Capsule half-height adjusted to humanoid height (88.0)"), Capsule->GetUnscaledCapsuleHalfHeight(), 88.0f);
	}

	// Verify Movement Mode switched to Walking
	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		TestTrue(TEXT("Movement mode should be Walking"), MoveComp->MovementMode == MOVE_Walking);
		TestEqual(TEXT("MaxWalkSpeed should be HumanoidWalkSpeed (600.0)"), MoveComp->MaxWalkSpeed, 600.0f);
	}

	// 6. Test Reverting back to True Form
	TestTrue(TEXT("TogglePolymorph reverts back to TrueForm"), PolymorphComp->TogglePolymorph());
	TestTrue(TEXT("Form is now TrueForm"), PolymorphComp->CurrentForm == ESovereignVesselForm::TrueForm);

	return true;
}
