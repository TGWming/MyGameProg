# 🏗️ 楼梯碰撞斜面化工具 — 完整设计纲领 v2

> **项目：** soul (UE4.27)
> **模块：** soulEditor (Editor-Only)
> **文档日期：** 2026-03-28
> **状态：** 架构设计完成 + 交叉验证完成，未开始编码

---

## 一、核心概念

### 1.1 目标

把楼梯的 **阶梯状碰撞（L形）** 变成 **一个光滑斜面（三角形填充）**，角色在上面跑就像在坡道上一样顺畅。参考类魂游戏（Elden Ring、Lies of P 等）的做法。

```
之前（阶梯碰撞）：              之后（斜面碰撞）：

    ┌─┐                         ╱─┐
   ┌┘ │                       ╱  │
  ┌┘  │          →          ╱   │
 ┌┘   │                  ╱    │
┌┘    │                ╱─────┘
└─────┘
                    填充的三角形区域
```

**关键理解：我们不是"替换"碰撞，而是在原始 Complex Collision 的阶梯凹槽上方"叠加"一个斜面碰撞体，把 L 形的凹陷抹平。原始碰撞保持不动。**

### 1.2 设计哲学

参考类魂游戏的做法：直接放弃楼梯的精细阶梯碰撞，用一个斜面覆盖整段楼梯。角色走在斜面上，感受到的是一个坡道，而不是一级一级的台阶。

### 1.3 名词表

| 名词 | 英文 | 说明 |
|------|------|------|
| 踏步面 | Tread | 每一级楼梯的水平面 |
| 踢面 | Riser | 每一级楼梯的垂直面 |
| 步高 | Step Height | 单级台阶的高度（通常 15-20cm） |
| 步深 | Step Depth / Run | 单级台阶的水平深度（通常 25-30cm） |
| 总升 | Total Rise | 楼梯从底到顶的总高度差 |
| 总进 | Total Run | 楼梯从底到顶的总水平距离 |
| 坡度角 | Slope Angle | 斜面与水平面的夹角 |
| 行走面 | Walkable Surface | UE 中角色可以站立/行走的面（法线 Z 分量 ≥ cos(WalkableFloorAngle)） |
| BoundingBox | AABB | 轴对齐包围盒，包裹 Mesh 的最小矩形 |
| Complex Collision | — | 使用原始三角形网格作为碰撞体 |
| Convex Hull | 凸包 | 包裹一组点的最小凸多面体 |
| 螺旋线 | Helix | 旋转楼梯的路径曲线 |
| 螺距 | Pitch (P) | 螺旋线旋转一圈上升的高度 |
| 法线 | Normal | 三角形面的朝向向量 |
| 聚类 | Clustering | 把相近的数据点归为一组 |
| Foot IK | 脚部反向动力学 | 动画系统中让角色脚部贴合地面的技术 |
| RANSAC | — | 随机抽样一致性算法，用于从噪声数据中拟合模型 |

---

## 二、生成策略选择 — 为什么选策略 A

### 2.1 三种策略对比

#### 策略 A：一整条光滑斜面（✅ 最终选择）

```
侧视图：

    ┌─┐
   ┌┘ │      一整个 Box，从底到顶一条直线
  ┌┘  │      ╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱
 ┌┘   │    ╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱
┌┘    │  ╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱
└─────┘

生成的碰撞体：1 个倾斜的 BoxComponent
角色感受：完全光滑的坡道，从底到顶一口气跑上去
```

#### 策略 B：每级台阶独立补齐（逐级填充��角形）

```
侧视图：

    ┌─┐
   ┌┘█│      每一级台阶的 L 形凹槽单独用一个小 Box 填平
  ┌┘██│      █ = 填充的碰撞体
 ┌┘███│
┌┘████│
└─────┘

生成的碰撞体：N 个小 Box（每级台阶一个）
角色感受：仍然是一级一级的，但每级台阶变平了，没有了L形卡顿
```

#### 策略 C：混合 — 逐级生成但拼成斜面（带可调节点）

```
侧视图：

    ┌─┐         每级台阶一个小 Ramp，但角度一致
   ┌┘ │         拼接起来效果接近一整条斜面
  ┌┘  │       ╱╱╱
 ┌┘   │     ╱╱╱    ← 每段小 Ramp 有独立的位置/旋转
┌┘    │   ╱╱╱       可以单独微调某一段的角度
└─────┘ ╱╱╱

生成的碰撞体：N 个小 Box（每级一个），默认角度一致
角色感受：默认和策略 A 一样光滑，但你可以调整单段
```

### 2.2 详细对比表

| | 策略 A（整条斜面） | 策略 B（逐级填平） | 策略 C（逐级拼斜面） |
|---|---|---|---|
| 光滑度 | ✅ 最光滑 | ❌ 仍有台阶感 | ✅ 光滑 |
| 碰撞体数量 | 1 个 | N 个（10级=10个） | N 个 |
| 性能 | ✅ 最好 | ⚠️ 碰撞体多 | ⚠️ 碰撞体多 |
| 可微调性 | ❌ 只能整体调 | ✅ 每级独立调 | ✅ 每级独立调 |
| 实现复杂度 | ⭐ 最简单 | ⭐⭐ | ⭐⭐⭐ |
| 类魂游戏做法 | ✅ 就是这个 | ❌ 不是 | ⚠️ 过度工程 |

### 2.3 选择策略 A 的原因

1. **类魂游戏标准做法** — Elden Ring、Lies of P 就是一条斜面，不纠结每一级台阶
2. **性能最优** — 1 个 BoxComponent 是 UE4 中最快的碰撞形状
3. **实现最简** — 开发成本最低，bug 最少
4. **光滑度最好** — 完全没有段落接缝
5. **可微调性低但可以接受** — 如果某段楼梯角度不完美，用 Block Volume 手动补缝即可

### 2.4 策略 A 对计算精度的要求

因为策略 A 一次生成就得到位，不像策略 C 可以逐级微调，所以 **计算精度是关键**。但精度只取决于两个端点的定位：

```
整条斜面的精度只取决于：

    A (起点) ─────────────────── B (终点)

    A 定准了，B 定准了，中间的斜面自然就准了
    不存在"中间某一段偏了"的问题
```

#### 需要精确计算的变量

```
1. 起点 A 的位置 (X, Y, Z)      ← 楼梯最底部踏步面的前缘
2. 终点 B 的位置 (X, Y, Z)      ← 楼梯最顶部踏步面的后缘
3. 楼梯宽度 W                    ← 踏步面的横向尺寸
4. 斜面角度 θ = atan2(H, L)      ← 由 A、B 两点自动算出

只要 A 和 B 准了，θ 和 S 都是精确的数学推导，不会有误差
```

#### 可能影响精度的因素及对策

**因素 1：BoundingBox vs 踏步面检测**

