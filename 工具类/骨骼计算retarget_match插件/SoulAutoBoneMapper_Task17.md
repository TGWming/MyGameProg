# 任务 17: 实现 BoneMatcher — 结构匹配（兄弟排行 + 链路指纹）

```
===== 任务 17: 实现 BoneMatcher — 结构匹配（兄弟排行 + 链路指纹） =====

目标: 在 BoneMatcher.cpp 末尾追加 ScoreSiblingSimilarity 和 ScoreFingerprintSimilarity

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneMatcher.cpp ���已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneMatcher.cpp 末尾追加以下代码:

float FBoneMatcher::ScoreSiblingSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 排行完全一致 → 1.0，否则按差距衰减
    if (Source.SiblingIndex == Target.SiblingIndex)
    {
        return 1.0f;
    }

    const int32 Diff = FMath::Abs(Source.SiblingIndex - Target.SiblingIndex);
    const int32 MaxIdx = FMath::Max(Source.SiblingIndex, Target.SiblingIndex);

    if (MaxIdx == 0)
    {
        return 0.0f;
    }

    return 1.0f - (float)Diff / (float)(MaxIdx + 1);
}

float FBoneMatcher::ScoreFingerprintSimilarity(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 指纹完全一致 → 1.0
    if (Source.ChainFingerprint == Target.ChainFingerprint)
    {
        return 1.0f;
    }

    // 按段比较：从根开始，连续匹配的段数越多越相似
    TArray<FString> SourceSegments;
    TArray<FString> TargetSegments;
    Source.ChainFingerprint.ParseIntoArray(SourceSegments, TEXT(","), true);
    Target.ChainFingerprint.ParseIntoArray(TargetSegments, TEXT(","), true);

    const int32 MinLen = FMath::Min(SourceSegments.Num(), TargetSegments.Num());
    const int32 MaxLen = FMath::Max(SourceSegments.Num(), TargetSegments.Num());

    if (MaxLen == 0)
    {
        return 1.0f;
    }

    int32 MatchCount = 0;
    for (int32 i = 0; i < MinLen; i++)
    {
        if (SourceSegments[i] == TargetSegments[i])
        {
            MatchCount++;
        }
        else
        {
            break; // 一旦不匹配就停止（从根开始的连续匹配）
        }
    }

    return (float)MatchCount / (float)MaxLen;
}

完成后汇报: 确认两个函数是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改

===== 任务 17 结束 =====
```