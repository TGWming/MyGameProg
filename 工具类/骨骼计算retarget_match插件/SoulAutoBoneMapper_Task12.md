# 任务 12: 实现 BoneScanner — ComputePositionMetrics

```
===== 任务 12: 实现 BoneScanner — ComputePositionMetrics =====

目标: 在 BoneScanner.cpp 末尾追加 ComputePositionMetrics 方法实现，
计算每根骨骼的世界坐标、归一化位置、方向、长度比例

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneScanner.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneScanner.cpp 末尾追加以下代码:

void FBoneScanner::ComputePositionMetrics(
    const FReferenceSkeleton& RefSkel,
    TArray<FScannedBoneInfo>& OutBones)
{
    const int32 NumBones = OutBones.Num();
    const TArray<FMeshBoneInfo>& BoneInfos = RefSkel.GetRawRefBoneInfo();
    const TArray<FTransform>& RefPose = RefSkel.GetRawRefBonePose();

    if (NumBones == 0)
    {
        return;
    }

    // 第 1 步：计算每根骨骼的世界空间坐标（Component Space）
    // RefPose 中存储的是相对父骨骼的局部 Transform，需要逐级累乘
    TArray<FTransform> WorldTransforms;
    WorldTransforms.SetNum(NumBones);

    for (int32 i = 0; i < NumBones; i++)
    {
        if (OutBones[i].ParentIndex == INDEX_NONE)
        {
            WorldTransforms[i] = RefPose[i];
        }
        else
        {
            WorldTransforms[i] = RefPose[i] * WorldTransforms[OutBones[i].ParentIndex];
        }

        OutBones[i].RefPosePosition = WorldTransforms[i].GetLocation();
    }

    // 第 2 步：计算包围盒
    FBox BBox(ForceInit);
    for (int32 i = 0; i < NumBones; i++)
    {
        BBox += OutBones[i].RefPosePosition;
    }

    // 第 3 步：归一化到 0~1 空间
    const FVector BBoxSize = BBox.GetSize();
    const FVector SafeSize(
        FMath::Max(BBoxSize.X, 1.0f),
        FMath::Max(BBoxSize.Y, 1.0f),
        FMath::Max(BBoxSize.Z, 1.0f));

    for (int32 i = 0; i < NumBones; i++)
    {
        OutBones[i].NormalizedPosition =
            (OutBones[i].RefPosePosition - BBox.Min) / SafeSize;
    }

    // 第 4 步：计算相对父骨骼的方向 + 骨骼长度比例
    const float TotalHeight = SafeSize.Z;

    for (int32 i = 0; i < NumBones; i++)
    {
        const int32 Parent = OutBones[i].ParentIndex;

        if (Parent != INDEX_NONE)
        {
            const FVector Delta = OutBones[i].RefPosePosition - OutBones[Parent].RefPosePosition;
            const float BoneLength = Delta.Size();

            OutBones[i].DirectionFromParent = (BoneLength > KINDA_SMALL_NUMBER)
                ? Delta.GetSafeNormal()
                : FVector::ZeroVector;

            OutBones[i].BoneLengthRatio = (TotalHeight > KINDA_SMALL_NUMBER)
                ? BoneLength / TotalHeight
                : 0.f;
        }
        else
        {
            OutBones[i].DirectionFromParent = FVector::ZeroVector;
            OutBones[i].BoneLengthRatio = 0.f;
        }
    }
}

完成后汇报: 确认 ComputePositionMetrics 是否已追加到 BoneScanner.cpp 末尾，
且已有函数未被修改。至此 BoneScanner 全部 5 个 Compute 方法实现完毕。

===== 任务 12 结束 =====
```