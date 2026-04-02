# 任务 18: 实现 BoneMatcher — 3D 位置匹配（位置距离 + 方向 + 长度比例）

```
===== 任务 18: ��现 BoneMatcher — 3D 位置匹配（位置距离 + 方向 + 长度比例） =====

目标: 在 BoneMatcher.cpp 末尾追加 ScorePositionSimilarity、ScoreDirectionSimilarity、ScoreLengthRatioSimilarity

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneMatcher.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneMatcher.cpp 末尾追加以下代码:

float FBoneMatcher::ScorePositionSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 归一化空间中两点距离，最大可能距离为 sqrt(3) ≈ 1.732
    const float Dist = FVector::Dist(
        Source.NormalizedPosition, Target.NormalizedPosition);

    // 将距离映射到 0~1 相似度
    // 距离 0 → 相似度 1.0，距离 >= 1.0 → 相似度 0.0
    return FMath::Clamp(1.0f - Dist, 0.0f, 1.0f);
}

float FBoneMatcher::ScoreDirectionSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 根骨骼没有方向
    if (Source.DirectionFromParent.IsNearlyZero() ||
        Target.DirectionFromParent.IsNearlyZero())
    {
        // 两个都是根 → 匹配，只有一个是根 → 不匹配
        return (Source.DirectionFromParent.IsNearlyZero() ==
                Target.DirectionFromParent.IsNearlyZero()) ? 1.0f : 0.0f;
    }

    // 方向向量点积：1 = 同向，-1 = 反向
    const float Dot = FVector::DotProduct(
        Source.DirectionFromParent, Target.DirectionFromParent);

    // 映射到 0~1：-1 → 0.0，1 → 1.0
    return (Dot + 1.0f) * 0.5f;
}

float FBoneMatcher::ScoreLengthRatioSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 长度比例差异
    const float Diff = FMath::Abs(Source.BoneLengthRatio - Target.BoneLengthRatio);

    // 差异 0 → 相似度 1.0，差异 >= 0.5 → 相似度 0.0
    return FMath::Clamp(1.0f - Diff * 2.0f, 0.0f, 1.0f);
}

完成后汇报: 确认三个函数是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改

===== 任务 18 结束 =====
```