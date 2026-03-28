// DebugConsolePlugin.Build.cs
using UnrealBuildTool;

public class DebugConsolePlugin : ModuleRules
{
    public DebugConsolePlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine",
            "DeveloperSettings"
        });
        
        // 编辑器模块依赖（用于编辑器事件监控）
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd"
            });
        }
        
        // 仅Windows平台启用 (兼容UE4.27和UE5)
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("WITH_DEBUG_CONSOLE=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_DEBUG_CONSOLE=0");
        }
    }
}
