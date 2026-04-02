# 任务 2: 创建 Build.cs 模块构建文件

```
===== 任务 2: 创建 Build.cs 模块构建文件 =====

目标: 创建插件的模块构建定义文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止��界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/SoulAutoBoneMapper.Build.cs 内容如下:

using UnrealBuildTool;

public class SoulAutoBoneMapper : ModuleRules
{
    public SoulAutoBoneMapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "EditorStyle",
            "UnrealEd",
            "AnimationCore",
            "Json",
            "JsonUtilities"
        });
    }
}

完成后汇报: 确认 Build.cs 文件是否已创建

===== 任务 2 结束 =====
```