# 任务 48: 实现 OnLoadPresetClicked

```
===== 任务 48: 实现 OnLoadPresetClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加加载预设回调

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp 末尾追加以下代码:

FReply SBoneMappingWidget::OnLoadPresetClicked()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        return FReply::Handled();
    }

    TArray<FString> OpenPaths;
    const bool bOpened = DesktopPlatform->OpenFileDialog(
        FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
        TEXT("Load Mapping Preset"),
        FPaths::ProjectSavedDir(),
        TEXT(""),
        TEXT("JSON Files (*.json)|*.json"),
        0,
        OpenPaths);

    if (!bOpened || OpenPaths.Num() == 0)
    {
        return FReply::Handled();
    }

    FMappingPreset Preset;
    if (!Preset.LoadFromJsonFile(OpenPaths[0]))
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Failed to load preset from %s"), *OpenPaths[0]);
        return FReply::Handled();
    }

    // 填充到 GUI 列表
    MappingEntries.Empty();
    for (FBoneMappingEntry& Entry : Preset.Mappings)
    {
        MappingEntries.Add(MakeShareable(new FBoneMappingEntry(Entry)));
    }

    if (MappingListView.IsValid())
    {
        MappingListView->RequestListRefresh();
    }

    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Loaded preset '%s' with %d mappings from %s"),
        *Preset.PresetName, Preset.Mappings.Num(), *OpenPaths[0]);

    return FReply::Handled();
}

完成后汇报: 确认 OnLoadPresetClicked 是否已追加到 SBoneMappingWidget.cpp 末尾，
且���有函数未被修改

===== 任务 48 结束 =====
```