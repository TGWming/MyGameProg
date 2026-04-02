# 热修复 7: 重写 RetargetApplier — 避免直接调用 URig API

```
===== 热修复 7: 重写 RetargetApplier — 避免直接调用 URig API =====

目标: 重写 RetargetApplier.cpp，不再直接调用 URig::FindNode / GetNodeNum / GetNodeName，
改用 USkeleton 自身提供的公开 API 来实现映射写入和查询

问题根因: 
  URig 类虽然头文件在 Animation/Rig.h（Engine 模块）中，
  但其 FindNode / GetNodeNum / GetNodeName 方法没有 ENGINE_API 导出标记，
  属于引擎内部实现，外部模块无法链接。

解决方案:
  - 写入映射：改用 USkeleton::SetRigBoneMapping（已有 ENGINE_API 导出）
  - 验证骨骼：改用 FReferenceSkeleton::FindBoneIndex
  - 查询摘要：改用 USkeleton::GetRigBoneMapping 逐骨骼查询
  - 检查 Rig：改用 USkeleton::GetRig() 返回值是否为空

操作类型: 替换已有文件内容
引擎版本: UE4.27

安全约束:
- 仅修改 RetargetApplier.cpp 和 RetargetApplier.h
- 禁止删除或修改其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和���块

执行:

步骤 1: 将 RetargetApplier.h 的完整内容替换为:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

class USkeleton;

/**
 * 将骨骼映射结果应用到 UE4.27 Retarget Manager
 * 通过 USkeleton 的公开 API 实现，不直接调用 URig 方法
 */
class FRetargetApplier
{
public:

    /**
     * 将映射结果写入 Target Skeleton 的 Rig 骨骼映射
     * @param SourceSkeleton 动画来源 Skeleton（UE4 Humanoid）
     * @param TargetSkeleton 目标 Skeleton（第三方）
     * @param Mappings 完整的映射结果
     * @return 成功写入的映射条数
     */
    static int32 ApplyMappingToRetargetManager(
        USkeleton* SourceSkeleton,
        USkeleton* TargetSkeleton,
        const TArray<FBoneMappingEntry>& Mappings);

    /**
     * 检查 Skeleton 是否已设置 Rig
     * @param Skeleton 要检查的 Skeleton
     * @return 是否已有 Rig
     */
    static bool HasHumanoidRig(const USkeleton* Skeleton);

    /**
     * 获取 Skeleton 当前的 Rig 映射信息摘要（用于日志）
     * @param Skeleton 要检查的 Skeleton
     * @return 摘要文本
     */
    static FString GetRigMappingSummary(const USkeleton* Skeleton);
};


步骤 2: 将 RetargetApplier.cpp 的完整内容替换为:

#include "RetargetApplier.h"
#include "Animation/Skeleton.h"

int32 FRetargetApplier::ApplyMappingToRetargetManager(
    USkeleton* SourceSkeleton,
    USkeleton* TargetSkeleton,
    const TArray<FBoneMappingEntry>& Mappings)
{
    if (!SourceSkeleton || !TargetSkeleton)
    {
        UE_LOG(LogTemp, Warning, TEXT("RetargetApplier: Invalid skeleton input"));
        return 0;
    }

    if (Mappings.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RetargetApplier: No mappings to apply"));
        return 0;
    }

    // 检查 Rig 是否已设置
    if (!HasHumanoidRig(TargetSkeleton))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RetargetApplier: Target skeleton '%s' has no Rig set. "
                 "Please set a Humanoid Rig in the Retarget Manager first."),
            *TargetSkeleton->GetName());
        return 0;
    }

    UE_LOG(LogTemp, Log, TEXT("RetargetApplier: Applying %d mappings to skeleton '%s'"),
        Mappings.Num(), *TargetSkeleton->GetName());

    int32 AppliedCount = 0;
    const FReferenceSkeleton& TargetRefSkel = TargetSkeleton->GetReferenceSkeleton();

    for (const FBoneMappingEntry& Entry : Mappings)
    {
        // 跳过未匹配的
        if (Entry.MatchType == EBoneMatchType::Unmatched || Entry.TargetBoneName == NAME_None)
        {
            continue;
        }

        // 验证 Target 骨骼名在 Target Skeleton 中确实存在
        const int32 TargetBoneIndex = TargetRefSkel.FindBoneIndex(Entry.TargetBoneName);
        if (TargetBoneIndex == INDEX_NONE)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("RetargetApplier: Target bone '%s' not found in skeleton, skipping"),
                *Entry.TargetBoneName.ToString());
            continue;
        }

        // Source 骨骼名即为 Rig 节点名
        const FName RigNodeName = Entry.SourceBoneName;

        // 写入映射：Rig 节点 → Target 骨骼
        // SetRigBoneMapping 是 USkeleton 的公开 API，内部处理 Rig 节点验证
        TargetSkeleton->SetRigBoneMapping(RigNodeName, Entry.TargetBoneName);
        AppliedCount++;

        UE_LOG(LogTemp, Verbose,
            TEXT("RetargetApplier: Mapped rig node '%s' -> target bone '%s'"),
            *RigNodeName.ToString(), *Entry.TargetBoneName.ToString());
    }

    // 标记 Skeleton 已修改
    if (AppliedCount > 0)
    {
        TargetSkeleton->MarkPackageDirty();
    }

    UE_LOG(LogTemp, Log,
        TEXT("RetargetApplier: Applied %d / %d mappings to '%s'"),
        AppliedCount, Mappings.Num(), *TargetSkeleton->GetName());

    return AppliedCount;
}

bool FRetargetApplier::HasHumanoidRig(const USkeleton* Skeleton)
{
    if (!Skeleton)
    {
        return false;
    }

    // GetRig() 是 USkeleton 的公开 API，返回 nullptr 表示未设置 Rig
    return Skeleton->GetRig() != nullptr;
}

FString FRetargetApplier::GetRigMappingSummary(const USkeleton* Skeleton)
{
    if (!Skeleton)
    {
        return TEXT("No skeleton");
    }

    if (!Skeleton->GetRig())
    {
        return FString::Printf(TEXT("Skeleton '%s': No Rig set"), *Skeleton->GetName());
    }

    // 遍历 Target Skeleton 的所有骨骼，检查哪些有 Rig 映射
    const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
    const int32 NumBones = RefSkel.GetNum();
    int32 MappedCount = 0;

    for (int32 i = 0; i < NumBones; i++)
    {
        const FName BoneName = RefSkel.GetBoneName(i);
        const FName MappedName = Skeleton->GetRigBoneMapping(BoneName);

        if (MappedName != NAME_None)
        {
            MappedCount++;
        }
    }

    return FString::Printf(
        TEXT("Skeleton '%s': %d / %d bones have rig mapping"),
        *Skeleton->GetName(), MappedCount, NumBones);
}

完成后汇报: 确认 RetargetApplier.h 和 RetargetApplier.cpp 是否已替换，
不再包含任何 URig::FindNode / GetNodeNum / GetNodeName 调用

===== 热修复 7 结束 =====
```