```
BoundingBox 方式（粗糙）：
┌──────────────────┐
│  扶手 ↑ 多出来的  │  ← BoundingBox 包含了扶手高度
│    ┌─┐           │     导致 H 偏大，θ 偏陡
│   ┌┘ │           │
│  ┌┘  │           │
│  └───┘           │
└──────────────────┘

踏步面检测方式（精确）：
     ┌─┐
    ┌┘ │  ← 只取踏步面的 Z 值
   ┌┘  │     H = 最高踏步面.Z - 最低踏步面.Z
   └───┘     精确排除了扶手、栏杆的干扰

结论：踏步面检测方式精度远高于 BoundingBox
```

**因素 2：起点终点的精确定义**

```
粗糙定义（用踏步面中心）：
   A●───────●B        A、B 是踏步面中心点
                      Ramp 两端各缩进了半个台阶
                      角色走到最后一级会"踩空"

精确定义（用踏步面边缘）：
 A●─────────────●B    A = 最低踏步面的前缘（靠近地面的边）
                      B = 最高踏步面的后缘（靠近上层地面的边）
                      Ramp 完全覆盖整段楼梯
```

**因素 3：Ramp 的垂直位置（Z 偏移）**

```
太低：
   ╱╱╱╱╱╱╱       Ramp 嵌入台阶内部
  ┌─┐              角色走在前几级时踩到的是原始台阶
 ┌┘ │              不是 Ramp → 前半段仍有卡顿
┌┘  │
└───┘

太高：
         ╱╱╱╱╱╱    Ramp 悬浮在台阶上方
  ┌─┐  ╱╱           角色会"漂浮"在空中走
 ┌┘ │╱              视觉上脚不贴地
┌┘  │
└───┘

刚好：
    ╱╱╱╱╱╱╱╱╱      Ramp 的底面刚好贴合
  ┌╱┐               每一级台阶的最高点
 ┌┘ │               = 贴着楼梯台阶的"尖角"走
┌┘  │
└───┘

精确 Z 偏移 = 让 Ramp 底面经过每级台阶的外上角
```

#### 算法如何保证精度

```
Step 1: ExtractMeshTriangles()
        获取完整三角形数据 ← 精确到每个顶点

Step 2: DetectStairSteps()
        筛选法线朝上的三角形 → 得到每一级踏步面
        按 Z 聚类 → 精确的每级高度值
        ↓
        这一步直接决定了 A、B 点的精度

Step 3: 精确定位 A 和 B
        A = 最低踏步面中所有三角形的最前方顶点（最小 ForwardDir 投影值）
        B = 最高踏步面中所有三角形的最后方顶点（最大 ForwardDir 投影值）
        ↓
        不是用 BoundingBox 估算，是从实际顶点中提取

Step 4: θ = atan2(B.Z - A.Z, HorizontalDistance(A, B))
        纯数学计算，零误差

Step 5: Ramp 位置微调
        Z 偏移 = 让 Ramp 表面经过台阶外上角
        具体：Ramp 中心 Z = (A.Z + B.Z) / 2 + (Thickness / 2)
        Thickness = Ramp 厚度（5cm）
```

#### Phase 1 精度加强项

```
加强项 1：起点终点用踏步面边缘，不用中心
  → A = 最低踏步面的前缘顶点
  → B = 最高踏步面的后缘顶点
  → 保证 Ramp 完全覆盖，不会两端缩进

加强项 2：Z 偏移精确计算
  → Ramp 底面要贴合台阶外上角连线
  → 不能太高（漂浮）也不能太低（嵌入）
  → 公式：Z_offset = Thickness / 2

加强项 3：宽度从踏步面三角形计算，不用 BoundingBox
  → 避免扶手/栏杆影响宽度
  → W = 踏步面三角形在横向的跨度

加强项 4：前进方向从踏步面序列推导
  → Direction = A → B 在 XY 平面的投影方向
  → 不依赖 Actor 的 ForwardVector（可能被旋转过）
```

---

## 三、交叉验证 — 与另一个 LLM 方案的对比

### 3.1 核心共识（两边一致的点）

| 共识点 | 我们的方案 | 另一个 LLM | 结论 |
|--------|-----------|-----------|------|
| 不替换视觉，只叠加碰撞 | ✅ | ✅ | **核心原则一致** |
| 用倾斜的碰撞体覆盖楼梯 | ✅ BoxComponent | ✅ BoxComponent / ProceduralMesh | 一致 |
| Editor-Time 生成，运行时零开销 | ✅ | ✅ | 一致 |
| 放在 soulEditor 模块 | ✅ | ✅ Editor Utility | 一致 |
| L形楼梯需要分段处理 | ✅ 按平台分段 | ✅ 按方向变化分段 | 一致 |
| 旋转楼梯需要多段拼接 | ✅ 螺旋线参数方程 | ✅ Spline / 多段 | 一致 |
| 建筑内嵌楼梯不能关闭原碰撞 | ✅ 叠加不替换 | ✅ 叠加不替换 | 一致 |
| 标签管理（Tag）便于后续删除 | ✅ "StairRamp_Auto" | ✅ "Stair" Tag | 一致 |

### 3.2 关键差异分析

#### 差异 1：碰撞体类型 — BoxComponent vs ProceduralMeshComponent

| | 我们的方案（BoxComponent） | 另一个 LLM（ProceduralMesh） |
|---|---|---|
| 实现复��度 | ⭐ 简单，5行代码 | ⭐⭐⭐ 需要手动构建顶点/三角形 |
| 精度（直梯） | ✅ 完全足够 | ✅ 完全足够 |
| 精度（旋转梯） | ⚠️ 多段 Box 有微小棱角 | ✅ 可以生成弧面 |
| 碰撞性能 | ✅ Box 是最快的碰撞形状 | ⚠️ TriMesh 碰撞比 Box 慢 |
| UE4.27 兼容性 | ✅ 原生支持 | ✅ 原生支持，但需加模块依赖 |
| 调试可视化 | ✅ Box 在编辑器中自带可视化 | ⚠️ 需要手动 DrawDebug |

**评估结论：BoxComponent 是更好的选择。** 直梯和 L 形梯完全足够；旋转梯用 15° 分段的多个 Box 拼接，棱角在角色感知范围内几乎不存在（15° 弧段 ≈ 39cm，角色胶囊体宽度 34-42cm）。ProceduralMesh 的额外复杂度不值得，而且 TriMesh 碰撞性能更差。

**决定：维持 BoxComponent 方案。**

#### 差异 2：楼梯识别方式 — 踏步面检测 vs Tag 标记

| | 我们的方案（三角形分析） | 另一个 LLM（Tag 标记） |
|---|---|---|
| 自动化程度 | ✅ 全自动，零标记 | ❌ 需要手动给每个楼梯打 Tag |
| 可靠性（单独楼梯） | ✅ 高（法线 + Z 聚类） | ✅ 高（Tag 精确定位） |
| 可靠性（建筑内嵌） | ⚠️ 中等（需要从大 Mesh 中分离） | ⚠️ 需要美术配合标记 |
| 工作量 | 开发阶段复杂，使用阶段零工作 | 开发简单，但每个楼梯都要手动打 Tag |
| 适应新资产包 | ✅ 自动适应 | ❌ 每次加新资产包都要重新打 Tag |

