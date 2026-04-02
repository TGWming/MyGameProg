# 任务 49: 补充 Build.cs — DesktopPlatform 依赖

```
===== 任务 49: 补充 Build.cs — DesktopPlatform 依赖 =====

目标: 在 Build.cs 中补充保存/加载对话框所需的 DesktopPlatform 模块依赖

操作类型: 仅修改已有文件（约 1 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SoulAutoBoneMapper.Build.cs，禁止修改其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/SoulAutoBoneMapper.Build.cs

在 PrivateDependencyModuleNames 数组中，在 "PropertyEditor" 之后追加:

            "DesktopPlatform"

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
            "DesktopPlatform"
        });

不要修改 PublicDependencyModuleNames 或其他代码。

完成后汇报: 确认 Build.cs 是否已更新，
PrivateDependencyModuleNames 包含 13 个模块

===== 任务 49 结束 =====
```