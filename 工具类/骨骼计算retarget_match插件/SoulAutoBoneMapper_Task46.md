# 任务 46: 实现 OnApplyMappingClicked

```
===== 任务 46: 实现 OnApplyMappingClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加应用映射回调

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

首先在 SBoneMappingWidget.cpp 顶部的 #include 区域末尾追加:

#include "RetargetApplier.h"

然后在文件末尾追加以下代码:

FReply SBoneMappingWidget::OnApplyMappingClicked()
{
    if (!SourceSkeleton.IsValid() || !TargetSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Cannot apply — source or target skeleton is invalid"));
        return FReply::Handled();
    }

    if (MappingEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: No mappings to apply. Run Scan & Match first."));
        return FReply::Handled();
    }

    // 检查 Target Skeleton 是否已设置 Rig
    if (!FRetargetApplier::HasHumanoidRig(TargetSkeleton.Get()))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BoneMapper: Target skeleton '%s' has no Humanoid Rig set. "
                 "Please set a Rig in Retarget Manager before applying."),
            *TargetSkeleton->GetName());
        return FReply::Handled();
    }

    // 将 SharedPtr 数组转为普通数组
    TArray<FBoneMappingEntry> PlainMappings;
    PlainMappings.Reserve(MappingEntries.Num());
    for (const auto& Entry : MappingEntries)
    {
        PlainMappings.Add(*Entry);
    }

    // 应用映射前的状态
    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Before apply — %s"),
        *FRetargetApplier::GetRigMappingSummary(TargetSkeleton.Get()));

    // 应用
    int32 AppliedCount = FRetargetApplier::ApplyMappingToRetargetManager(
        SourceSkeleton.Get(), TargetSkeleton.Get(), PlainMappings);

    // 应用后的状态
    UE_LOG(LogTemp, Log, TEXT("BoneMapper: After apply — %s"),
        *FRetargetApplier::GetRigMappingSummary(TargetSkeleton.Get()));

    UE_LOG(LogTemp, Log, TEXT("BoneMapper: Successfully applied %d mappings"), AppliedCount);

    return FReply::Handled();
}

完成后汇报: 确认 OnApplyMappingClicked 是否已追加，
且新增了 #include "RetargetApplier.h"

===== 任务 46 结束 =====
```