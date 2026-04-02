# 任务 76: 在 SBoneMappingWidget.h 中新增权重配置成员和 GUI 方法

```
===== 任务 76: 在 SBoneMappingWidget.h 中新增权重配置成员和 GUI 方法 =====

目标: 在 SBoneMappingWidget.h 中新增 FMatchWeightConfig 成员和权重显示方法

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

在 private 区域的 AliasTable 成员声明之后，新增:

    /** 匹配权重配置 */
    FMatchWeightConfig WeightConfig;

在 OnClearHighlightsClicked 声明之后，新增:

    /** 创建权重配置面板 Widget */
    TSharedRef<SWidget> BuildWeightConfigPanel();

    /** 权重 SpinBox 值变化回调 */
    void OnWeightValueChanged(float NewValue, float* TargetWeight);

不要删除或修改任何其他已有声明。

完成后汇报: 确认修改是否完成:
  1. 新增 WeightConfig 成员
  2. 新增 BuildWeightConfigPanel 声明
  3. 新增 OnWeightValueChanged 声明

===== 任务 76 结束 =====
```