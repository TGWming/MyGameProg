# 任务 32: 实现 SBoneMappingWidget — 资产选择回调

```
===== 任务 32: 实现 SBoneMappingWidget — 资产选择回调 =====

目标: 在 SBoneMappingWidget.cpp 末尾追加 OnSourceAssetChanged 和 OnTargetAssetChanged

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的 Construct 函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp 末尾追加以下代码:

void SBoneMappingWidget::OnSourceAssetChanged(const FAssetData& AssetData)
{
    SourceAsset = AssetData.GetAsset();
    SourceSkeleton = FBoneScanner::GetSkeletonFromAsset(SourceAsset.Get());

    if (SourceSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("BoneMapper: Source skeleton set: %s (%d bones)"),
            *SourceSkeleton->GetName(),
            SourceSkeleton->GetReferenceSkeleton().GetNum());
    }
    else if (SourceAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Could not extract skeleton from source asset: %s"),
            *SourceAsset->GetName());
    }
}

void SBoneMappingWidget::OnTargetAssetChanged(const FAssetData& AssetData)
{
    TargetAsset = AssetData.GetAsset();
    TargetSkeleton = FBoneScanner::GetSkeletonFromAsset(TargetAsset.Get());

    if (TargetSkeleton.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("BoneMapper: Target skeleton set: %s (%d bones)"),
            *TargetSkeleton->GetName(),
            TargetSkeleton->GetReferenceSkeleton().GetNum());
    }
    else if (TargetAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneMapper: Could not extract skeleton from target asset: %s"),
            *TargetAsset->GetName());
    }
}

完成后汇报: 确认两个回调方法是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 32 结束 =====
```