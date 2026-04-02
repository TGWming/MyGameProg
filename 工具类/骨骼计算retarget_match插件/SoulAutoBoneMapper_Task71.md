# 任务 71: 在 SBoneMappingWidget.h 中新增 3D 高亮联动相关声明

```
===== 任务 71: 在 SBoneMappingWidget.h 中新增 3D 高亮联动相关声明 =====

目标: 在 SBoneMappingWidget.h 中新增列表行点击高亮和清除高亮的方法声明

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.h，禁止修改其他文件
- 禁止删除已有的成员变量和方法声明
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/SBoneMappingWidget.h

修改 1: 在 #include "MappingPreset.h" 之后新增:

#include "BoneHighlighter.h"

修改 2: 在 private 区域的 OnBatchRetargetClicked 声明之后，新增:

    /** 列表行选中变化 → 3D 高亮对应骨骼 */
    void OnMappingSelectionChanged(
        TSharedPtr<FBoneMappingEntry> SelectedEntry,
        ESelectInfo::Type SelectInfo);

    /** 点击"清除高亮"按钮 */
    FReply OnClearHighlightsClicked();

不要删除或修改任何其他已有声明。

完成后汇报: 确认修改是否完成:
  1. 新增 #include "BoneHighlighter.h"
  2. 新增 OnMappingSelectionChanged 声明
  3. 新增 OnClearHighlightsClicked 声明

===== 任务 71 结束 =====
```