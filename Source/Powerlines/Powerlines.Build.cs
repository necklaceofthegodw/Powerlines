// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Powerlines : ModuleRules
{
	public Powerlines(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" , "InteractiveToolsFramework",
			"EditorInteractiveToolsFramework",
			"ScriptableToolsFramework", "CableComponent"
		});
	}
}
