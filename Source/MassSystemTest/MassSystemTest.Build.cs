// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MassSystemTest : ModuleRules
{
	public MassSystemTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayTags","MassEntity", 
			"MassCommon", "MassNavigation", "MassMovement", "NavigationSystem",
            "AIModule", "MassAIBehavior", "StateTreeModule", "GameplayTags", "SmartObjectsModule", "MassSmartObjects", "MassSignals",
            "MassRepresentation", "MassLOD", "WorldResources" });

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"GameplayTags" 
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
