# 任务 15: 实现 BoneMatcher — 精确匹配 + 大小写无关匹配

```
===== 任务 15: 实现 BoneMatcher — 精确匹配 + 大小写无关匹配 =====

目标: 在 BoneMatcher.cpp 末尾追加 ScoreExactMatch 和 ScoreCaseInsensitiveMatch

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneMatcher.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数���模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneMatcher.cpp 末尾追加以下代码:

float FBoneMatcher::ScoreExactMatch(const FName& SourceName, const FName& TargetName)
{
    // FName 比较默认大小写无关，所以用字符串精确比较
    return SourceName.ToString() == TargetName.ToString() ? 1.0f : 0.0f;
}

float FBoneMatcher::ScoreCaseInsensitiveMatch(const FName& SourceName, const FName& TargetName)
{
    return SourceName.ToString().ToLower() == TargetName.ToString().ToLower() ? 1.0f : 0.0f;
}

完成后汇报: 确认两个函数是否已追加到 BoneMatcher.cpp 末尾，
且已有函数未被修改

===== 任务 15 结束 =====
```