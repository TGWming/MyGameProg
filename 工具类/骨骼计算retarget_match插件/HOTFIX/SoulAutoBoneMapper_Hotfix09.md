# 热修复 9: 修复 BoneAliasTable 中 IPluginManager 的使用方式

```
===== 热修复 9: 修复 BoneAliasTable 中 IPluginManager 的使用方式 =====

目标: 确认 IPluginManager 链接问题已通过 "Projects" 模块解决。
如果仍有问题，改用 FPaths 方式定位插件目录，避免依赖 IPluginManager

操作类型: 仅修改已有文件（约 5 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 BoneAliasTable.cpp
- 禁止删除或修改其他函数（除 LoadDefaultAliases 外）
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneAliasTable.cpp

步骤 1: 将顶部的:

#include "Interfaces/IPluginManager.h"

替换为:

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

步骤 2: 将 LoadDefaultAliases 方法整体替换为:

bool FBoneAliasTable::LoadDefaultAliases()
{
    // 方式 1：通过 IPluginManager 定位
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SoulAutoBoneMapper"));
    if (Plugin.IsValid())
    {
        const FString DefaultPath = FPaths::Combine(
            Plugin->GetBaseDir(), TEXT("Resources"), TEXT("DefaultAliases.json"));
        return LoadFromJsonFile(DefaultPath);
    }

    // 方式 2：备用路径（直接在项目 Plugins 目录下搜索）
    const FString FallbackPath = FPaths::Combine(
        FPaths::ProjectPluginsDir(),
        TEXT("SoulAutoBoneMapper"), TEXT("Resources"), TEXT("DefaultAliases.json"));

    if (FPaths::FileExists(FallbackPath))
    {
        return LoadFromJsonFile(FallbackPath);
    }

    UE_LOG(LogTemp, Warning, TEXT("BoneAliasTable: Could not find DefaultAliases.json"));
    return false;
}

完成后汇报: 确认 LoadDefaultAliases 是否已更新，
包含 IPluginManager 主路径 + FPaths 备用路径

===== 热修复 9 结束 =====
```