# 任务 10: 实现 BoneScanner — ComputeSiblingIndices

```
===== 任务 10: 实现 BoneScanner — ComputeSiblingIndices =====

目标: 在 BoneScanner.cpp 末尾追加 ComputeSiblingIndices 方法实现

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

void FBoneScanner::ComputeSiblingIndices(
    const FReferenceSkeleton& RefSkel,
    TArray<FScannedBoneInfo>& OutBones)
{
    const int32 NumBones = OutBones.Num();

    // 用 Map 记录每个父骨骼当前已分配的排行计数器
    TMap<int32, int32> ParentChildCounter;

    for (int32 i = 0; i < NumBones; i++)
    {
        const int32 Parent = OutBones[i].ParentIndex;

        if (Parent == INDEX_NONE)
        {
            // 根骨骼，排行为 0
            OutBones[i].SiblingIndex = 0;
        }
        else
        {
            // 从 Map 中取当前计数，分配后递增
            int32& Counter = ParentChildCounter.FindOrAdd(Parent, 0);
            OutBones[i].SiblingIndex = Counter;
            Counter++;
        }
    }
}

完成后汇报: 确认 ComputeSiblingIndices 是否已追加到 BoneScanner.cpp 末尾，
且已有函数未被修改

===== 任务 10 结束 =====
```