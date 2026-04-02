# 任务 62: 实现 BatchRetargeter — GetSelectedAnimSequences

```
===== 任务 62: 实现 BatchRetargeter — GetSelectedAnimSequences =====

目标: 创建 BatchRetargeter.cpp，实现从 Content Browser 获取选中动画资产

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BatchRetargeter.cpp 内容如下:

#include "BatchRetargeter.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

TArray<UAnimSequence*> FBatchRetargeter::GetSelectedAnimSequences()
{
    TArray<UAnimSequence*> Result;

    FContentBrowserModule& ContentBrowserModule =
        FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    TArray<FAssetData> SelectedAssets;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

    for (const FAssetData& AssetData : SelectedAssets)
    {
        UObject* Asset = AssetData.GetAsset();
        if (UAnimSequence* AnimSeq = Cast<UAnimSequence>(Asset))
        {
            Result.Add(AnimSeq);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BatchRetargeter: Found %d AnimSequences in selection"), Result.Num());

    return Result;
}

完成后汇报: 确认 BatchRetargeter.cpp 是否已创建，包含:
  1. GetSelectedAnimSequences 完整实现
  注意: RetargetAnimations 将在下一个任务中追加

===== 任务 62 结束 =====
```