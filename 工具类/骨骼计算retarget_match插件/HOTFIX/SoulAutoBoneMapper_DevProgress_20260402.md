# SoulAutoBoneMapper 开发进度存档

> 存档日期: 2026-04-02
> 存档人: minglikesrunning-png
> 下次计划: 2026-04-03 晚上，启用插件到编辑器内测试功能
> 引擎版本: UE4.27

---

## 一、Prompt 执行状态

| 批次 | 任务范围 | 内容 | 状态 |
|------|---------|------|------|
| 1-5 | 插件框架 | 目录 + .uplugin + Build.cs + 模块注册 + 数据结构 | ✅ 编译通过 |
| 6-10 | BoneScanner 上半 | 头文件 + 框架 + 深度 + 子节点 + 排行 | ✅ 编译通过 |
| 11-20 | BoneScanner 下半 + BoneMatcher | 指纹 + 归一化 + 匹配引擎全部 | ✅ 编译通过 |
| 21-30 | AliasTable + Preset + GUI 头文件 | 别名表 + 预设 + Widget 声明 | ✅ 编译通过 |
| 31-40 | GUI 实现 + 模块注册 | Construct + 回调 + 列表行 + 菜单入口 | ✅ 编译通过 |
| 41-50 | RetargetApplier + 按钮栏 | 写入 Retarget + Apply/Save/Load | ✅ 编译通过 |
| 51-60 | 手动编辑交互 | Edit/Clear + 下拉选择器 + Action 列 | ✅ 编译通过 |
| 61-70 | 批量重定向 + 3D 高亮 | BatchRetargeter + BoneHighlighter | ✅ 编译通过 |
| 71-80 | 高亮联动 + 权重配置 UI | 列表选中联动 + SpinBox 面板 | ✅ 编译通过 |
| **81-84** | **别名编辑 UI** | **别名面板 + 添加/重载 + 最终编译** | **⏳ 待执行** |

```
已完成: 80 / 84 个 Prompt
待执行: 81, 82, 83, 84（别名编辑 UI + 最终编译验证）
```

---

## 二、文件清单与完成度

```
Plugins/SoulAutoBoneMapper/
├── SoulAutoBoneMapper.uplugin                   ✅ 已创建
├── Resources/
│   └── DefaultAliases.json                      ✅ 已创建（Mixamo/Biped/CC）
└── Source/SoulAutoBoneMapper/
    ├── SoulAutoBoneMapper.Build.cs              ✅ 14 个模块依赖
    ├── Public/
    │   ├── SoulAutoBoneMapper.h                 ✅ 模块定义 + 窗口注册
    │   ├── BoneMappingTypes.h                   ✅ 5 个数据结构（含 FMatchWeightConfig）
    │   ├── BoneScanner.h                        ✅ 扫描器声明
    │   ├── BoneMatcher.h                        ✅ 匹配引擎声明（含 WeightConfig 参数）
    │   ├── BoneAliasTable.h                     ✅ 别名表声明
    │   ├── MappingPreset.h                      ✅ 预设声明
    │   ├── RetargetApplier.h                    ✅ Retarget 写入声明
    │   ├── BatchRetargeter.h                    ✅ 批量重定向声明
    │   ├── BoneHighlighter.h                    ✅ 3D 高亮声明
    │   └── SBoneMappingWidget.h                 ✅ GUI 声明（含别名编辑声明，待实现）
    └── Private/
        ├── SoulAutoBoneMapper.cpp               ✅ 模块注册 + Tab + 菜单
        ├── BoneScanner.cpp                      ✅ 完整实现（7 个方法）
        ├── BoneMatcher.cpp                      ✅ 完整实现（15 个方法）
        ├── BoneAliasTable.cpp                   ✅ 完整实现（7 个方法）
        ├── MappingPreset.cpp                    ✅ 完整实现
        ├── RetargetApplier.cpp                  ✅ 完整实现
        ├── BatchRetargeter.cpp                  ✅ 完整实现
        ├── BoneHighlighter.cpp                  ✅ 完整实现
        └── SBoneMappingWidget.cpp               ✅ 大部分完成（别名面板待嵌入）
```

---

## 三、功能完成度

| 功能模块 | 状态 | 说明 |
|---------|------|------|
| 插件框架 + 模块注册 | ✅ 完工 | Editor Only, Window 菜单入口 |
| BoneScanner 骨骼扫描器 | ✅ 完工 | 12 维度元数据提取 |
| BoneMatcher 匹配引擎 | ✅ 完工 | 三层漏斗 + 加权评分 + 锚点传播 |
| BoneAliasTable 别名表 | ✅ 完工 | JSON 可扩展 + 正反向索引 |
| MappingPreset 预设系统 | ✅ 完工 | Save/Load JSON |
| RetargetApplier 映射写入 | ✅ 完工 | 写入 Rig 映射 + 验证 |
| BatchRetargeter 批量重定向 | ✅ 完工 | Content Browser 选中 + 复制模式 |
| BoneHighlighter 3D 高亮 | ✅ 完工 | Debug 球 + 连线 + 5 秒持续 |
| GUI — 资产选择 | ✅ 完工 | 支持 Skeleton + SkeletalMesh |
| GUI — Scan & Match | ✅ 完工 | 一键扫描匹配 |
| GUI — 映射结果列表 | ✅ 完工 | 5 列 + 颜色状态 + 统计 |
| GUI — 手动编辑映射 | ✅ 完工 | Edit + Clear + 下拉选择 |
| GUI — Apply / Save / Load / Batch | ✅ 完工 | 4 个功能按钮 |
| GUI — 3D 高亮联动 | ✅ 完工 | 列表选中 → Viewport 高亮 |
| GUI — 权重配置面板 | ✅ 完工 | 可折叠 + 6 个 SpinBox |
| GUI — 别名编辑面板 | ⏳ 待执行 | 任务 81-83（声明已在 .h 中） |
| 最终编译验证 | ⏳ 待执行 | 任务 84 |

