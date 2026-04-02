# 任务 51: 在 SBoneMappingWidget.h 中新增手动编辑映射相关声明

```
===== 任务 51: 在 SBoneMappingWidget.h 中新增手动编辑映射相关声明 =====

目标: 在 SBoneMappingWidget.h 中新增手动修改映射所需的方法和成员声明

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

在 private 区域的 OnLoadPresetClicked 声明之后，新增以下声明:

    // ===== 手动编辑映射 =====

    /** 当前正在编辑的映射条目（nullptr 表示未在编辑） */
    TSharedPtr<FBoneMappingEntry> EditingEntry;

    /** Target 侧所有骨骼名（供手动选择下拉列表使用） */
    TArray<TSharedPtr<FName>> AllTargetBoneNames;

    /** 点击某行的"Edit"按钮 */
    FReply OnEditMappingClicked(TSharedPtr<FBoneMappingEntry> Entry);

    /** 点击某行的"Clear"按钮（取消该行映射） */
    FReply OnClearMappingClicked(TSharedPtr<FBoneMappingEntry> Entry);

    /** 用户从下拉列表中选择了新的 Target 骨骼 */
    void OnTargetBoneSelected(TSharedPtr<FName> NewValue, ESelectInfo::Type SelectInfo);

    /** 构建 Target 骨骼名下拉列表的数据源 */
    void BuildTargetBoneNameList();

不要删除或修改任何其他已有声明。

完成后汇报: 确认 SBoneMappingWidget.h 修改是否完成:
  1. 新增 EditingEntry 成员
  2. 新增 AllTargetBoneNames 成员
  3. 新增 4 个方法声明

===== 任务 51 结束 =====
```