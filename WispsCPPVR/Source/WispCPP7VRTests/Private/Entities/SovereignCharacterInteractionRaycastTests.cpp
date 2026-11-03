// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07

#include "Misc/AutomationTest.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN CHARACTER INTERACTION RAYCAST TESTS - B-071 Verification
 * ============================================================================
 * Verifies that ASovereignBaseCharacter interaction distance dynamically adapts
 * to vessel scale, capsule bounds, and provides visual feedback flags.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignCharacterInteractionRaycastTest,
	"Sovereign.Interaction.VesselRaycast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignCharacterInteractionRaycastTest::RunTest(const FString& Parameters)
{
	ASovereignBaseCharacter* Character = NewObject<ASovereignBaseCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to instantiate ASovereignBaseCharacter"));
		return false;
	}

	// 1. Verify default interaction properties
	TestEqual(TEXT("Default BaseInteractionRange should be 500u"), Character->BaseInteractionRange, 500.0f);
	TestEqual(TEXT("Default InteractionRadius should be 30u"), Character->InteractionRadius, 30.0f);
	TestTrue(TEXT("Default bShowDebugInteractionRaycast should be true"), Character->bShowDebugInteractionRaycast);

	// 2. Test base unscaled range
	TestEqual(TEXT("Scaled range on standard size should match base"), Character->GetScaledInteractionRange(), 500.0f);

	// 3. Test scale-adaptive scaling via Mesh RelativeScale3D (e.g., Large Dragon at 2.5x scale)
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		Mesh->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
		float ScaledRange = Character->GetScaledInteractionRange();
		TestEqual(TEXT("Scaled range for 2.5x Dragon mesh should be 1250u"), ScaledRange, 1250.0f);
	}

	// 4. Test scale-adaptive scaling via large capsule bounds
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		// Reset mesh scale to 1.0
		if (Character->GetMesh())
		{
			Character->GetMesh()->SetRelativeScale3D(FVector(1.0f));
		}

		Capsule->SetCapsuleSize(60.0f, 176.0f); // 2x standard height (176 / 88 = 2.0)
		float ScaledCapsuleRange = Character->GetScaledInteractionRange();
		TestEqual(TEXT("Scaled range for 2x height capsule should be 1000u"), ScaledCapsuleRange, 1000.0f);
	}

	return true;
}
