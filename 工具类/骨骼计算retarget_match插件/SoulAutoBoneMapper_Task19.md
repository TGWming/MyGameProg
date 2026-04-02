# 任务 19: 实现 BoneMatcher — 左右对称性检测

```
===== 任务 19: 实现 BoneMatcher — 左右对称性检测 =====

目标: 在 BoneMatcher.cpp 末尾追加 IsSameBodySide 方法实现

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneMatcher.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneMatcher.cpp 末尾追加以下代码:

bool FBoneMatcher::IsSameBodySide(
    const FScannedBoneInfo& Source, const FScannedBoneInfo& Target)
{
    // 先尝试从名称判断左右侧
    // 常见左侧标识：_l, _L, Left, left, .L, .l
    // 常见右侧标识：_r, _R, Right, right, .R, .r
    const FString SourceStr = Source.BoneName.ToString().ToLower();
    const FString TargetStr = Target.BoneName.ToString().ToLower();

    auto IsLeftByName = [](const FString& Name) -> int32
    {
        if (Name.Contains(TEXT("left")) || Name.EndsWith(TEXT("_l")) ||
            Name.EndsWith(TEXT(".l")) || Name.Contains(TEXT("_l_")))
        {
            return -1; // 左侧
        }
        if (Name.Contains(TEXT("right")) || Name.EndsWith(TEXT("_r")) ||
            Name.EndsWith(TEXT(".r")) || Name.Contains(TEXT("_r_")))
        {
            return 1; // 右侧
        }
        return 0; // 中间 / 不确定
    };

    const int32 SourceSide = IsLeftByName(SourceStr);
    const int32 TargetSide = IsLeftByName(TargetStr);

    // 如果名称能判断左右，直接比较
    if (SourceSide != 0 && TargetSide != 0)
    {
        return SourceSide == TargetSide;
    }

    // 名称无法判断时，用 3D 位置的 X 坐标判断
    // 归一化空间中，X < 0.45 为左侧，X > 0.55 为右侧，中间为躯干
    const float SourceX = Source.NormalizedPosition.X;
    const float TargetX = Target.NormalizedPosition.X;

    const bool bSourceLeft = SourceX < 0.45f;
    const bool bSourceRight = SourceX > 0.55f;
    const bool bTargetLeft = TargetX < 0.45f;
    const bool bTargetRight = TargetX > 0.55f;

    // 如果一个明确在左，另一个明确在右 → 不同侧
    if ((bSourceLeft && bTargetRight) || (bSourceRight && bTargetLeft))
    {
        return false;
    }

    // 其他情况（都在中间、或只有一方偏侧）→ 不否决
    return true;
}

完成后汇报: 确认 IsSameBodySide 是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改

===== 任务 19 结束 =====
```