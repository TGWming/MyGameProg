# 任务 33: 实现 SBoneMappingWidget — OnScanAndMatchClicked

```
===== 任务 33: 实现 SBoneMappingWidget — OnScanAndMatchClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加 OnScanAndMatchClicked，
执行完整的扫描 → 匹配 → 填充列表流程

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

FReply SBoneMappingWidget::OnScanAndMatchClicked()
{
    // 校验输入
    if (!SourceSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Please select a Source skeleton or mesh"));
        return FReply::Handled();
    }

    if (!TargetSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Please select a Target skeleton or mesh"));
        return FReply::Handled();
    }

    // 扫描双方骨骼
    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Scanning source skeleton..."));
    TArray<FScannedBoneInfo> SourceBones = FBoneScanner::ScanSkeleton(SourceSkeleton.Get());

    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Scanning target skeleton..."));
    TArray<FScannedBoneInfo> TargetBones = FBoneScanner::ScanSkeleton(TargetSkeleton.Get());

    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Source bones: %d, Target bones: %d"),
        SourceBones.Num(), TargetBones.Num());

    // 执行匹配
    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Running match engine..."));
    TArray<FBoneMappingEntry> Results = FBoneMatcher::MatchBones(
        SourceBones, TargetBones, &AliasTable);

    // 填充到 GUI 列表数据源
    MappingEntries.Empty();
    for (FBoneMappingEntry& Entry : Results)
    {
        MappingEntries.Add(MakeShareable(new FBoneMappingEntry(Entry)));
    }

    // 刷新列表
    if (MappingListView.IsValid())
    {
        MappingListView->RequestListRefresh();
    }

    // 统计日志
    int32 ExactCount = 0, CaseCount = 0, AliasCount = 0;
    int32 FuzzyCount = 0, StructCount = 0, UnmatchedCount = 0;

    for (const auto& Entry : Results)
    {
        switch (Entry.MatchType)
        {
            case EBoneMatchType::Exact:           ExactCount++; break;
            case EBoneMatchType::CaseInsensitive: CaseCount++; break;
            case EBoneMatchType::Alias:           AliasCount++; break;
            case EBoneMatchType::Fuzzy:           FuzzyCount++; break;
            case EBoneMatchType::Structure:       StructCount++; break;
            case EBoneMatchType::Position:        /* fall through */
            case EBoneMatchType::Unmatched:       UnmatchedCount++; break;
            default: break;
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("BoneMapper: Match complete — Exact:%d CaseIns:%d Alias:%d Fuzzy:%d Structure:%d Unmatched:%d"),
        ExactCount, CaseCount, AliasCount, FuzzyCount, StructCount, UnmatchedCount);

    return FReply::Handled();
}

完成后汇报: 确认 OnScanAndMatchClicked 是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 33 结束 =====
```