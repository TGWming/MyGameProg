# 任务 59: 在 OnLoadPresetClicked 中同步刷新 Target 骨骼名列表

```
===== 任务 59: 在 OnLoadPresetClicked 中同步刷新 Target 骨骼名列表 =====

目标: 修改 OnLoadPresetClicked，在加载预设后也构建 Target 骨骼名列表，
确保加载预设后手动编辑功能可用

操作类型: 仅修改已有文件（约 1 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.cpp 中的 OnLoadPresetClicked 函数
- 禁止删除或修改其他函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SBoneMappingWidget.cpp

在 OnLoadPresetClicked 函数中，找到:

    // 填充到 GUI 列表
    MappingEntries.Empty();

在该行之前插入:

    // 确保 Target 骨骼名列表可用
    BuildTargetBoneNameList();

不要修改函数中的其他代码。

完成后汇报: 确认 BuildTargetBoneNameList() 调用是否已插入到
OnLoadPresetClicked 中正确的位置

===== 任务 59 结束 =====
```