# 任务 75: 修改 BoneMatcher — 使用 FMatchWeightConfig

```
===== 任务 75: 修改 BoneMatcher — 使用 FMatchWeightConfig =====

目标: 修改 BoneMatcher.h 和 BoneMatcher.cpp 中的 MatchBones 方法，
使用 FMatchWeightConfig 替代硬编码的权重值

操作类型: 仅修改已有文件（约 8 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 BoneMatcher.h 和 BoneMatcher.cpp，禁止修改其他文件
- 禁止删除已有的辅助方法
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

步骤 1: 修改 BoneMatcher.h

将 MatchBones 的签名修改为（增加权重配置参数）:

    static TArray<FBoneMappingEntry> MatchBones(
        const TArray<FScannedBoneInfo>& SourceBones,
        const TArray<FScannedBoneInfo>& TargetBones,
        const FBoneAliasTable* AliasTable = nullptr,
        const FMatchWeightConfig& WeightConfig = FMatchWeightConfig());

步骤 2: 修改 BoneMatcher.cpp

修改 2a: 将 MatchBones 函数签名更新为:

TArray<FBoneMappingEntry> FBoneMatcher::MatchBones(
    const TArray<FScannedBoneInfo>& SourceBones,
    const TArray<FScannedBoneInfo>& TargetBones,
    const FBoneAliasTable* AliasTable,
    const FMatchWeightConfig& WeightConfig)

修改 2b: 在 MatchBones 函数中，找到第 2 层的硬编码权重:

    const float W_Name = 0.30f;
    const float W_Structure = 0.25f;
    const float W_Position = 0.25f;
    const float W_DirLen = 0.10f;
    const float W_Symmetry = 0.10f;

替换为:

    const float W_Name = WeightConfig.NameWeight;
    const float W_Structure = WeightConfig.StructureWeight;
    const float W_Position = WeightConfig.PositionWeight;
    const float W_DirLen = WeightConfig.DirectionLengthWeight;
    const float W_Symmetry = WeightConfig.SymmetryWeight;

修改 2c: 在 MatchBones 函数中，找到阈值判断:

        if (BestScore >= 0.4f && BestTargetIdx != INDEX_NONE)

替换为:

        if (BestScore >= WeightConfig.MatchThreshold && BestTargetIdx != INDEX_NONE)

不要修改函数中的其他代码。

完成后汇报: 确认修改是否完成:
  1. MatchBones 签名增加了 WeightConfig 参数
  2. 硬编码权重替换为 WeightConfig 成员
  3. 阈值替换为 WeightConfig.MatchThreshold

===== 任务 75 结束 =====
```