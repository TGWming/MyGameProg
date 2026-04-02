# 热修复 5: 补充 Build.cs 缺失模块依赖

```
===== 热修复 5: 补充 Build.cs 缺失模块依赖 =====

目标: 补充 URig API 和 IPluginManager 所需的模块依赖

操作类型: 仅修改已有文件（约 2 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SoulAutoBoneMapper.Build.cs，禁止修改其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

错误分析:
  1. URig::FindNode / GetNodeNum / GetNodeName → 需要 "AnimationBlueprintLibrary" 或直接链接动画模块
     URig 类定义在 AnimationCore 模块中，但其实现符号在 "Engine" 中
     实际上 URig 的链接符号在 UE4.27 中归属于引擎的 Animation 模块
     需要添加 "AnimGraph" 模块依赖

  2. IPluginManager::Get() → 需要 "Projects" 模块依赖

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/SoulAutoBoneMapper.Build.cs

在 PrivateDependencyModuleNames 数组中，在 "ContentBrowser" 之后追加:

            "AnimGraph",
            "Projects"

修改后完整的 PrivateDependencyModuleNames 数组应为:

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
            "JsonUtilities",
            "LevelEditor",
            "InputCore",
            "PropertyEditor",
            "DesktopPlatform",
            "ContentBrowser",
            "AnimGraph",
            "Projects"
        });

不要修改 PublicDependencyModuleNames 或其他代码。

完成后汇报: 确认 Build.cs 是否已更新，
PrivateDependencyModuleNames 包含 16 个模块

===== 热修复 5 结束 =====
```