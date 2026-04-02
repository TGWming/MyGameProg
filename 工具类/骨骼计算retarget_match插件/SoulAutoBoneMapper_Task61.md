# 任务 61: 创建 BatchRetargeter 头文件

```
===== 任务 61: 创建 BatchRetargeter 头文件 =====

目标: 创建批量重定向处理器的头文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BatchRetargeter.h 内容如下:

#pragma once

#include "CoreMinimal.h"

class USkeleton;
class UAnimSequence;

/**
 * 批量动画重定向处理器
 * 将选中的多个动画资产从 Source Skeleton 重定向到 Target Skeleton
 * 前提：Target Skeleton 已通过 RetargetApplier 设置好 Rig 映射
 */
class FBatchRetargeter
{
public:

    /**
     * 批量重定向动画
     * @param SourceSkeleton 动画来源 Skeleton
     * @param TargetSkeleton 目标 Skeleton（已设置 Rig 映射）
     * @param AnimSequences 要重定向的动画资产数组
     * @param bDuplicateAssets 是否复制新资产（true）还是原地修改（false）
     * @param OutputPath 输出目录（仅当 bDuplicateAssets=true 时使用）
     * @return 成功重定向的动画数量
     */
    static int32 RetargetAnimations(
        USkeleton* SourceSkeleton,
        USkeleton* TargetSkeleton,
        const TArray<UAnimSequence*>& AnimSequences,
        bool bDuplicateAssets = true,
        const FString& OutputPath = TEXT(""));

    /**
     * 从 Content Browser 选中的资产中筛选出 AnimSequence
     * @return 选中的 AnimSequence 数组
     */
    static TArray<UAnimSequence*> GetSelectedAnimSequences();
};

完成后汇报: 确认 BatchRetargeter.h 是否已创建，包含:
  1. RetargetAnimations 方法
  2. GetSelectedAnimSequences 方法

===== 任务 61 结束 =====
```