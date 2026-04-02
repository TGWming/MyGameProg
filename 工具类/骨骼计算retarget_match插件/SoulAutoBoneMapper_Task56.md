# 任务 56: 修改 OnGenerateMappingRow — 添加 Edit/Clear 按钮和下拉选择器

```
===== 任务 56: 修改 OnGenerateMappingRow — 添加 Edit/Clear 按钮和下拉选择器 =====

目标: 重写 OnGenerateMappingRow 方法，在每行末尾添加 Edit 和 Clear 按钮，
当处于编辑状态时在 Target 列显示下拉选择器

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 OnGenerateMappingRow 函数
- 禁止删除或修改其他函数（包括 GetMatchTypeDisplayText 和 GetMatchTypeColor 辅助函数）
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在顶部 #include 区域确认已有（如果没有则添加）:

#include "Widgets/Input/SComboBox.h"

将现有的 OnGenerateMappingRow 函数整体替换为以下代码
（不要修改它之前的 GetMatchTypeDisplayText 和 GetMatchTypeColor 两个辅助函数）:

TSharedRef<ITableRow> SBoneMappingWidget::OnGenerateMappingRow(
    TSharedPtr<FBoneMappingEntry> InEntry,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const bool bIsEditing = (EditingEntry == InEntry);

    // Target 列内容：编辑状态下显示下拉，否则显示文本
    TSharedRef<SWidget> TargetWidget =
        bIsEditing
        ? StaticCastSharedRef<SWidget>(
            SNew(SComboBox<TSharedPtr<FName>>)
            .OptionsSource(&AllTargetBoneNames)
            .OnSelectionChanged(this, &SBoneMappingWidget::OnTargetBoneSelected)
            .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
            {
                return SNew(STextBlock)
                    .Text(Item.IsValid() && *Item != NAME_None
                        ? FText::FromName(*Item)
                        : FText::FromString(TEXT("[None]")));
            })
            [
                SNew(STextBlock)
                .Text(InEntry->TargetBoneName != NAME_None
                    ? FText::FromName(InEntry->TargetBoneName)
                    : FText::FromString(TEXT("[Select...]")))
            ]
        )
        : StaticCastSharedRef<SWidget>(
            SNew(STextBlock)
            .Text(InEntry->TargetBoneName != NAME_None
                ? FText::FromName(InEntry->TargetBoneName)
                : FText::FromString(TEXT("[Unmatched]")))
        );

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
            TargetWidget
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

        // Edit 按钮
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2.f)
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .Text(bIsEditing
                ? FText::FromString(TEXT("Cancel"))
                : FText::FromString(TEXT("Edit")))
            .OnClicked_Lambda([this, InEntry]() -> FReply
            {
                return OnEditMappingClicked(InEntry);
            })
        ]

        // Clear 按钮
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2.f)
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("X")))
            .OnClicked_Lambda([this, InEntry]() -> FReply
            {
                return OnClearMappingClicked(InEntry);
            })
        ]
    ];
}

完成后汇报: 确认 OnGenerateMappingRow 是否已替换，包含:
  1. 编辑状态判断（bIsEditing）
  2. Target 列条件显示（下拉 / 文本）
  3. Edit 按钮（切换编辑/取消）
  4. Clear 按钮（X）
  5. GetMatchTypeDisplayText 和 GetMatchTypeColor 辅助函数未被修改

===== 任务 56 结束 =====
```