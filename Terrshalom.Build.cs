// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Terrshalom : ModuleRules
{
	public Terrshalom(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "ALSV4_CPP", "Core", "CoreUObject", "Engine", "EnhancedInput", "InputCore", "RealtimeMeshComponent", "ProceduralMeshComponent" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

    }
}
