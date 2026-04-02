# 任务 53: 在 OnScanAndMatchClicked 中调用 BuildTargetBoneNameList

```
===== 任务 53: 在 OnScanAndMatchClicked 中调用 BuildTargetBoneNameList =====

目标: 修改 OnScanAndMatchClicked 方法，在匹配完成后构建 Target 骨骼名列表

操作类型: 仅修改已有文件（约 2 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 OnScanAndMatchClicked 函数
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 OnScanAndMatchClicked 函数中，找到以下代码:

    // 刷新列表
    if (MappingListView.IsValid())
    {
        MappingListView->RequestListRefresh();
    }

在该段代码之前（在填充 MappingEntries 循环之后），插入一行:

    // 构建 Target 骨骼名下拉列表
    BuildTargetBoneNameList();

不要修改函数中的其他代码。

完成后汇报: 确认 BuildTargetBoneNameList() 调用是否已插入到
OnScanAndMatchClicked 中正确的位置

===== 任务 53 结束 =====
```