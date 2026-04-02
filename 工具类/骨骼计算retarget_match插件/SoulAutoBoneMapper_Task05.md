# 任务 5: 创建核心数据结构头文件

```
===== 任务 5: 创建核心数据结构头文件 =====

目标: 创建骨骼匹配的核心数据结构定义，包含匹配类型枚举、扫描骨骼信息、候选骨骼、映射记录

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneMappingTypes.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.generated.h"

// 匹配方式枚举
UENUM(BlueprintType)
enum class EBoneMatchType : uint8
{
    Exact           UMETA(DisplayName = "Exact"),
    CaseInsensitive UMETA(DisplayName = "CaseInsensitive"),
    Alias           UMETA(DisplayName = "Alias"),
    Fuzzy           UMETA(DisplayName = "Fuzzy"),
    Structure       UMETA(DisplayName = "Structure"),
    Position        UMETA(DisplayName = "Position"),
    Manual          UMETA(DisplayName = "Manual"),
    Unmatched       UMETA(DisplayName = "Unmatched")
};

// BoneScanner 扫描后每根骨骼的元数据
USTRUCT(BlueprintType)
struct FScannedBoneInfo
{
    GENERATED_BODY()

    // 骨骼名称
    UPROPERTY()
    FName BoneName;

    // 在 FReferenceSkeleton 中的索引
    UPROPERTY()
    int32 BoneIndex = INDEX_NONE;

    // 父骨骼索引（根骨骼为 INDEX_NONE）
    UPROPERTY()
    int32 ParentIndex = INDEX_NONE;

    // 在骨骼树中的深度（根 = 0）
    UPROPERTY()
    int32 Depth = 0;

    // 直接子骨骼数量
    UPROPERTY()
    int32 ChildCount = 0;

    // 该骨骼下属所有后代骨骼数量
    UPROPERTY()
    int32 SubtreeSize = 0;

    // 在父骨骼的子骨骼列表中的排行（从 0 开始）
    UPROPERTY()
    int32 SiblingIndex = 0;

    // 归一化后的 3D 位置（0~1 单位空间）
    UPROPERTY()
    FVector NormalizedPosition = FVector::ZeroVector;

    // 相对父骨骼的方向（单位向量）
    UPROPERTY()
    FVector DirectionFromParent = FVector::ZeroVector;

    // 骨骼长度占全身高度的比例（0~1）
    UPROPERTY()
    float BoneLengthRatio = 0.f;

    // 链路指纹编码（从根到该骨骼的路径，格式: "3-0,1-0,2-1"）
    // 每段含义: 父骨骼子节点数-该骨骼在兄弟中的排行
    UPROPERTY()
    FString ChainFingerprint;

    // Reference Pose 中的世界空间坐标（归一化前的原始值）
    UPROPERTY()
    FVector RefPosePosition = FVector::ZeroVector;
};

// 候选骨骼（供 GUI 下拉选择）
USTRUCT(BlueprintType)
struct FBoneCandidate
{
    GENERATED_BODY()

    UPROPERTY()
    FName BoneName;

    UPROPERTY()
    float Similarity = 0.f;

    UPROPERTY()
    EBoneMatchType MatchType = EBoneMatchType::Unmatched;
};

// 单条映射记录
USTRUCT(BlueprintType)
struct FBoneMappingEntry
{
    GENERATED_BODY()

    // Source 侧（UE4 Humanoid）骨骼名
    UPROPERTY()
    FName SourceBoneName;

    // Target 侧（第三方）骨骼名
    UPROPERTY()
    FName TargetBoneName;

    // 匹配置信度 0.0 ~ 1.0
    UPROPERTY()
    float Confidence = 0.f;

    // 匹配方式
    UPROPERTY()
    EBoneMatchType MatchType = EBoneMatchType::Unmatched;

    // 是否被开发者手动修改过
    UPROPERTY()
    bool bManuallyOverridden = false;

    // 候选列表（供 GUI 下拉选择，按相似度降序排列）
    UPROPERTY()
    TArray<FBoneCandidate> Candidates;
};

完成后汇报: 确认数据结构头文件是否已创建，包含以下 4 个定义:
  1. EBoneMatchType 枚举（8 个值）
  2. FScannedBoneInfo 结构体（12 个字段）
  3. FBoneCandidate 结构体（3 个字段）
  4. FBoneMappingEntry 结构体（6 个字段）

===== 任务 5 结束 =====
```