**评估结论：** 另一个 LLM 的 Tag 方案是"懒人方案"——开发简单但使用麻烦。对于大量不同美术包的楼梯，每次加新资产包都手动打 Tag 是不可接受的工作量。

**决定：单独楼梯保持全自动几何分析；建筑内嵌采用全自动检测 + 手动 Block Volume 补齐。**

#### 差异 3：原碰撞处理方式

| | 我们的方案 | 另一个 LLM |
|---|---|---|
| 单独楼梯 | 叠加，不动原碰撞 | 关闭原碰撞（NoCollision） |
| 建筑内嵌 | 叠加，不动原碰撞 | 叠加，不动原碰撞 |

**评估结论：** 另一个 LLM 对单独楼梯建议关闭原碰撞有一定道理（减少碰撞体重叠计算），但对建筑内嵌绝对不能关闭。

**决定：增加可选参数 `bDisableOriginalCollision`，默认 false。单独楼梯 Actor 可选择关闭原碰撞。**

### 3.3 另一个 LLM 提出但我们采纳的改进

| 改进项 | 说明 | 采纳方式 |
|--------|------|---------|
| `bDisableOriginalCollision` 参数 | 单独楼梯可选关闭原碰撞 | 加入 Phase 1 接口 |
| CharacterMovementComponent 参数建议 | Max Step Height 调低、Walkable Floor Angle 按需调整 | 写入文档（保守设置） |
| Foot IK 建议 | 让角色脚部贴合视觉台阶 | 记录为未来优化项 |
| RANSAC 拟合 | 从噪声数据中拟合最优斜面 | Phase 2 可选优化 |
| Debug 可视化 | 生成后可视化检查 Ramp | 加入 Phase 1 |

### 3.4 另一个 LLM 提出但我们不采纳的部分

| 不采纳项 | 原因 |
|----------|------|
| 预制 Ramp Mesh + Scale 适配 | Scale 拉伸导致碰撞变形，不如 BoxComponent 指定精确尺寸 |
| ProceduralMeshComponent | 过度复杂，TriMesh 碰撞性能差 |
| 纯 Tag 标记方案 | 维护成本太高，每次新资产包都要手动打 Tag |
| DBSCAN/K-Means 全自动识别建筑内嵌楼梯 | 另一个 LLM 自己也承认成功率只有 30-50% |
| Walkable Floor Angle 调到 80° | 太激进，可能导致角色在不该走的陡坡上也能走 |

### 3.5 另一个 LLM 代码中的 Bug

另一个 LLM 给出的示意代码有多处错误，不能直接使用：

```cpp
// Bug 1：FVector 没有 Max/Min 静态常量
FVector Min = FVector::Max, Max = FVector::Min;  // ❌
// 应该是：
FVector Min = FVector(FLT_MAX);                   // ✅
FVector Max = FVector(-FLT_MAX);                  // ✅

// Bug 2：TotalRise 累加逻辑在单 Mesh 时正确但多组件时错误
TotalRise += (WorldMax.Z - WorldMin.Z);  // ❌ 多组件时累加得到错误值
// 应该是：
TotalRise = GlobalMax.Z - GlobalMin.Z;   // ✅ 取全局最大最小

// Bug 3：FRotator 参数顺序错误
FRotator(0, 0, StairSlopeAngle);  // ❌ 把坡度放在了 Roll
// FRotator 顺序是 (Pitch, Yaw, Roll)，应该是：
FRotator(-StairSlopeAngle, 0, 0); // ✅ 坡度放在 Pitch
```

---

## 四、三种楼梯类型的数学原理

### 4.1 Type A：直梯（Straight Staircase）

最简单的情况，一个倾斜的长方体就能搞定。

#### 几何模型

```
侧视图：
                    B (顶端)
                   ╱│
                 ╱  │
               ╱    │ Rise (总升 H)
             ╱      │
           ╱        │
    A ───╱──────────┘
         Run (总进 L)

A = 楼梯底部起点（最低踏步面前缘）
B = 楼梯顶部终点（最高踏步面后缘）
H = B.Z - A.Z（总高度差）
L = 沿楼梯前进方向的水平距离
W = 楼梯宽度（垂直于前进方向）
```

#### 数学计算

```
输入：
  H = 楼梯总高度（从踏步面检测得到，非 BoundingBox）
  L = 楼梯总长度（沿前进方向，从踏步面边缘得到）
  W = 楼梯宽度（从踏步面三角形横向跨度得到）

计算：
  θ = atan2(H, L)              // 坡度角（弧度）
  θ_deg = θ × 180 / π          // 坡度角（度）
  S = √(H² + L²)              // 斜面长度（勾股定理）

生成的 Box Collision：
  尺寸（Half Extents）= (S/2, W/2, 薄厚度)
  薄厚度建议 = 2.5cm ~ 5cm（确保角色不穿透）

  位置 = ((A.X+B.X)/2, (A.Y+B.Y)/2, (A.Z+B.Z)/2 + Thickness/2)
         Thickness/2 的偏移让 Ramp 表面刚好贴合台阶外上角

  旋转 = 绕 Pitch 轴（Y轴）旋转 -θ_deg 度
         注意：需要先对齐楼梯的朝向（Yaw），再叠加 Pitch
```

#### 确定"前进方向"的方法

**方案 1：用 Actor 的 Forward Vector（简单）**
- 大多数楼梯 Mesh 沿 X 轴建模
- 直接用 Actor 的 ForwardVector 作为前进方向
- L = BoundingBox 在 Forward 方向的投影长度

**方案 2：分析踏步面（精确，推荐）**
- 找最低踏步面和最高踏步面的中心点
- 连线方向投影到 XY 平面 = 前进方向
- 这个方法适用于楼梯 Actor 旋转过的情况

#### 示例计算

```
假设一段楼梯：
  10 级台阶，每级步高 18cm，步深 28cm

  H = 10 × 18cm = 180cm
  L = 10 × 28cm = 280cm
  W = 120cm（楼梯宽度）

  θ = atan2(180, 280) = 32.7°
  S = √(180² + 280²) = √(32400 + 78400) = √110800 ≈ 332.9cm

  Box Half Extents = (166.5, 60, 2.5) cm
  Box Rotation = Pitch -32.7°
```

---

### 4.2 Type B：L 形 / 拐角楼梯（L-Shaped / Multi-Flight）

由多段直梯 + 中间平台组成。

#### 几何模型

```
俯视图：
    ┌──────┐
    │ 第2段 │ ↑ 前进方向2
    │      │
    ├──────┤ ← 平台（Landing，水平）
    │      │
    │ 第1段 │ ↑ 前进方向1
    └──────┘

侧视图展开：

              B2
             ╱│
    B1─────╱ │    第2段
   ╱│  平台   │
  ╱ │         │
 A  │         │    第1段

平台是水平的，不需要 Ramp
第1段和第2段各自独立生成 Ramp
```

