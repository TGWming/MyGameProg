# 任务 8: 实现 BoneScanner — ComputeDepths

```
===== 任务 8: 实现 BoneScanner — ComputeDepths =====

目标: 在 BoneScanner.cpp 末尾追加 ComputeDepths 方法实现

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneScanner.cpp 中已有的 GetSkeletonFromAsset 和 ScanSkeleton 函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不���用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneScanner.cpp 末尾追加以下代码:

void FBoneScanner::ComputeDepths(
    const FReferenceSkeleton& RefSkel,
    TArray<FScannedBoneInfo>& OutBones)
{
    const int32 NumBones = OutBones.Num();

    for (int32 i = 0; i < NumBones; i++)
    {
        int32 Depth = 0;
        int32 Current = OutBones[i].ParentIndex;

        while (Current != INDEX_NONE)
        {
            Depth++;
            Current = RefSkel.GetParentIndex(Current);
        }

        OutBones[i].Depth = Depth;
    }
}

完成后汇报: 确认 ComputeDepths 是否已追加到 BoneScanner.cpp 末尾，
且已有函数未被修改

===== 任务 8 结束 =====
```