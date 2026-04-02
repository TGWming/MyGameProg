# 任务 66: 在 Construct 底部按钮栏中添加批量重定向按钮

```
===== 任务 66: 在 Construct 底部按钮栏中添加批量重定向按钮 =====

目标: 修改 Construct 方法中的底部按钮栏，在 Load Preset 按钮之后添加 Batch Retarget 按钮

操作类型: 仅修改已有文件（约 8 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中 Construct 方法的底部按钮栏部分
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 Construct 方法的底部按钮栏中，找到 Load Preset 按钮的 Slot，
在它之后追加:

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(16.f, 0.f, 0.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Batch Retarget")))
                .OnClicked(this, &SBoneMappingWidget::OnBatchRetargetClicked)
            ]

注意 Padding 左侧 16.f 用于与前三个按钮拉开间距，视觉区分不同功能组。

完成后汇报: 确认 Batch Retarget 按钮是否已添加到底部按钮栏

===== 任务 66 结束 =====
```