# UE4.27 Light Function 质感灯区域遮罩方案

## 一、项目背景

### 关卡结构
- 关卡分为 **山上**、**山下**、**地下** 三个区域
- 美术风格追求 **黑魂3（Dark Souls 3）** 的阴暗柔和质感
- 玩家出生点位于 **山上**，需要保持恐怖不安的氛围

### 现有全局光照设置
| 光源 | 用途 | 关键设置 |
|------|------|----------|
| Sky Light | 全场景基础照明 | 全局 |
| 大型 Spot Light（质感灯） | 从远方往近处投射，配合雾气营造柔和质感 | Cast Shadow = **Off**，衰减范围覆盖整个山上山下 |
| 闪电系统 | 全局闪电曝光效果 | 动态触发 |

### 现有 Post Process Volume 设置
| PPV | 覆盖范围 | Priority | 用途 |
|-----|---------|----------|------|
| 全局 PPV | Unbound | 0 | 黑魂3整体色调 |
| 教堂内部 PPV | 教堂区域 | 1 | 屏蔽闪电全局曝光 |

---

## 二、核心问题

大型质感射灯（Spot Light）的衰减范围必须覆盖整个山上山下，因为：

- **山下效果极好**：大衰减范围使光线在远距离衰减后非常柔和，岩石背光面和雾气配合产生黑魂3风格的质感
- **如果缩小衰减范围**：山下最需要这种质感的区域会丢失效果

但由此带来的问题：

- **山上（玩家出生区域）被照亮**，恐怖感减弱
- **教堂内部也会被照到**（光从门口射入）
- 该射灯 Cast Shadow = Off（因为它是质感灯不是拟真光源），所以无法通过任何遮挡物投射阴影来遮挡

### 核心矛盾

> **���减范围大 = 山下质感好但山上被照亮**
> **衰减范围小 = 山上没问题但山下质感丢失**
>
> 需要的是：同一盏灯，对不同区域输出不同强度。

---

## 三、已评估并排除的方案

| 方案 | 排除原因 |
|------|----------|
| 缩小射灯衰减范围 | 山下质感丢失 |
| 换 Rect Light 缩小范围 | 同上，本质是同一个矛盾 |
| Lighting Channel 分区 | 粒度是 per-component，且需要批量改山上所有 mesh 和灯光的 Channel；Foliage（植被）全图共享同一设置无法按区域分；Landscape（地形）按 Section 分块会出现锯齿分界线；工作量大且有副作用 |
| 负值光源抵消 | UE4.27 Intensity 最小值 = 0，不支持负值 |
| 透明 Box 投射阴影遮挡 | 射灯 Cast Shadow = Off，任何物体都不会为它投射阴影 |
| 动态调整射灯 Intensity | 玩家可能回访出生点，动态碰撞调整 Intensity 会导致曝光跳变，架构不合理 |
| PPV 局部压暗 | 只改后处理画面亮度，物体表面高光和受光面仍在，质感会不对；可作为兜底但不能作为主方案 |

---

## 四、最终方案：Light Function（灯光函数材质）

### 原理

Light Function 是赋予灯光的一个 Material，作用类似影视片场的 **旗板（Flag）遮光**：
- 灯本身不动、衰减范围不变、亮度不变
- 通过材质输出 0~1 的值，控制灯光在不同方向/位置上的实际亮度
- **1 = 正常亮度**（山下区域，保留质感）
- **0 = 完全不照**（山上区域，保持黑暗）
- **0~1 之间 = 柔和过渡**（避免硬边分界线）

### 为什么这是最优解

| 维度 | 说明 |
|------|------|
| 衰减范围 | **不需要改动**，保持大范围覆盖 |
| 山下柔和质感 | **完全保留**，Light Function 在山下输出 1.0 |
| 山上照射 | **被材质遮罩为 0**，恐怖感保留 |
| 过渡效果 | 通过 Divide 节点控制渐变宽度，**无硬边** |
| 性能开销 | **极低**，仅一个像素级材质运算 |
| 侵入性 | **只修改这一盏灯**，不碰任何 mesh、Lighting Channel、PPV |
| Cast Shadow 依赖 | **无关**，不需要开启 Cast Shadow |
| 教堂问题 | 可以在同一个材质中 **同时处理教堂区域的遮罩** |

