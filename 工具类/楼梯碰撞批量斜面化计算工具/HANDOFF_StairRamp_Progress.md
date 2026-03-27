# 🔄 Hand-Off 进度文档
> 最后更新：2026-03-27

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

**状态：** ✅ 父级文件夹变色正常 ✅ 只标记被自己开发内容引用的资源包 ⚠️ Clear 需要重启编辑器才能生效

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

**核心原理：** 在原始楼梯碰撞上方叠加一个倾斜的 BoxCollision，不替换原始碰撞。

**状态：** 📐 架构设计完成，未开始编码

#### 支持的楼梯类型

| 类型 | 方案 | 优先级 |
|------|------|--------|
| 直梯 | 1 个倾斜 Box，θ = atan2(H, L) | Phase 1 |
| 建筑内嵌楼梯 | 叠加 Ramp 在楼梯区域上方，不动建筑碰撞 | Phase 2 |
| L形/拐角楼梯 | 按平台分段，每段独立 Ramp | Phase 3 |
| 旋转楼梯 | 沿螺旋线每 15-30° 一段小 Ramp 拼接 | Phase 4 |

#### 工作流设计
```
用户选中场景中的楼梯 Actor
  → 点击 "Generate Stair Ramp" 按钮
  → 工具自动：
    1. 读取 Mesh BoundingBox / 三角形
    2. 检测踏步面，识别楼梯类型
    3. 计算斜面角度
    4. 生成不可见的倾斜 BoxCollision Actor
    5. Tag 标记为 "StairRamp_Auto"
```

#### 数学公式

**直梯：**
```
θ = atan2(总高度 H, 总长度 L)
斜面长度 S = √(H² + L²)
Box 尺寸 = (S/2, W/2, 薄厚度)
Box 旋转 = Pitch 轴 -θ 度
```

**旋转楼梯（螺旋线参数方程）：**
```
x(t) = R × cos(t)
y(t) = R × sin(t)
z(t) = P × t / (2π)

R = 旋转半径
P = 螺距（一圈上升高度）
每 15-30° 生成一段小 Ramp
```

#### 核心算法

**踏步面检测 DetectStairSteps：**
1. 从 Mesh 提取所有三角形
2. 筛选法线 Z > 0.95 的三角形（近似水平面）
3. 按 Z 高度聚类（容差 2cm）
4. 过滤面积太小的，识别大面积平台
5. 按高度排序输出

**楼梯类型识别 ClassifyStairType：**
- 踏步面中心投影 XY 平面，角度变化 < 10° → 直梯
- 有明显角度跳变 > 45° + 大面积平台 → L形
- 角度持续变化 > 90° 且无大平台 → 旋转

#### 架构
```
StairRampGenerator (soulEditor 模块)
├── GenerateRampsForSelected()     入口函数
├── ExtractMeshTriangles()         从 StaticMesh 提取三角形
├── DetectStairSteps()             识别踏步面
├── ClassifyStairType()            判断楼梯类型
├── StraightStairGenerator()       直梯 Ramp 生成
├── LShapedStairGenerator()        L形楼梯 Ramp 生成
├── SpiralStairGenerator()         旋转楼梯 Ramp 生成
└── SpawnRampCollision()           通用碰撞体生成
```

#### 风险点
- UE4.27 从 StaticMesh 提取碰撞三角形的 API 权限需验证
- 建筑内嵌楼梯需要手动选区域，无法全自动
- 旋转楼梯的参数估算是���似值，可能需微调

#### 下一步
- [ ] 开始 Phase 1：直梯 Ramp 生成器
- [ ] 验证 UE4.27 中 `GetPhysicsTriMeshData` / `BodySetup` API 可用性
- [ ] 在 soulEditor 模块中新增 StairRampGeneratorBFL 类
- [ ] UI：在 FolderColorTools Widget 中追加按钮，或新建独立 Widget

---

## ⚠️ 已知问题

| 问题 | 状态 | 说明 |
|------|------|------|
| FolderColorMarker Clear 需重启编辑器 | ⚠️ 未修复 | UE4 Content Browser 不支持运行时刷新 PathColor |
| 左侧树状面板不显示文件夹颜色 | ℹ️ UE4 限制 | PathColor 只在右侧主面板生效 |

---

## 🔧 开发环境注意事项

1. **编译失败找不到 .lib 文件：** 删除 `Intermediate/` 和 `Binaries/`，重新编译
2. **模块加载失败（'soul' could not be loaded）：** 同上，删除后重新编译
3. **不要在模块全局作用域或 static 初始化中使用 UE_LOG：** 会导致启动崩溃（EXCEPTION_ACCESS_VIOLATION）
4. **修改 soulEditor 模块后务必重启编辑器**
5. **Agent 安全约束模板：** 每次给 Agent 的 Prompt 都要包含以下安全约束：
```
### 安全约束：
- 不要修改 Source/soul/ 下的任何文件（除非明确指示）
- 不要修改 .uproject（除非明确指示）
- 不要修改 .Target.cs 或 .Build.cs（除非明确指示）
- 不要删除 Intermediate 或 Binaries 目录
- 如果编译失败，只报告错误，不要自行修复
```