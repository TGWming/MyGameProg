# 🎮 游戏开发 UE 技术参考文档（综合精华版）

> **整理日期**：2026-03-17
> **资料来源**：7篇核心技术文档 + xiaxia9/Game-Development-Notes Repo + 5篇补充文档
> **涵盖领域**：GAS战斗框架、资源管理、NPC避障、过场动画、Motion Matching、移动组件与网络同步、动画技术体系

---

## 目录

- [一、UE GAS 战斗系统性能优化](#一ue-gas-战斗系统性能优化)
- [二、UE UAssetManager 资源管理](#二ue-uassetmanager-资源管理)
- [三、游戏NPC避障算法（VO → RVO → ORCA）](#三游戏npc避障算法vo--rvo--orca)
- [四、UE4 LevelSequence 源码解析](#四ue4-levelsequence-源码解析)
- [五、UE5 Motion Matching 实现](#五ue5-motion-matching-实现)
- [六、UE4 移动网络同步](#六ue4-移动网络同步)
- [七、UE4 移动组件详解（原理分析）](#七ue4-移动组件详解原理分析)
- [八、动画技术参考链接库](#八动画技术参考链接库)
- [九、补充资料索引](#九补充资料索引)
- [附录：技术栈关联图](#附录技术栈关联图)

---

# 一、UE GAS 战斗系统性能优化

> **原文**：https://zhuanlan.zhihu.com/p/12646608505

## 1.1 GAS 核心架构

GAS（Gameplay Ability System）是 UE 内置的技能/战斗框架，核心组件：

| 组件 | 职责 |
|---|---|
| **AbilitySystemComponent (ASC)** | 挂载在 Actor 上的核心组件，管理技能、属性、效果 |
| **GameplayAbility (GA)** | 技能逻辑本体（释放、冷却、消耗等） |
| **GameplayEffect (GE)** | 属性修改器（伤害、Buff、持续效果） |
| **AttributeSet** | 属性集（血量、攻击力、防御等） |
| **GameplayCue** | 表现层（特效、音效），与逻辑解耦 |
| **GameplayTag** | 标签系统，驱动技能/状态的条件判断 |

## 1.2 七大性能优化策略

### ① 资源加载优化
- 使用 `FStreamableManager` 异步加载
- 利用 `UAssetManager` 按需管理（详见第二章）
- 避免在 GA 构造函数中硬引用资源

### ② GE 池化与复用
- 频繁创建/销毁 GE 开销大
- 维护 GE 对象池，复用已分配实例

### ③ 属性计算优化
- `AttributeSet` 的 `PreAttributeChange` / `PostGameplayEffectExecute` 避免重复计算
- 对频繁变动的属性使用脏标记

### ④ GameplayCue 优化
- Cue 独立于逻辑层，可异步加载
- 对远距离/不可见角色跳过 Cue 表现
- 使用 LOD 策略分级表现

### ⑤ Tag 查询优化
- `FGameplayTagContainer` 的 `HasTag` 内部是哈希比较，已经很快
- 避免每帧做大量 Tag 查询，可缓存结果

### ⑥ 网络同步优化
- 最小化 GE 同步频率
- 使用 `Minimal Replication` 模式
- 仅同步关键属性变化

### ⑦ 批处理与延迟执行
- 多个 GE 合并为一次 Apply
- 延迟非关键计算到下一帧

---

# 二、UE UAssetManager 资源管理

> **原文**：https://zhuanlan.zhihu.com/p/685072527

## 2.1 为什么需要 UAssetManager

| 传统方式 | UAssetManager |
|---|---|
| 硬引用 → 启动时全部加载 | 按需加载，运行时管理 |
| 资源依赖不透明 | Primary Asset + 依赖链清晰 |
| 内存管理困难 | 引用计数 + 卸载策略 |

## 2.2 核心概念

### Primary Asset（主资产）
- 由 `FPrimaryAssetId`（类型+名称）唯一标识
- 开发者主动管理的资源单位（角色、关卡、技能定义等）

### Secondary Asset（次级资产）
- 被 Primary Asset 引用的依赖资源（纹理、Mesh、音效等）
- 自动跟随 Primary Asset 加载/卸载

### Asset Bundle（资产包）
- 给 Primary Asset 的依赖分组打标签
- 例：角色资产分 `UI`（只加载头像）和 `Game`（加载完整模型）两个 Bundle

## 2.3 异步加载 API

```cpp
// 异步加载单个资产
FStreamableHandle Handle = AssetManager.LoadPrimaryAsset(
    FPrimaryAssetId("Character:Hero_01"),
    {"Game"},  // Bundle 名
    FStreamableDelegate::CreateUObject(this, &UMyClass::OnAssetLoaded)
);

// 异步加载多个资产
AssetManager.LoadPrimaryAssets(AssetIdArray, Bundles, Callback);

// 卸载
AssetManager.UnloadPrimaryAsset(AssetId);
```

## 2.4 关键实践

- 继承 `UAssetManager` 自定义管理器，重写 `StartInitialLoading()`
- 在 `DefaultGame.ini` 中配置资产扫描路径
- 利用 `AssetManagerEditor` 工具查看依赖关系
- 结合 Chunk 进行分包，支持热更新

---

# 三、游戏NPC避障算法（VO → RVO → ORCA）

> **原文**：https://zhuanlan.zhihu.com/p/1919204895858340269
> **Demo**：https://github.com/Xiaojie-Huang/ORCA (Unity)

## 3.1 算法演进路线

```
射线避障（Simple）→ VO（速度障碍）→ RVO（互惠速度障碍）→ ORCA（最优互惠避障）
```

## 3.2 射线避障
- **原理**：前方发射射线 → 碰到障碍 → 沿法线旋转
- **缺陷**：大量 Agent 穿模/卡住，射线有物理性能开销

## 3.3 VO（Velocity Obstacle）
- **核心**：在速度域中构建"碰撞锥体"，选锥体外的速度
- **流程**：构建速度障碍锥 → 计算碰撞时间 → 生成候选速度（~180个） → 筛选可行速度 → 选最优
- **缺陷**：两 Agent 互相预判 → 来回抖动（"路上两人同时让路"问题）

## 3.4 RVO（Reciprocal VO）
- **改进**：假设对方也避障 → 每方只偏移 **50%**
- **缺陷**：多 Agent 场景仍抖动（三者互相预判全部失败）

## 3.5 ORCA（最优互惠避障）⭐
- **突破**：VO 从锥体 → **有向半平面**，求解从搜索 → **线性规划**

| 特性 | RVO | ORCA |
|---|---|---|
| VO 形状 | 无限高锥体 | 有向半平面 |
| 求解方式 | 候选速度搜索 | 线性规划（半平面交集） |
| 多 Agent | 抖动 | **稳定** |
| 效率 | 一般 | **更优** |

**ORCA 半平面生成**（三种情况）：
1. **已碰撞**：求指向 VO 域外的向量 `u` → 作为半平面法线
2. **未碰撞，w 在圆弧上**：根据投影角度求切线
3. **未碰撞，w 在边界线上**：两条边界方向几何求解

---

# 四、UE4 LevelSequence 源码解析

> **原文**：https://zhuanlan.zhihu.com/p/544571505

## 4.1 整体结构

```
ALevelSequenceActor（关卡实体）
├── UMovieSceneSequence  ← 资产数据（不依赖 Level）
└── ULevelSequencePlayer ← 播放器（持有场景数据）
```

## 4.2 数据层级（五层）

```
MovieSceneSequence
  └── MovieSceneTrack          ← 按属性类型（Transform/Visibility...）
       └── MovieSceneSection   ← 关键帧区间（可多段混合）
            └── MovieSceneChannel ← 通道数据
                 └── MovieSceneValue  ← 关键帧值
```

- Track 按 Actor 组织，有独特 **Bind 系统**
- 所有 Track 继承 `UMovieScenePropertyTrack` → 可扩展自定义属性

## 4.3 Eval 过程演进

| UE 版本 | Eval 方式 |
|---|---|
| < 4.26 | Tokens 方式（单线程） |
| ≥ 4.26 | **EntitySystem**（多线程） |

## 4.4 关键帧数据（FMovieSceneFloatValue）

```cpp
float Value;                    // 关键帧值
float Tangent.ArriveTangent;    // 入切线角度
float Tangent.LeaveTangent;     // 出切线角度
float Tangent.ArriveTangentWeight;
float Tangent.LeaveTangentWeight;
ERichCurveTangentWeightMode TangentWeightMode;
ERichCurveInterpMode InterpMode;
ERichCurveTangentMode TangentMode;
```

## 4.5 数据导出（Transform → Curve）

### Step 1：获取绑定 Actor
```cpp
UMovieScene* MyScene = MyLevelSequence->GetMovieScene();
// Possessable（关卡中已有）
for(int i = 0; i < MyScene->GetPossessableCount(); i++)
    const FMovieScenePossessable& p = MyScene->GetPossessable(i);
// Spawnable（Sequence 自行生成）
for(int i = 0; i < MyScene->GetSpawnableCount(); i++)
    const FMovieSceneSpawnable& s = MyScene->GetSpawnable(i);
```

### Step 2：提取轨道数据
```
Track("Transform") → Sections → ChannelProxy
  → FMovieSceneFloatChannel[] → GetTimes() + GetValues()
    → 写入 FRichCurve
```

**⚠️ 关键**：切线值需乘 `MyResolution.Numerator`（Internal Tick 帧率）

## 4.6 实用 Tips

1. CameraComponent/SkeletonMesh 在 Sequence 中与 Actor **同层级**，顺序紧随其后
2. 相对位置通过 `bOverrideInstanceData` + `DefaultInstanceData` 获取
3. 调试入口：`/Editor/SequencerRecord` 打运行时断点看调用栈

---

# 五、UE5 Motion Matching 实现

> **原文**：https://zhuanlan.zhihu.com/p/2011816308874962280

## 5.1 核心思想

> 放弃传统状态机 → **数据驱动** → 每帧从大量动画数据中搜索最匹配的帧

UE5 通过 **Pose Search 插件** 实现。

## 5.2 动画蓝图核心节点

| 节点 | 职责 |
|---|---|
| **Motion Matching Node** | 基于上下文 + 上一帧 Pose → 决定输出 |
| **PoseHistory Node** | 缓存历史 Pose + 运动轨迹 |

## 5.3 关键资产体系

### CHT（ChooserTable）
```
GamePlay 上下文 → CHT 条件匹配 → 输出 PoseSearchDatabase
```
- 支持嵌套 Chooser、MM LOD（Dense/Sparse）

### PSD（Pose Search Database）
- 存储 **特征向量**（非原始骨骼变换）
- 按采样率逐帧扫描，每帧 = 一个候选姿态
- 数据组织：**KD-Tree**（PCA 降维后建树）

### PSS（Pose Search Schema）
特征向量 = **轨迹特征** + **姿态特征**

| 轨迹特征 | 姿态特征 |
|---|---|
| 过去点（惯性匹配） | 关键骨骼位置/旋转 |
| 未来点（**最重要**，匹配移动意图） | 脚部腾空/落地状态 |
| 当前速度 | |

### PSN（Pose Search Normalization）
- **Z-Score 标准化**：`Normalized = (Value - Mean) / StdDev`
- 多个 PSD 共享同一 PSN → 跨库无缝切换

## 5.4 完整 PSD 处理流程

```
采样 → 加权 → 统计(存入PSN) → Z-Score归一化 → PCA降维 → 建KD-Tree
```

## 5.5 运行时匹配

1. 构建查询向量（Schema + 当前状态 + 输入）
2. 计算 Cost：`Σ wᵢ × (Qᵢ - Cᵢ)²`
3. 两阶段搜索（Broad → Narrow）
4. **Bias 防抖**：新帧 Cost 须显著低于"继续播放当前帧"的 Cost + Bias

### 相关源码
```
Engine/Plugins/Animation/PoseSearch/Source/Runtime/Private/
├── PoseSearchDatabase.cpp
├── PoseSearchSchema.cpp
├── AnimNode_MotionMatching.cpp
├── KDTree.cpp
└── PoseSearchLibrary.cpp
```

## 5.6 攀爬系统实践（MM + Motion Warping + Chooser）

### 流程
```
障碍物检测 → 上下文收集 → 动画选择 → 运动适配
```

### 三类翻越行为

| 类型 | 核心条件 |
|---|---|
| **Vault**（支撑翻越） | 有前后边缘 + 无落脚点 + 高度50-125cm + 深度<59cm |
| **Hurdle**（跨越） | 有前后边缘 + 有落脚点 + 高度50-125cm + 深度>59cm |
| **Mantle**（攀爬） | 有前边缘 + 高度50-275cm + 深度>59cm |

### 运动适配
- Motion Warping：通过 Notify State 进行 Root Motion 匹配
- Distance Match 方式读取 CurveValue 决定位移距离

---

# 六、UE4 移动网络同步

> **原文**：https://zhuanlan.zhihu.com/p/114341957（网易雷火）

## 6.1 移动组件继承体系

```
UMovementComponent
  └── UNavMovementComponent（AI寻路）
       └── UPawnMovementComponent（接受输入）
            └── UCharacterMovementComponent（网络同步，最复杂）
```

## 6.2 网络同步四大目标

| 目标 | 说明 |
|---|---|
| Autonomous 无延迟 | 玩家操控立即响应 |
| Server 权威 | 服务器位置是唯一真理 |
| Simulate 平滑 | 其他客户端移动表现平滑 |
| 反外挂 | 服务器校验移动合法性 |

## 6.3 三种 NetRole

| NetRole | 位置 | 职责 |
|---|---|---|
| **Autonomous** | 玩家本地 | 输入→本地移动→RPC 发 Server |
| **Authority** | 服务器 | 校验→同步给所有客户端 |
| **Simulate** | 其他客户端 | 接收同步→插值平滑 |

## 6.4 Autonomous 客户端流程

### 核心数据：FSavedMove_Character
```
TimeStamp, DeltaTime, StartLocation, SavedLocation,
StartVelocity, SavedVelocity, Acceleration,
StartPackedMovementMode, EndPackedMovementMode
```

### ReplicateMoveToServer 流程
```
1. 查找 ImportantMove（与上次 Ack 有显著差异）
2. 创建 FSavedMove → 初始化
3. PerformMovement → 真正移动
4. 补充 Move 属性 → 加入 SavedMoves
5. 判断延迟发送（Move 合并优化）
6. CallServerMove → 发送 RPC
```

### Move 合并条件（不可合并的情况）
- `bForceNoCombine` 为 true
- 包含 RootMotion
- 加速度从非零变零
- DeltaTime 总和超 `MaxMoveDeltaTime`
- MovementMode 改变
- 跳跃/下蹲状态改变
- 胶囊体尺寸改变
- 站立表面改变

### FSavedMove 缓存池
- `FreeMoves`：默认 96，用 `CreateSavedMove`/`FreeMove`
- `SavedMoves`：默认 96，超出直接清空

## 6.5 Server 执行与校验

### ServerMove RPC 家族（全部 unreliable）

| RPC | 用途 |
|---|---|
| `ServerMove` | 单次移动 |
| `ServerMoveNoBase` | 无 Base 信息，省带宽 |
| `ServerMoveDual` | 一次两个 Move |
| `ServerMoveOld` | 冗余保险，快速追赶 |

### 校验流程
```
TimeStamp 校验 → 解压数据 → PerformMovement
→ 位置校验：
  ├── 通过 → ClientAckGoodMove（清理 SavedMoves）
  └── 不通过 → ClientAdjustPosition（纠正 + 重播）
```

### 位置校验不通过 → 重播
```
1. 设置位置/速度/Mode 为服务器值
2. 清理已 Ack 的 SavedMoves
3. bUpdatePosition = true
4. 下帧重播剩余 SavedMoves（不发 RPC）
5. 误差逐渐收敛
```

## 6.6 Simulate 客户端平滑

### 核心：Capsule 立即更新 + Mesh 插值平滑

| 特性 | Linear | Exponential |
|---|---|---|
| 运动 | 匀速 | 先快后慢 |
| 周期 | 基于服务器时间戳 | 固定周期 |
| 网络波动 | 较大影响 | 较小 |
| 公式 | 线性插值 | `Offset *= (1 - dt/SmoothTime)` |
| 适合 | 一般场景 | **FPS游戏** |

### Simulate 预测
- 使用当前速度方向做本地预测
- 延迟从 "RTT + 平滑周期" 降为**仅 1 RTT**

## 6.7 流量优化：FVector_NetQuantize

| 类型 | 精度 | 范围 | 用途 |
|---|---|---|---|
| `NetQuantize10` | 0.1 | ±1,677,721.6 | 加速度 |
| `NetQuantize100` | 0.01 | ±167,772.16 | 位置 |

**序列化流程**：FVector × Scale → float→int → 计算 Bits → 加 Bias → 写入 Bit 流

**示例**：(-1.0, 5.1, 100.0) × 10 → **41 bits**（原 128 bits 的 32%）

---

# 七、UE4 移动组件详解（原理分析）

> **原文**：https://zhuanlan.zhihu.com/p/34257208（Jerish）

## 7.1 UpdateComponent 机制

- 移动本质 = 修改 `UpdatedComponent` 的坐标
- 默认胶囊体，可自定义（必须继承 `USceneComponent`）
- `DefaultPawn` → SphereComponent，`AWheeledVehicle` → Mesh

## 7.2 Walking 状态（最复杂）

### 地面检测 FindFloor
- 胶囊体 **Sweep 检测**（非射线）
- 只检测脚下，忽略腰部
- `PerchRadiusThreshold > 0.15` 时额外判断站立空间

### PhysWalking 流程
```
一帧分 N 段（每段 ≤ MaxSimulationTimeStep）
→ 计算加速度 → CalcVelocity()（摩擦/水中阻力）
→ 斜面：ComputeGroundMovementDelta()
→ SafeMoveUpdatedComponent()
→ 碰撞处理：
   ├── 可行走斜面 → 沿斜面移动
   ├── 可踩上去 → StepUp()
   └── 不可踩 → HandleImpact() + SlideAlongSurface()
```

## 7.3 Falling 状态

- 水平速度受玩家输入影响（空中可微调）
- 重力从 `PhysicsVolume` 获取 → **可实现不同区域不同重力**
- 位移：`0.5 × (OldVelocity + Velocity) × timeTick`
- 碰撞：落地 / 平台边缘 / 墙面（三种处理）

### Jump 流程
```
按键 → bPressedJump = true
→ CheckJumpInput() → CanJump() → DoJump()
→ JumpCurrentCount++（支持多段跳 JumpMaxCount）
→ PerformMovement 后 ClearJumpInput
```

## 7.4 Swimming 状态

| 特性 | 实现 |
|---|---|
| 检测 | UpdateComponent 所在 Volume 是否 WaterVolume |
| 浮力 | `Buoyancy × ImmersionDepth(0~1)` |
| 变慢 | FluidFriction 传入 CalcVelocity |
| 惯性 | 摩擦力 0.15（地面8）+ 刹车速度0（地面2048） |
| 出水 | Swim() 强制调整到水面位置 |

## 7.5 Flying 状态
- 最简单，有惯性
- GM cheatFlying 特殊处理：加速度=0 时强制速度=0

## 7.6 FScopedMovementUpdate 优化
- 一帧内多次移动，中间过程无意义
- 物理/子组件更新延迟到作用域析构时一次性处理

## 7.7 同步方案 — Simulate 角色详细流程

### 同步属性
```cpp
ReplicatedMovement        // 位置/旋转/速度（SimulatedOnly）
ReplicatedMovementMode    // 移动模式
ReplicatedBasedMovement   // 脚下 Component
ReplicatedServerLastTransformUpdateTimeStamp  // 服务器最后更新时间
bIsCrouched               // 蹲伏状态
```

### SimulateTick 14 步流程
```
1. Role == SimulatedProxy → SimulatedTick
2. 回放模式 → 直接 SmoothClientPosition
3. RootMotion 检查
4. FScopedPreventAttachedComponentMove 锁定 Mesh
5. 前置检查（物理/静止/Base 未解析）
6. FScopedMovementUpdate 限制重复更新
7. bNetworkUpdateReceived → Mode变化/Teleport
8. 读取 LinearVelocity
9. 更新 BasedMovement
10. MoveSmooth()（Walking→MoveAlongFloor / 其他→Safe移动）
11. 地面检测 → 可能 Falling
12. 回调 OnMovementUpdated
13. 更新 Mesh 位置
14. SmoothClientPosition（Interpolate + UpdateVisuals）
```

### 核心设计：胶囊体与 Mesh 分离
```
胶囊体（不可见）→ 立即更新到新位置
Mesh（可见）→ MeshTranslationOffset / MeshRotationOffset 插值平滑
```
- 使用 `FScopedPreventAttachedComponentMove` 阻止 Mesh 跟随
- 平滑本质 = 更新 Mesh **相对胶囊体的偏移**

### SmoothCorrection 流程
```
1. 计算 NewToOldVector
2. 距离检查（> MaxSmoothNetUpdateDist → 不平滑）
3. MeshTranslationOffset += NewToOldVector
4. 线性模式：记录 Original 偏移，设胶囊体到新位置
5. 指数模式：记录 Rotation 差异，同时设 Location + Rotation
6. 时间戳处理：计算 LastCorrectionDelta
```

## 7.8 物理托管移动
- `SimulatePhysics` 开启 → 移动组件失效
- 移动数据从 `BodyInstance` 获取
- 常见应用：**布娃娃系统**

## 7.9 特殊移动模式实现思路

| 模式 | 基于 | 核心思路 |
|---|---|---|
| **多段跳** | Falling | `JumpMaxCount` + 上升阶段判断 + 每段不同速度 |
| **喷气背包** | Falling | 重写 `NewFallVelocity` → 推力+重力 → 燃料控制 |
| **爬墙** | 自定义/Flying | 障碍检测→自定义加速度/速度→旋转胶囊体 |
| **爬梯子** | 自定义/Flying | Attach到梯子→Z轴速度→高摩擦停止→RootMotion+IK |

---

# 八、动画技术参考链接库

> **来源**：xiaxia9/Game-Development-Notes「动画技术文章合集」

## 8.1 ALS（Advanced Locomotion System）

行走、奔跑、原地转身、急停转向、IK、Ragdoll

- [UE4高级运动系统V3插件分析](https://zhuanlan.zhihu.com/p/101477611)
- [ALS V3动画表现设计](https://zhuanlan.zhihu.com/p/117405363)
- [ALS V4学习笔记 01-04](https://zhuanlan.zhihu.com/p/159646345)
- [AdvancedLocomotionV4笔记 1-5](https://zhuanlan.zhihu.com/p/141266454)

## 8.2 Motion Matching

2016 GDC 育碧提出，3A 标配。UE5 通过 MotionTrajectory + PoseSearch 插件实现。

- [网易雷火：Motion Matching 技术介绍和实现](https://zhuanlan.zhihu.com/p/136971426)
- [《荣耀战魂》中的 MM](https://zhuanlan.zhihu.com/p/401890149)
- [《最后生还者2》中的 MM](https://zhuanlan.zhihu.com/p/403923793)
- [《Control》中的 MM](https://zhuanlan.zhihu.com/p/405873194)
- [《黑神话：悟空》的 MM（B站）](https://www.bilibili.com/video/BV1GK4y1S7Zw)
- [GDC 2016: Motion Matching and The Road to Next-Gen Animation](https://www.gdcvault.com/play/1023280/Motion-Matching-and-The-Road)
- [Learned Motion Matching（育碧蒙特利尔）](https://montreal.ubisoft.com/en/learned-motion-matching-2/)

## 8.3 Motion Warping
- [UE5 Motion Warping 原理剖析及 UE4 适配](https://zhuanlan.zhihu.com/p/378948277)
- [官方文档](https://docs.unrealengine.com/5.0/en-US/AnimationFeatures/MotionWarping/)

## 8.4 Distance Matching & Speed Warping
- [UE4 Locomotion 距离适配](https://zhuanlan.zhihu.com/p/357650112)
- [UE4 Locomotion 速度适配](https://zhuanlan.zhihu.com/p/363656339)
- [Paragon 动画技术 Feature Highlight](https://www.youtube.com/watch?v=1UOY-FMm-xo)

## 8.5 IK（逆向运动学）

算法：Two Bone IK、CCDIK、FABRIK、FullBody IK

- [【游戏开发】逆向运动学详解（45KB完整文档）](https://zhuanlan.zhihu.com/p/499405167)
- [UE4 蓝图实现脚部 IK](https://zhuanlan.zhihu.com/p/84399021)
- [UE4 C++ IK 实现方法](https://zhuanlan.zhihu.com/p/42140318)
- [UE5 FullBody IK 学习](https://zhuanlan.zhihu.com/p/381469750)
- [Control Rig + FullBody IK 足部 IK](https://zhuanlan.zhihu.com/p/412251528)
- [UE4 C++ 手部扶墙 IK](https://zhuanlan.zhihu.com/p/335357497)
- [GDC 2016: Biomechanical Approach to Foot IK](https://www.gdcvault.com/play/1023316/)
- [IK Survey (PDF)](http://www.andreasaristidou.com/publications/papers/IK_survey.pdf)

## 8.6 Control Rig
- [官方文档](https://docs.unrealengine.com/5.0/en-US/AnimationFeatures/ControlRig/)
- [介绍 Control Rig 方案](https://zhuanlan.zhihu.com/p/353524801)

## 8.7 骨骼动画
- [骨骼动画技术原理](https://zhuanlan.zhihu.com/p/431446337)
- [Skinned Mesh 原理解析](https://happyfire.blog.csdn.net/article/details/3105872)
- [【UE4】图解动画系统源码](https://zhuanlan.zhihu.com/p/446851284)

## 8.8 物理动画 & AI
- [GDC 2020: Ragdoll Motion Matching](https://www.youtube.com/watch?v=JZKaqQKcAnw)
- [GDC 2020: Physical Animation in Star Wars](https://www.youtube.com/watch?v=TmAU8aPekEo)
- [GDC 2017: Physics Animation in Uncharted 4](https://www.youtube.com/watch?v=7S-_vuoKgR4)

## 8.9 GDC 精选演讲

| 年份 | 演讲 | 关键词 |
|---|---|---|
| 2018 | Gears of War Inertialization | 高性能动画转换 |
| 2018 | Shadow of War Combat Animations | 多角色战斗动作同步 |
| 2018 | Parkour in FPS | 攀爬、移动、跑步 |
| 2017 | Overwatch Animation Pipeline | 动画管道 |
| 2017 | Overwatch FP Animation | 守望先锋 IK + 弹簧 |
| 2017 | DOOM Full Body Animation | AI控制、Delta Correction |
| 2018 | Neural Networks Character Control | 机器学习 + MM |
| 2016 | IK Rig Procedural Pose | 跨角色动画共享 |
| 2015 | Just Cause 3 Movement | 物理系统、高响应 |
| 2021 | Take Control of Animation | Motion Matching 工具集 |

## 8.10 推荐书籍章节

| 书籍 | 章节 |
|---|---|
| 《游戏���擎架构》 | 第11章 动画系统、11.7.1 程序式动画、11.8 压缩 |
| 《游戏编程精粹3》 | 2.5 受限IK、4.7-4.9 骨骼运动 |
| 《游戏编程精粹4》 | 2.6 雅可比转置IK、5.13 动捕压缩、5.14 骨骼碰撞 |
| 《游戏编程精粹5》 | 2.3 样条时间控制、4.5 布娃娃 |
| 《游戏编程精粹8》 | 2.2 Curved Paths、2.3 NCF IK |
| 《GPU精粹1》 | 第4章 动画、6.2 动画精灵 |
| 《GPU精粹3》 | 第2章 群体动画渲染 |

---

# 九、补充资料索引

## 9.1 Repo 中可读取��完整文档

| 文档 | 路径 | 大小 |
|---|---|---|
| 游戏开发中的 DataNormalization | `DataNormalization/游戏开发中的DataNormalization.md` | 8.5KB |
| 【UE4】GAS技能系统介绍（一） | `UE/【UE4】GAS技能系统介绍/` | 6.6KB + PDF + 流程图 |
| 【UE4】一图看懂移动同步（上） | `UE/【UE4】一图看懂移动同步（上）/` | 5KB + PDF + PNG + Visio |
| 【UE4】一图看懂移动同步（下） | `UE/【UE4】一图看懂移动同步（下）/` | 1.4KB + PDF + PNG + Visio |
| 【UE4】图解动画系统源码 | `UE/【UE4】图解动画系统源码/` | 含源码分析 |
| 【UE5】物理系统源码分析 | `UE/【UE5】物理系统源码分析/` | Chaos 物理引擎 |
| 【UE5】基于物理的角色动画（基础篇） | `UE/` | 物理驱动动画 |
| 【UE5】基于物理的角色动画（应用篇）PhysicsControlComponent | `UE/` | 应用实践 |
| 图解 Chaos 源码 | `UE/图解Chaos源码.pdf` | 557KB |
| 【游戏开发】逆向运动学（IK）详解 | `逆向运动学（IK）详解/` | **45KB 大文件** |
| 【记录】游戏服务器开发知识 | `服务器/` | 服务器开发笔记 |

## 9.2 知乎补充文档链接

| 文档 | 链接 | 主题 |
|---|---|---|
| 一图看懂移动同步（上） | https://zhuanlan.zhihu.com/p/432303699 | 客户端驱动+预测修正流程图 |
| 一图看懂移动同步（下） | https://zhuanlan.zhihu.com/p/434494875 | 服务器校验+Reconcile+性能优化 |
| UE5 物理系统源码分析 | https://zhuanlan.zhihu.com/p/580817139 | Chaos/碰撞检测/物理世界Tick |
| UE GameFeatures 学习 | https://zhuanlan.zhihu.com/p/696773991 | 动态加载模块/DLC/热更 |
| LevelSequence 对象绑定分析 | https://zhuanlan.zhihu.com/p/548155341 | 持久绑定 vs 临时绑定 |

## 9.3 主文档原文链接

| # | 文档 | 链接 |
|---|---|---|
| 1 | GAS 性能优化 | https://zhuanlan.zhihu.com/p/12646608505 |
| 2 | UAssetManager | https://zhuanlan.zhihu.com/p/685072527 |
| 3 | ORCA 避障 | https://zhuanlan.zhihu.com/p/1919204895858340269 |
| 4 | LevelSequence 源码 | https://zhuanlan.zhihu.com/p/544571505 |
| 5 | Motion Matching | https://zhuanlan.zhihu.com/p/2011816308874962280 |
| 6 | 移动网络同步 | https://zhuanlan.zhihu.com/p/114341957 |
| 7 | 移动组件详解 | https://zhuanlan.zhihu.com/p/34257208 |

---

# 附录：技术栈关联图

```
┌─────────────────────────────────────────────────────────┐
│                游戏战斗/动画系统技术栈                      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ① GAS性能优化 ←───资源管理───→ ② UAssetManager          │
│       ↕ 战斗框架                    ↕ 异步加载            │
│  ④ LevelSequence ←──对象绑定──→ 补5 对象绑定分析          │
│       ↕ 过场动画                                        │
│  ⑤ Motion Matching ←──动画驱动──→ Motion Warping         │
│       ↕ Pose Search                  ↕ Distance Match   │
│  ⑥ 移动网络同步 ←────互补────→ ⑦ 移动组件详解             │
│       ↕ 同步机制深度            ↕ 移动物理深度            │
│    补1+补2 一图看懂          特殊移动(爬墙/背包)          │
│       ↕                                                │
│    补3 UE5物理系统 ←→ Chaos源码 ←→ 物理角色动画           │
│                                                         │
│  ③ ORCA避障 ←── NPC大规模移动 ──→ ⑥⑦ 移动系统            │
│                                                         │
│  补4 GameFeatures ←── 模块化架构/热更                     │
│                                                         │
│  IK详解(45KB) ←── 动画后处理 ──→ ALS / FootIK / 手部IK   │
│                                                         │
│  DataNormalization ←── 数据标准化 ��─→ ⑤ PSN归一化         │
│                                                         │
│  服务器开发知识 ←── 后端架构                               │
└─────────────────────────────────────────────────────────┘
```

---

> **文档结束**
>
> 本文档整合了 7 篇核心技术文章 + 1 个 GitHub Repo（含 11+ 子文档）+ 100+ 参考链接的精华内容。
> 建议配合原文链接深入阅读具体实现细节。
>
> Repo 地址：https://github.com/xiaxia9/Game-Development-Notes