---

## 五、Light Function 材质制作步骤

### 第一步：创建材质

1. Content Browser → 右键 → Material → 命名为 `M_LightFunction_QualityLight`
2. 打开材质，设置：
   - **Material Domain = Light Function**
   - Blend Mode 保持默认

### 第二步：节点连接（基础版 - 按高度遮罩）

```
[WorldPosition]  →  [Break/ComponentMask: Z轴]
                          ↓
                    [Subtract: Z - 高度阈值]
                    （高度阈值 = 山上山下分界线的Z值，建议用 Scalar Parameter 方便调整）
                          ↓
                    [Divide: 结果 / 过渡宽度]
                    （过渡宽度越大，渐变越柔和，建议用 Scalar Parameter）
                          ↓
                    [Saturate / Clamp 0-1]
                          ↓
                    [OneMinus: 翻转]
                    （使高处=0不照，低处=1正常照）
                          ↓
                    → 连接到 Emissive Color 输出
```

### 关键参数说明

| 参数 | 含义 | 建议初始值 |
|------|------|-----------|
| HeightThreshold（高度阈值） | 山上山下分界线的世界空间 Z 坐标 | 根据关卡实际高度设定 |
| TransitionWidth（过渡宽度） | 从全亮到全暗的渐变距离 | 500~2000（单位 cm），越大越柔和 |

### 第三步：赋予灯光

1. 选中大型质感射灯（Spot Light）
2. Details 面板 → **Light Function Material** → 选择 `M_LightFunction_QualityLight`
3. 完成

### 第四步：调试

- 调整 HeightThreshold 确认分界高度正确
- 调整 TransitionWidth 确认过渡区域自然，无明显分界线
- 确认山下质感未受影响
- 确认山上出生区域恢复黑暗氛围

---

## 六、后续可扩展方向

以下是可以在此基础上继续探索的内容：

### 1. 多区域遮罩
如果不止山上需要遮罩（例如教堂内部也要遮），可以在材质中叠加多个条件：
- 按 Z 高度遮罩山上
- 按 XY 范围遮罩教堂区域
- 多个条件用 **Min（取最小值）** 节点合并输出

### 2. 使用 Texture 做遮罩
如果区域形状复杂，不方便用数学公式描述：
- 制作一张从灯光视角的灰度遮罩贴图
- 在 Light Function 材质中采样这张贴图
- 类似影视中的 Gobo/Cookie 投影

### 3. 与 Rect Light 结合
如果后续决定将射灯换为 Rect Light：
- Rect Light 同样支持 Light Function
- Rect Light 的矩形投射 + Light Function 遮罩 = 更精确的控制

### 4. Blueprint 动态控制
- Light Function 的 Scalar Parameter 可以通过 Material Parameter Collection 或 Dynamic Material Instance 在运行时修改
- 可实现：随剧情推进动态改变遮罩区域

---

## 七、当前关卡光照架构总览

```
全局光照层
├── Sky Light（全局基础照明，所有区域）
├── 大型质感射灯 + Light Function 遮罩
│   ├── 山下：Light Function 输出 1.0 → 正常照射，柔和质感
│   ├── 山上：Light Function 输出 0.0 → 不照射，保持黑暗
│   └── 过渡带：Light Function 输出 0~1 → 柔和渐变
└── 闪电系统（全局动态曝光）

后处理层
├── 全局 PPV (Priority 0)：黑魂3色调
├── 教堂 PPV (Priority 1)：屏蔽闪电曝光
└── [可选] 山上 PPV (Priority 1)：额外氛围微调兜底

区域局部灯光
├── 山上：火把、局部 Point Light 等
├── 山下：局部灯光
└── 教堂内：室内灯光
```