# 任务 65: 在 GUI 中添加批量重定向按钮 — 修改 SBoneMappingWidget.h

```
===== 任务 65: 在 GUI 中添加批量重定向按钮 — 修改 SBoneMappingWidget.h =====

目标: 在 SBoneMappingWidget.h 中新增批量重定向回调声明

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SBoneMappingWidget.h，禁止修改其他文件
- 禁止删除已有的成员变量和方法声明
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/SBoneMappingWidget.h

在 private 区域的 BuildTargetBoneNameList 声明之后，新增:

    /** 点击"批量重定向"按钮 */
    FReply OnBatchRetargetClicked();

不要删除或修改任何其他已有声明。

完成后汇报: 确认新增 OnBatchRetargetClicked 声明

===== 任务 65 结束 =====
```