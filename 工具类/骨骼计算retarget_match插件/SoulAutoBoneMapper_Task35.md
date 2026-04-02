# 任务 35: 实现 SBoneMappingWidget — 统计文本方法

```
===== 任务 35: 实现 SBoneMappingWidget — 统计文本方法 =====

目标: 在 SBoneMappingWidget.cpp 末尾追加 GetMatchStatisticsText 和 GetMatchRateText

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

FText SBoneMappingWidget::GetMatchStatisticsText() const
{
    if (MappingEntries.Num() == 0)
    {
        return FText::FromString(TEXT("No results. Select assets and click Scan & Match."));
    }

    int32 ExactCount = 0, AliasCount = 0, FuzzyCount = 0, UnmatchedCount = 0;

    for (const auto& Entry : MappingEntries)
    {
        switch (Entry->MatchType)
        {
            case EBoneMatchType::Exact:
            case EBoneMatchType::CaseInsensitive:
                ExactCount++; break;
            case EBoneMatchType::Alias:
                AliasCount++; break;
            case EBoneMatchType::Fuzzy:
            case EBoneMatchType::Structure:
            case EBoneMatchType::Position:
                FuzzyCount++; break;
            case EBoneMatchType::Manual:
                break;
            default:
                UnmatchedCount++; break;
        }
    }

    return FText::FromString(FString::Printf(
        TEXT("Exact: %d  |  Alias: %d  |  Fuzzy/Struct: %d  |  Unmatched: %d"),
        ExactCount, AliasCount, FuzzyCount, UnmatchedCount));
}

FText SBoneMappingWidget::GetMatchRateText() const
{
    if (MappingEntries.Num() == 0)
    {
        return FText::GetEmpty();
    }

    int32 MatchedCount = 0;
    for (const auto& Entry : MappingEntries)
    {
        if (Entry->MatchType != EBoneMatchType::Unmatched)
        {
            MatchedCount++;
        }
    }

    const float Rate = (float)MatchedCount / (float)MappingEntries.Num() * 100.f;

    return FText::FromString(FString::Printf(TEXT("Match Rate: %.0f%%"), Rate));
}

完成后汇报: 确认两个统计方法是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改。至此 SBoneMappingWidget 基础功能全部实现。

===== 任务 35 结束 =====
```