#### 处理算法

```
算法：L-Shaped Stair Processing

1. 获取 Mesh 所有三角形
2. 用 DetectStairSteps() 筛选所有踏步面
3. 识别"平台"：
   - 面积明显大于普通踏步面的水平面（面积 > 平均踏步面积 × 2）
   - 或者：连续多级踏步面高度相同（ΔZ ≈ 0）
4. 用平台把楼梯分成多个"段"（Flight）
5. 每段独立处理：

   For each Flight:
     H_flight = 该段最高踏步面.Z - 该段最低踏步面.Z

     // 前进方向 = 最低踏步面中心 → 最高踏步面中心（XY投影）
     Direction = (HighestStep.Center - LowestStep.Center)
     Direction.Z = 0
     Direction.Normalize()

     L_flight = |HighestStep.Center - LowestStep.Center| 在 Direction 上的投影
     W_flight = 楼梯宽度（从踏步面的横向尺寸推算）

     θ = atan2(H_flight, L_flight)
     S = √(H_flight² + L_flight²)

     生成 Box Collision：
       尺寸 = (S/2, W_flight/2, 薄厚度)
       位置 = 该段中心
       旋转 = Yaw(朝向 Direction) + Pitch(-θ)
```

#### 平台处理

```
平台不生成 Ramp，保持原始碰撞即可。
平台本身就是水平面，角色走在上面不会有问题。
```

---

### 4.3 Type C：旋转楼梯（Spiral Staircase）

旋转楼梯的路径是一条 **螺旋线（Helix）**。

#### 螺旋线参数方程

```
三维螺旋线的参数方程：

  x(t) = R × cos(t)
  y(t) = R × sin(t)
  z(t) = P × t / (2π)

参数说明：
  R = 旋转半径（楼梯中心到踏步面外缘的距离）
  P = 螺距（旋转一圈 360° 上升的高度）
  t = 角度参数（弧度），从 0 到 TotalAngle

导出参数：
  TotalAngle = 楼梯总旋转角度（如 360° = 2π, 540° = 3π）
  TotalHeight = P × TotalAngle / (2π)

  反推螺距：
  P = TotalHeight × (2π) / TotalAngle
```

#### 几何模型

```
俯视图：                    侧视图展开：
      ╱──╲                      ╱
    ╱      ╲                  ╱
   │   中心  │              ╱   ← 螺旋展开后就是一个斜面
    ╲      ╱              ╱
      ╲──╱              ╱

3D 视图（多段小 Ramp 拼接）：

   每段 Ramp 覆盖 15-30° 弧度
   段越多，近似越精确

        ╱▓▓╲         ▓ = 单段 Ramp
      ╱▓▓▓▓▓▓╲
     │▓▓ 中心 ▓▓│
      ╲▓▓▓▓▓▓╱
        ╲▓▓╱
```

#### 参数估算方法

```
1. 检测旋转楼梯特征：
   - BoundingBox 在 XY 平面近似正方形（Width ≈ Depth）
   - 高度 H 远大于 Width/2

2. 估算旋转半径 R：
   R = BoundingBox.Width / 2
   （或更精确：找所有踏步面中心到 BoundingBox 中心的平均距离）

3. 分析踏步面确定旋转方向和总角度：
   a. 找所有踏步面的中心点
   b. 投影到 XY 平面
   c. 计算每个踏步面相对于 BoundingBox XY 中心的角度：
      angle_i = atan2(Step_i.Center.Y - BB.Center.Y,
                      Step_i.Center.X - BB.Center.X)
   d. 按 Z 高度排序踏步面
   e. 按排序后的角度序列判断旋转方向（顺时针/逆时针）
   f. 累加角度差 = TotalAngle

4. 计算螺距 P：
   P = TotalHeight × (2π) / TotalAngle
```

#### 分段生成算法

```
算法：Spiral Stair Ramp Generation

输入：R（半径）, P（螺距）, TotalAngle, TotalHeight
      StairWidth（楼梯宽度）, SegmentAngle（每段弧度，建议 15°-30°）
      Center（螺旋中心 XY 坐标）, BaseZ（楼梯底部 Z）

NumSegments = ceil(TotalAngle / SegmentAngle)

For i = 0 to NumSegments - 1:
    // 这一段的起止角度
    t0 = i × SegmentAngle
    t1 = min((i + 1) × SegmentAngle, TotalAngle)

    // 起点坐标
    Start.X = Center.X + R × cos(t0)
    Start.Y = Center.Y + R × sin(t0)
    Start.Z = BaseZ + P × t0 / (2π)

    // 终点坐标
    End.X = Center.X + R × cos(t1)
    End.Y = Center.Y + R × sin(t1)
    End.Z = BaseZ + P × t1 / (2π)

    // 这一段的方向向量
    Direction = End - Start

    // 水平分量和垂直分量
    HorizontalDir = Direction
    HorizontalDir.Z = 0
    SegmentRun = HorizontalDir.Length()    // 水平距离
    SegmentRise = End.Z - Start.Z          // 垂直距离
    SegmentLength = Direction.Length()      // 斜面长度

    // 这一段的坡度角
    θ_segment = atan2(SegmentRise, SegmentRun)

    // 这一段的朝向（Yaw）
    Yaw_segment = atan2(HorizontalDir.Y, HorizontalDir.X)

    // 生成 Box Collision
    Box 位置 = (Start + End) / 2
    Box 旋转 = FRotator(-θ_segment_deg, Yaw_segment_deg, 0)
    Box 尺寸 = (SegmentLength/2, StairWidth/2, 薄厚度)

    SpawnRampCollision(位置, 旋转, 尺寸)
```

#### 精度分析

```
每段弧度 vs 近似误差：

  30° 一段 → 12 段/圈 → 误差较明显，弧线处有棱角
  15° 一段 → 24 段/圈 → 误差小，基本光滑
  10° 一段 → 36 段/圈 → 非常精确，几乎无感

推荐：15° 一段，平衡精度和碰撞体数量

角色感知：
  角色胶囊体宽度通常 34-42cm
  15° 弧段在 R=150cm 的螺旋梯上，弧长 ≈ 39cm
  接近角色宽度，走起来几乎感觉不到段落接缝
```

---

## 五、核心算法详解

### 5.1 从 Mesh 提取三角形 — ExtractMeshTriangles

