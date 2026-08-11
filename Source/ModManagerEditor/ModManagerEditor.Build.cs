using UnrealBuildTool;

public class ModManagerEditor : ModuleRules
{
    public ModManagerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "PluginBrowser",
                "UnrealEd",
                "Projects"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "UATHelper", 
                // TODO : Main frame is need in 4.27
                "MainFrame",
                "DesktopPlatform"
            }
        );
    }
}