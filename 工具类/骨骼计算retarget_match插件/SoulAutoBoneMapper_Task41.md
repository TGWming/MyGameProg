# 任务 41: 创建 RetargetApplier 头文件

```
===== 任务 41: 创建 RetargetApplier 头文件 =====

目标: 创建负责将映射结果写入 UE4.27 Retarget Manager 的类头文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/RetargetApplier.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

class USkeleton;

/**
 * 将骨骼映射结果应用到 UE4.27 Retarget Manager
 * 通过 USkeleton 的 Rig 映射 API 实现
 */
class FRetargetApplier
{
public:

    /**
     * 将映射结果写入 Target Skeleton 的 Humanoid Rig 映射
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
     * 检查 Skeleton 是否已设置 Humanoid Rig
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

完成后汇报: 确认 RetargetApplier.h 是否已创建，包含:
  1. ApplyMappingToRetargetManager
  2. HasHumanoidRig
  3. GetRigMappingSummary

===== 任务 41 结束 =====
```