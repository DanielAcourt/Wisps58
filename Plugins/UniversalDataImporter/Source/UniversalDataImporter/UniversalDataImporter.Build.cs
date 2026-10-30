// Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.

using UnrealBuildTool;

public class UniversalDataImporter : ModuleRules
{
	public UniversalDataImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"AssetTools",
				"EditorScriptingUtilities",
				"Slate",
				"SlateCore"
			}
		);
	}
}