```
函数：ExtractMeshTriangles(UStaticMesh* Mesh)
返回：TArray<FTriangle>（每个三角形包含 3 个顶点 + 法线）

实现路径（UE4.27）：

方案 A — 从 RenderData 提取（编辑器模式可用，推荐）：
  1. Mesh->GetRenderData()->LODResources[0]
  2. 读取 PositionVertexBuffer → 所有顶点位置
  3. 读取 IndexBuffer → 三角形索引
  4. 每 3 个索引 = 1 个三角形
  5. 计算法线：Normal = Cross(v1-v0, v2-v0).Normalize()

方案 B — 从 BodySetup 提取碰撞三角形：
  1. Mesh->GetBodySetup()
  2. BodySetup->TriMeshes → 物理三角形网格
  3. 遍历 PxTriangleMesh 的顶点和索引

方案 C — 从 Complex Collision 提取：
  1. UBodySetup* BS = Mesh->GetBodySetup()
  2. BS->GetCookInfo() 或直接访问 BS->TriMeshBodies
  3. 需要 PhysX API 访问权限

推荐：方案 A（RenderData），在 Editor 模式下最稳定
```

#### 代码结构

```cpp
struct FStairTriangle
{
    FVector V0, V1, V2;    // 三个顶点（世界坐标）
    FVector Normal;         // 面法线
    float Area;             // 三角形面积
    FVector Center;         // 三角形中心 = (V0+V1+V2)/3
};

TArray<FStairTriangle> ExtractMeshTriangles(
    UStaticMeshComponent* MeshComp)
{
    // 1. 获取 StaticMesh
    UStaticMesh* Mesh = MeshComp->GetStaticMesh();
    FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];

    // 2. 读取顶点
    FPositionVertexBuffer& VertexBuffer = LOD.VertexBuffers.PositionVertexBuffer;
    uint32 NumVertices = VertexBuffer.GetNumVertices();

    // 3. 读取索引
    FRawStaticIndexBuffer& IndexBuffer = LOD.IndexBuffer;
    TArray<uint32> Indices;
    IndexBuffer.GetCopy(Indices);

    // 4. 组装三角形（转换到世界坐标）
    FTransform WorldTransform = MeshComp->GetComponentTransform();
    TArray<FStairTriangle> Triangles;

    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        FStairTriangle Tri;
        Tri.V0 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i]));
        Tri.V1 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i+1]));
        Tri.V2 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i+2]));

        FVector Edge1 = Tri.V1 - Tri.V0;
        FVector Edge2 = Tri.V2 - Tri.V0;
        FVector CrossProduct = FVector::CrossProduct(Edge1, Edge2);

        Tri.Area = CrossProduct.Size() * 0.5f;
        Tri.Normal = CrossProduct.GetSafeNormal();
        Tri.Center = (Tri.V0 + Tri.V1 + Tri.V2) / 3.0f;

        Triangles.Add(Tri);
    }

    return Triangles;
}
```

---

### 5.2 踏步面检测 — DetectStairSteps

```
函数：DetectStairSteps(TArray<FStairTriangle>& Triangles)
返回：TArray<FStairStep> 按高度排序的踏步面列表

算法步骤：

Step 1: 筛选近似水平的三角形
  条件：Normal.Z > 0.95（法线几乎朝上 = 水平面）
  数学原理：
    完全水平面的法线 = (0, 0, 1)，Normal.Z = 1.0
    Normal.Z > 0.95 对应倾斜角 < acos(0.95) ≈ 18.2°
    这个容差允许轻微不平的踏步面通过

Step 2: 按 Z 高度聚类
  容差 = 2cm（同一级踏步面上的三角形 Z 可能有微小差异）

  算法：
    Sort candidates by Center.Z ascending
    clusters = []
    current_cluster = [candidates[0]]

    For i = 1 to N-1:
      If |candidates[i].Center.Z - current_cluster.last.Center.Z| < 2cm:
        current_cluster.Add(candidates[i])
      Else:
        clusters.Add(current_cluster)
        current_cluster = [candidates[i]]
    clusters.Add(current_cluster)

Step 3: 为每个聚类生成 StairStep 信息
  struct FStairStep:
    float Height           // 该级踏步面的平均 Z 高度
    FVector Center         // 该级所有三角形中心的平均值
    float TotalArea        // 该级所有三角形面积之和
    int32 TriangleCount    // 三角形数量
    bool bIsPlatform       // 是否是平台（大面积）

  For each cluster:
    Step.Height = cluster 所有三角形 Center.Z 的平均值
    Step.Center = cluster 所有三角形 Center 的平均值
    Step.TotalArea = cluster 所有三角形面积之和
    Step.TriangleCount = cluster.Num()

Step 4: 过滤噪声
  - 面积太小的聚类（< 100cm²）→ 不是踏步面，可能是装饰细节，丢弃

Step 5: 识别平台
  AverageStepArea = 所有 Step 的面积平均值
  For each Step:
    If Step.TotalArea > AverageStepArea × 2.0:
      Step.bIsPlatform = true

Step 6: 按高度排序输出
  Sort Steps by Height ascending
```

#### 数据结构

```cpp
struct FStairStep
{
    float Height;           // 平均 Z 高度
    FVector Center;         // 中心位置（世界坐标）
    float TotalArea;        // 总面积 (cm²)
    int32 TriangleCount;    // 三角形数量
    bool bIsPlatform;       // 是否是平台
};
```

---

### 5.3 楼梯类型识别 — ClassifyStairType

```
函数：ClassifyStairType(TArray<FStairStep>& Steps)
返回：EStairType { Straight, LShaped, Spiral }

算法：基于踏步面中心点在 XY 平面上的角度变化

Step 1: 计算所有踏步面的 XY 中心
  BB_Center = 所有 Step.Center 的 BoundingBox 中心（XY）

Step 2: 计算每个踏步面相对于 BB_Center 的角度
  For each Step (按 Height 排序):
    dx = Step.Center.X - BB_Center.X
    dy = Step.Center.Y - BB_Center.Y
    angle_i = atan2(dy, dx) × 180 / π    // 角度制

Step 3: 分析角度变化模式

  // 计算相邻踏步面的角度差
  TArray<float> AngleDeltas;
  For i = 1 to N-1:
    delta = angle[i] - angle[i-1]
    // 处理 -180°/180° 跳变
    While delta > 180: delta -= 360
    While delta < -180: delta += 360
    AngleDeltas.Add(delta)

  TotalAngleChange = Sum(AngleDeltas)
  MaxSingleDelta = Max(|AngleDeltas|)

  // 判断规则：
  If |TotalAngleChange| < 10°:
    // 角度几乎没变 → 直梯
    return EStairType::Straight

  Else If |TotalAngleChange| > 90° AND 没有大面积平台:
    // 角度持续变化超过 90° 且没有平台 → 旋转楼梯
    return EStairType::Spiral

  Else If 存在大面积平台 (bIsPlatform == true):
    // 有平台 → L形/多段楼梯
    return EStairType::LShaped

  Else:
    // 默认当直梯处理
    return EStairType::Straight
```

#### 枚举定义

```cpp
UENUM(BlueprintType)
enum class EStairType : uint8
{
    Straight    UMETA(DisplayName = "Straight"),
    LShaped     UMETA(DisplayName = "L-Shaped"),
    Spiral      UMETA(DisplayName = "Spiral"),
    Unknown     UMETA(DisplayName = "Unknown")
};
```

