# 任务 13: 创建 BoneMatcher 头文件

```
===== 任务 13: 创建 BoneMatcher 头文件 =====

目标: 创建骨骼匹配引擎的头文件，声明 BoneMatcher 类及其公共接口

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneMatcher.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

/**
 * 骨骼匹配引擎：三层漏斗策略
 * 第 1 层：名称匹配（精确 → 大小写无关 → 别名 → 模糊）
 * 第 2 层：结构 + 3D 位置综合评分
 * 第 3 层：锚点传播（已确认邻居反推）
 */
class FBoneMatcher
{
public:

    /**
     * 执行完整的三层漏斗匹配
     * @param SourceBones Source 侧扫描结果（UE4 Humanoid）
     * @param TargetBones Target 侧扫描结果（第三方）
     * @return 完整的映射结果数组（每条 Source 骨骼一条记录）
     */
    static TArray<FBoneMappingEntry> MatchBones(
        const TArray<FScannedBoneInfo>& SourceBones,
        const TArray<FScannedBoneInfo>& TargetBones);

    // ===== 第 1 层：名称匹配 =====

    /** 精确匹配（名称完全一致） */
    static float ScoreExactMatch(const FName& SourceName, const FName& TargetName);

    /** 大小写无关匹配 */
    static float ScoreCaseInsensitiveMatch(const FName& SourceName, const FName& TargetName);

    /** Levenshtein 编辑距离 → 归一化相似度 0~1 */
    static float ScoreLevenshteinSimilarity(const FString& A, const FString& B);

    /** 计算两个字符串的 Levenshtein 编辑距离 */
    static int32 LevenshteinDistance(const FString& A, const FString& B);

    // ===== 第 2 层：结构匹配 =====

    /** 深度相似度 */
    static float ScoreDepthSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target,
        int32 MaxSourceDepth, int32 MaxTargetDepth);

    /** 子节点数相似度 */
    static float ScoreChildCountSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    /** 子树规模相似度 */
    static float ScoreSubtreeSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target,
        int32 MaxSourceSubtree, int32 MaxTargetSubtree);

    /** 兄弟排行相似度 */
    static float ScoreSiblingSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    /** 链路指纹相似度 */
    static float ScoreFingerprintSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    // ===== 3D 位置匹配 =====

    /** 归一化位置距离 → 相似度 */
    static float ScorePositionSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    /** 相对父骨骼方向相似度 */
    static float ScoreDirectionSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    /** 骨骼长度比例相似度 */
    static float ScoreLengthRatioSimilarity(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);

    /** 左右对称性检测（一票否决） */
    static bool IsSameBodySide(
        const FScannedBoneInfo& Source, const FScannedBoneInfo& Target);
};

完成后汇报: 确认 BoneMatcher.h 是否已创建，包含:
  1. MatchBones 主函数
  2. 4 个名称匹配方法
  3. 5 个结构匹配方法
  4. 4 个 3D 位置匹配方法

===== 任务 13 结束 =====
```