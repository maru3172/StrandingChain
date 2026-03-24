// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/StrandingChain.Build.cs

using UnrealBuildTool;
using System.IO;

public class StrandingChain : ModuleRules
{
	public StrandingChain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 모듈 루트 및 서브폴더 인클루드 경로 명시 등록
		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory),
			Path.Combine(ModuleDirectory, "Battle"),
			Path.Combine(ModuleDirectory, "Battle", "Skills"),
			Path.Combine(ModuleDirectory, "UI")
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
