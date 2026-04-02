# SoulAutoBoneMapper — Handoff 文档

> 日期：2026-04-02
> 状态：核心功能全部实现，编译通过，待实测

---

## 今日完成内容

### 60 个原子化 Prompt 全部执行完毕

| 批次 | 任务范围 | 内容 |
|------|---------|------|
| 1-5 | 插件框架 | 目录结构 + .uplugin + Build.cs + 模块注册 + 数据结构 |
| 6-12 | BoneScanner | 骨骼扫描器（深度/子节点/排行/指纹/归一化位置） |
| 13-20 | BoneMatcher | 匹配引擎（Levenshtein + 结构 5 维度 + 3D 位置 + 对称性 + 三层漏斗） |
| 21-30 | AliasTable + Preset + GUI 头文件 | 别名表 JSON + 预设保存加载 + Widget 声明 |
| 31-40 | GUI 实现 + 模块注册 | Construct 布局 + 回调 + 列表行 + Window 菜单入口 |
| 41-50 | RetargetApplier + 按钮栏 | 写入 Retarget Manager + Apply/Save/Load 按钮 |
| 51-60 | 手动编辑交互 | Edit/Clear 按钮 + 下拉选择器 + Action 列 |

---

## 文件清单

```
Plugins/SoulAutoBoneMapper/
├── SoulAutoBoneMapper.uplugin
├── Resources/
│   └── DefaultAliases.json              ← 可扩展别名表（Mixamo/Biped/CC）
└── Source/SoulAutoBoneMapper/
    ├── SoulAutoBoneMapper.Build.cs      ← 13 个模块依赖
    ├── Public/
    │   ├── SoulAutoBoneMapper.h         ← 模块定义 + 窗口注册
    │   ├── BoneMappingTypes.h           ← 枚举 + 4 个数据结构
    │   ├── BoneScanner.h                ← 骨骼扫描器（2 public + 5 private）
    │   ├── BoneMatcher.h                ← 匹配引擎（15 个方法）
    │   ├── BoneAliasTable.h             ← 别名表（7 个方法 + 2 Map）
    │   ├── MappingPreset.h              ← 预设保存/加载
    │   ├── RetargetApplier.h            ← 写入 Retarget Manager
    │   └── SBoneMappingWidget.h         ← Slate GUI 主界面
    └── Private/
        ├── SoulAutoBoneMapper.cpp       ← 模块实现 + Tab + 菜单注册
        ├── BoneScanner.cpp              ← 扫描器完整实现
        ├── BoneMatcher.cpp              ← 匹配引擎完整实现
        ├── BoneAliasTable.cpp           ← 别���表完整实现
        ├── MappingPreset.cpp            ← 预设序列化完整实现
        ├── RetargetApplier.cpp          ← Retarget 写入完整实现
        └── SBoneMappingWidget.cpp       ← GUI 完整实现
```

---

## 功能清单

```
✅ BoneScanner
   - GetSkeletonFromAsset（支持 Skeleton / SkeletalMesh 输入）
   - ScanSkeleton（提取 12 个维度元数据）
   - ComputeDepths / ChildCounts / SubtreeSize / SiblingIndices
   - ComputeChainFingerprints（路径编码）
   - ComputePositionMetrics（世界坐标 → 归一化 0~1 → 方向 → 长度比例）

✅ BoneMatcher（三层漏斗）
   - 第 1 层：精确 → 大小写无关 → 别名 → Levenshtein
   - 第 2 层：结构 5 维度 + 3D 位置 + 方向 + 长度 + 对称性一票否决
   - 第 3 层：锚点传播（已确认邻居反推）
   - 权重：名称 30% + 结构 25% + 位置 25% + 方向长度 10% + 对称 10%

✅ BoneAliasTable
   - JSON 可扩展别名表
   - DefaultAliases.json 覆盖 Mixamo / Biped / CC
   - 正向 + 反向索引查询

✅ MappingPreset
   - SaveToJsonFile / LoadFromJsonFile
   - 完整元数据（预设名 + Skeleton 名 + 映射数组）

✅ RetargetApplier
   - ApplyMappingToRetargetManager（写入 Rig 映射）
   - HasHumanoidRig / GetRigMappingSummary

✅ SBoneMappingWidget（Slate GUI）
   - Source / Target 资产选择器（支持 Skeleton + SkeletalMesh）
   - Scan & Match 一键匹配
   - 映射结果列表（5 列：Source / Status / Target / Conf / Action）
   - 状态颜色区分（绿=精确 / 蓝=别名 / 黄=模糊 / 紫=手动 / 红=未匹配）
   - 每行 Edit 按钮 → 下拉选择器切换 Target 骨骼
   - 每行 Clear 按钮 → 清除映射
   - Apply Mapping 按钮 → 写入 Retarget Manager
   - Save Preset / Load Preset 按钮 → JSON 文件对话框
   - 匹配统计文本 + 匹配率

✅ 模块注册
   - Window 菜单 → "Soul Auto Bone Mapper" 入口
   - Nomad Tab 窗口
```

---

## 下一步：实测验证

```
测试步骤：

1. 启用插件：
   Edit → Plugins → 搜索 "Soul Auto Bone Mapper" → 勾选 → 重启编辑器

2. 打开窗口：
   Window → Soul Auto Bone Mapper

3. 选择资产：
   Source: 拖入 UE4 Mannequin Skeleton（SK_Mannequin_Skeleton）
   Target: 拖入任意第三方 SkeletalMesh

4. 点击 Scan & Match

5. 检查 Output Log：
   搜索 "BoneMapper" 查看扫描和匹配日志
   应看到 Source bones / Target bones 数量
   应看到 Exact / Alias / Fuzzy / Structure / Unmatched 统计

6. 在列表中：
   - 确认绿色行（精确匹配）是否正确
   - 点击 Edit 按钮测试下拉选择
   - 点击 Clear 按钮测试清除
   - 检查匹配率数字

7. 测试 Apply：
   - 先在 Target Skeleton 的 Retarget Manager 中设置 Humanoid Rig
   - 回到插件窗口点击 Apply Mapping
   - 检查 Output Log 中的写入结果

8. 测试 Save / Load：
   - 点击 Save Preset → 选择路径保存
   - 点击 Load Preset → 加载刚才保存的文件
   - 确认列表恢复正确
```

---

## 后续可扩展功能（未实现）

| # | 功能 | 优先级 | 说明 |
|---|------|--------|------|
| 1 | 3D 骨骼高亮联动 | P2 | 点击列表行高亮 Viewport 中骨骼位置 |
| 2 | 批量重定向 | P2 | 选中多个动画资产一键重定向 |
| 3 | 权重配置 UI | P3 | GUI 中可调整各维度权重 |
| 4 | 别名表编辑 UI | P3 | 在插件内直接添加别名 |
| 5 | 非 Humanoid 支持 | P4 | 四足 / 多足骨骼映射 |