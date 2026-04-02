# 任务 26: 实现 ScoreAliasMatch + 更新 MatchBones 第 1 层加入别名匹配

```
===== 任务 26: 实现 ScoreAliasMatch + 更新 MatchBones 第 1 层加入别名匹配 =====

目标: 在 BoneMatcher.cpp 中追加 ScoreAliasMatch 实现，
并修改 MatchBones 函数签名和第 1 层逻辑以支持别名匹配

操作类型: 追加 + 修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 BoneMatcher.cpp，禁止修改其他文件
- 禁止删除已有的辅助函数（Levenshtein、ScoreExact、ScoreCaseInsensitive 等）
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

步骤 1: 在 BoneMatcher.cpp 末尾追加 ScoreAliasMatch 实现:

float FBoneMatcher::ScoreAliasMatch(
    const FName& SourceName, const FName& TargetName,
    const FBoneAliasTable* AliasTable)
{
    if (!AliasTable)
    {
        return 0.0f;
    }

    // 查找 Target 骨骼名对应的 UE4 标准名
    const FName StandardName = AliasTable->FindStandardNameByAlias(TargetName);

    if (StandardName == NAME_None)
    {
        return 0.0f;
    }

    // 比较标准名与 Source 名（大小写无关）
    if (StandardName.ToString().ToLower() == SourceName.ToString().ToLower())
    {
        return 1.0f;
    }

    return 0.0f;
}

步骤 2: 修改 MatchBones 函数 — 仅修改以下两处，其他代码不变:

修改 2a: 将函数签名从
  TArray<FBoneMappingEntry> FBoneMatcher::MatchBones(
      const TArray<FScannedBoneInfo>& SourceBones,
      const TArray<FScannedBoneInfo>& TargetBones)
改为:
  TArray<FBoneMappingEntry> FBoneMatcher::MatchBones(
      const TArray<FScannedBoneInfo>& SourceBones,
      const TArray<FScannedBoneInfo>& TargetBones,
      const FBoneAliasTable* AliasTable)

修改 2b: 在第 1 层的 for 循环中，在大小写无关匹配的 if 块之后、
内层 for 循环的结尾大括号之前，追加别名匹配逻辑:

            // 别名匹配
            if (AliasTable && ScoreAliasMatch(SourceBones[i].BoneName, TargetBones[j].BoneName, AliasTable) == 1.0f)
            {
                Results[i].TargetBoneName = TargetBones[j].BoneName;
                Results[i].Confidence = 0.90f;
                Results[i].MatchType = EBoneMatchType::Alias;
                UsedTargetBones.Add(TargetBones[j].BoneName);
                break;
            }

不要修改第 2 层和第 3 层的代码。
不要删除或修改任何其他已有函数。

完成后汇报: 确认修改是否完成:
  1. ScoreAliasMatch 已追加
  2. MatchBones 签名已更新
  3. 第 1 层新增别名匹配逻辑（精确 → 大小写无关 → 别名）

===== 任务 26 结束 =====
```