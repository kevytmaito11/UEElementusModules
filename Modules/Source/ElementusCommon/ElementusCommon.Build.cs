// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

using UnrealBuildTool;

public class ElementusCommon : ModuleRules
{
    public ElementusCommon(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "GameplayTags"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject",
            "DeveloperSettings"
        });

        SetupIrisSupport(Target);
    }
}