---

### 5.4 坡度角计算 — CalculateSlopeAngle

```
函数：CalculateSlopeAngle(FVector Start, FVector End)
返回：float 角度（度）

计算：
  Direction = End - Start

  // 水平分量
  HorizontalDistance = √(Direction.X² + Direction.Y²)

  // 垂直分量
  VerticalDistance = |Direction.Z|

  // 坡度角
  SlopeRadians = atan2(VerticalDistance, HorizontalDistance)
  SlopeDegrees = SlopeRadians × 180 / π

  return SlopeDegrees

注意：
  UE4 的 WalkableFloorAngle 默认为 44.765°
  如果计算出的坡度 > WalkableFloorAngle，角色将无法行走
  一般楼梯坡度在 25°-40° 之间，不会超过限制
```

---

### 5.5 Ramp 碰撞体生成 — SpawnRampCollision

```
函数：SpawnRampCollision(UWorld* World,
                          FVector Location,
                          FRotator Rotation,
                          FVector BoxHalfExtents,
                          AActor* ParentStairActor)
返回：生成的 Ramp Actor

Step 1: 在关卡中生成一个空 Actor
  AActor* RampActor = World->SpawnActor<AActor>(Location, Rotation)

Step 2: 添加 SceneComponent 作为根
  USceneComponent* Root = NewObject<USceneComponent>(RampActor)
  RampActor->SetRootComponent(Root)
  Root->RegisterComponent()

Step 3: 添加 BoxComponent
  UBoxComponent* Box = NewObject<UBoxComponent>(RampActor)
  Box->SetupAttachment(Root)
  Box->SetBoxExtent(BoxHalfExtents)
  Box->SetCollisionProfileName(TEXT("BlockAll"))
  Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)
  Box->SetVisibility(false)  // 不可见
  Box->RegisterComponent()

Step 4: 设置标签便于管理
  RampActor->Tags.Add(FName("StairRamp_Auto"))

Step 5: 碰撞响应设置
  Box->SetCollisionResponseToAllChannels(ECR_Block)
  // 确保对 Pawn 阻挡
  Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block)
  // 对 Visibility 通道忽略（不影响射线检测视线）
  Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore)
```

---

## 六、建筑内嵌楼梯的特殊处理

### 6.1 问题描述

部分资产是一整栋建筑，楼梯是建筑的一部分。使用 Complex Collision As Simple 时，楼梯已经有精准的阶梯碰撞。

```
整栋建筑 Mesh：
┌─────────────────┐
│  2F             │
│  ┌─┐            │
│ ┌┘ │ ← 楼梯部分 │
│┌┘  │            │
││   │            │
│└───┘            │
│  1F             │
└─────────────────┘
```

### 6.2 处理原则

```
1. 不动原始 Mesh 的碰撞设置（不能关闭整栋建筑的碰撞）
2. 只在楼梯区域上方叠加 Ramp Collision
3. Ramp Collision 的设置：
   - Collision Response: Block for Pawn/Character
   - 厚度 ≥ 5cm（确保角色不穿透 Ramp 碰到下面的台阶）
4. 角色踩在 Ramp 上 → 不会碰到原始阶梯碰撞
   （因为 Ramp 在上方，角色被 Ramp 支撑住了）
```

### 6.3 效果示意

```
叠加 Ramp 后：
┌─────────────────┐
│  2F             │
│  ╱─┐            │
│ ╱  │ ← Ramp 覆盖在楼梯上方
│╱   │            │
│    │            │
│    │            │
│  1F             │
└─────────────────┘

角色走在 Ramp 上，感觉是斜面
原始楼梯碰撞仍在，但角色碰不到（被 Ramp 挡住了）
```

### 6.4 工作流（全自动 + 手动补齐）

```
工具自动做的事：
  1. 扫描建筑 Mesh 三角形
  2. 检测所有踏步面特征区域
  3. 对每个检测到的楼梯区域自动生成 Ramp
  4. 可能有少量角度偏差或遗漏

你手动补的事：
  - 运行游戏测试
  - 发现某个 Ramp 角度不够好 → 微调旋转/位置
  - 发现某个角落有缝隙 → 手动放一个 Block Volume 补上
  - 工具完全没检测到的楼梯 → 手动放 Block Volume
```

这比"半自动预览确认"模式更高效，因为：
- **90% 的情况工具能自动搞定**，不用逐个确认
- **剩下 10% 的边角问题**用 Block Volume 手动补，比做复杂的预览/确认 UI 快得多
- 开发成本也更低——不需要做编辑器预览高亮系统

---

## 七、角色动画 — Foot IK 与 Ramp 工具的关系

### 7.1 两件事完全独立

```
楼梯斜面工具（我们在做的）     角色脚部适配（动画侧）
          │                           │
    物理层 / 碰撞层                ���画层 / 视觉层
          │                           │
  "角色的胶囊体走在哪里"        "角色的脚看起来踩在哪里"
          │                           │
  Ramp 让角色平滑移动            IK 让脚贴合视觉台阶
```

### 7.2 类魂游戏实际做了什么

```
玩家看到的效果：
  角色走楼梯，身体平滑上升，脚一前一后踩在台阶上

实际分成两层：

层 1 — 碰撞层（我们的 Ramp）：
  角色胶囊体走在隐形斜面上
  CharacterMovementComponent 认为自己在走坡道
  身体平滑上升，没有卡顿
  → 这就是我们做的工具

层 2 — 动画层（Foot IK）：
  每帧从角色脚部向下发射射线（Line Trace）
  射线穿过隐形 Ramp，命中下方真实台阶的视觉面
  IK 系统把脚骨骼拉到命中点
  脚看起来踩在真实台阶上
  → 这是动画蓝图里做的，和我们的工具无关
```

### 7.3 侧视图详解

```
侧视图 — 实际发生的事：

  角色胶囊体 ●          ● 在 Ramp 表面滑动（平滑）
             │         ╱
  角色骨骼   ─┤       ╱  Ramp（隐形碰撞）
             │     ╱
  左脚 IK ───┤───╱─→ 射线命中台阶面 → 脚贴合台阶
  右脚 IK ───┤─╱──→ 射线命中台阶面 → 脚贴合台阶
             ┌┘
            ┌┘  ← 视觉台阶（原始 Mesh）
           ┌┘
           └──
```

### 7.4 Foot IK 实现原理（AnimBP 中）

```
AnimBP 中每帧执行：

1. 从左脚骨骼位置向下发射 Line Trace
   TraceStart = 左脚骨骼世界位置
   TraceEnd = TraceStart - FVector(0, 0, 50)  // 向下 50cm

   关键：Trace 要忽略 Ramp 碰撞体（Tag: StairRamp_Auto）
         只命中原始视觉 Mesh

2. 如果命中：
   IK_Offset = HitPoint.Z - FootBone.Z
   用 Two Bone IK 节点把脚移到 HitPoint
   同时旋转脚踝对齐 HitNormal（法线）

3. 右脚同理

4. 骨盆调整：
   PelvisOffset = Min(LeftFoot_Offset, RightFoot_Offset)
   整体下移骨盆，防止腿被过度拉伸
```

