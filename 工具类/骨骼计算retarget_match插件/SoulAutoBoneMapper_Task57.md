# 任务 57: 修改 Construct 列表 HeaderRow — 增加 Action 列

```
===== 任务 57: 修改 Construct 列表 HeaderRow — 增加 Action 列 =====

目标: 在 Construct 方法中的 SHeaderRow 增加 Action 列头

操作类型: 仅修改已有文件（约 3 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中 Construct 方法的 SHeaderRow 部分
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 Construct 方法中，找到 SHeaderRow 的定义，在最后一个 Column("Confidence") 之后追加:

                + SHeaderRow::Column("Action")
                .DefaultLabel(LOCTEXT("ColAction", "Action"))
                .FillWidth(0.15f)

同时将现有列的 FillWidth 微调以腾出空间:
  - "Source" 列: FillWidth(0.25f) → 保持 0.25f
  - "Status" 列: FillWidth(0.12f) → 保持 0.12f
  - "Target" 列: FillWidth(0.25f) → 保持 0.25f
  - "Confidence" 列: FillWidth(0.10f) → 保持 0.10f
  - "Action" 列: FillWidth(0.15f) → 新增

注意: LOCTEXT 需要在 Construct 函数的 #define LOCTEXT_NAMESPACE 范围内。
如果 Construct 中的 LOCTEXT_NAMESPACE 已在函数外 undef 了，
改用 FText::FromString(TEXT("Action")) 替代 LOCTEXT。

完成后汇报: 确认 SHeaderRow 是否已增加 Action 列，
且其他列未被删除

===== 任务 57 结束 =====
```