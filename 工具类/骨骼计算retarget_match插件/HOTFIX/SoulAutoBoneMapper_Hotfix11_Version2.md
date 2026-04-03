# 热修复 11: 修复 SExpandableArea 语法

```
===== 热修复 11: 修复 SExpandableArea 语法 =====

目标: 修复 SBoneMappingWidget.cpp Construct 方法中两处 SExpandableArea 的语法错误

操作类型: 仅修改已有文件（2 处改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中 Construct 方法的两处 SExpandableArea
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

问题根因:
  UE4.27 中 SExpandableArea 的 BodyContent 是 Slate Slot，
  应使用方括号 [] 传递子 Widget，不能用圆括号 ()

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

修改 1: 找到权重配置面板的 SExpandableArea（约在 Construct 中部）:

错误写法:
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Match Weights")))
            .InitiallyCollapsed(true)
            .BodyContent
            (
                BuildWeightConfigPanel()
            )

替换为:
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Match Weights")))
            .InitiallyCollapsed(true)
            .BodyContent
            [
                BuildWeightConfigPanel()
            ]

修改 2: 找到别名编辑面板的 SExpandableArea（紧跟在权重面板之后）:

错误写法:
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Alias Editor")))
            .InitiallyCollapsed(true)
            .BodyContent
            (
                BuildAliasEditPanel()
            )

替换为:
            SNew(SExpandableArea)
            .AreaTitle(FText::FromString(TEXT("Advanced: Alias Editor")))
            .InitiallyCollapsed(true)
            .BodyContent
            [
                BuildAliasEditPanel()
            ]

总结: 两处圆括号 () 改为方括号 []，其他代码不动。

完成后汇报: 确认两处 SExpandableArea 的 BodyContent 是否已改为方括号

===== 热修复 11 结束 =====
```