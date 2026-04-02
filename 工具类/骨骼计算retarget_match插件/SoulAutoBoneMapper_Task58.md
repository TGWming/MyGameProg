# 任务 58: 在 Construct 中为 Source 选择器增加 SkeletalMesh 类型提示

```
===== 任务 58: 在 Construct 中为资产选择器增加类型提示文本 =====

目标: 在 Source 和 Target 资产选择器下方各添加一行小字提示，
告知用户支持拖入 Skeleton 或 SkeletalMesh

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 Construct 方法
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 Construct 方法中，在 Source 资产选择器的 Slot 之后、
Target 资产选择器的 Slot 之前，插入一行提示:

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(128.f, 0.f, 8.f, 4.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Supports: Skeleton or SkeletalMesh (auto-extracts Skeleton)")))
            .Font(FCoreStyle::GetDefaultFontStyle("Italic", 8))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
        ]

同样在 Target 资产选择器的 Slot 之后、Scan & Match 按钮之前，
插入同样的提示（可以复用相同代码）:

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(128.f, 0.f, 8.f, 4.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Supports: Skeleton or SkeletalMesh (auto-extracts Skeleton)")))
            .Font(FCoreStyle::GetDefaultFontStyle("Italic", 8))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
        ]

完成后汇报: 确认两行提示文本是否已插入到正确位置

===== 任务 58 结束 =====
```