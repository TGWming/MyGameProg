# 任务 74: 创建 WeightConfig 数据结构

```
===== 任务 74: 创建 WeightConfig 数据结构 =====

目标: 在 BoneMappingTypes.h 中新增权重配置结构体

操作类型: 仅修改已有文件（约 10 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 BoneMappingTypes.h，禁止修改其他文件
- 禁止删除已有的枚举和结构体
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneMappingTypes.h

在 FBoneMappingEntry 结构体定义之后（文件末尾），追加:

// 匹配权重配置（可通过 GUI 调整）
USTRUCT(BlueprintType)
struct FMatchWeightConfig
{
    GENERATED_BODY()

    // 名称匹配权重（默认 30%）
    UPROPERTY()
    float NameWeight = 0.30f;

    // 结构匹配权重（默认 25%）
    UPROPERTY()
    float StructureWeight = 0.25f;

    // 3D 位置匹配权重（默认 25%）
    UPROPERTY()
    float PositionWeight = 0.25f;

    // 方向 + 长度匹配权重（默认 10%）
    UPROPERTY()
    float DirectionLengthWeight = 0.10f;

    // 对称性匹配权重（默认 10%）
    UPROPERTY()
    float SymmetryWeight = 0.10f;

    // 综合评分低于此阈值视为未匹配（默认 0.4）
    UPROPERTY()
    float MatchThreshold = 0.40f;
};

不要删除或修改任何已有的枚举和结构体。

完成后汇报: 确认 FMatchWeightConfig 结构体是否已追加到 BoneMappingTypes.h 末尾

===== 任务 74 结束 =====
```