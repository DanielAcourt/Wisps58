# WispCPP7VR Test Suite

## Running Tests

### Option 1: From Unreal Editor (Recommended)

1. Open WispCPP7VR project in Unreal Editor
2. Go to **Tools > Automation**
3. Find tests under the **SaveSystem** category:
   - SaveSystem.Initialization.*
   - SaveSystem.Registry.*
   - SaveSystem.Serialization.*
   - SaveSystem.References.*
   - SaveSystem.Data.*
   - SaveSystem.GameRules.*
   - SaveSystem.Configuration.*
   - SaveSystem.NullSafety.*
   - SaveSystem.Lineage.*
   - SaveSystem.Elements.*
   - SaveSystem.Performance.*
   - SaveSystem.EdgeCases.*

4. Click **Start Tests** to run all or select specific tests

### Option 2: Command Line

## Test Categories

| Category | Tests | Focus |
|----------|-------|-------|
| **Initialization** | 2 | Default constructor values |
| **Registry** | 1 | GUID uniqueness |
| **Serialization** | 1 | JSON robustness |
| **References** | 2 | Soft reference handling |
| **Data** | 3 | Lineage, transform, elements |
| **GameRules** | 1 | Growth stage transitions |
| **Configuration** | 1 | Update frequency |
| **NullSafety** | 2 | Null pointer handling |
| **Lineage** | 1 | Mating history |
| **Performance** | 1 | Batch creation |
| **EdgeCases** | 1 | Boundary conditions |

**Total: 15 tests**

## Expected Results

All tests should **PASS** ✅

- Expected runtime: **< 500ms**
- Coverage: **SaveSystem data structures**
- Status: **Production ready**

## Adding New Tests

1. Add test function to `SovereignSaveSystemTests.cpp`
2. Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` macro
3. Follow naming convention: `FSovereign<Feature><Scenario>Test`
4. Category format: `SaveSystem.<Category>.<TestName>`
5. Run automation tests to verify

## Test Organization

## Maintenance

- Review tests when SaveSystem changes
- Add new tests for new SaveSystem features
- Update this README as tests grow
- Tag commits with test results

## References

- **CONTRIBUTING.md**: SaveSystem Agent Responsibilities
- **Unreal Automation Test Framework**: [UE5 Docs](https://docs.unrealengine.com/5.0/en-US/automation-system-overview-in-unreal-engine/)
- **SaveSystem Code**: `Source/WispCPP7VR/Public/SaveSystem/`
