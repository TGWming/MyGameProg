# xiaxia9 技术资产与我的项目关联性分析

> 项目类型：单机类魂/恐怖生存 | UE5 C++ | 独立开发 | PS5/Steam
> 参考标杆：血源诅咒 + 黑魂3 连招体系
> 关卡设计：箱庭式一本道 + 分支收集/支线/捷径
> 生成日期：2026-03-17

---

## 一、总览：哪些技术资产与我的项目直接相关

### 1.1 资产来源全景

| 来源 | 类型 | 与我项目的关联度 |
|---|---|---|
| Game-Development-Notes（笔记仓库） | UE5 系统理论 + 实践笔记 | ⭐⭐⭐⭐⭐ 核心参考 |
| AnimationProject（ALS C++ 重写） | 完整角色动画系统源码 | ⭐⭐⭐⭐⭐ 直接可用架构 |
| NewLocomotionSystem（UE5 模板） | 基础运动框架 | ⭐⭐⭐ 入门参考 |
| ebook（书籍仓库） | 游戏开发/图形学书单 | ⭐⭐⭐ 学习路径 |
| XiaIM（C++ IM 系统） | 网络编程 | ⭐ 基本无关 |
| blogs（早期博客） | C++ 基础 | ⭐ 基本无关 |

---

## 二、按优先级排序：可取的技术细节

### 🔴 P0 — 立即可用，直接影响项目核心

#### 2.1 角色运动系统架构（AnimationProject + 笔记）

**为什么是 P0：** 类魂游戏的核心体验就是角色操控手感，血源/黑魂的灵魂在于精准的运动反馈。

**可取的技术细节：**

| 技术点 | 来源 | 如何用于你的项目 |
|---|---|---|
| **EGait 三级步态系统** (Walk/Run/Sprint) | AnimationProject | 类魂标配：巡逻Walk、跑图Run、冲刺Sprint（耐力消耗） |
| **EMovementState 状态机** (Grounded/InAir/Mantling/Ragdoll) | AnimationProject | 你需要：Grounded（地面战斗）、InAir（跳跃/坠落）、Ragdoll（死亡/击飞）。**Mantling 可以去掉或简化**——血源/黑魂没有攀爬 |
| **ERotationMode** (VelocityDirection/LookingDirection/Aiming) | AnimationProject | 类魂核心：非锁定=VelocityDirection，锁定目标=LookingDirection。**Aiming 模式可以忽略**（你不做TPS射击） |
| **FMovementSettings 数据表驱动** | AnimationProject | 直接可用：不同状态下速度/旋转速率用 DataTable 配置，不硬编码 |
| **SmoothCharacterRotation + CalculateGroundedRotationRate** | AnimationProject | 类魂的锁定/非锁定旋转手感全靠这个 |
| **接口驱动解耦** (ICharacterInterface/IAnimationInterface) | AnimationProject | 你的角色类会很大，用接口解耦角色↔动画↔相机是必要的架构决策 |

**你需要修改/新增的：**
- 新增 **Lock-On Target System**（锁定系统）——AnimationProject 没有，但 RotationMode 切换逻辑可以复用
- 新增 **Stamina System**（耐力系统）——绑定到 Sprint/Roll/Attack
- 新增 **Dodge/Roll 作为核心动作**——不是 AnimationProject 里的可选翻滚，而是类魂的核心闪避机制

---

#### 2.2 动画实例核心流程（AnimationProject UAnimInstanceBase）

**为什么是 P0：** 动画手感 = 游戏手感。

**可取的技术细节：**

