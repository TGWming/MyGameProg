# 任务 16: 实现 BoneMatcher — 结构匹配（深度 + 子节点数 + 子树规模）

```
===== 任务 16: 实现 BoneMatcher — 结构匹配（深度 + 子节点数 + 子树规模） =====

目标: 在 BoneMatcher.cpp 末尾追加 ScoreDepthSimilarity、ScoreChildCountSimilarity、ScoreSubtreeSimilarity

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

float FBoneMatcher::ScoreDepthSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target,
    int32 MaxSourceDepth, int32 MaxTargetDepth)
{
    // 将深度归一化到 0~1 后比较差异
    if (MaxSourceDepth == 0 && MaxTargetDepth == 0)
    {
        return 1.0f;
    }

    const float NormSource = (MaxSourceDepth > 0)
        ? (float)Source.Depth / (float)MaxSourceDepth : 0.f;
    const float NormTarget = (MaxTargetDepth > 0)
        ? (float)Target.Depth / (float)MaxTargetDepth : 0.f;

    return 1.0f - FMath::Abs(NormSource - NormTarget);
}

float FBoneMatcher::ScoreChildCountSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    const int32 MaxCC = FMath::Max(Source.ChildCount, Target.ChildCount);

    if (MaxCC == 0)
    {
        // 都是叶子节点 → 完美匹配
        return 1.0f;
    }

    const int32 Diff = FMath::Abs(Source.ChildCount - Target.ChildCount);
    return 1.0f - (float)Diff / (float)MaxCC;
}

float FBoneMatcher::ScoreSubtreeSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target,
    int32 MaxSourceSubtree, int32 MaxTargetSubtree)
{
    if (MaxSourceSubtree == 0 && MaxTargetSubtree == 0)
    {
        return 1.0f;
    }

    const float NormSource = (MaxSourceSubtree > 0)
        ? (float)Source.SubtreeSize / (float)MaxSourceSubtree : 0.f;
    const float NormTarget = (MaxTargetSubtree > 0)
        ? (float)Target.SubtreeSize / (float)MaxTargetSubtree : 0.f;

    return 1.0f - FMath::Abs(NormSource - NormTarget);
}

完成后汇报: 确认三个函数是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改

===== 任务 16 结束 =====
```