# 任务 73: 在 Construct 中绑定列表选中事件 + 添加清除高亮按钮

```
===== 任务 73: 在 Construct 中绑定列表选中事件 + 添加清除高亮按钮 =====

目标: 修改 Construct 方法，在 SListView 上绑定 OnSelectionChanged，
并在底部按钮栏添加 Clear Highlights 按钮

操作类型: 仅修改已有文件（约 10 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 Construct 方法
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

修改 1: 在 Construct 方法中，找到 SListView 的 SAssignNew 部分，
在 .OnGenerateRow 之后、.HeaderRow 之前，添加:

            .OnSelectionChanged(this, &SBoneMappingWidget::OnMappingSelectionChanged)

修改 2: 在底部按钮栏中，在 Batch Retarget 按钮的 Slot 之后，追加:

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(8.f, 0.f, 0.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Clear Highlights")))
                .OnClicked(this, &SBoneMappingWidget::OnClearHighlightsClicked)
            ]

不要修改 Construct 中的其他代码。

完成后汇报: 确认修改是否完成:
  1. SListView 绑定了 OnSelectionChanged
  2. 底部按钮栏新增 Clear Highlights 按钮

===== 任务 73 结束 =====
```