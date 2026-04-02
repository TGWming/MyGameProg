# 任务 7: 实现 BoneScanner — GetSkeletonFromAsset + ScanSkeleton 框架

```
===== 任务 7: 实现 BoneScanner — GetSkeletonFromAsset + ScanSkeleton 框架 =====

目标: 创建 BoneScanner.cpp，实现资产提取和扫描主函数框架

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneScanner.cpp 内容如下:

#include "BoneScanner.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"

USkeleton* FBoneScanner::GetSkeletonFromAsset(UObject* Asset)
{
    if (!Asset)
    {
        return nullptr;
    }

    // 直接就是 Skeleton
    if (USkeleton* Skel = Cast<USkeleton>(Asset))
    {
        return Skel;
    }

    // 从 SkeletalMesh 提取 Skeleton
    if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset))
    {
        return Mesh->Skeleton;
    }

    return nullptr;
}

TArray<FScannedBoneInfo> FBoneScanner::ScanSkeleton(const USkeleton* Skeleton)
{
    TArray<FScannedBoneInfo> Result;

    if (!Skeleton)
    {
        return Result;
    }

    const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
    const int32 NumBones = RefSkel.GetNum();

    if (NumBones == 0)
    {
        return Result;
    }

    // 初始化：填充基本信息（名称、索引、父索引）
    Result.SetNum(NumBones);
    for (int32 i = 0; i < NumBones; i++)
    {
        Result[i].BoneName = RefSkel.GetBoneName(i);
        Result[i].BoneIndex = i;
        Result[i].ParentIndex = RefSkel.GetParentIndex(i);
    }

    // 依次计算各维度元数据
    ComputeDepths(RefSkel, Result);
    ComputeChildCountsAndSubtreeSize(RefSkel, Result);
    ComputeSiblingIndices(RefSkel, Result);
    ComputeChainFingerprints(RefSkel, Result);
    ComputePositionMetrics(RefSkel, Result);

    return Result;
}

完成后汇报: 确认 BoneScanner.cpp 是否已创建，包含:
  1. GetSkeletonFromAsset 完整实现
  2. ScanSkeleton 框架（初始化 + 调用 5 个 Compute 方法）
  注意: 5 个 Compute 方法尚未实现，将在后续任务中补充

===== 任务 7 结束 =====
```