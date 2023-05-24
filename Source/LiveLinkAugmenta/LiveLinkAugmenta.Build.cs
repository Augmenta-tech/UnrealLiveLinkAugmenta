// Copyright Augmenta 2023, All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class LiveLinkAugmenta : ModuleRules
{
	public LiveLinkAugmenta(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Networking",
				"Sockets",
				"LiveLinkInterface",
				"LiveLink",
				"Messaging",
			}
			);

		//Dependencies preventing Mac compatibility
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"DisplayCluster"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);

		// 3rd party dependencies
		AddThirdPartyDependencies(Target);
	}

	public void AddThirdPartyDependencies(ReadOnlyTargetRules Target) {

		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/"));

		PublicIncludePaths.Add(ThirdPartyPath);
	}
}
