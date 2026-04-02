# 任务 44: 在 GUI 中添加"应用映射"按钮 — 修改 SBoneMappingWidget.h

```
===== 任务 44: 在 GUI 中添加"应用映射"按钮 — 修改 SBoneMappingWidget.h =====

目标: 在 SBoneMappingWidget.h 中新增应用映射和保存/加载预设的回调声明

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

修改 1: 在 #include "BoneAliasTable.h" 之后新增:

#include "MappingPreset.h"

修改 2: 在 private 区域的 GetMatchRateText 声明之后，新增以下方法声明:

    /** 点击"应用映射"按钮 → 写入 Retarget Manager */
    FReply OnApplyMappingClicked();

    /** 点击"保存预设"按钮 */
    FReply OnSavePresetClicked();

    /** 点击"加载预设"按钮 */
    FReply OnLoadPresetClicked();

不要删除或修改任何其他已有声明。

完成后汇报: 确认 SBoneMappingWidget.h 修改是否完成:
  1. 新增 #include "MappingPreset.h"
  2. 新增 OnApplyMappingClicked 声明
  3. 新增 OnSavePresetClicked 声明
  4. 新增 OnLoadPresetClicked 声明

===== 任务 44 结束 =====
```