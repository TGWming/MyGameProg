# 任务 21: 创建 BoneAliasTable 头文件

```
===== 任务 21: 创建 BoneAliasTable 头文件 =====

目标: 创建骨骼别名表的头文件，声明 BoneAliasTable 类及其公共接口

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/BoneAliasTable.h 内容如下:

#pragma once

#include "CoreMinimal.h"

/**
 * 骨骼别名表：维护 UE4 标准骨骼名 → 第三方常见别名的映射
 * 支持从外部 JSON 文件加载，用户可自行扩展
 *
 * JSON 格式:
 * {
 *     "pelvis": ["Hips", "hip", "Bip001_Pelvis"],
 *     "spine_01": ["Spine", "spine1", "Bip001_Spine"],
 *     ...
 * }
 *
 * Key = UE4 标准骨骼名
 * Value = 该骨骼在第三方格式中的常见别名数组
 */
class FBoneAliasTable
{
public:

    /**
     * 从 JSON 文件加载别名表
     * @param FilePath JSON 文件的完整路径
     * @return 是否加载成功
     */
    bool LoadFromJsonFile(const FString& FilePath);

    /**
     * 加载插件自带的默认别名表（Resources/DefaultAliases.json）
     * @return 是否加载成功
     */
    bool LoadDefaultAliases();

    /**
     * 查询某个 Target 骨骼名是否是某个 UE4 标准骨骼的别名
     * @param TargetBoneName 第三方骨骼名
     * @return 匹配到的 UE4 标准骨骼名，未找到则返回 NAME_None
     */
    FName FindStandardNameByAlias(const FName& TargetBoneName) const;

    /**
     * 查询某个 UE4 标准骨骼名的所有已知别名
     * @param StandardBoneName UE4 标准骨骼名
     * @return 别名数组，未找到则返回空数组
     */
    TArray<FName> GetAliasesForBone(const FName& StandardBoneName) const;

    /**
     * 手动添加一条别名映射
     * @param StandardBoneName UE4 标准骨骼名
     * @param Alias 第三方别名
     */
    void AddAlias(const FName& StandardBoneName, const FName& Alias);

    /** 清空所有别名数据 */
    void Clear();

    /** 获取已加载的别名总条数 */
    int32 GetTotalAliasCount() const;

private:

    // Key = UE4 标准骨骼名（小写），Value = 该骨骼的所有别名
    TMap<FName, TArray<FName>> StandardToAliases;

    // 反向索引：Key = 别名（小写），Value = UE4 标准骨骼名
    TMap<FName, FName> AliasToStandard;

    /** 将别名添加到反向索引 */
    void RebuildReverseIndex();
};

完成后汇报: 确认 BoneAliasTable.h 是否已创建，包含:
  1. LoadFromJsonFile 方法
  2. LoadDefaultAliases 方法
  3. FindStandardNameByAlias 方法
  4. GetAliasesForBone 方法
  5. AddAlias 方法
  6. Clear / GetTotalAliasCount 方法
  7. 两个 TMap 数据成员 + RebuildReverseIndex

===== 任务 21 结束 =====
```