### 7.5 优先级和接入点

```
1. 先做 Ramp 工具（我们现在的工作）
   → 解决角色移动卡顿的核心问题
   → 没有 Foot IK 也能用，只是脚会"滑"在斜面上
   → 很多独立游戏就停在这一步，也完全可以接受

2. 后做 Foot IK（未来优化项）
   → 纯视觉提升，让脚贴合台阶
   → 需要在 AnimBP 中配置，和 Ramp 工具代码无关
   → 唯一的交互点：Line Trace 要忽略 StairRamp_Auto Tag
   → 我们的 Ramp 工具已经为此做好了准备（Tag 标记）
```

---

## 八、CharacterMovementComponent 参数建议

生成 Ramp 后，可以考虑调整以下参数（非必须，按需调整）：

| 参数 | 默认值 | 建议值 | 说明 |
|------|--------|--------|------|
| Max Step Height | 45cm | 30-45cm | 有 Ramp 后不再需要"爬"台阶，可以调低 |
| Walkable Floor Angle | 44.765° | 保持默认或最多 50° | 一般楼梯坡度 25-40°，不会超限。不要调到 80°（太激进） |

---

## 九、整体架构

### 9.1 模块位置

```
Source/soulEditor/
├── Public/
│   ├── FolderColorMarkerBFL.h     （已有）
│   └── StairRampGeneratorBFL.h    （新增）
├── Private/
│   ├── soulEditorModule.cpp       （已有）
│   ├── FolderColorMarkerBFL.cpp   （已有）
│   └── StairRampGeneratorBFL.cpp  （新增）
└── soulEditor.Build.cs            （已有，可能需添加依赖）
```

### 9.2 类架构图

```
┌─────────────────────────────────────────────────┐
│              StairRampGenerator                  │
│         (Editor Utility in soulEditor)          │
├─────────────────────────────────────────────────┤
│                                                  │
│  ┌──────────────┐    ┌────────────────────┐     │
│  │  入口函数      │    │  楼梯类型识别器      │     │
│  │ GenerateRamps │───→│ ClassifyStairType  │     │
│  │ ForSelected() │    │                    │     │
│  └──────┬───────┘    └────────┬───────────┘     │
│         │                     │                  │
│         │         ┌───────────┼───────────┐     │
│         │         ▼           ▼           ▼     │
│         │  ┌──────────┐ ┌──────────┐ ┌────────┐│
│         │  │ Straight  │ │ L-Shaped │ │ Spiral ││
│         │  │ Generator │ │ Generator│ │Generator││
│         │  └────┬─────┘ └────┬─────┘ └───┬────┘│
│         │       │            │            │     │
│         │       ▼            ▼            ▼     │
│         │  ┌────────────────────────────────┐   │
│         │  │     Ramp Collision Spawner      │   │
│         │  │  - 生成 BoxCollision Actor      │   │
│         │  │  - 设置位置/旋转/大小            │   │
│         │  │  - 不动原始楼梯碰撞（默认）       │   │
│         │  └────────────────────────────────┘   │
│         │                                        │
│  ┌──────┴───────┐                               │
│  │ 共用工具函数   │                               │
│  │              │                               │
│  │ - ExtractMeshTriangles()  从 Mesh 提取三角形  │
│  │ - DetectStairSteps()      识别踏步面          │
│  │ - ClassifyStairType()     判断直梯/L形/旋转    │
│  │ - CalculateSlopeAngle()   计算斜面角度        │
│  │ - SpawnRampCollision()    生成碰撞体          │
│  │ - RemoveAutoRamps()       删除自动生成的 Ramp  │
│  └──────────────┘                               │
│                                                  │
└─────────────────────────────────────────────────┘
```

### 9.3 公开接口（Blueprint Callable）

```cpp
UCLASS()
class SOULEDITOR_API UStairRampGeneratorBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 为编辑器中当前选中的 Actor 生成楼梯 Ramp
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void GenerateRampsForSelected();

    // 删除关卡中所有自动生成的 Ramp
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void RemoveAllAutoRamps();

    // 为选中的 Actor 生成 Ramp（自定义参数）
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void GenerateRampsWithParams(
        float Thickness = 5.0f,
        bool bDisableOriginalCollision = false);
};
```

---

## 十、数据流详解

```
完整数据流：

Step 1: 用户在编辑器中选中一个或多个楼梯 Actor
         │
         ▼
Step 2: GenerateRampsForSelected()
         │
         ├─ 获取编辑器当前选中的 Actor 列表
         │  GEditor->GetSelectedActors()
         │
         ▼
Step 3: For each selected Actor:
         │
         ├─ 获取 StaticMeshComponent
         │  Actor->FindComponentByClass<UStaticMeshComponent>()
         │
         ├─ 获取 StaticMesh
         │  MeshComp->GetStaticMesh()
         │
         ▼
Step 4: ExtractMeshTriangles(MeshComp)
         │
         ├─ StaticMesh->GetRenderData()->LODResources[0]
         ├─ 读取 PositionVertexBuffer → 顶点位置数组
         ├─ 读取 IndexBuffer → 三角形索引数组
         ├─ 转换到世界坐标（MeshComp->GetComponentTransform()）
         ├─ 计算每个三角形的法线和面积
         │
         ▼ 输出：TArray<FStairTriangle>

Step 5: DetectStairSteps(Triangles)
         │
         ├─ 筛选 Normal.Z > 0.95 的三角形
         ├─ 按 Center.Z 排序
         ├─ Z 高度聚类（容差 2cm）
         ├─ 计算每级踏步面的 Height, Center, Area
         ├─ 过滤面积 < 100cm² 的噪声
         ├─ 识别平台（面积 > 平均值 × 2）
         │
         ▼ 输出：TArray<FStairStep>（按高度排序）

Step 6: ClassifyStairType(Steps)
         │
         ├─ 计算每级踏步面相对中心的 XY 角度
         ├─ 分析角度变化序列
         │
         ├─ 角度变化 < 10° → Straight
         ├─ 有平台 + 角度跳变 → LShaped
         ├─ 角度持续变化 > 90° → Spiral
         │
         ▼ 输出：EStairType

Step 7: 根据类型分发
         │
         ├─ Straight → StraightStairGenerator(Steps)
         │   ├─ A = 最低踏步面前缘顶点
         │   ├─ B = 最高踏步面后缘顶点
         │   ├─ H = B.Z - A.Z
         │   ├─ Direction = A → B 在 XY 平面的投影
         │   ├─ L = 水平距离
         │   ├─ θ = atan2(H, L)
         │   ├─ S = √(H² + L²)
         │   └─ 生成 1 个 Box: (S/2, W/2, Thickness/2), Pitch=-θ
         │
         ├─ LShaped → LShapedStairGenerator(Steps)
         │   ├─ 用平台分割 Steps 为多个 Flight
         │   ├─ 每个 Flight 独立按直梯算法处理
         │   └─ 生成 N 个 Box
         │
         └─ Spiral → SpiralStairGenerator(Steps)
             ├─ 估算 R, P, TotalAngle
             ├─ 分段（每 15° 一段）
             ├─ 每段计算起止点、方向、坡度
             └─ 生成 N 个 Box
         │
         ▼
Step 8: SpawnRampCollision() （每个 Box 调用一次）
         │
         ├─ World->SpawnActor()
         ├─ 添加 UBoxComponent
         ├─ 设置碰撞为 BlockAll（对 Pawn 阻挡）
         ├─ SetVisibility(false)
         ├─ 添加 Tag "StairRamp_Auto"
         ├─ 可选：关闭原始楼梯碰撞（bDisableOriginalCollision）
         │
         ▼ 输出：关卡中生成的不可见 Ramp Actor
```

