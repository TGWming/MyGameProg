# 🔄 Hand-Off 进度文档 v2

> **最后更新：** 2026-03-28
> **文档版本：** v2（包含交叉验证结果和策略选型）

---

## 📋 项目概况

- **项目名称：** soul
- **引擎版本：** Unreal Engine 4.27
- **项目路径：** `E:\MyProject\soul\`
- **仓库：** minglikesrunning-png/soul（私有）

---

## ✅ 已完成功能

### 1. FolderColorMarker 插件（已完成，可用）

**功能：** 在 Content Browser 中将被你自己关卡/蓝图引用的资源包文件夹标记为绿色。

**状态：**
- ✅ 编译通过
- ✅ 父级文件夹变色正常
- ✅ 只标记被自己开发内容引用的资源包
- ⚠️ Clear 需要重启编辑器才能生效（未修复，暂时搁置）

**模块位置：**
- `Source/soulEditor/` — Editor-Only 模块
- `Source/soulEditor/Public/FolderColorMarkerBFL.h` — 头文件
- `Source/soulEditor/Private/FolderColorMarkerBFL.cpp` — 实现
- `Source/soulEditor/Private/soulEditorModule.cpp` — 模块启动
- `Source/soulEditor/soulEditor.Build.cs` — 编译配置

**你的开发文件夹配置（写在 .cpp 顶部）：**
```
/Game/blueprint
/Game/Data
/Game/DataTables
/Game/Levels
/Game/LoadingScreen
/Game/MergeActors
/Game/ThirdPerson
/Game/ThirdPersonBP
/Game/CharactersType
```

**UI 工具：** `FolderColorTools` Editor Utility Widget（手动在编辑器中创建）

**完整搭建指南：** 见 `FolderColorMarker_Plugin_Guide.md`

---

## 🚧 待开发功能

### 2. 楼梯碰撞斜面化工具（Stair Ramp Generator）

**需求：** 类魂游戏风格，把楼梯的阶梯碰撞变成光滑斜面，让角色跑上去顺畅。

**状态：** 📐 架构设计完成 + 交叉验证完成，未开始编码

#### 已完成的设计工作

| 设计项 | 状态 | 文档 |
|--------|------|------|
| 核心架构 | ✅ 完成 | StairRamp_Complete_Design_Document_v2.md |
| 数学原理（直梯/L形/旋转） | ✅ 完成 | 同上 |
| 算法详解（5个核心函数） | ✅ 完成 | 同上 |
| 数据流 | ✅ 完成 | 同上 |
| 交叉验证（与另一个 LLM 对比） | ✅ 完成 | 同上 |
| 生成策略选型（A/B/C） | ✅ 选定策略 A | 同上 |
| 碰撞体类型选型 | ✅ 选定 BoxComponent | 同上 |
| 建筑内嵌楼梯方案 | ✅ 全自动+手动补齐 | 同上 |
| Foot IK 分析 | ✅ 确认独立于 Ramp 工具 | 同上 |
| 精度加强项 | ✅ 4 项加强 | 同上 |

#### 关键决策记录

| 决策 | 选择 | 原因 |
|------|------|------|
| 生成策略 | 策略 A（一整条光滑斜面） | 类魂标准做法，性能最优，实现最简 |
| 碰撞体类型 | BoxComponent | 比 ProceduralMesh 简单、碰撞性能好 |
| 楼梯识��� | 全自动三角形分析 | 不依赖 Tag 标记，适应新资产包 |
| 建筑内嵌 | 全自动检测 + Block Volume 手动补齐 | 比半自动预览模式更高效 |
| 原碰撞处理 | 叠加不替换（可选关闭） | 安全，加 bDisableOriginalCollision 参数 |
| Foot IK | 未来优化项，与 Ramp 工具独立 | 不在本工具范围内 |

#### 开发顺序

```
Phase 1: 直梯 Ramp ← 下一步
  - ExtractMeshTriangles()
  - DetectStairSteps()
  - StraightStairGenerator()
  - SpawnRampCollision()
  - RemoveAllAutoRamps()
  - bDisableOriginalCollision 参数
  - 精度加强（边缘定位、Z偏移、宽度、方向）

Phase 2: 建筑内嵌楼梯（全自动 + Block Volume 补齐）
Phase 3: L形楼梯（分段 Ramp）
Phase 4: 旋转楼梯（螺旋线分段）
Future:  Foot IK / Debug 可视化 / CharacterMovement 优化
```

---

## 📁 本地文档清单

| 文件名 | 内容 | 版本 |
|--------|------|------|
| `FolderColorMarker_Plugin_Guide.md` | 文件夹标记插件完整搭建 Prompt | v1 |
| `StairRamp_Complete_Design_Document_v2.md` | 楼梯工具完整设计（含交叉验证） | v2 |
| `HANDOFF_StairRamp_Progress_v2.md` | 本文件 | v2 |

---

## ⚠️ 已知问题

| 问题 | 状态 | 说明 |
|------|------|------|
| FolderColorMarker Clear 需重启编辑器 | ⚠️ 搁置 | UE4 Content Browser 不支持运行时刷新 PathColor |
| 左侧树状面板不显示文件夹颜色 | ℹ️ UE4 限制 | PathColor 只在右侧主面板生效 |

---

## 🔧 开发环境注意事项

1. **编译失败找不到 .lib 文件：** 删除 `Intermediate/` 和 `Binaries/`，重新编译
2. **模块加载失败（'soul' could not be loaded）：** 同上，删除后重新编译
3. **不要在模块全局作用域或 static 初始化中使用 UE_LOG：** 会导致启动崩溃（EXCEPTION_ACCESS_VIOLATION）
4. **修改 soulEditor 模块后务必重启编辑器**
5. **Agent 安全约束模板：**
```
### 安全约束：
- 不要修改 Source/soul/ 下的任何文件（除非明确指示）
- 不要修改 .uproject（除非明确指示）
- 不要修改 .Target.cs 或 .Build.cs（除非明确指示）
- 不要删除 Intermediate 或 Binaries 目录
- 如果编译失败，只报告错误，不要自行修复
```

---

## 🔜 下次继续时

给新 Chat 提供以下文件：
1. `HANDOFF_StairRamp_Progress_v2.md`（本文件）
2. `StairRamp_Complete_Design_Document_v2.md`（完整设计文档）
3. `FolderColorMarker_Plugin_Guide.md`（文件夹标记插件指南）

开场说：
> "Fetch repo minglikesrunning-png/soul，这是我的 UE4.27 项目。附件是三份进度文档，请先阅读 HANDOFF 了解当前状态，我们开始开发楼梯 Ramp 工具的 Phase 1。"