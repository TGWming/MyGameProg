# 任务 83: 实现 BuildAliasEditPanel + 嵌入 Construct

```
===== 任务 83: 实现 BuildAliasEditPanel + 嵌入 Construct =====

目标: 在 SBoneMappingWidget.cpp 末尾追加 BuildAliasEditPanel，
并修改 Construct 在权重配置面板之后插入别名编辑面板

操作类型: 追加 + 修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp
- 禁止删除或修改其他已有函数（除 Construct 的指定插入点外）
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

步骤 1: 确认顶部 #include 区域已有（如果没有则添加）:

#include "Widgets/Input/SEditableTextBox.h"

步骤 2: 在 SBoneMappingWidget.cpp 末尾追加 BuildAliasEditPanel:

TSharedRef<SWidget> SBoneMappingWidget::BuildAliasEditPanel()
{
    return SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 4.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Add Custom Alias")))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
        ]

        // 标准骨骼名输入
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
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
                    .Text(FText::FromString(TEXT("Standard Name:")))
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SEditableTextBox)
                .HintText(FText::FromString(TEXT("e.g. pelvis")))
                .OnTextChanged(this, &SBoneMappingWidget::OnAliasStandardBoneTextChanged)
            ]
        ]

        // 别名输入
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
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
                    .Text(FText::FromString(TEXT("Alias:")))
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SEditableTextBox)
                .HintText(FText::FromString(TEXT("e.g. Hips")))
                .OnTextChanged(this, &SBoneMappingWidget::OnAliasNewAliasTextChanged)
            ]
        ]

        // 按钮行
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Add Alias")))
                .OnClicked(this, &SBoneMappingWidget::OnAddAliasClicked)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Reload Default")))
                .OnClicked(this, &SBoneMappingWidget::OnReloadAliasTableClicked)
            ]
        ]

        // 统计文本
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(this, &SBoneMappingWidget::GetAliasTableStatsText)
            .Font(FCoreStyle::GetDefaultFontStyle("Italic", 8))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
        ];
}

步骤 3: 修改 Construct 方法

在 Construct 中，找到权重配置面板的 SExpandableArea Slot 之后、
Scan & Match 按钮 Slot 之前，插入:

        // ===== 别名编辑面板（可折叠） =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f, 4.f)
        [
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Alias Editor")))
            .InitiallyCollapsed(true)
            .BodyContent
            (
                BuildAliasEditPanel()
            )
        ]

不要修改 Construct 中的其他代码。

完成后汇报: 确认修改是否完成:
  1. BuildAliasEditPanel 已追加到文件末尾
  2. Construct 中已插入别名编辑面板（在权重面板之后、Scan & Match 之前）

===== 任务 83 结束 =====
```