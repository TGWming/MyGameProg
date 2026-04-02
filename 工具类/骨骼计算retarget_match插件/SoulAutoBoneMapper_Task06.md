# 任务 6: 创建 BoneScanner 头文件

```
===== 任务 6: 创建 BoneScanner 头文件 =====

目标: 创建骨骼扫描器的头文件，声明 BoneScanner 类及其公共接口

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneScanner.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

class USkeleton;
class USkeletalMesh;

/**
 * 骨骼扫描器：读取 Skeleton 骨骼树，提取每根骨骼的元数据
 * 支持输入 USkeleton 或 USkeletalMesh（自动提取 Skeleton）
 */
class FBoneScanner
{
public:

    /**
     * 从任意资产提取 USkeleton（支持 USkeleton 或 USkeletalMesh）
     * @param Asset 输入资产
     * @return 提取到的 Skeleton，失败返回 nullptr
     */
    static USkeleton* GetSkeletonFromAsset(UObject* Asset);

    /**
     * 扫描 Skeleton，提取所有骨骼的元数据
     * @param Skeleton 要扫描的骨骼资产
     * @return 所有骨骼的扫描信息数组，按骨骼索引排序
     */
    static TArray<FScannedBoneInfo> ScanSkeleton(const USkeleton* Skeleton);

private:

    /** 计算每根骨骼的深度（根 = 0） */
    static void ComputeDepths(
        const FReferenceSkeleton& RefSkel,
        TArray<FScannedBoneInfo>& OutBones);

    /** 计算每根骨骼的子节点数和子树规模 */
    static void ComputeChildCountsAndSubtreeSize(
        const FReferenceSkeleton& RefSkel,
        TArray<FScannedBoneInfo>& OutBones);

    /** 计算每根骨骼在兄弟中的排行 */
    static void ComputeSiblingIndices(
        const FReferenceSkeleton& RefSkel,
        TArray<FScannedBoneInfo>& OutBones);

    /** 计算链路指纹（从根到每根骨骼的路径编码） */
    static void ComputeChainFingerprints(
        const FReferenceSkeleton& RefSkel,
        TArray<FScannedBoneInfo>& OutBones);

    /** 计算归一化位置、方向、长度比例（需要 RefPose 世界坐标） */
    static void ComputePositionMetrics(
        const FReferenceSkeleton& RefSkel,
        TArray<FScannedBoneInfo>& OutBones);
};

完成后汇报: 确认 BoneScanner.h 是否已创建，包含:
  1. GetSkeletonFromAsset 静态方法
  2. ScanSkeleton 静态方法
  3. 5 个 private 辅助方法

===== 任务 6 结束 =====
```