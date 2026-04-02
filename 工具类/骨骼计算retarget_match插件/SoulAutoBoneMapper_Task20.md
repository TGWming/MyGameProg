# 任务 20: 实现 BoneMatcher — MatchBones 主函数（三层漏斗）

```
===== 任务 20: 实现 BoneMatcher — MatchBones 主函数（三层漏斗） =====

目标: 在 BoneMatcher.cpp 末尾追加 MatchBones 主函数，实现三层漏斗匹配策略

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

TArray<FBoneMappingEntry> FBoneMatcher::MatchBones(
    const TArray<FScannedBoneInfo>& SourceBones,
    const TArray<FScannedBoneInfo>& TargetBones)
{
    TArray<FBoneMappingEntry> Results;

    if (SourceBones.Num() == 0 || TargetBones.Num() == 0)
    {
        return Results;
    }

    // 预计算全局最大值（用于归一化）
    int32 MaxSourceDepth = 0;
    int32 MaxTargetDepth = 0;
    int32 MaxSourceSubtree = 0;
    int32 MaxTargetSubtree = 0;

    for (const auto& Bone : SourceBones)
    {
        MaxSourceDepth = FMath::Max(MaxSourceDepth, Bone.Depth);
        MaxSourceSubtree = FMath::Max(MaxSourceSubtree, Bone.SubtreeSize);
    }
    for (const auto& Bone : TargetBones)
    {
        MaxTargetDepth = FMath::Max(MaxTargetDepth, Bone.Depth);
        MaxTargetSubtree = FMath::Max(MaxTargetSubtree, Bone.SubtreeSize);
    }

    // 记录已被占用的 Target 骨骼（避免一对多）
    TSet<FName> UsedTargetBones;

    Results.SetNum(SourceBones.Num());

    // ===== 第 1 层：名称匹配（精确 → 大小写无关） =====
    for (int32 i = 0; i < SourceBones.Num(); i++)
    {
        Results[i].SourceBoneName = SourceBones[i].BoneName;
        Results[i].MatchType = EBoneMatchType::Unmatched;

        for (int32 j = 0; j < TargetBones.Num(); j++)
        {
            if (UsedTargetBones.Contains(TargetBones[j].BoneName))
            {
                continue;
            }

            // 精确匹配
            if (ScoreExactMatch(SourceBones[i].BoneName, TargetBones[j].BoneName) == 1.0f)
            {
                Results[i].TargetBoneName = TargetBones[j].BoneName;
                Results[i].Confidence = 1.0f;
                Results[i].MatchType = EBoneMatchType::Exact;
                UsedTargetBones.Add(TargetBones[j].BoneName);
                break;
            }

            // 大小写无关匹配
            if (ScoreCaseInsensitiveMatch(SourceBones[i].BoneName, TargetBones[j].BoneName) == 1.0f)
            {
                Results[i].TargetBoneName = TargetBones[j].BoneName;
                Results[i].Confidence = 0.95f;
                Results[i].MatchType = EBoneMatchType::CaseInsensitive;
                UsedTargetBones.Add(TargetBones[j].BoneName);
                break;
            }
        }
    }

    // ===== 第 2 层：结构 + 3D 位置综合评分（处理第 1 层未匹配的） =====
    // 权重：名称30% 结构25% 位置25% 方向+长度10% 对称性10%
    const float W_Name = 0.30f;
    const float W_Structure = 0.25f;
    const float W_Position = 0.25f;
    const float W_DirLen = 0.10f;
    const float W_Symmetry = 0.10f;

    for (int32 i = 0; i < SourceBones.Num(); i++)
    {
        // 跳过已匹配的
        if (Results[i].MatchType != EBoneMatchType::Unmatched)
        {
            continue;
        }

        float BestScore = -1.0f;
        int32 BestTargetIdx = INDEX_NONE;
        TArray<FBoneCandidate> Candidates;

        for (int32 j = 0; j < TargetBones.Num(); j++)
        {
            if (UsedTargetBones.Contains(TargetBones[j].BoneName))
            {
                continue;
            }

            // 左右对称性一票否决
            if (!IsSameBodySide(SourceBones[i], TargetBones[j]))
            {
                continue;
            }

            // 名称分（Levenshtein）
            const float NameScore = ScoreLevenshteinSimilarity(
                SourceBones[i].BoneName.ToString(),
                TargetBones[j].BoneName.ToString());

            // 结构分（5 维度取平均）
            const float DepthScore = ScoreDepthSimilarity(
                SourceBones[i], TargetBones[j], MaxSourceDepth, MaxTargetDepth);
            const float ChildScore = ScoreChildCountSimilarity(
                SourceBones[i], TargetBones[j]);
            const float SubtreeScore = ScoreSubtreeSimilarity(
                SourceBones[i], TargetBones[j], MaxSourceSubtree, MaxTargetSubtree);
            const float SiblingScore = ScoreSiblingSimilarity(
                SourceBones[i], TargetBones[j]);
            const float FingerprintScore = ScoreFingerprintSimilarity(
                SourceBones[i], TargetBones[j]);
            const float StructureScore =
                (DepthScore + ChildScore + SubtreeScore + SiblingScore + FingerprintScore) / 5.0f;

            // 位置分
            const float PosScore = ScorePositionSimilarity(
                SourceBones[i], TargetBones[j]);

            // 方向 + 长度分
            const float DirScore = ScoreDirectionSimilarity(
                SourceBones[i], TargetBones[j]);
            const float LenScore = ScoreLengthRatioSimilarity(
                SourceBones[i], TargetBones[j]);
            const float DirLenScore = (DirScore + LenScore) * 0.5f;

            // 综合得分
            const float TotalScore =
                W_Name * NameScore +
                W_Structure * StructureScore +
                W_Position * PosScore +
                W_DirLen * DirLenScore +
                W_Symmetry * 1.0f; // 已通过对称性检测的都得满分

            // 记录候选
            FBoneCandidate Candidate;
            Candidate.BoneName = TargetBones[j].BoneName;
            Candidate.Similarity = TotalScore;
            Candidate.MatchType = (NameScore > 0.7f)
                ? EBoneMatchType::Fuzzy
                : EBoneMatchType::Structure;
            Candidates.Add(Candidate);

            if (TotalScore > BestScore)
            {
                BestScore = TotalScore;
                BestTargetIdx = j;
            }
        }

        // 按相似度降序排列候选
        Candidates.Sort([](const FBoneCandidate& A, const FBoneCandidate& B)
        {
            return A.Similarity > B.Similarity;
        });

        // 最多保留前 5 个候选
        if (Candidates.Num() > 5)
        {
            Candidates.SetNum(5);
        }

        Results[i].Candidates = Candidates;

        // 只有超过阈值 0.4 才认为是有效匹配
        if (BestScore >= 0.4f && BestTargetIdx != INDEX_NONE)
        {
            Results[i].TargetBoneName = TargetBones[BestTargetIdx].BoneName;
            Results[i].Confidence = BestScore;
            Results[i].MatchType = Candidates.Num() > 0
                ? Candidates[0].MatchType
                : EBoneMatchType::Structure;
            UsedTargetBones.Add(TargetBones[BestTargetIdx].BoneName);
        }
    }

    // ===== 第 3 层：锚点传播（已确认邻居反推未确认的） =====
    // 遍历仍未匹配的骨骼，看其父骨骼和子骨骼是否已匹配
    for (int32 i = 0; i < SourceBones.Num(); i++)
    {
        if (Results[i].MatchType != EBoneMatchType::Unmatched)
        {
            continue;
        }

        const int32 SourceParent = SourceBones[i].ParentIndex;

        // 检查父骨骼是否已匹配
        if (SourceParent != INDEX_NONE &&
            Results[SourceParent].MatchType != EBoneMatchType::Unmatched)
        {
            // 找到父骨骼在 Target 侧的对应骨骼
            const FName TargetParentName = Results[SourceParent].TargetBoneName;

            // 在 Target 侧找该父骨骼的子骨骼中，
            // 排行与 Source 当前骨骼排行相同的
            for (int32 j = 0; j < TargetBones.Num(); j++)
            {
                if (UsedTargetBones.Contains(TargetBones[j].BoneName))
                {
                    continue;
                }

                // 找到 Target 侧的父骨骼索引
                if (TargetBones[j].ParentIndex != INDEX_NONE)
                {
                    const FName TgtParent = TargetBones[TargetBones[j].ParentIndex].BoneName;

                    if (TgtParent == TargetParentName &&
                        TargetBones[j].SiblingIndex == SourceBones[i].SiblingIndex)
                    {
                        Results[i].TargetBoneName = TargetBones[j].BoneName;
                        Results[i].Confidence = 0.6f;
                        Results[i].MatchType = EBoneMatchType::Structure;
                        UsedTargetBones.Add(TargetBones[j].BoneName);
                        break;
                    }
                }
            }
        }
    }

    return Results;
}

完成后汇报: 确认 MatchBones 是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改。至此 BoneMatcher 全部方法实现完毕。

===== 任务 20 结束 =====
```