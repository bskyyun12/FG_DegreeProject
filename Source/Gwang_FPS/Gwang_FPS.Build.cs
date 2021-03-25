// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gwang_FPS : ModuleRules
{
	public Gwang_FPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "PhysicsCore", "OnlineSubSystem", "OnlineSubSystemSteam", "SlateCore" });
	}
}