| 技术点 | 关联度 | 说明 |
|---|---|---|
| **NativeUpdateAnimation 主循环结构** | ⭐⭐⭐⭐⭐ | UpdateCharacterInfo → UpdateAimingValues → UpdateMovementValues → UpdateRotationValues 这个流程直接可用 |
| **VelocityBlend (F/B/L/R)** | ⭐⭐⭐⭐⭐ | 锁定状态下的四方向混合移动——血源/黑魂标配 |
| **StrideBlend + WalkRunBlend + PlayRate** | ⭐⭐⭐⭐⭐ | 步幅匹配速度，防止滑步，类魂精确操控的基础 |
| **ShouldMoveCheck 移动/静止判断** | ⭐⭐⭐⭐⭐ | 决定播放移动动画还是Idle，含速度阈值判断 |
| **TurnInPlaceCheck (8组转身资产)** | ⭐⭐⭐⭐ | Idle 状态下原地转身——血源/黑魂都有，非锁定时必要 |
| **FootLocking + FootOffsets + PelvisIK** | ⭐⭐⭐⭐ | 脚部 IK 防滑步、适应地形——类魂战斗中脚步精确度影响打击感 |
| **DynamicTransitionCheck** | ⭐⭐⭐ | 动态过渡，可以后期打磨时加入 |
| **AdditiveLeaning (LeanAmount)** | ⭐⭐⭐ | 移动时身体倾斜——增加运动质感，非必须但好看 |

**可以忽略/简化的：**

| 技术点 | 原因 |
|---|---|
| **RotateInPlaceCheck** | 这是 ALS Aiming 模式专用，你不做TPS瞄准 |
| **InputYaw偏移计算** | 与 ALS 的第三人称射击瞄准相关，类魂不需要 |
| **UpdateLayerValues (身体部位权重)** | ALS 用于分层叠加（上半身独立瞄准），类魂用全身动画，简化处理 |

---

#### 2.3 GAS 战斗系统框架（笔记）

**为什么是 P0：** 类魂的连招/受击/状态效果系统是 GAS 的最佳应用场景。

**可取的技术细节：**

| 技术点 | 优先级 | 说明 |
|---|---|---|
| **AbilitySystemComponent 挂载在 Character 上** | ⭐⭐⭐⭐⭐ | 单机游戏不需要挂 PlayerState，直接挂 Character 更简单 |
| **GameplayAbility → 攻击/翻滚/使用道具** | ⭐⭐⭐⭐⭐ | 每个战斗动作都是一个 GA |
| **GameplayEffect → 伤害/耐力消耗/Buff/Debuff** | ⭐⭐⭐⭐⭐ | 毒/流血/体力恢复/增益药水全用 GE |
| **GameplayTag 状态标记** | ⭐⭐⭐⭐⭐ | State.Dead / State.Staggered / State.Invincible 等 |
| **AttributeSet → HP/Stamina/Poise** | ⭐⭐⭐⭐⭐ | 类魂核心属性 |
| **AbilityTask → WaitGameplayEvent / PlayMontageAndWait** | ⭐⭐⭐⭐⭐ | 连招的 Montage ��动 + 事件触发 |

---

### 🟡 P1 — 重要但可以第二阶段实现

#### 2.4 布娃娃系统（AnimationProject）

| 技术点 | 说明 |
|---|---|
| **RagdollStart/Update/End 完整流程** | 死亡时切换布娃娃 → 经典类魂表现 |
| **SetActorLocationDuringRagdoll** | 防止布娃娃穿墙 |
| **从布娃娃恢复 (GetGetUpAnimation)** | 如果你有"起身"机制（血源没有但一些游戏有） |

**建议：** 可以先用简单死亡动画，后期再加布娃娃物理。

#### 2.5 相机系统（笔记 + AnimationProject ICameraInterface）

| 技术点 | 说明 |
|---|---|
| **FCameraSettings (TP_Fov/FP_Fov/BoomLength)** | 类魂需要可配置的第三人称相机距离 |
| **碰撞拉近 (bDoCollisionTest)** | 墙角战斗时相机不穿墙 |
| **锁定目标时相机行为** | 需要自己实现：相机平滑看向锁定目标 + 角色面朝目标 |
| **战斗触发相机 FOV 变化** | 处决/特殊攻击时拉近 → 增强打击感 |

#### 2.6 AI 感知系统（笔记）

| 技术点 | 说明 |
|---|---|
| **AIPerceptionComponent** | 敌人视觉/听觉感知 → 恐怖生存的核心 |
| **行为树 (Behavior Tree)** | 类魂怪物 AI：巡逻→发现→追击→攻击→回退 |
| **EQS (Environment Query System)** | 较复杂 AI 的位置选择，Boss 战可能用到 |

**恐怖生存特化：** 比普通类魂更重视听觉感知（脚步/开门声触发敌人）+ 视线检测（手电筒/火把照射范围）。

