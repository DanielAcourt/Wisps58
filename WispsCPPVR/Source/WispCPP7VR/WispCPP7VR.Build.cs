// Copyright (c) 2013-2025 Daniel Acourt. All Rights Reserved. Confidential & Proprietary.

using UnrealBuildTool;

public class WispCPP7VR : ModuleRules
{
    public WispCPP7VR(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "HTTP",
            "Json",
            "JsonUtilities",
            "GameplayTags",
            "GameplayTasks",
            "Niagara",
            "NavigationSystem",
            "AIModule",
            "UMG"
        });

        // Enable automation tests
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "FunctionalTesting",
                "AutomationTest"
            });
        }

        // Modules that should ONLY be included when building for the Editor
        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "AssetTools",
                    "UnrealEd",
                    "TestFramework"  // Add testing framework for editor
				}
            );
        }
    }
}
