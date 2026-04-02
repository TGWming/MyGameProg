# 任务 63: 实现 BatchRetargeter — RetargetAnimations

```
===== 任务 63: 实现 BatchRetargeter — RetargetAnimations =====

目标: 在 BatchRetargeter.cpp 末尾追加 RetargetAnimations 方法

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BatchRetargeter.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BatchRetargeter.cpp 末尾追加以下代码:

int32 FBatchRetargeter::RetargetAnimations(
    USkeleton* SourceSkeleton,
    USkeleton* TargetSkeleton,
    const TArray<UAnimSequence*>& AnimSequences,
    bool bDuplicateAssets,
    const FString& OutputPath)
{
    if (!SourceSkeleton || !TargetSkeleton)
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchRetargeter: Invalid skeleton input"));
        return 0;
    }

    if (AnimSequences.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchRetargeter: No animations to retarget"));
        return 0;
    }

    UE_LOG(LogTemp, Log, TEXT("BatchRetargeter: Retargeting %d animations from '%s' to '%s'"),
        AnimSequences.Num(), *SourceSkeleton->GetName(), *TargetSkeleton->GetName());

    int32 SuccessCount = 0;

    // 确定输出路径
    FString ActualOutputPath = OutputPath;
    if (ActualOutputPath.IsEmpty())
    {
        ActualOutputPath = FString::Printf(TEXT("/Game/Retargeted/%s"), *TargetSkeleton->GetName());
    }

    for (UAnimSequence* SourceAnim : AnimSequences)
    {
        if (!SourceAnim)
        {
            continue;
        }

        // 验证动画属于 Source Skeleton
        if (SourceAnim->GetSkeleton() != SourceSkeleton)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("BatchRetargeter: Animation '%s' does not belong to source skeleton, skipping"),
                *SourceAnim->GetName());
            continue;
        }

        if (bDuplicateAssets)
        {
            // 复制模式：创建新资产
            FString NewAssetName = FString::Printf(TEXT("%s_Retargeted"), *SourceAnim->GetName());
            FString NewAssetPath = FString::Printf(TEXT("%s/%s"), *ActualOutputPath, *NewAssetName);

            UAnimSequence* DuplicatedAnim = DuplicateObject<UAnimSequence>(
                SourceAnim, CreatePackage(nullptr, *NewAssetPath), *NewAssetName);

            if (DuplicatedAnim)
            {
                DuplicatedAnim->ReplaceSkeleton(TargetSkeleton);
                DuplicatedAnim->MarkPackageDirty();
                SuccessCount++;

                UE_LOG(LogTemp, Log, TEXT("BatchRetargeter: Retargeted '%s' -> '%s'"),
                    *SourceAnim->GetName(), *NewAssetPath);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BatchRetargeter: Failed to duplicate '%s'"),
                    *SourceAnim->GetName());
            }
        }
        else
        {
            // 原地修改模式
            SourceAnim->ReplaceSkeleton(TargetSkeleton);
            SourceAnim->MarkPackageDirty();
            SuccessCount++;

            UE_LOG(LogTemp, Log, TEXT("BatchRetargeter: Retargeted in-place '%s'"),
                *SourceAnim->GetName());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BatchRetargeter: Completed %d / %d retargets"),
        SuccessCount, AnimSequences.Num());

    return SuccessCount;
}

完成后汇报: 确认 RetargetAnimations 是否已追加到 BatchRetargeter.cpp 末尾，
且已有函数未被修改

===== 任务 63 结束 =====
```