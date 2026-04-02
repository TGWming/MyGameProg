# 任务 45: 在 GUI Construct 中添加底部按钮栏

```
===== 任务 45: 在 GUI Construct 中添加底部按钮栏 =====

目标: 修改 SBoneMappingWidget.cpp 的 Construct 方法，
在映射结果列表下��添加"应用映射"、"保存预设"、"加载预设"三个按钮

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 的 Construct 方法
- 禁止删除或修改其他已有函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 Construct 方法中，在映射结果列表 SListView 的 Slot 之后、
ChildSlot 的最外层闭合括号之前，追加以下按钮栏 Slot:

        // ===== 分隔线 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.f)
        [
            SNew(SSeparator)
        ]

        // ===== 底部按钮栏 =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Apply Mapping")))
                .OnClicked(this, &SBoneMappingWidget::OnApplyMappingClicked)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Save Preset")))
                .OnClicked(this, &SBoneMappingWidget::OnSavePresetClicked)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Load Preset")))
                .OnClicked(this, &SBoneMappingWidget::OnLoadPresetClicked)
            ]
        ]

确保追加位置正确：在列表 Slot 之后，在最终的 ]; 之前。

完成后汇报: 确认 Construct 中是否已追加底部按钮栏（分隔线 + 3 个按钮），
且其他已有代码未被修改

===== 任务 45 结束 =====
```