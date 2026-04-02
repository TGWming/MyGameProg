# 任务 67: 实现 OnBatchRetargetClicked

```
===== 任务 67: 实现 OnBatchRetargetClicked =====

目标: 在 SBoneMappingWidget.cpp 末尾追加批量重定向回调

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

首先在 SBoneMappingWidget.cpp 顶部 #include 区域末尾追加（如果没有）:

#include "BatchRetargeter.h"

然后在文件末尾追加以下代码:

FReply SBoneMappingWidget::OnBatchRetargetClicked()
{
    if (!SourceSkeleton.IsValid() || !TargetSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BoneMapper: Cannot batch retarget — source or target skeleton is invalid"));
        return FReply::Handled();
    }

    // 从 Content Browser 获取选中的动画
    TArray<UAnimSequence*> SelectedAnims = FBatchRetargeter::GetSelectedAnimSequences();

    if (SelectedAnims.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BoneMapper: No AnimSequences selected in Content Browser. "
                 "Please select animations to retarget first."));
        return FReply::Handled();
    }

    // 执行批量重定向（复制模式）
    int32 Count = FBatchRetargeter::RetargetAnimations(
        SourceSkeleton.Get(),
        TargetSkeleton.Get(),
        SelectedAnims,
        true); // bDuplicateAssets = true

    UE_LOG(LogTemp, Log,
        TEXT("BoneMapper: Batch retarget completed — %d / %d animations processed"),
        Count, SelectedAnims.Num());

    return FReply::Handled();
}

完成后汇报: 确认 OnBatchRetargetClicked 是否已追加，
且新增了 #include "BatchRetargeter.h"

===== 任务 67 结束 =====
```