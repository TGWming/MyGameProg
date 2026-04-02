# 任务 54: 实现 OnEditMappingClicked + OnClearMappingClicked

```
===== 任务 54: 实现 OnEditMappingClicked + OnClearMappingClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加编辑和清除映射的回调

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

FReply SBoneMappingWidget::OnEditMappingClicked(TSharedPtr<FBoneMappingEntry> Entry)
{
    if (!Entry.IsValid())
    {
        return FReply::Handled();
    }

    // 切换编辑状态：如果当前正在编辑同一行则关闭，否则打开新行
    if (EditingEntry == Entry)
    {
        EditingEntry = nullptr;
    }
    else
    {
        EditingEntry = Entry;
    }

    // 刷新列表以显示/隐藏下拉选择器
    if (MappingListView.IsValid())
    {
        MappingListView->RequestListRefresh();
    }

    UE_LOG(LogTemp, Verbose, TEXT("BoneMapper: Editing mapping for '%s'"),
        *Entry->SourceBoneName.ToString());

    return FReply::Handled();
}

FReply SBoneMappingWidget::OnClearMappingClicked(TSharedPtr<FBoneMappingEntry> Entry)
{
    if (!Entry.IsValid())
    {
        return FReply::Handled();
    }

    Entry->TargetBoneName = NAME_None;
    Entry->Confidence = 0.f;
    Entry->MatchType = EBoneMatchType::Unmatched;
    Entry->bManuallyOverridden = false;

    // 如果正在编辑这一行，关闭编辑状态
    if (EditingEntry == Entry)
    {
        EditingEntry = nullptr;
    }

    if (MappingListView.IsValid())
    {
        MappingListView->RequestListRefresh();
    }

    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Cleared mapping for '%s'"),
        *Entry->SourceBoneName.ToString());

    return FReply::Handled();
}

完成后汇报: 确认两个回调方法是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 54 结束 =====
```