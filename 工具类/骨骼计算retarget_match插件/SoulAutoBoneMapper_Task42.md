# 任务 42: 实现 RetargetApplier — ApplyMappingToRetargetManager

```
===== 任务 42: 实现 RetargetApplier — ApplyMappingToRetargetManager =====

目标: 创建 RetargetApplier.cpp，实现写入 Retarget Manager 的核心方法

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

重要说明:
UE4.27 的 Retarget 机制基于 URig + USkeleton::SetRigBoneMapping。
需要先确保 Target Skeleton 设置了 Humanoid Rig，
然后对每条映射调用 SetRigBoneMapping 将 Rig 节点名映射到实际骨骼名。

在 UE4.27 中，Humanoid Rig 的节点名就是 UE4 标准骨骼名
（如 pelvis, spine_01, head 等），而映射的目标是第三方骨骼的实际名称。

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/RetargetApplier.cpp 内容如下:

#include "RetargetApplier.h"
#include "Animation/Skeleton.h"
#include "Animation/Rig.h"

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

    // 获取 Target Skeleton 的 Rig
    const URig* Rig = TargetSkeleton->GetRig();
    if (!Rig)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RetargetApplier: Target skeleton '%s' has no Rig set. "
                 "Please set a Humanoid Rig in the Retarget Manager first."),
            *TargetSkeleton->GetName());
        return 0;
    }

    UE_LOG(LogTemp, Log, TEXT("RetargetApplier: Applying %d mappings to skeleton '%s' with rig '%s'"),
        Mappings.Num(), *TargetSkeleton->GetName(), *Rig->GetName());

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

        // Source 骨骼名即为 Rig 节点名（UE4 Humanoid Rig 节点名 = 标准骨骼名）
        const FName RigNodeName = Entry.SourceBoneName;

        // 检查 Rig 中是否存在该节点
        int32 RigNodeIndex = Rig->FindNode(RigNodeName);
        if (RigNodeIndex == INDEX_NONE)
        {
            // 某些骨骼可能不在 Humanoid Rig 的标准节点列表中（如 twist 骨骼）
            UE_LOG(LogTemp, Verbose,
                TEXT("RetargetApplier: Rig node '%s' not found in rig, skipping"),
                *RigNodeName.ToString());
            continue;
        }

        // 写入映射：Rig 节点 → Target 骨骼
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

完成后汇报: 确认 RetargetApplier.cpp 是否已创建，包含:
  1. ApplyMappingToRetargetManager 完整实现
  注意: HasHumanoidRig 和 GetRigMappingSummary 将在下一个任务中追加

===== 任务 42 结束 =====
```