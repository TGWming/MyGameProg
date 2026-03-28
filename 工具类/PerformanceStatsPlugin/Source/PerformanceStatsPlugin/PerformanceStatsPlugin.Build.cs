using UnrealBuildTool;

public class PerformanceStatsPlugin : ModuleRules
{
    public PerformanceStatsPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine",
            "DeveloperSettings"
        });
        
        // 用于获取渲染数据
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI"
        });
        
        // 仅Windows平台启用 (兼容UE4.27和UE5)
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("WITH_PERFORMANCE_STATS=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_PERFORMANCE_STATS=0");
        }
    }
}
