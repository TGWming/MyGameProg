# 任务 47: 实现 OnSavePresetClicked

```
===== 任务 47: 实现 OnSavePresetClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加保存预设回调

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

首先确认 SBoneMappingWidget.cpp 顶部已有以下 include（如果没有则添加）:

#include "DesktopPlatformModule.h"

然后在文件末尾追加以下代码:

FReply SBoneMappingWidget::OnSavePresetClicked()
{
    if (MappingEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: No mappings to save"));
        return FReply::Handled();
    }

    // 打开保存对话框
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        return FReply::Handled();
    }

    TArray<FString> SavePaths;
    const bool bSaved = DesktopPlatform->SaveFileDialog(
        FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
        TEXT("Save Mapping Preset"),
        FPaths::ProjectSavedDir(),
        TEXT("BoneMapping.json"),
        TEXT("JSON Files (*.json)|*.json"),
        0,
        SavePaths);

    if (!bSaved || SavePaths.Num() == 0)
    {
        return FReply::Handled();
    }

    // 构建预设
    FMappingPreset Preset;
    Preset.PresetName = FPaths::GetBaseFilename(SavePaths[0]);
    Preset.SourceSkeletonName = SourceSkeleton.IsValid() ? SourceSkeleton->GetName() : TEXT("Unknown");
    Preset.TargetSkeletonName = TargetSkeleton.IsValid() ? TargetSkeleton->GetName() : TEXT("Unknown");

    for (const auto& Entry : MappingEntries)
    {
        Preset.Mappings.Add(*Entry);
    }

    if (Preset.SaveToJsonFile(SavePaths[0]))
    {
        UE_LOG(LogTemp, Log, TEXT("BoneMapper: Preset saved to %s"), *SavePaths[0]);
    }

    return FReply::Handled();
}

完成后汇报: 确认 OnSavePresetClicked 是否已追加，
且新增了 #include "DesktopPlatformModule.h"（如果之前没有）

===== 任务 47 结束 =====
```