# UE4.27 Landscape Tessellation —— 原理与决策深度解析

> 本文档汇总了关于 UE4.27 Landscape Pro V2 项目中 Tessellation 相关的原理探讨、决策推理和操作定位。  
> 作为《关闭 Landscape Tessellation 操作手册》和《地面视觉行动手册》的**理论补充文档**。  
> 阅读对象：希望深入理解"为什么这么做"而不只是"怎么做"的开发者。

---

## 目录

1. [三种 Tessellation Mode 的本质区别](#一三种-tessellation-mode-的本质区别)
2. [Scalar Displacement 与 Tessellation 的关系](#二scalar-displacement-与-tessellation-的关系)
3. [崩溃的根本原理（Cook 时的判定 bug）](#三崩溃的根本原理cook-时的判定-bug)
4. [为何不选择 Flat Tessellation 或 PN Triangles](#四为何不选择-flat-tessellation-或-pn-triangles)
5. [反向思考：开启 Tessellation 让两边对齐能否解决？](#五反向思考开启-tessellation-让两边对齐能否解决)
6. [断开材质引脚为何不影响最终渲染](#六断开材质引脚为何不影响最终渲染)
7. [Tessellation 与 Scalar Displacement 的设置位置](#七tessellation-与-scalar-displacement-的设置位置)
8. [核心结论速查](#八核心结论速查)

---

## 一、三种 Tessellation Mode 的本质区别

UE4 材质里的 **D3D11 Tessellation Mode** 共有三个选项。

### 速查表

| 模式 | 中文 | 是否产生新几何 | 平滑表面 | 性能开销 | 触发崩溃风险 |
|---|---|---|---|---|---|
| **No Tessellation** | 关闭曲面细分 | ❌ 不产生 | ❌ 否 | ✅ 零 | ✅ 无 |
| **Flat Tessellation** | 平面曲面细分 | ✅ 增加三角形 | ❌ 否（保持原形状） | ⚠️ 中（5~10×） | ⚠️ 有 |
| **PN Triangles** | PN 三角形曲面细分 | ✅ 增加三角形 | ✅ 是（自动平滑） | 🔴 高（8~15×） | 🔴 有 |

### 1. No Tessellation（关闭曲面细分）
- **原理**：不做任何几何细分，像素级渲染只用 Mesh 原本的三角形
- **完全不会**生成 Adjacency Index Buffer
- **视觉**：完全依赖 Normal Map / Detail Normal 表现表面细节
- **性能**：零开销
- **适用**：99% 的现代项目、所有要打包到 Shipping 的项目、UE5 项目

### 2. Flat Tessellation（平面曲面细分）—— "切碎但不变形"
- **原理**：把每个三角形等分成更多小三角形，但**不改变原本形状**
- **必须配合 World Displacement 才有效果**（用 Heightmap 把切出来的小顶点"顶"出来）
- **视觉**：
  - 没有 Heightmap 时：完全看不出区别（只是切碎了）
  - 配合 Heightmap：地面真实凸起，掠射角能看到轮廓变化
- **性能**：比 No Tessellation 高，但比 PN Triangles 便宜
- **典型用途**：历史上常用于 Static Mesh 单材质做 Displacement（如砖墙）

### 3. PN Triangles（PN 三角形曲面细分）—— "切碎 + 自动平滑变弯"
- **原理**：切碎三角形 + 根据顶点法线自动计算曲面（类似 Bezier 曲面）
- **不需要 Heightmap 也能让低模自动变圆润**
- **视觉**：原本带棱角的低模会自动变得圆润，配合 Heightmap 既圆润又有凹凸
- **性能**：三种模式中最贵
- **典型用途**：让低模角色 / Mesh 在近景下变圆润
- **缺点**：在硬边几何（方块、建筑）上会过度平滑，反而看起来"软趴趴"

### 对比示例

#### 砖墙（带 Heightmap）的视觉
| 模式 | 效果 | 性能 |
|---|---|---|
| No Tessellation | 平的砖墙，靠 Normal Map 制造光影凹凸，掠射角穿帮 | 1× |
| Flat Tessellation | 砖块真的从墙面凸出来，掠射角有真实轮廓 | 5~10× |
| PN Triangles | 砖块凸出 + 边缘自动圆润 | 8~15× |

#### 低模球（无 Heightmap）的视觉
| 模式 | 效果 |
|---|---|
| No Tessellation | 棱角分明的多面体球 |
| Flat Tessellation | 还是棱角分明的多面体球（只是三角形变多） |
| PN Triangles | 自动变成光滑球 |

→ 本质区别：**Flat = 切碎，必须配 Heightmap 才有用**；**PN = 切碎 + 自动平滑，无 Heightmap 也有效果**。

---

## 二、Scalar Displacement 与 Tessellation 的关系

### Scalar Displacement 是什么？
"Scalar Displacement" **不是引擎的固定术语**，而是美术约定俗成的叫法，指的是：
> Material Instance 里所有控制位移强度的**标量（数值）参数**

常见名字：
- `Tessellation Multiplier` / `Tess Mult` / `TessMult`
- `World Displacement Amount`
- `Displacement Strength` / `Displacement Height` / `Height Amount`

### 在 No Tessellation 模式下，它们的状态

| 参数类型 | 在 No Tessellation 下 |
|---|---|
| 连到 `World Displacement` 引脚的参数 | ❌ 完全失效（调了没用） |
| 连到 `Tessellation Multiplier` 引脚的参数 | ❌ 完全失效 |
| 连到 `Pixel Depth Offset` 引脚的参数 | ✅ 仍然有效 |
| 连到 `Heightmap Layer Blend` 的参数 | ✅ 仍然有效（很重要，别误关） |

**为什么失效？**
> Displacement 的本质是"把细分出来的新顶点按高度顶起来"。  
> 没有 Tessellation = 没有新顶点 = 没东西可以"顶" = Displacement 参数完全失效。

### Heightmap 的 4 种用途（破除误区）

很多人误以为 "关掉 Tessellation = Heightmap 没用了"，**这是错的**。

| 用途 | 是否依赖 Tessellation | No Tessellation 下是否可用 |
|---|---|---|
| World Displacement（真位移） | ✅ 依赖 | ❌ 失效 |
| Heightmap Layer Blend（高度混合） | ❌ 不依赖 | ✅ 仍可用 |
| Pixel Depth Offset（伪位移） | ❌ 不依赖 | ✅ 仍可用 |
| POM Sample UV Offset（视差） | ❌ 不依赖 | ✅ 仍可用 |

### ⚠️ 关键提醒：不要"一刀切"归零所有 Height 参数

#### ✅ 可以放心归零（Tessellation 相关）
- `Tessellation Multiplier` → 0
- `World Displacement Amount` → 0
- `Displacement Height` → 0
- `PN Triangles` / `Crack Free Displacement` → False
- `Distance Tessellation` / `Adaptive Tessellation` → False

#### ❌ **不要**乱关（影响 Layer Blend 视觉）
- `Layer Height Blend` 相关参数
- `Heightmap Contrast` / `Height Blend Sharpness`
- `Detail Normal Intensity`
- `Macro Variation` 相关
- `Normal Strength`

如果误关了 Layer Blend 相关 Height 参数，Landscape Pro V2 的层与层之间过渡会**变得很硬、很难看**，那不是 Tessellation 的问题，是 Layer Blend 被误伤了。

---

## 三、崩溃的根本原理（Cook 时的判定 bug）

### 崩溃报错
```
Assertion failed: !bCurrentRequiresAdjacencyInformation || 
(bCurrentRequiresAdjacencyInformation && SharedBuffers->AdjacencyIndexBuffers)
File: D:/Build/++UE4/Sync/Engine/Source/Runtime/Landscape/Private/LandscapeRender.cpp
Line: 2164
```

### Cook 时的判定逻辑（简化版）
```
判定 1：bRequiresAdjacencyInformation
  ├─ World Displacement 引脚有连线？
  ├─ Tessellation Multiplier 引脚有连线？
  └─ Crack Free Displacement = true？
  → 任一为真 → 标记需要 Adjacency Buffer

判定 2：是否生成 SharedBuffers->AdjacencyIndexBuffers
  └─ TessellationMode != None？
     → 是 → 生成
     → 否 → 不生成

运行时检查：
  if (bRequiresAdjacencyInformation && !AdjacencyIndexBuffers)
      → Fatal Error 💥
```

### 关键 bug
引擎判定"是否需要 Adjacency Buffer"时，**只看引脚有没有连线**，**不看 Tessellation Mode 是否为 None**。

所以如果你：
- ✅ Tessellation Mode = No Tessellation
- ❌ 但 World Displacement 引脚还连着东西

结果就是：
- 引擎标记：`bRequiresAdjacencyInformation = true`
- 但又因为 No Tessellation，没生成 AdjacencyIndexBuffers
- 运行时检查：**"要求有，实际没有"** → **触发 Assert，Fatal Error 崩溃** 💥

→ **这就是为什么必须断开引脚，不能只改 Mode**。

### Crack Free Displacement 同样的坑
`Crack Free Displacement = true` 也会单独触发 `bRequiresAdjacencyInformation = true`，  
即使 Tessellation Mode 是 None。  
→ 这就是为什么手册要求**两个都关 + 引脚都断**。

### 完整修复必须做这三件事

| 操作 | 是否可省略 | 为什么 |
|---|---|---|
| Tessellation Mode → No Tessellation | ❌ 不可省略 | 关闭 Tessellation 路径本身 |
| 取消 Crack Free Displacement | ❌ 不可省略 | 它会单独触发 Adjacency 需求 |
| 断开 World Displacement 引脚 | ❌ **不可省略** | 引擎会因为有连线就标记需要 Adjacency Buffer |
| 断开 Tessellation Multiplier 引脚 | ❌ **不可省略** | 同上 |
| Material Instance Scalar 参数归零 | ⚠️ 锦上添花 | 视觉无影响，但归零更干净 |

---

## 四、为何不选择 Flat Tessellation 或 PN Triangles

### 决策树
```
你的项目要打包发布吗？
├─ 是 → No Tessellation ✅
└─ 否（只在 Editor 内用）
    ├─ 想做位移效果？
    │   ├─ 是 → POM / PDO（更稳定）✅
    │   └─ 否 → No Tessellation ✅
    └─ 想做低模自动平滑？
        ├─ 是 → 用 Static Mesh 的 LOD / Subdivision Surface ✅
        └─ 否 → No Tessellation ✅
```

→ 在 UE4.27 任何情况下都推荐 **No Tessellation**。

### 三种模式不推荐的根本原因

#### 历史背景
- Tessellation 是 **DirectX 11 时代**（2009 年）的硬件特性
- 当时 GPU 算力有限，"按需细分"是性价比方案
- 但有一系列根本性问题：

#### 共通缺点
1. **打包后 Shader Permutation 容易出错**（你遇到的崩溃就是这个）
2. **不支持移动端 / 部分主机**（兼容性差）
3. **与 Nanite、Lumen 等新技术冲突**
4. **性能开销不稳定**（取决于视角和距离，难预算）
5. **裂缝、LOD pop 问题难以彻底解决**

#### Epic 官方态度
- **UE4.26 起标记为 Deprecated（弃用）**
- **UE5.0 起完全移除**
- 替代方案：**Nanite**（虚拟化几何）、**POM / PDO**（屏幕空间假位移）

### 为何 Landscape Pro 默认开 Flat Tessellation？
1. Landscape 是大面积平面，不需要 PN 的"自动平滑"
2. Landscape Pro 的设计依赖每层 Heightmap 做位移
3. Flat Tessellation 配 Heightmap 是经典的"切碎 + 顶起来"组合
4. 比 PN 便宜一些

但代价就是：**Cook 后丢 Adjacency Buffer 就崩** ← 你遇到的问题。

---

## 五、反向思考：开启 Tessellation 让两边对齐能否解决？

### 原理上成立 ✅
两条"对齐"的路径：

| 方案 | 判定 1（需要 Adjacency） | 判定 2（生成 Buffer） | 结果 |
|---|---|---|---|
| 现在（崩溃） | true（引脚有线） | false（Mode = None） | ❌ 崩 |
| **方案 A：彻底关** | false（引脚断开） | false | ✅ 不崩 |
| **方案 B：彻底开** | true | true（Mode = Flat/PN） | ✅ 不崩 |

理论上"打开 Tessellation 让两边对齐"也能消除这次崩溃。

### 但实际上方案 B 不可行，原因如下：

#### 原因 1：Tessellation 还有其他打包稳定性问题 🔴
即使绕过了 Adjacency Buffer 的 assert，Landscape Tessellation 在 UE4.27 打包后还有**一堆其他坑**：

- **Shader Permutation 缺失**：某些组合的 Tessellation Shader 在 cook 时漏掉，运行时崩
- **裂缝问题（Crack）**：Landscape 块与块之间的细分边界对不上，出现可见裂缝
- **LOD pop 跳变**：相机移动时细分等级切换，地形"抖一下"
- **DX12 / Vulkan 兼容性差**：很多用户反馈 DX12 + Tessellation = 必崩
- **多人项目 + Tessellation**：网络同步时偶发崩溃

→ 你只是修好了**这一个 assert**，**还会撞上其他坑**。Epic 把它标记为 Deprecated 不是没有道理的。

#### 原因 2：性能开销巨大 🔴
| 模式 | GPU 开销（相对） |
|---|---|
| No Tessellation | 1× |
| Flat Tessellation | 5~10× |
| PN Triangles | 8~15× |

Landscape 是大面积渲染，每一帧都要为整片地形做 Tessellation：
- 中端显卡掉 30~50% 帧率
- 低端显卡直接卡顿
- 对类魂项目希望兼容多种硬件来说，**性能损失不可接受**

#### 原因 3：技术债 🔴
- 项目绑定在已弃用技术上
- 未来升级 UE5 = **几乎要重做整个材质系统**（UE5 完全没有 Tessellation）
- 团队成员未来维护这个材质会非常痛苦

#### 原因 4：你本来就没在用它的视觉效果 🔴
- 你之前根本不知道 Landscape Pro 默认开了 Tessellation
- 也就是说你**没有"依赖"它的视觉效果**
- 关掉它对项目视觉影响**几乎为零**

### 完整对比

| 维度 | 方案 A：关闭 Tessellation | 方案 B：开启 Tessellation |
|---|---|---|
| 当前崩溃 | ✅ 解决 | ✅ 解决 |
| 其他打包问题 | ✅ 都不会有 | 🔴 还可能撞其他坑 |
| 性能 | ✅ 最快 | 🔴 慢 5~15× |
| 视觉损失 | ⚠️ 少量（可补偿） | ✅ 无 |
| 未来维护 | ✅ 简单 | 🔴 困难 |
| 升级 UE5 | ✅ 平滑 | 🔴 要重做 |
| 兼容低端硬件 | ✅ 好 | 🔴 差 |
| 业界主流方向 | ✅ 一致 | 🔴 逆流 |

→ 没有一个维度是方案 B 更好的，**除了"理论上也能修"这一点**。

### 工程哲学
> 修 bug 时，**"消除问题源"永远优于"满足问题需求"**。  
> 关掉 Tessellation = 消除源（不再需要 Adjacency Buffer）  
> 打开 Tessellation = 满足需求（Adjacency Buffer 真的有了）  
>  
> 在工程上，**前者总是更稳定、更可维护**。

---

## 六、断开材质引脚为何不影响最终渲染

### 简短结论
> **不需要任何占位符节点，直接断开就好，零问题。** ✅

### 原理：Material 节点系统是"输出驱动"的

UE4 的材质编辑器编译机制：
- 编译器从主输出节点（最右边）**反向追溯**
- 只编译"**有路径连接到输出引脚的节点**"
- 没连到输出的节点 = 编译器看不见 = 不会被��译

类比：材质图是一棵树
- 输出节点 = 树根
- 编译器从树根开始往上遍历每个分支
- 不属于任何分支的节点 = 这棵树的"外人"

### 断开后会发生什么？

#### 引脚端（材质主输出节点这一侧）
- `World Displacement` 引脚变成"空"
- `Tessellation Multiplier` 引脚变成"空"
- 引擎在编译时遇到空引脚，会**自动使用默认值**：
  - World Displacement 默认 = `(0, 0, 0)`（向量零）
  - Tessellation Multiplier 默认 = `1.0`（标量）
- **完全不会报错**，材质照常编译

#### 节点端（前面那一堆连着的节点）
- 那些节点变成"**孤岛节点**"——存在于材质图中，但没人使用
- 引擎编译 shader 时**自动检测并剔除**未使用的节点
- 这些节点**不会进入最终 shader**
- **不消耗任何运行时性能** ✅
- **不占用任何 GPU 指令** ✅

### 视觉验证
断开后在 Material Editor 里观察：

| 现象 | 含义 |
|---|---|
| 孤岛节点的外框颜色变暗 / 变灰 | 引擎告诉你"这些节点没用了" |
| Material Editor 上方 Stats 面板 Instruction Count 减少 | shader 指令变少，确实剔除了 |
| Apply 后没有报错或警告 | 一切正常 |

### 唯一需要注意的小坑
如果断开的节点链里包含 Material Function 或 Static Switch：
- 这些孤岛节点的"配置"还会留在材质资产里
- 文件大小可能略微大一点点
- 但**不影响运行时性能**

→ 这种情况也**不需要做任何处理**。如果想彻底干净可以删除这些孤岛节点，但**完全可选**。

### 三个保留方案（如果想以后还能用）

#### 方案 1：直接留着（最简单）✅ 推荐
- 断开就完了
- 节点都还在，未来想用重新连回去就行

#### 方案 2：用注释框标记
- 框选所有孤岛节点 → 按 `C` 创建 Comment 框
- 命名：`[DEPRECATED] Tessellation - DO NOT RECONNECT`
- 适合团队协作

#### 方案 3：备份原材质
- 复制一份完整的母材质，命名 `M_LandscapePro_Master_WithTess_BACKUP`
- 在新版本上断开
- 最保险

### 一句话总结
> **直接断开，零风险。**  
> 断开 = 那些节点变成孤岛 = 编译器自动忽略 = 不进 shader = 不消耗性能 = 不占任何资源。  
> **不需要占位符**，UE4 材质编译器会"装作没看见"那些节点。

---

## 七、Tessellation 与 Scalar Displacement 的设置位置

### Master Material 与 Material Instance 的关系
```
Master Material（母材质）         ← Tessellation Mode 在这里
  │
  │ "继承"关系
  │
  ├─ Material Instance A         ← Scalar Displacement 数值参数在这里
  ├─ Material Instance B         ← Scalar Displacement 数值参数在这里
  └─ Material Instance C         ← Scalar Displacement 数值参数在这里
```

### 类比理解
| 概念 | 类比 | 包含什么 |
|---|---|---|
| **Master Material** | 食谱模板 | 材质的"结构和逻辑"——节点、参数、Tessellation 模式 |
| **Material Instance** | 按食谱做的菜 | 只能"调味"——修改母材质暴露出来的具体数值 |

### 图标和文件区别

#### Master Material（母材质）
- **图标**：🟢 **绿色实心球**
- **命名**：通常以 `M_` 开头，如 `M_LandscapePro_Master`
- **打开后**：复杂的节点连线图（材质蓝图）

#### Material Instance（材质实例）
- **图标**：🟡 **黄色球**（带渐变效果）
- **命名**：通常以 `MI_` 开头或 `_Inst` 结尾
- **打开后**：参数列表面板，没有节点连线图

### 各项设置具体位置

#### ✅ 在 Master Material（绿球）里
| 设置项 | 位置 |
|---|---|
| **D3D11 Tessellation Mode** | Master Material → Details → Tessellation 分类 |
| **Crack Free Displacement** | Master Material → Details → Tessellation 分类 |
| **Material Domain** | Master Material → Details → Material 分类 |
| **Blend Mode** | Master Material → Details → Material 分类 |
| **World Displacement 引脚连线** | Master Material → 主输出节点 |
| **Tessellation Multiplier 引脚连线** | Master Material → 主输出节点 |
| **整个材质的节点逻辑** | Master Material → 节点画布 |

> ⚠️ 这些**只能在 Master Material 里改**，Material Instance 看不到也改不了。

#### ✅ 在 Material Instance（黄球）里
| 设置项 | 位置 |
|---|---|
| **Scalar Parameters**（Tessellation Multiplier 等） | Details → Scalar Parameter Values |
| **Vector Parameters** | Details → Vector Parameter Values |
| **Texture Parameters** | Details → Texture Parameter Values |
| **Static Switch Parameters** | Details → Static Switch Parameter Values |
| **Layer Parameters**（Landscape 层参数） | Details → Layer Parameters |

> ⚠️ Material Instance **只能改母材质里"暴露出来"的参数**。

### 为什么 Tessellation Mode 不能放在 Material Instance 里改？
> Tessellation Mode 影响的是 **Shader 编译方式**，不是运行时数值。  
> Material Instance 只能改"运行时可调的数值"。  
> Tessellation Mode 一旦决定，整个 Shader 的编译路径就定了，必须在 Master Material 里"一次决定"。

类比：
- Master Material = 决定"这道菜是炒还是炸"（烹饪方式）
- Material Instance = 决定"放多少盐和糖"（调味）

### Landscape Pro V2 典型文件结构
```
Content/
└── LandscapePro/
    ├── Materials/
    │   ├── 🟢 M_LandscapePro_Master            ← Tessellation 设置在这里
    │   ├── 🟢 M_LandscapePro_AutoMaterial      ← Tessellation 设置在这里
    │   ├── 🟢 M_LandscapePro_Hole              ← Hole 材质，也要检查
    │   │
    │   ├── 🟡 MI_LandscapePro_Forest           ← Scalar 参数在这里
    │   ├── 🟡 MI_LandscapePro_Snow             ← Scalar 参数在这里
    │   ├── 🟡 MI_LandscapePro_Desert           ← Scalar 参数在这里
    │   └── 🟡 MI_LandscapePro_Custom           ← Scalar 参数在这里
    │
    ├── Functions/                              ← Material Function 也要检查
    │   ├── MF_LandscapePro_Tessellation        ← 隐藏位移逻辑可能藏在这
    │   └── MF_LandscapePro_LayerBlend
    │
    └── Textures/
```

→ 修复涉及：
- 所有 🟢 Master Material 的 Tessellation 设置（核心，必做）
- 所有 🟡 Material Instance 的 Scalar 归零（可选，建议做）
- 检查 Material Function 内部（保险起见）

### 快速定位方法

#### 方法 1：从 Landscape 反查（推荐）
1. 选中场景里的 Landscape Actor
2. Details 面板 → Landscape 分类
3. Landscape Material 字段 → 点旁边的放大镜图标 🔍
4. 在 Content Browser 中跳转到该资产

#### 方法 2：判断打开的是哪种
| 特征 | Master Material（绿球） | Material Instance（黄球） |
|---|---|---|
| 编辑界面 | 大画布 + 节点连线 | 仅参数列表面板 |
| 工具栏 | 有 Apply 按钮 | 没有 Apply（自动应用） |

#### 方法 3：从 Material Instance 找母材质
1. 打开 Material Instance（黄球）
2. Details 面板顶部 → General → Parent
3. 点 Parent 旁边的放大镜 🔍 → 跳转到母材质

---

## 八、核心结论速查

### 最终修复操作矩阵

| 操作 | 在哪里做 | 必须做吗 | 为什么 |
|---|---|---|---|
| Tessellation Mode → No Tessellation | 🟢 Master Material | ✅ 必须 | 关闭 Tessellation 路径 |
| Crack Free Displacement → ❌ | 🟢 Master Material | ✅ 必须 | 单独触发 Adjacency 需求 |
| 断开 World Displacement 引脚 | 🟢 Master Material | ✅ **必须** | 有连线就触发 Adjacency 判定 |
| 断开 Tessellation Multiplier 引脚 | 🟢 Master Material | ✅ **必须** | 同上 |
| Scalar Displacement 参数归零 | 🟡 Material Instance | ⚠️ 可选 | 视觉无影响，归零更干净 |
| 检查 Material Function 内部 | Material Function | ⚠️ 推荐 | 防止隐藏位移逻辑漏掉 |
| 删 Saved/Cooked + Intermediate | 项目根目录 | ✅ 必须 | 旧 cook 缓存残留导致继续崩 |

### 为什么是 No Tessellation 而非其他方案

| 候选方案 | 是否推荐 | 主要原因 |
|---|---|---|
| **No Tessellation（关闭）** | ✅ 推荐 | 稳定、零开销、可维护、面向未来 |
| Flat Tessellation（保留） | ❌ 不推荐 | Cook 还有其他坑、5~10× 开销、技术债 |
| PN Triangles（升级） | ❌ 不推荐 | 性能更差、Landscape 用不到平滑、技术债 |
| 反向思路：彻底开 + 修对齐 | ❌ 不推荐 | 修一个 assert 但还会撞其他坑、性能/维护成本高 |

### 三句话总结

1. **崩溃的根因不是 Tessellation Mode 本身，而是"引脚连线判定"和"Tessellation Mode 判定"在 cook 时没对齐 → 必须断开引脚才能彻底修复。**

2. **Flat / PN Triangles 看似能"对齐解决"，但 Tessellation 在 UE4.27 打包后还有一系列其他不稳定问题，且性能、维护、未来升级都不利 → 业界共识就是关掉它。**

3. **关闭后，Tessellation 路径上的 Scalar 参数和孤岛节点都不会再影响渲染，UE4 材质编译器会自动剔除未使用的逻辑 → 直接断开，零风险，无需占位符。**

---

## 配套文档

本文档作为理论补充，配套以下两份操作手册一起使用：

1. **《关闭 Landscape Tessellation 操作手册》**：具体的崩溃修复步骤
2. **《地面视觉行动手册》**：关闭 Tessellation 后的视觉补偿方案

三份文档关系：
- 本文档 = **为什么这么做**（原理）
- 关闭手册 = **怎么修复崩溃**（操作）
- 视觉手册 = **修完之后怎么做漂亮**（增强）

---

完 🚀