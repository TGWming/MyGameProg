# 任务 31: 实现 SBoneMappingWidget — Construct 主界面布局

```
===== 任务 31: 实现 SBoneMappingWidget — Construct 主界面布局 =====

目标: 创建 SBoneMappingWidget.cpp，实现 Construct 方法，搭建完整 GUI 布局

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp 内容如下:

#include "SBoneMappingWidget.h"
#include "BoneScanner.h"
#include "BoneMatcher.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "SBoneMappingWidget"

void SBoneMappingWidget::Construct(const FArguments& InArgs)
{
    // 加载默认别名表
    AliasTable.LoadDefaultAliases();

    ChildSlot
    [
        SNew(SVerticalBox)

        // ===== 标题 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Title", "Soul Auto Bone Mapper"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.f)
        [
            SNew(SSeparator)
        ]

        // ===== Source 资产选择 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SBox)
                .WidthOverride(120.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("SourceLabel", "Source (Animation):"))
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SObjectPropertyEntryBox)
                .AllowedClass(USkeleton::StaticClass())
                .OnObjectChanged(this, &SBoneMappingWidget::OnSourceAssetChanged)
                .AllowClear(true)
            ]
        ]

        // ===== Target 资产选择 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SBox)
                .WidthOverride(120.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("TargetLabel", "Target (Character):"))
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SObjectPropertyEntryBox)
                .AllowedClass(USkeletalMesh::StaticClass())
                .OnObjectChanged(this, &SBoneMappingWidget::OnTargetAssetChanged)
                .AllowClear(true)
            ]
        ]

        // ===== Scan & Match 按钮 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f)
        .HAlign(HAlign_Center)
        [
            SNew(SButton)
            .Text(LOCTEXT("ScanMatch", "Scan & Match"))
            .OnClicked(this, &SBoneMappingWidget::OnScanAndMatchClicked)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.f)
        [
            SNew(SSeparator)
        ]

        // ===== 匹配率 + 统计 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(this, &SBoneMappingWidget::GetMatchStatisticsText)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(this, &SBoneMappingWidget::GetMatchRateText)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
            ]
        ]

        // ===== 映射结果列表 =====
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(8.f, 4.f)
        [
            SAssignNew(MappingListView, SListView<TSharedPtr<FBoneMappingEntry>>)
            .ItemHeight(24.f)
            .ListItemsSource(&MappingEntries)
            .OnGenerateRow(this, &SBoneMappingWidget::OnGenerateMappingRow)
            .HeaderRow
            (
                SNew(SHeaderRow)
                + SHeaderRow::Column("Source")
                .DefaultLabel(LOCTEXT("ColSource", "Source Bone"))
                .FillWidth(0.25f)
                + SHeaderRow::Column("Status")
                .DefaultLabel(LOCTEXT("ColStatus", "Status"))
                .FillWidth(0.12f)
                + SHeaderRow::Column("Target")
                .DefaultLabel(LOCTEXT("ColTarget", "Target Bone"))
                .FillWidth(0.25f)
                + SHeaderRow::Column("Confidence")
                .DefaultLabel(LOCTEXT("ColConf", "Conf"))
                .FillWidth(0.10f)
            )
        ]
    ];
}

#undef LOCTEXT_NAMESPACE

完成后汇报: 确认 SBoneMappingWidget.cpp 是否已创建，包含:
  1. Construct 方法完整实现
  2. GUI 布局：标题 + Source 选择 + Target 选择 + 按钮 + 统计 + 列表
  注意: 回调方法尚未实现，将在后续任务中补充

===== 任务 31 结束 =====
```