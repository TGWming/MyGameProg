# 任务 23: 实现 BoneAliasTable — 查询方法

```
===== 任务 23: 实现 BoneAliasTable — 查询方法 =====

目标: 在 BoneAliasTable.cpp 末尾追加 FindStandardNameByAlias 和 GetAliasesForBone

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneAliasTable.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneAliasTable.cpp 末尾追加以下代码:

FName FBoneAliasTable::FindStandardNameByAlias(const FName& TargetBoneName) const
{
    const FName LowerName = FName(*TargetBoneName.ToString().ToLower());

    // 先查反向索引
    const FName* Found = AliasToStandard.Find(LowerName);
    if (Found)
    {
        return *Found;
    }

    // 也检查是否本身就是标准名
    if (StandardToAliases.Contains(LowerName))
    {
        return LowerName;
    }

    return NAME_None;
}

TArray<FName> FBoneAliasTable::GetAliasesForBone(const FName& StandardBoneName) const
{
    const FName LowerName = FName(*StandardBoneName.ToString().ToLower());

    const TArray<FName>* Found = StandardToAliases.Find(LowerName);
    if (Found)
    {
        return *Found;
    }

    return TArray<FName>();
}

完成后汇报: 确认两个查询方法是否已追加到 BoneAliasTable.cpp 末尾，
且已有函数未被修改。至此 BoneAliasTable 全部方法实现完毕。

===== 任务 23 结束 =====
```