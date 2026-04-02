# 任务 68: 创建 BoneHighlighter 头文件（3D 骨骼高亮）

```
===== 任务 68: 创建 BoneHighlighter 头文件 =====

目标: 创建 3D 骨骼高亮显示器的头文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已��文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneHighlighter.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

class USkeleton;
class UDebugSkelMeshComponent;

/**
 * 3D 骨骼高亮显示器
 * 在编辑器 Viewport 中绘制骨骼位置标记，
 * 用于帮助用户直观确认映射是否正确
 */
class FBoneHighlighter
{
public:

    /**
     * 高亮 Source 侧的指定骨骼（绿色球）
     * @param Skeleton Source Skeleton
     * @param BoneName 要高亮的骨骼名
     */
    static void HighlightSourceBone(const USkeleton* Skeleton, const FName& BoneName);

    /**
     * 高亮 Target 侧的指定骨骼（蓝色球）
     * @param Skeleton Target Skeleton
     * @param BoneName 要高亮的骨骼名
     */
    static void HighlightTargetBone(const USkeleton* Skeleton, const FName& BoneName);

    /**
     * 同时高亮一对映射（Source 绿色 + Target 蓝色）
     * @param SourceSkeleton Source Skeleton
     * @param TargetSkeleton Target Skeleton
     * @param Entry 映射条目
     */
    static void HighlightMappingPair(
        const USkeleton* SourceSkeleton,
        const USkeleton* TargetSkeleton,
        const FBoneMappingEntry& Entry);

    /** 清除所有高亮 */
    static void ClearAllHighlights();

private:

    /**
     * 获取骨骼在世界空间中的 Reference Pose 位置
     * @param Skeleton 骨骼资产
     * @param BoneName 骨骼名
     * @param OutPosition 输出位置
     * @return 是否成功找到骨骼
     */
    static bool GetBoneRefPoseWorldPosition(
        const USkeleton* Skeleton,
        const FName& BoneName,
        FVector& OutPosition);

    /**
     * 在 Viewport 中绘制一个标记球
     * @param Position 世界坐标
     * @param Color 颜色
     * @param Radius 半径
     * @param Label 标签文本
     */
    static void DrawDebugBoneMarker(
        const FVector& Position,
        const FColor& Color,
        float Radius,
        const FString& Label);
};

完成后汇报: 确认 BoneHighlighter.h 是否已创建，包含:
  1. HighlightSourceBone / HighlightTargetBone / HighlightMappingPair
  2. ClearAllHighlights
  3. 2 个 private 辅助方法

===== 任务 68 结束 =====
```