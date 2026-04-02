# 任务 34: 实现 SBoneMappingWidget — OnGenerateMappingRow

```
===== 任务 34: 实现 SBoneMappingWidget — OnGenerateMappingRow =====

目标: 在 SBoneMappingWidget.cpp 末尾追加列表行生成方法

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

static FText GetMatchTypeDisplayText(EBoneMatchType Type)
{
    switch (Type)
    {
        case EBoneMatchType::Exact:           return FText::FromString(TEXT("Exact"));
        case EBoneMatchType::CaseInsensitive: return FText::FromString(TEXT("CaseIns"));
        case EBoneMatchType::Alias:           return FText::FromString(TEXT("Alias"));
        case EBoneMatchType::Fuzzy:           return FText::FromString(TEXT("Fuzzy"));
        case EBoneMatchType::Structure:       return FText::FromString(TEXT("Structure"));
        case EBoneMatchType::Position:        return FText::FromString(TEXT("Position"));
        case EBoneMatchType::Manual:          return FText::FromString(TEXT("Manual"));
        default:                              return FText::FromString(TEXT("Unmatched"));
    }
}

static FSlateColor GetMatchTypeColor(EBoneMatchType Type)
{
    switch (Type)
    {
        case EBoneMatchType::Exact:           return FSlateColor(FLinearColor::Green);
        case EBoneMatchType::CaseInsensitive: return FSlateColor(FLinearColor::Green);
        case EBoneMatchType::Alias:           return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f));
        case EBoneMatchType::Fuzzy:           return FSlateColor(FLinearColor::Yellow);
        case EBoneMatchType::Structure:       return FSlateColor(FLinearColor::Yellow);
        case EBoneMatchType::Position:        return FSlateColor(FLinearColor::Yellow);
        case EBoneMatchType::Manual:          return FSlateColor(FLinearColor(0.8f, 0.5f, 1.0f));
        default:                              return FSlateColor(FLinearColor::Red);
    }
}

TSharedRef<ITableRow> SBoneMappingWidget::OnGenerateMappingRow(
    TSharedPtr<FBoneMappingEntry> InEntry,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FBoneMappingEntry>>, OwnerTable)
    [
        SNew(SHorizontalBox)

        // Source Bone 列
        + SHorizontalBox::Slot()
        .FillWidth(0.25f)
        .Padding(4.f, 2.f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromName(InEntry->SourceBoneName))
        ]

        // Status 列
        + SHorizontalBox::Slot()
        .FillWidth(0.12f)
        .Padding(4.f, 2.f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(GetMatchTypeDisplayText(InEntry->MatchType))
            .ColorAndOpacity(GetMatchTypeColor(InEntry->MatchType))
        ]

        // Target Bone 列
        + SHorizontalBox::Slot()
        .FillWidth(0.25f)
        .Padding(4.f, 2.f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(InEntry->TargetBoneName != NAME_None
                ? FText::FromName(InEntry->TargetBoneName)
                : FText::FromString(TEXT("[Unmatched]")))
        ]

        // Confidence 列
        + SHorizontalBox::Slot()
        .FillWidth(0.10f)
        .Padding(4.f, 2.f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(InEntry->bManuallyOverridden
                ? FText::FromString(TEXT("--"))
                : FText::FromString(FString::Printf(TEXT("%.0f%%"), InEntry->Confidence * 100.f)))
        ]
    ];
}

完成后汇报: 确认 OnGenerateMappingRow 及辅助函数是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 34 结束 =====
```