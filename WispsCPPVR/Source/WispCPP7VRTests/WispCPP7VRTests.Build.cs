// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

using UnrealBuildTool;

public class WispCPP7VRTests : ModuleRules
{
    public WispCPP7VRTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false; // Disable Unity builds for faster iteration on tests

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "GameplayTags",
            "TestFramework",
            "UnrealEd"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "WispCPP7VR"  // Depend on the main module
        });

        // Only include tests in Editor builds
        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.Add("TestFramework");
        }
    }
}
