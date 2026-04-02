# 任务 14: 实现 BoneMatcher — Levenshtein 编辑距离

```
===== 任务 14: 实现 BoneMatcher — Levenshtein 编辑距离 =====

目标: 创建 BoneMatcher.cpp，实现 LevenshteinDistance 和 ScoreLevenshteinSimilarity

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneMatcher.cpp 内容如下:

#include "BoneMatcher.h"

int32 FBoneMatcher::LevenshteinDistance(const FString& A, const FString& B)
{
    const int32 LenA = A.Len();
    const int32 LenB = B.Len();

    if (LenA == 0) return LenB;
    if (LenB == 0) return LenA;

    // DP 矩阵，使用一维数组优化空间
    TArray<int32> Prev;
    TArray<int32> Curr;
    Prev.SetNum(LenB + 1);
    Curr.SetNum(LenB + 1);

    for (int32 j = 0; j <= LenB; j++)
    {
        Prev[j] = j;
    }

    for (int32 i = 1; i <= LenA; i++)
    {
        Curr[0] = i;

        for (int32 j = 1; j <= LenB; j++)
        {
            int32 Cost = (A[i - 1] == B[j - 1]) ? 0 : 1;

            Curr[j] = FMath::Min3(
                Prev[j] + 1,       // 删除
                Curr[j - 1] + 1,   // 插入
                Prev[j - 1] + Cost  // 替换
            );
        }

        Swap(Prev, Curr);
    }

    return Prev[LenB];
}

float FBoneMatcher::ScoreLevenshteinSimilarity(const FString& A, const FString& B)
{
    if (A.IsEmpty() && B.IsEmpty())
    {
        return 1.0f;
    }

    const int32 MaxLen = FMath::Max(A.Len(), B.Len());
    const int32 Dist = LevenshteinDistance(A.ToLower(), B.ToLower());

    return 1.0f - (float)Dist / (float)MaxLen;
}

完成后汇报: 确认 BoneMatcher.cpp 是否已创建，包含:
  1. LevenshteinDistance 完整实现（DP 算法）
  2. ScoreLevenshteinSimilarity 完整实现（归一化到 0~1）

===== 任务 14 结束 =====
```