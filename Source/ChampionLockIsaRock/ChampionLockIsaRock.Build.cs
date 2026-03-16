// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChampionLockIsaRock : ModuleRules
{
	public ChampionLockIsaRock(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ChampionLockIsaRock",
			"ChampionLockIsaRock/Variant_Strategy",
			"ChampionLockIsaRock/Variant_Strategy/UI",
			"ChampionLockIsaRock/Variant_TwinStick",
			"ChampionLockIsaRock/Variant_TwinStick/AI",
			"ChampionLockIsaRock/Variant_TwinStick/Gameplay",
			"ChampionLockIsaRock/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