#### 2.7 物理动画插件概念（AnimationProject PhysicsAnimationTools）

| 说明 |
|---|
| 插件本身代码量不大，但"物理动画"概念对类魂打击感极其重要 |
| 受击时的物理反馈（Hit Reaction）、武器碰撞物理 |
| 建议关注这个方向但不直接用他的插件，自己按需实现 |

---

### 🟢 P2 — 可选，打磨阶段加入

#### 2.8 OverlayState 系统（AnimationProject）

| 原设计 | 你的项目适配 |
|---|---|
| 13 种 OverlayState (Default/Rifle/Pistol/Bow/Torch...) | 你只需要 3-5 种：Default / OneHandSword / TwoHandSword / Torch / Item |
| AttachToHand + ClearHeldObject | 武器切换时的挂载逻辑直接可用 |
| UpdateHeldObject 每帧更新 | 简化为事件驱动（切换武器时才更新） |

#### 2.9 地形适应 & 高级 IK（AnimationProject）

| 技术点 | 说明 |
|---|---|
| 坡面/台阶脚部适应 | 箱庭设计中地形变化不会太极端，P2 优先级 |
| 骨盆 IK 调整 | 美观细节，后期打磨 |

#### 2.10 渲染 & 图形学（笔记 + ebook）

| 技术点 | 说明 |
|---|---|
| Nanite/Lumen 使用策略 | 恐怖游戏 Lumen 全局光照 + 动态阴影非常重要，但需要性能预算 |
| 后处理 (Post-Processing) | 恐怖氛围：暗角/色调/雾/颗粒 |
| Shader/材质 | ebook 中 Unity Shader 书不适用 UE，但图形学原理通用 |

---

### ⚫ 可以忽视的技术细节

| 技术内容 | 忽视原因 |
|---|---|
| **XiaIM 即时通讯系统** | 单机游戏，完全无关 |
| **blogs 早期博客** | C++ 左右值引用/虚函数是基础知识，不需要参考 |
| **Mantling 攀爬系统** | 血源/黑魂没有攀爬，你不需要。如果后期加翻越低矮障碍可以再回头看 |
| **ALS Aiming 模式全套** | TPS 射击瞄准体系，类魂不需要 |
| **AnimationProject 的 Overlay: Rifle/Pistol1H/Pistol2H/Binoculars/Box/Barrel** | 射击类/工具类持物，你不需要 |
| **ebook 中 Android/iOS/Python/XMPP 书籍** | 与游戏开发无关 |
| **NewLocomotionSystem 大部分代码** | 标准模板，AnimationProject 已经完全包含并超越 |
| **HipsDirection 六方向系统** | ALS 的高精度方向标记，类魂用四方向（前后左右）足够 |
| **AnimFeatureExample (StrideBlending/AdditiveLeaning/SprintImpulse)** | 这些是 ALS demo 切换开关，你直接选择要/不要即可 |

---

## 三、技术架构优先级路线图

### Phase 1 — 核心可玩原型（0~3个月）

```
[角色 C++ 基础类]
├── CharacterBase (参考 AnimationProject 架构，精简到类魂需求)
│   ├── EGait: Walk/Run/Sprint (绑定耐力)
│   ├── EMovementState: Grounded/InAir/Ragdoll (去掉 Mantling)
│   ├── ERotationMode: VelocityDirection/LookingDirection (去掉 Aiming)
│   └── Lock-On System (新建，AnimationProject 无此功能)
│
├── AnimInstanceBase (参考 AnimationProject 流程)
│   ├── VelocityBlend 四方向混合
│   ├── StrideBlend + PlayRate
│   └── ShouldMoveCheck
│
├── GAS 战斗框架
│   ├── GA_Attack (连招: Montage序列)
│   ├── GA_Dodge (翻滚: i-frame 无敌帧)
│   ├── GE_Damage / GE_StaminaCost
│   └── AttributeSet: HP/Stamina/Poise
│
└── 基础相机 (CameraBoom + FollowCamera + 锁定目标)
```

### Phase 2 — 完整战斗 + 关卡原型（3~6个月）

