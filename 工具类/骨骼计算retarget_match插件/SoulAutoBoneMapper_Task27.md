# 任务 27: 创建 MappingPreset 头文件

```
===== 任务 27: 创建 MappingPreset 头文件 =====

目标: 创建映射预设的头文件，用于保存和加载映射结果

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/MappingPreset.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "BoneMappingTypes.h"

/**
 * 映射预设：保存和加载完整的骨骼映射结果
 * 格式为 JSON，支持导入导出共享
 *
 * JSON 格式:
 * {
 *     "PresetName": "Mixamo_Standard",
 *     "SourceSkeleton": "SK_Mannequin_Skeleton",
 *     "TargetSkeleton": "SK_Enemy_Orc_Skeleton",
 *     "Mappings": [
 *         { "Source": "pelvis", "Target": "Hips", "Confidence": 1.0, "MatchType": "Alias" },
 *         ...
 *     ]
 * }
 */
class FMappingPreset
{
public:

    /** 预设名称 */
    FString PresetName;

    /** Source / Target Skeleton 名称（用于标识） */
    FString SourceSkeletonName;
    FString TargetSkeletonName;

    /** 完整映射数据 */
    TArray<FBoneMappingEntry> Mappings;

    /**
     * 保存预设到 JSON 文件
     * @param FilePath 输出文件路径
     * @return 是否保存成功
     */
    bool SaveToJsonFile(const FString& FilePath) const;

    /**
     * 从 JSON 文件加载预设
     * @param FilePath 输入文件路径
     * @return 是否加载成功
     */
    bool LoadFromJsonFile(const FString& FilePath);
};

完成后汇报: 确认 MappingPreset.h 是否已创建，包含:
  1. 预设元数据（名称 + Skeleton 名称）
  2. Mappings 数组
  3. SaveToJsonFile / LoadFromJsonFile 声明

===== 任务 27 结束 =====
```