# 任务 43: 实现 RetargetApplier — HasHumanoidRig + GetRigMappingSummary

```
===== 任务 43: 实现 RetargetApplier — HasHumanoidRig + GetRigMappingSummary =====

目标: 在 RetargetApplier.cpp 末尾追加辅助查询方法

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 RetargetApplier.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/RetargetApplier.cpp 末尾追加以下代码:

bool FRetargetApplier::HasHumanoidRig(const USkeleton* Skeleton)
{
    if (!Skeleton)
    {
        return false;
    }

    const URig* Rig = Skeleton->GetRig();
    return Rig != nullptr;
}

FString FRetargetApplier::GetRigMappingSummary(const USkeleton* Skeleton)
{
    if (!Skeleton)
    {
        return TEXT("No skeleton");
    }

    const URig* Rig = Skeleton->GetRig();
    if (!Rig)
    {
        return FString::Printf(TEXT("Skeleton '%s': No Rig set"), *Skeleton->GetName());
    }

    int32 TotalNodes = Rig->GetNodeNum();
    int32 MappedCount = 0;

    for (int32 i = 0; i < TotalNodes; i++)
    {
        const FName& NodeName = Rig->GetNodeName(i);
        const FName MappedBone = Skeleton->GetRigBoneMapping(NodeName);

        if (MappedBone != NAME_None)
        {
            MappedCount++;
        }
    }

    return FString::Printf(
        TEXT("Skeleton '%s': Rig '%s', %d / %d nodes mapped"),
        *Skeleton->GetName(), *Rig->GetName(), MappedCount, TotalNodes);
}

完成后汇报: 确认两个方法是否已追加到 RetargetApplier.cpp 末尾，
且已有函数未被修改。至此 RetargetApplier 全部方法实现完毕。

===== 任务 43 结束 =====
```