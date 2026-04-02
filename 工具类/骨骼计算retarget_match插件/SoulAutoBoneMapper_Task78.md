# 任务 78: ��� Construct 中嵌入权重配置面板

```
===== 任务 78: 在 Construct 中嵌入权重配置面板 =====

目标: 修改 Construct 方法，在 Scan & Match 按钮之前插入可折叠的权重配置面板

操作类型: 仅修改已有文件
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

确认顶部 #include 区域已有（如果没有则添加）:

#include "Widgets/Layout/SExpandableArea.h"

在 Construct 方法中，找到第二个 "Supports: Skeleton or SkeletalMesh" 提示文本 Slot 之后、
Scan & Match 按钮 Slot 之前，插入以下代码:

        // ===== 权重配置面板（可折叠） =====
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.f, 4.f)
        [
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Match Weights")))
            .InitiallyCollapsed(true)
            .BodyContent
            (
                BuildWeightConfigPanel()
            )
        ]

不要修改 Construct 中的其他代码。

完成后汇报: 确认权重配置面板是否已插入到正确位置（Target 提示文本之后、Scan & Match 按钮之前）

===== 任务 78 结束 =====
```