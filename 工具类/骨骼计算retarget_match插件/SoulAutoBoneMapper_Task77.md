# 任务 77: 实现 BuildWeightConfigPanel + OnWeightValueChanged

```
===== 任务 77: 实现 BuildWeightConfigPanel + OnWeightValueChanged =====

目标: 在 SBoneMappingWidget.cpp 末尾追加权重配置面板构建方法

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

首先确认 SBoneMappingWidget.cpp 顶部已有（如果没有则添加）:

#include "Widgets/Input/SSpinBox.h"

然后在文件末尾追加以下代码:

void SBoneMappingWidget::OnWeightValueChanged(float NewValue, float* TargetWeight)
{
    if (TargetWeight)
    {
        *TargetWeight = NewValue;
    }
}

TSharedRef<SWidget> SBoneMappingWidget::BuildWeightConfigPanel()
{
    auto MakeWeightRow = [this](const FString& Label, float* WeightPtr) -> TSharedRef<SHorizontalBox>
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SBox)
                .WidthOverride(120.f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Label))
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SSpinBox<float>)
                .MinValue(0.0f)
                .MaxValue(1.0f)
                .Delta(0.05f)
                .Value(*WeightPtr)
                .OnValueChanged_Lambda([this, WeightPtr](float NewVal)
                {
                    OnWeightValueChanged(NewVal, WeightPtr);
                })
            ];
    };

    return SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 4.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Match Weights")))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            MakeWeightRow(TEXT("Name:"), &WeightConfig.NameWeight)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            MakeWeightRow(TEXT("Structure:"), &WeightConfig.StructureWeight)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            MakeWeightRow(TEXT("Position:"), &WeightConfig.PositionWeight)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            MakeWeightRow(TEXT("Dir + Length:"), &WeightConfig.DirectionLengthWeight)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f)
        [
            MakeWeightRow(TEXT("Symmetry:"), &WeightConfig.SymmetryWeight)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 4.f)
        [
            MakeWeightRow(TEXT("Threshold:"), &WeightConfig.MatchThreshold)
        ];
}

完成后汇报: 确认两个方法是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 77 结束 =====
```