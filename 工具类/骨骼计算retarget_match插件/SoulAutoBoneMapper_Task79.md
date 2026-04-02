# 任务 79: 修改 OnScanAndMatchClicked — 传入 WeightConfig

```
===== 任务 79: 修改 OnScanAndMatchClicked — 传入 WeightConfig =====

目标: 修改 OnScanAndMatchClicked 中的 MatchBones 调用，传入当前权重配置

操作类型: 仅修改已有文件（约 2 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 OnScanAndMatchClicked 函数
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 OnScanAndMatchClicked 函数中，找到:

    TArray<FBoneMappingEntry> Results = FBoneMatcher::MatchBones(
        SourceBones, TargetBones, &AliasTable);

替换为:

    TArray<FBoneMappingEntry> Results = FBoneMatcher::MatchBones(
        SourceBones, TargetBones, &AliasTable, WeightConfig);

不要修改函数中的其他代码。

完成后汇报: 确认 MatchBones 调用是否已更新为传入 WeightConfig

===== 任务 79 结束 =====
```