---

## 十一、可行性与风险评估

| 项目 | 直梯 | L形 | 旋转梯 | 建筑内嵌 |
|------|------|------|--------|---------|
| 自动检测 | ✅ 简单 | ⚠️ 中等 | ⚠️ 中等 | ⚠️ 全自动+手动补齐 |
| 角度计算精度 | ✅ 精准 | ✅ 分段后精准 | ⚠️ 近似（分段越多越精准） | ✅ 精准 |
| 实现难度 | ⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| 提取 Mesh 三角形 | ⚠️ UE4.27 API 需验证 | 同左 | 同左 | 同左 |

### 风险详情

```
风险 1: UE4.27 Mesh 数据访问
  问题：从 StaticMesh 提取三角形数据的 API 在 Editor 模式下可能有限制
  缓解：优先使用 RenderData 路径，fallback 到 BodySetup
  验证方法：Phase 1 开始时先写一个测试函数提取三角形并打印数量

风险 2: BoundingBox 包含装饰元素
  问题：楼梯 Mesh 可能包含扶手、栏杆等装饰，影响 BoundingBox 精度
  缓解：使用踏步面检测而不是 BoundingBox 来确定楼梯范围

风险 3: 角色穿透 Ramp
  问题：如果 Ramp 太薄，高速移动的角色可能穿透
  缓解：默认厚度 5cm，提供可调参数

风险 4: 多个碰撞体重叠（L形/旋转楼梯）
  问题：分段 Ramp 之间可能有缝隙或重叠
  缓解：相邻段之间有少量重叠（1-2cm），确保无缝隙

风险 5: 建筑内嵌楼梯误识别
  问题：大楼 Mesh 中可能把非楼梯区域（斜屋顶、装饰台阶）误识别为楼梯
  缓解：全自动生成 + 手动 Block Volume 补齐/删除误判
```

---

## 十二、开发顺序

```
Phase 1: 直梯 Ramp（用户选中 → 按钮 → 自动分析 → 生成）
  - ExtractMeshTriangles()
  - DetectStairSteps()（基础版）
  - StraightStairGenerator()
  - SpawnRampCollision()
  - RemoveAllAutoRamps()
  - 可选参数 bDisableOriginalCollision
  - 精度加强：踏步面边缘定位、Z 偏移、宽度计算、方向推导
  预计：1-2 次 Agent 会话
         ↓
Phase 2: 建筑内嵌楼梯（全自动 + 手动补齐）
  - 自动扫描建筑 Mesh 检测楼梯区域
  - 自动生成 Ramp（复用 Phase 1 逻辑）
  - 不做预览/确认 UI（省掉开发）
  - 用户手动用 Block Volume 补齐不完美部分
  - 可选：RANSAC 优化拟��精度
  预计：1 次 Agent 会话
         ↓
Phase 3: L形楼梯
  - ClassifyStairType()
  - 平台识别 + 分段
  - 多段 Ramp 生成
  预计：1-2 次 Agent 会话
         ↓
Phase 4: 旋转楼梯
  - 螺旋线参数估算
  - 分段算法（每 15° 一段）
  - 多段小 Ramp 拼接
  预计：2 次 Agent 会话
         ↓
Future: 优化项
  - Foot IK 适配（AnimBP 侧，与 Ramp 工具独立）
  - Debug 可视化工具（显示/隐藏所有 Ramp）
  - CharacterMovement 参数一键优化
```

---

## 十三、UI 设计

### Editor Utility Widget 按钮

```
在现有的 FolderColorTools Widget 中追加，或新建 StairRampTools Widget：

┌─────────────────────────────┐
│  Stair Ramp Generator       │
│                             │
│  [Generate Ramp]            │  ← 为选中的楼梯生成 Ramp
│  [Remove All Ramps]         │  ← 删除所有自动生成的 Ramp
│                             │
│  Thickness: [5.0] cm        │  ← 可选：Ramp 厚度参数
│  Segment Angle: [15] °      │  ← 可选：旋转楼梯分段角度
│  □ Disable Original         │  ← 可选：关闭原始碰撞
│    Collision                │
│                             │
└─────────────────────────────┘
```

### 操作流程

```
1. 在场景中选中一个或多个楼梯 Actor
2. 运行 StairRampTools Widget
3. 点击 "Generate Ramp"
4. 查看 Output Log 确认生成结果
5. 运行游戏测试角色行走
6. 如不满意：
   - 调整整体 Ramp 的 Rotation/Position
   - 放 Block Volume 补缝
   - 或点击 "Remove All Ramps" 清除，调整参数后重新生成
```

---

## 十四、测试验证计划

```
测试 1: 直梯基础功能
  - 放置一个简单的直梯 Mesh
  - 选中 → Generate Ramp
  - 验证 Ramp 角度和位置正确
  - 运行游戏，角色能平滑走上去

测试 2: 多个直梯批量处理
  - 选中场景中 5+ 个直梯
  - 一次性 Generate
  - 所有梯子都有 Ramp

测试 3: 角色行走体验
  - 角色从底部跑到顶部，无卡顿
  - 角色从顶部跑到底部，无弹跳
  - AI 敌人能正常寻路通过

测试 4: Remove All Ramps
  - 清除所有 Ramp
  - 确认关卡中无残留

测试 5: 建筑内嵌楼梯
  - 选中含楼梯的建筑
  - 验证自动检测楼梯区域
  - 手动用 Block Volume 补齐不完美部分
  - 建筑其他部分碰撞不受影响

测试 6: bDisableOriginalCollision
  - 对单独楼梯开启此参数
  - 确认原始碰撞已关闭
  - 角色只走在 Ramp 上

测试 7: 精度验证
  - 角色走在 Ramp 上不应"漂浮"或"嵌入"
  - Ramp 两端应完全覆盖楼梯（不缩进）
  - 楼梯宽度匹配（不超出也不缩进）
```