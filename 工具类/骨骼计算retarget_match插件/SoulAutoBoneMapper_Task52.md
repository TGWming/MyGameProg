# 任务 52: 实现 BuildTargetBoneNameList

```
===== 任务 52: 实现 BuildTargetBoneNameList =====

目标: 在 SBoneMappingWidget.cpp 末尾追加构建 Target 骨骼名列表的方法

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 SBoneMappingWidget.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp 末尾追加以下代码:

void SBoneMappingWidget::BuildTargetBoneNameList()
{
    AllTargetBoneNames.Empty();

    if (!TargetSkeleton.IsValid())
    {
        return;
    }

    const FReferenceSkeleton& RefSkel = TargetSkeleton->GetReferenceSkeleton();
    const int32 NumBones = RefSkel.GetNum();

    // 添加 "None" 选项（用于取消映射）
    AllTargetBoneNames.Add(MakeShareable(new FName(NAME_None)));

    for (int32 i = 0; i < NumBones; i++)
    {
        AllTargetBoneNames.Add(MakeShareable(new FName(RefSkel.GetBoneName(i))));
    }
}

完成后汇报: 确认 BuildTargetBoneNameList 是否已追加到 SBoneMappingWidget.cpp 末尾，
且已有函数未被修改

===== 任务 52 结束 =====
```