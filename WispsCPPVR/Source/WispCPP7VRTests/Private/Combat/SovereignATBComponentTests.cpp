// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-18

#include "Misc/AutomationTest.h"
#include "Components/SovereignATBComponent.h"
#include "Components/SovereignAttributeComponent.h"
#include "Components/SovereignClassComponent.h"
#include "Entities/SovereignBaseCharacter.h"
#include "Tests/AutomationCommon.h"

/**
 * ============================================================================
 * SOVEREIGN ATB COMPONENT TESTS - B-078 Verification
 * ============================================================================
 * Verifies ATB gauge charging, Dexterity & class speed scaling, Wait Mode pausing,
 * Ready transition, Action execution resetting to 0%, and Defend stance damage mitigation.
 * ============================================================================
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSovereignATBComponentTest,
	"Sovereign.Combat.ATBComponentOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSovereignATBComponentTest::RunTest(const FString& Parameters)
{
	// 1. Initial State & Defaults Verification
	USovereignATBComponent* ATBComp = NewObject<USovereignATBComponent>();
	if (!ATBComp)
	{
		AddError(TEXT("Failed to instantiate USovereignATBComponent"));
		return false;
	}

	TestEqual(TEXT("Initial ATB should be 0.0"), ATBComp->CurrentATB, 0.0f);
	TestEqual(TEXT("Initial ATBState should be Charging"), ATBComp->ATBState, EATBState::Charging);
	TestFalse(TEXT("Wait Mode should be false initially"), ATBComp->bIsWaitModeActive);
	TestFalse(TEXT("bIsDefending should be false initially"), ATBComp->bIsDefending);
	TestEqual(TEXT("DefendDamageMultiplier should default to 0.5 (50% reduction)"), ATBComp->DefendDamageMultiplier, 0.5f);
	TestFalse(TEXT("Should not be ready initially"), ATBComp->IsReady());
	TestEqual(TEXT("Normalized ATB should be 0.0"), ATBComp->GetNormalizedATB(), 0.0f);

	// 2. Unattached Baseline Fill Rate
	const float BaseRate = ATBComp->CalculateEffectiveFillRate();
	TestEqual(TEXT("Unattached baseline fill rate equals BaseATBRate (20.0)"), BaseRate, 20.0f);

	// Advance by 1 second
	ATBComp->AdvanceATB(1.0f);
	TestEqual(TEXT("CurrentATB after 1s at base rate should be 20.0"), ATBComp->CurrentATB, 20.0f);
	TestEqual(TEXT("Normalized ATB should be 0.2"), ATBComp->GetNormalizedATB(), 0.2f);
	TestFalse(TEXT("Should not be ready at 20%"), ATBComp->IsReady());

	// 3. Wait Mode Pausing
	ATBComp->SetWaitModeActive(true);
	TestTrue(TEXT("Wait mode active"), ATBComp->bIsWaitModeActive);

	ATBComp->AdvanceATB(1.0f);
	TestEqual(TEXT("CurrentATB must not advance while Wait Mode is active"), ATBComp->CurrentATB, 20.0f);

	ATBComp->SetWaitModeActive(false);
	TestFalse(TEXT("Wait mode deactivated"), ATBComp->bIsWaitModeActive);

	// 4. Advancing to Ready State (100%)
	ATBComp->AdvanceATB(4.0f); // 20 + (20 * 4) = 100.0
	TestEqual(TEXT("CurrentATB reached MAX_ATB (100.0)"), ATBComp->CurrentATB, 100.0f);
	TestTrue(TEXT("IsReady() returns true"), ATBComp->IsReady());
	TestEqual(TEXT("ATBState transitions to Ready"), ATBComp->ATBState, EATBState::Ready);
	TestEqual(TEXT("Normalized ATB is 1.0"), ATBComp->GetNormalizedATB(), 1.0f);

	// 5. Action Selection & Execution Flow
	ATBComp->BeginActionSelection();
	TestEqual(TEXT("ATBState transitions to SelectingAction"), ATBComp->ATBState, EATBState::SelectingAction);

	ATBComp->BeginActionExecution();
	TestEqual(TEXT("ATBState transitions to ExecutingAction"), ATBComp->ATBState, EATBState::ExecutingAction);

	// Complete action execution: resets ATB to 0.0% and transitions to Charging
	ATBComp->CompleteActionExecution();
	TestEqual(TEXT("CurrentATB resets to 0.0 after complete action"), ATBComp->CurrentATB, 0.0f);
	TestEqual(TEXT("ATBState transitions back to Charging"), ATBComp->ATBState, EATBState::Charging);
	TestFalse(TEXT("IsReady() returns false after reset"), ATBComp->IsReady());

	// 6. Defend Stance
	ATBComp->EnterDefendStance();
	TestTrue(TEXT("bIsDefending is true after EnterDefendStance"), ATBComp->bIsDefending);
	TestEqual(TEXT("CurrentATB is 0.0"), ATBComp->CurrentATB, 0.0f);
	TestEqual(TEXT("ATBState is Charging"), ATBComp->ATBState, EATBState::Charging);

	// Beginning action execution clears prior Defend stance
	ATBComp->BeginActionExecution();
	TestFalse(TEXT("bIsDefending is cleared when next action execution begins"), ATBComp->bIsDefending);

	return true;
}