---

## 四、待执行 Prompt（4 个）

```
任务 81: SBoneMappingWidget.h — 新增别名编辑声明（2 成员 + 5 方法）
         注意：.h 中的声明部分已包含在任务 81 中
         
任务 82: SBoneMappingWidget.cpp — 追加 5 个别名编辑回调方法
         OnAliasStandardBoneTextChanged
         OnAliasNewAliasTextChanged
         OnAddAliasClicked
         OnReloadAliasTableClicked
         GetAliasTableStatsText

任务 83: SBoneMappingWidget.cpp — 追加 BuildAliasEditPanel
         + 修改 Construct 嵌入可折叠别名编辑面板

任务 84: 最终编译验证（Development Editor + Win64）
```

---

## 五、明天测试计划

### 步骤 1: 执行剩余 4 个 Prompt（任务 81-84）

### 步骤 2: 启用插件
```
Edit → Plugins → 搜索 "Soul Auto Bone Mapper" → 勾选 Enabled → 重启编辑器
```

### 步骤 3: 打开窗口
```
Window → Soul Auto Bone Mapper
```

### 步骤 4: 基础功能测试
```
测试项 1 — 资产选择:
  Source: 拖入 SK_Mannequin_Skeleton
  Target: 拖入任意第三方 SkeletalMesh
  验证: 控制台出现 "Source skeleton set" / "Target skeleton set" 日志

测试项 2 — Scan & Match:
  点击 Scan & Match
  验证: 列表填充映射结果，统计栏显示数字
  验证: 控制台出现 Exact/Alias/Fuzzy/Structure/Unmatched 统计

测试项 3 — 手动编辑:
  点击任意行的 Edit → 下拉选择新骨骼 → 确认状态变为 Manual
  点击 X → 确认状态变为 Unmatched

测试项 4 — 3D 高亮:
  点击列表中的一行 → Viewport 中出现绿/蓝球 + 黄色连线
  点击 Clear Highlights → 高亮消失

测试项 5 — 权重调整:
  展开 "Advanced: Match Weights" → 调整权重 → 重新 Scan & Match
  验证: 不同权重产生不同匹配结果

测试项 6 — Apply Mapping:
  先在 Target Skeleton 的 Retarget Manager 中设置 Humanoid Rig
  回到插件点击 Apply Mapping
  验证: 控制台出现 "Applied X mappings" 日志
  打开 Retarget Manager 验证映射是否写入

测试项 7 — Save / Load 预设:
  点击 Save Preset → 选路径保存 → 确认 JSON 文件生成
  点击 Load Preset → 选刚才的文件 → 确认列表恢复

测试项 8 — 批量重定向:
  在 Content Browser 选中几个 AnimSequence
  回到插件点击 Batch Retarget
  验证: /Game/Retargeted/ 目录下生成新动画资产
```

### 步骤 5: 记录问题
```
测试中发现任何问题，记录以下信息发给我：
  - 问题描述
  - Output Log 中 "BoneMapper" / "RetargetApplier" / "BatchRetargeter" 相关日志
  - 如果崩溃，记录完整调用栈
```

---

## 六、技术备忘

```
关键 API:
  USkeleton::GetReferenceSkeleton()           → 骨骼树
  FReferenceSkeleton::GetBoneName()           → 名称
  FReferenceSkeleton::GetParentIndex()        → 层级
  FReferenceSkeleton::GetRawRefBonePose()     → 局部 Transform
  USkeletalMesh::Skeleton                     → 从 Mesh 提取 Skeleton（UE4.27 用 ->Skeleton）
  USkeleton::GetRig()                         → 获取 Rig
  USkeleton::SetRigBoneMapping()              → 写入 Rig 映射
  USkeleton::GetRigBoneMapping()              → 读取 Rig 映射
  URig::FindNode() / GetNodeNum() / GetNodeName() → Rig 节点操作

UE4.27 ��意事项:
  - USkeletalMesh::GetSkeleton() 可能不存在，用 Mesh->Skeleton 直接访问
  - FBox 的构造需要 ForceInit
  - Algo::Reverse 用于 TArray 翻转
  - SExpandableArea 在 Widgets/Layout/SExpandableArea.h
  - SSpinBox 在 Widgets/Input/SSpinBox.h
  - SEditableTextBox 在 Widgets/Input/SEditableTextBox.h
  - SComboBox 在 Widgets/Input/SComboBox.h

匹配引擎权重（默认值，可通过 GUI 调整）:
  名称 30% / 结构 25% / 位置 25% / 方向长度 10% / 对称 10%
  阈值 0.4（低于此值视为未匹配）
```

---

## 七、Prompt 规范备忘

```
插件名: SoulAutoBoneMapper
引擎: UE4.27
原子化任务，每步改动尽量不超过 10 行（纯新增除外）
禁止越界删除无关函数和模块
禁止自动编译或生成解决方案
不使用 PowerShell 或终端命令搜索文件
每步执行后简单汇报完成状态
每个步骤的完整 Prompt 在 code 框内可一键复制
```

---

> 存档完毕。明天晚上先执行任务 81-84，然后按测试计划验证。
> 测试中有任何问题或日志，发到这个 Chat 里继续。