```
[战斗完善]
├── Hit Reaction 系统 (物理动画方向)
├── Poise/Stagger 韧性系统
├── 连招分支 (轻攻/重攻/蓄力)
│
[动��打磨]
├── TurnInPlace (原地转身)
├── FootIK (脚部锁定 + 地面适应)
│
[AI 系统]
├── 敌人 AI 行为树
├── AIPerception (视觉 + 听觉 → 恐怖要素)
│
[关卡]
├── 箱庭关卡白盒
├── Level Streaming (参考笔记 World Partition)
└── 捷径/分支路线设计
```

### Phase 3 — 打磨 + 平台适配（6~12个月）

```
[视觉]
├── Lumen 全局光照 (恐怖氛围)
├── 后处理管线
├── 布娃娃死亡
│
[性能]
├── 低配优化 (Scalability Settings)
├── PS5 适配 (DualSense 触觉反馈)
│
[内容]
├── OverlayState 武器切换动画
├── 收集品/支线任务系统
└── Boss AI + 特殊相机演出
```

---

## 四、从 AnimationProject 到类魂项目的关键差异

| 维度 | AnimationProject (ALS) | 你的类魂项目 | 差异处理 |
|---|---|---|---|
| **核心循环** | 运动 → 开放世界探索 | 运动 → 战斗 → 探索 | 需要新增完整战斗层 |
| **状态复杂度** | 13种OverlayState | 3-5种武器持握 | 大幅精简 |
| **旋转模式** | 3种(含Aiming) | 2种(自由/锁定) | 去掉Aiming，新增锁定逻辑 |
| **攀爬** | 完整Mantling系统 | 无需 | 直接删除 |
| **战斗** | 无 | 核心系统 | 完全新建(GAS) |
| **耐力** | 无 | Sprint/Roll/Attack 消耗 | 完全新建 |
| **AI** | 无 | 行为树+感知 | 完全新建 |
| **恐怖要素** | 无 | 声音/光照/AI感知 | 完全新建 |

---

## 五、ebook 书单中对你项目有用的书籍

### 🎯 直接相关（建议阅读）

| 书名 | 用途 |
|---|---|
| 游戏引擎架构（第2版） | 理解 UE5 架构的底层原理 |
| Real-Time Rendering (3rd/4th) | 光照/阴影/渲染管线原理 → 恐怖视觉 |
| 3D数学基础 图形与游戏开发 | 向量/矩阵/四元数 → 运动/相机数学 |
| Game Programming Gems 系列 | 各种游戏编程技巧合集 |

### 📚 有参考价值

| 书名 | 用途 |
|---|---|
| Effective C++ / More Effective C++ | C++ 编码质量 |
| C++ 性能优化指南 | 性能敏感代码优化 |
| 深度探索 C++ 对象模型 | 理解 UE5 的 C++ 底层 |
| GPU Gems / GPU Pro 系列 | 高级渲染技术参考 |
| 计算机图形学导论 | 图形学基础 |

### ❌ 无需关注

| 类别 | 原因 |
|---|---|
| Android/iOS 开发书籍 | 不是你的目标平台 |
| Python 书籍 | 不是你的开发语言 |
| Go 书籍 | 不是你的开发语言 |
| 逆向工程书籍 | 与游戏开发无关 |
| 运维书籍 | 单机游戏无需 |
| WebGL 书籍 | 不是你的渲染管线 |
| Unity Shader 书籍 | Unity 专用，UE5 用 HLSL/Material Editor |

---

## 六、核心结论

> **AnimationProject 是你最大的技术宝藏。** 它的角色运动架构（状态机 + 接口解耦 + 数据表驱动 + 动画实例流程）可以直接作为你类魂项目的骨架，但你需要：
>
> 1. **砍掉** 30%（Mantling/Aiming/射击类Overlay）
> 2. **新建** 40%（GAS战斗/锁定系统/耐力系统/AI/恐怖机制）
> 3. **保留** 30%（运动状态机/动画混合/IK/布娃娃/相机/接口架构）
>
> 这个 70% 需要改动的比例是正常的——因为 ALS 是一个"通用角色运动系统"，而你需要的是一个"战斗驱动的角色系统"。ALS 给你的不是成品，而是**架构范式和工程模式**。