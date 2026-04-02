# 任务 30: 创建 SBoneMappingWidget 头文件（GUI 基础框架）

```
===== 任务 30: 创建 SBoneMappingWidget 头文件（GUI 基础框架） =====

目标: 创建 Slate GUI 的头文件，声明主界面 Widget 类

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/SBoneMappingWidget.h 内容如下:

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "BoneMappingTypes.h"
#include "BoneAliasTable.h"

class USkeleton;

/**
 * Soul Auto Bone Mapper 主界面
 * Slate Widget：包含资产选择 + Scan & Match 按钮 + 映射结果列表
 */
class SBoneMappingWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBoneMappingWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:

    // ===== 资产引用 =====

    /** Source 侧资产（Skeleton 或 Mesh） */
    TWeakObjectPtr<UObject> SourceAsset;

    /** Target 侧资产（Skeleton 或 Mesh） */
    TWeakObjectPtr<UObject> TargetAsset;

    /** 从资产提取的 Skeleton */
    TWeakObjectPtr<USkeleton> SourceSkeleton;
    TWeakObjectPtr<USkeleton> TargetSkeleton;

    // ===== 匹配结果 =====

    /** 映射结果（用 SharedPtr 供 SListView 使用） */
    TArray<TSharedPtr<FBoneMappingEntry>> MappingEntries;

    /** 别名表 */
    FBoneAliasTable AliasTable;

    // ===== GUI 组件引用 =====

    /** 映射结果列表 */
    TSharedPtr<SListView<TSharedPtr<FBoneMappingEntry>>> MappingListView;

    // ===== 回调方法 =====

    /** Source 资产选择变化 */
    void OnSourceAssetChanged(const FAssetData& AssetData);

    /** Target 资产选择变化 */
    void OnTargetAssetChanged(const FAssetData& AssetData);

    /** 点击 Scan & Match 按钮 */
    FReply OnScanAndMatchClicked();

    /** 生成列表的每一行 */
    TSharedRef<ITableRow> OnGenerateMappingRow(
        TSharedPtr<FBoneMappingEntry> InEntry,
        const TSharedRef<STableViewBase>& OwnerTable);

    /** 获取匹配统计文本 */
    FText GetMatchStatisticsText() const;

    /** 获取匹配率文本 */
    FText GetMatchRateText() const;
};

完成后汇报: 确认 SBoneMappingWidget.h 是否已创建，包含:
  1. SCompoundWidget 继承
  2. Source / Target 资产引用
  3. MappingEntries 数组
  4. AliasTable 成员
  5. MappingListView 引用
  6. 回调方法声明

===== 任务 30 结束 =====
```