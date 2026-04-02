# 任务 11: 实现 BoneScanner — ComputeChainFingerprints

```
===== 任务 11: 实现 BoneScanner — ComputeChainFingerprints =====

目标: 在 BoneScanner.cpp 末尾追加 ComputeChainFingerprints 方法实现

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneScanner.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneScanner.cpp 末尾追加以下代码:

void FBoneScanner::ComputeChainFingerprints(
    const FReferenceSkeleton& RefSkel,
    TArray<FScannedBoneInfo>& OutBones)
{
    const int32 NumBones = OutBones.Num();

    for (int32 i = 0; i < NumBones; i++)
    {
        // 从当前骨骼向根回溯，收集路径
        TArray<FString> PathSegments;
        int32 Current = i;

        while (Current != INDEX_NONE)
        {
            int32 ParentChildCount = 1;
            const int32 Parent = OutBones[Current].ParentIndex;

            if (Parent != INDEX_NONE)
            {
                ParentChildCount = OutBones[Parent].ChildCount;
            }

            // 格式: "父节点子节点数-当前骨骼排行"
            PathSegments.Add(FString::Printf(TEXT("%d-%d"),
                ParentChildCount, OutBones[Current].SiblingIndex));

            Current = OutBones[Current].ParentIndex;
        }

        // 回溯得到的路径是从叶到根，需要翻转为从根到叶
        Algo::Reverse(PathSegments);

        // 拼接为逗号分隔的字符串
        OutBones[i].ChainFingerprint = FString::Join(PathSegments, TEXT(","));
    }
}

完成后汇报: 确认 ComputeChainFingerprints 是否已追加到 BoneScanner.cpp 末尾，
且已有函数未被修改

===== 任务 11 结束 =====
```