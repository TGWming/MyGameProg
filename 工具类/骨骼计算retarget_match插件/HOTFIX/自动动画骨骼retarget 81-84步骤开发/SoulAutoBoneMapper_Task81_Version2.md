# 任务 81: 在 SBoneMappingWidget.h 中新增别名编辑相关声明

```
===== 任务 81: 在 SBoneMappingWidget.h 中新增别名编辑相关声明 =====

目标: 在 SBoneMappingWidget.h 中新增别名编辑 UI 的方法和成员声明

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

在 private 区域的 OnWeightValueChanged 声明之后，新增:

    // ===== 别名编辑 =====

    /** 别名编辑面板中的"标准骨骼名"输入框内容 */
    FText AliasStandardBoneText;

    /** 别名编辑面板中的"别名"输入框内容 */
    FText AliasNewAliasText;

    /** 创建别名编辑面板 Widget */
    TSharedRef<SWidget> BuildAliasEditPanel();

    /** 标准骨骼名输入变化 */
    void OnAliasStandardBoneTextChanged(const FText& NewText);

    /** 别名输入变化 */
    void OnAliasNewAliasTextChanged(const FText& NewText);

    /** 点击"添加别名"按钮 */
    FReply OnAddAliasClicked();

    /** 点击"重新加载别名表"按钮 */
    FReply OnReloadAliasTableClicked();

    /** 获取当前别名表统计文本 */
    FText GetAliasTableStatsText() const;

不要删除或修改任何其他已有声明。

完成后汇报: 确认修改是否完成:
  1. 新增 AliasStandardBoneText / AliasNewAliasText 成员
  2. 新增 5 个方法声明

===== 任务 81 结束 =====
```