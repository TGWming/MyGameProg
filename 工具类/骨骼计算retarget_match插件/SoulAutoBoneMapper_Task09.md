# 任务 9: 实现 BoneScanner — ComputeChildCountsAndSubtreeSize

```
===== 任务 9: 实现 BoneScanner — ComputeChildCountsAndSubtreeSize =====

目标: 在 BoneScanner.cpp 末尾追�� ComputeChildCountsAndSubtreeSize 方法实现

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

void FBoneScanner::ComputeChildCountsAndSubtreeSize(
    const FReferenceSkeleton& RefSkel,
    TArray<FScannedBoneInfo>& OutBones)
{
    const int32 NumBones = OutBones.Num();

    // 第 1 步：计算每根骨骼的直接子节点数
    for (int32 i = 0; i < NumBones; i++)
    {
        const int32 Parent = OutBones[i].ParentIndex;
        if (Parent != INDEX_NONE)
        {
            OutBones[Parent].ChildCount++;
        }
    }

    // 第 2 步：从叶子节点向根反向累加子树规模
    // 骨骼索引保证 子 > 父，所以倒序遍历即可
    for (int32 i = NumBones - 1; i >= 0; i--)
    {
        // 自身也算 1（SubtreeSize 包含自己）
        OutBones[i].SubtreeSize += 1;

        const int32 Parent = OutBones[i].ParentIndex;
        if (Parent != INDEX_NONE)
        {
            OutBones[Parent].SubtreeSize += OutBones[i].SubtreeSize;
        }
    }
}

完成后汇报: 确认 ComputeChildCountsAndSubtreeSize 是否已追加到 BoneScanner.cpp 末尾，
且已有函数未被修改

===== 任务 9 结束 =====
```