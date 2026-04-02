# 任务 25: 将别名表集成到 BoneMatcher — 修改 BoneMatcher.h

```
===== 任务 25: 将别名表集成到 BoneMatcher — 修改 BoneMatcher.h =====

目标: 在 BoneMatcher.h 中新增别名匹配方法声明，并修改 MatchBones 签名以接收别名表

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 BoneMatcher.h，禁止修改其他文件
- 禁止删除已有的方法声明
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneMatcher.h

修改 1: 在 #include "BoneMappingTypes.h" 之后添加:

#include "BoneAliasTable.h"

修改 2: 将 MatchBones 的签名修改为（增加别名表参数）:

    static TArray<FBoneMappingEntry> MatchBones(
        const TArray<FScannedBoneInfo>& SourceBones,
        const TArray<FScannedBoneInfo>& TargetBones,
        const FBoneAliasTable* AliasTable = nullptr);

修改 3: 在 ScoreCaseInsensitiveMatch 声明之后、ScoreLevenshteinSimilarity 声明之前，新增:

    /** 别名匹配：检查 Target 名是否是 Source 名的已知别名 */
    static float ScoreAliasMatch(
        const FName& SourceName, const FName& TargetName,
        const FBoneAliasTable* AliasTable);

不要删除或修改任何其他已有声明。

完成后汇报: 确认 BoneMatcher.h 修改是否完成:
  1. 新增 #include "BoneAliasTable.h"
  2. MatchBones 签名增加了 AliasTable 参数
  3. 新增 ScoreAliasMatch 声明

===== 任务 25 结束 =====
```