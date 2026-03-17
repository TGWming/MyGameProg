# 类魂游戏相机系统 - 设计师使用手册
## Souls-like Camera System Designer Guide

版本：1.0
最后更新：2026-01-01

---

# 目录

1. [系统概述](#1-系统概述)
2. [快速开始](#2-快速开始)
3. [相机状态配置](#3-相机状态配置)
4. [模块参数详解](#4-模块参数详解)
5. [修改器使用指南](#5-修改器使用指南)
6. [碰撞系统配置](#6-碰撞系统配置)
7. [预设与风格](#7-预设与风格)
8. [调试工具](#8-调试工具)
9. [常见问题解答](#9-常见问题解答)
10. [参数速查表](#10-参数速查表)

---

# 1. 系统概述

## 1.1 什么是类魂相机系统？

本相机系统专为类魂（Souls-like）游戏设计，提供：

- **8种相机状态**：探索、战斗、Boss锁定、对话、过场、瞄准、骑乘、游泳
- **39个计算模块**：位置、旋转、距离、FOV、偏移、约束
- **26个效果修改器**：震动、缩放、反应、过场、效果
- **20个碰撞策略**：检测、响应、遮挡、恢复、特殊场景

## 1.2 核心概念

```
┌─────────────────────────────────────────────────────────────┐
│                     相机处理流程                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  玩家输入 → 状态判断 → 模块计算 → 修改器应用 → 碰撞处理 → 输出 │
│                                                             │
│  Stage1    Stage2    Stage3-5    Stage7      Stage6    Stage8│
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 1.3 设计师可控制的内容

| 类别 | 可调整内容 | 调整方式 |
|------|-----------|----------|
| 状态配置 | 各状态的相机参数 | DataTable / 蓝图 |
| 模块参数 | 位置、旋转、距离等计算参数 | DataTable / 蓝图 |
| 修改器 | 震动、特效的触发和强度 | 蓝图调用 |
| 碰撞 | 碰撞响应方式和恢复速度 | DataTable / 蓝图 |
| 预设 | 游戏风格（黑魂/血源/只狼/法环） | 预设选择 |

---

# 2. 快速开始

## 2.1 在蓝图中获取相机管理器

```
方法1：通过BlueprintLibrary
────────────────────────
Get Souls Camera Manager
  ├─ 输入: World Context Object
  └─ 输出:  Souls Camera Manager (引用)

方法2：从玩家Pawn获取
────────────────────────
Get Component by Class
  ├─ 输入: Souls Camera Manager Class
  └─ 输出:  Souls Camera Manager Component
```

## 2.2 常用蓝图节点

### 状态切换
```
Request Camera State Change
  ├─ 输入:  New State (枚举)
  ├─ 输入: Blend Time (过渡时间，秒)
  └─ 输出: 是否成功

示例：
  切换到战斗状态，0.3秒过渡
  Request Camera State Change(Combat, 0.3)
```

### 目标锁定
```
Set Lock On Target
  ├─ 输入: Target Actor (目标Actor)
  └─ 输出: 是否成功

Clear Lock On Target
  └─ 清除当前锁定目标

Switch Lock On Target
  ├─ 输入: Direction (左/右)
  └─ 输出: 新目标Actor
```

### 触发效果
```
Trigger Camera Shake
  ├─ 输入: Intensity (0. 0-2.0)
  ├─ 输入: Duration (秒)
  └─ 输入: Shake Type (类型)

Trigger Hit Reaction
  ├─ 输入:  Direction (方向向量)
  ├─ 输入: Intensity (强度)
  └─ 效果: 相机向受击方向偏移

Trigger FOV Pulse
  ├─ 输入:  FOV Change (度数)
  ├─ 输入: Duration (秒)
  └─ 效果: FOV短暂变化后恢复
```

## 2.3 DataTable配置

### 创建状态配置DataTable

1. 内容浏览器 → 右键 → Miscellaneous → Data Table
2. 选择行结构：`FCameraStateConfig`
3. 添加行，每行代表一个相机状态的配置
4. 在SoulsCameraManager中引用此DataTable

---

# 3. 相机状态配置

## 3.1 状态类型说明

| 状态 | 枚举值 | 使用场景 | 典型特征 |
|------|--------|----------|----------|
| **Exploration** | 0 | 正常探索、行走 | 距离远、FOV标准、响应慢 |
| **Combat** | 1 | 进入战斗、有敌人 | 距离近、FOV略窄、响应快 |
| **BossLockOn** | 2 | 锁定Boss | 距离中、FOV宽、包含双目标 |
| **Dialogue** | 3 | NPC对话 | 距离近、FOV窄、固定角度 |
| **Cinematic** | 4 | 过场动画 | 完全可控、无输入响应 |
| **Aiming** | 5 | 弓箭瞄准 | 距离近、FOV窄、高精度 |
| **Mounted** | 6 | 骑乘坐骑 | 距离远、位置高、FOV宽 |
| **Swimming** | 7 | 水中游泳 | 特殊约束、俯仰限制 |

## 3.2 状态基础参数（A组 - StateBase）

### A1: 距离参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **BaseDistance** | float | 100-800 | 相机到角色的基础距离（厘米） |
| **MinDistance** | float | 50-300 | 最小允许距离 |
| **MaxDistance** | float | 300-1000 | 最大允许距离 |

```
距离示意图：
                        MaxDistance
                            ↓
角色●────────────────────────────────────●相机最远
    ├────────────────────●相机（BaseDistance）
    ├────●相机最近
        ↑
    MinDistance
```

**调整建议：**
- 探索状态：400-500cm，给玩家宽广视野
- 战斗状态：300-400cm，更紧凑的战斗感
- Boss战：450-550cm，容纳大型Boss

### A2: FOV参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **BaseFOV** | float | 60-120 | 基础视野角度（度） |
| **MinFOV** | float | 45-80 | 最小FOV |
| **MaxFOV** | float | 90-130 | 最大FOV |

```
FOV对比：
┌─────────────────────────────────────┐
│  窄FOV (60°)        宽FOV (100°)    │
│  ┌───┐              ┌───────┐       │
│  │   │              │       │       │
│  │ 🎯 │              │  🎯    │       │
│  │   │              │       │       │
│  └───┘              └───────┘       │
│  聚焦、紧张          开阔、宏大      │
└─────────────────────────────────────┘
```

**调整建议：**
- 探索：85-90°，标准第三人称视野
- 战斗：80-85°，略窄增加紧张感
- Boss：90-95°，容纳大型敌人
- 瞄准：65-70°，聚焦目标

### A3: 旋转参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **BasePitch** | float | -89 to 89 | 基础俯仰角（度） |
| **MinPitch** | float | -89 to 0 | 最小俯仰角（向下看） |
| **MaxPitch** | float | 0 to 89 | 最大俯仰角（向上看） |
| **BaseYaw** | float | -180 to 180 | 基础偏航角 |

```
俯仰角说明：
        MaxPitch (+60°) 向上看
              ↑
              │
    ─────────●───────── 水平 (0°)
              │
              ↓
        MinPitch (-60°) 向下看
```

**调整建议：**
- 通常MinPitch = -60°，MaxPitch = +60°
- 游泳状态可能需要更大的俯仰范围
- 对话状态可能需要固定角度

### A4: 偏移参数

| 参数名 | 类型 | 说明 |
|--------|------|------|
| **BaseOffset** | FVector | 相机相对于角色的偏移(X,Y,Z) |
| **ShoulderOffset** | float | 肩部横向偏移量（左负右正） |
| **VerticalOffset** | float | 垂直偏移量 |

```
偏移示意图（俯视）：
                    
        ●相机（右肩视角）
       ╱
      ╱ ShoulderOffset = +50
     ╱
角色●────────────────→ 前方
     ╲
      ╲ ShoulderOffset = -50
       ╲
        ●相机（左肩视角）
```

**调整建议：**
- 探索：ShoulderOffset = 50（右肩偏移）
- 瞄准：ShoulderOffset = 80（更明显的肩部视角）
- 对话：ShoulderOffset = 0（居中）

### A5: 延迟/平滑参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **PositionLagSpeed** | float | 1-30 | 位置跟随速度（值越大跟随越快） |
| **RotationLagSpeed** | float | 1-30 | 旋转跟随速度 |
| **DistanceLagSpeed** | float | 1-30 | 距离调整速度 |
| **FOVLagSpeed** | float | 1-30 | FOV调整速度 |

```
延迟效果对比：
低LagSpeed (3):  角色移动 ────→ 相机缓慢跟随（电影感）
高LagSpeed (20): 角色移动 ────→ 相机快速跟随（响应快）
```

**调整建议：**
- 探索：8-12，平滑但不迟钝
- 战斗：15-20，快速响应
- 过场：3-5，电影感跟随

### A6: 过渡参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **BlendInTime** | float | 0.1-2.0 | 进入此状态的过渡时间（秒） |
| **BlendOutTime** | float | 0.1-2.0 | 离开此状态的过渡时间（秒） |
| **BlendCurve** | enum | - | 过渡曲线类型 |

**过渡曲线类型：**
- Linear：线性，匀速过渡
- EaseIn：缓入，慢启动
- EaseOut：缓出，慢结束
- EaseInOut：缓入缓出，平滑过渡（推荐）

### A7: 碰撞参数

| 参数名 | 类型 | 说明 |
|--------|------|------|
| **bEnableCollision** | bool | 是否启用碰撞检测 |
| **CollisionRadius** | float | 碰撞检测球体半径 |
| **CollisionChannel** | enum | 碰撞检测通道 |

**调整建议：**
- 通常所有状态都启用碰撞
- 过场状态可能禁用（由动画完全控制）
- CollisionRadius通常12-20cm

---

# 4. 模块参数详解

## 4.1 模块参数概述（B组）

模块参数控制39个计算模块的具体行为。

### B1: 位置模块参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **PredictionTime** | float | 0-1 | 预测时间（秒），用于预测性跟随 |
| **OrbitRadiusMultiplier** | float | 0.5-2.0 | 锁定时的轨道半径倍数 |
| **MidPointBias** | float | 0-1 | 双目标中点偏向（0=玩家，1=目标） |
| **FixedPointLocation** | FVector | - | 固定点位置（过场用） |
| **bEnablePositionSmoothing** | bool | - | 启用位置平滑 |
| **PositionSmoothingStrength** | float | 0-1 | 平滑强度 |

**PredictionTime 说明：**
```
PredictionTime = 0:    相机跟随角色当前位置
PredictionTime = 0.2:  相机预测角色0.2秒后的位置

效果：角色快速移动时，相机会"领先"一点，
      减少拖拽感，更适合快节奏战斗
```

**MidPointBias 说明（Boss战重要）：**
```
MidPointBias = 0:   焦点完全在玩家
MidPointBias = 0.5: 焦点在玩家和Boss中间
MidPointBias = 1:   焦点完全在Boss

推荐Boss战设置：0.3-0.4（略偏向玩家）
```

### B2: 旋转模块参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **InputSensitivity** | float | 0.1-5.0 | 输入灵敏度倍数 |
| **SoftLockStrength** | float | 0-1 | 软锁定强度（0=自由，1=硬锁定） |
| **AutoOrientSpeed** | float | 0-10 | 自动朝向速度 |
| **AutoOrientMinSpeed** | float | 0-500 | 触发自动朝向的最小移动速度 |
| **LookAtInterpSpeed** | float | 0-20 | 看向目标的插值速度 |
| **bEnableRotationLag** | bool | - | 启用旋转延迟 |

**SoftLockStrength 说明：**
```
SoftLockStrength = 0:   完全自由控制，锁定目标不影响旋转
SoftLockStrength = 0.7: 相机会被轻柔地拉向目标
SoftLockStrength = 1:   硬锁定，相机始终朝向目标

推荐：战斗用0.6-0.8，Boss战用0.8-0.9
```

### B3: 距离模块参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **TargetSizeThreshold** | float | 50-1000 | 目标尺寸阈值（cm） |
| **TargetSizeDistanceScale** | float | 0.5-3.0 | 尺寸对距离的缩放系数 |
| **SpeedDistanceThreshold** | float | 0-1000 | 速度距离阈值（cm/s） |
| **SpeedDistanceMax** | float | 0-500 | 速度引起的最大距离增加 |
| **CombatDistanceOffset** | float | -200 to 200 | 战斗距离偏移 |
| **BossPhaseDistanceScale** | float | 0.5-2.0 | Boss阶段距离缩放 |

**TargetSizeDistanceScale 说明：**
```
当锁定目标时，根据目标尺寸自动调整距离：
- 小型敌人（100cm）：距离不变
- 中型Boss（300cm）：距离增加
- 巨型Boss（800cm）：距离大幅增加

公式：实际距离 = 基础距离 × (1 + (目标尺寸/阈值) × 缩放系数)
```

### B4: FOV模块参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **SpeedFOVIncrease** | float | 0-30 | 高速移动时FOV增加量 |
| **SpeedFOVThreshold** | float | 0-1000 | 触发速度FOV的速度阈值 |
| **AimFOVReduction** | float | 0-30 | 瞄准时FOV减少量 |
| **CombatFOVOffset** | float | -20 to 20 | 战斗FOV偏移 |
| **BossFOVOffset** | float | -20 to 20 | Boss战FOV偏移 |
| **ImpactFOVAmount** | float | 0-30 | 受击FOV冲击量 |
| **ImpactDuration** | float | 0.05-1.0 | 受击FOV冲击持续时间 |

**速度FOV效果：**
```
低速（<阈值）：     FOV = BaseFOV
高速（>阈值）：     FOV = BaseFOV + SpeedFOVIncrease × 速度比例

效果：冲刺时视野变宽，增强速度感
推荐：SpeedFOVIncrease = 10-15，Threshold = 400
```

### B5: 偏移模块参数

| 参数名 | 类型 | 范围 | 说明 |
|--------|------|------|------|
| **ShoulderSwitchTime** | float | 0.1-2.0 | 肩部切换过渡时间 |
| **CrouchVerticalOffset** | float | -100 to 0 | 蹲下时垂直偏移 |
| **MountVerticalOffset** | float | 0-200 | 骑乘时垂直偏移 |
| **bAutoShoulderSwitch** | bool | - | 自动切换肩部视角 |
| **ShoulderInterpSpeed** | float | 1-20 | 肩部偏移插值速度 |

**自动肩部切换：**
```
bAutoShoulderSwitch = true 时：
- 锁定目标在左边 → 自动切换到左肩视角
- 锁定目标在右边 → 自动切换到右肩视角

这样可以避免角色身体遮挡目标
```

### B6: 约束模块参数

| 参数名 | 类型 | 说明 |
|--------|------|------|
| **bEnsureTargetVisible** | bool | 确保锁定目标可见 |
| **VisibilityCheckRate** | float | 可见性检查频率（次/秒） |
| **bUseSoftPitchLimits** | bool | 使用软俯仰限制 |
| **SoftLimitStrength** | float | 软限制强度 |

**软俯仰限制说明：**
```
硬限制：到达MinPitch/MaxPitch后完全停止
软限制：接近限制时逐渐增加阻力

软限制更自然，推荐SoftLimitStrength = 0.5
```

---

# 5. 修改器使用指南

## 5.1 修改器概述

修改器是临时效果，由游戏事件触发，自动结束。

```
修改器生命周期：
触发 → 淡入 → 激活 → 淡出 → 结束
     BlendIn  Active  BlendOut
```

## 5.2 震动修改器（S01-S08）

### 如何触发震动

**蓝图节点：**
```
Trigger Camera Shake
├─ Shake Type: (选择下方类型)
├─ Intensity: 0.0 - 2.0
├─ Duration: 持续时间（秒），0=使用默认
└─ Custom Data: 额外参数（可选）
```

### 震动类型说明

| ID | 名称 | 触发场景 | 推荐强度 | 默认时长 |
|----|------|----------|----------|----------|
| S01 | Damage | 玩家受伤 | 0.5-1.0 | 0.3s |
| S02 | Impact | 命中敌人 | 0.3-0.6 | 0.15s |
| S03 | Explosion | 爆炸发生 | 0.8-1.5 | 0.5s |
| S04 | Landing | 高处落地 | 0.4-0.8 | 0.2s |
| S05 | FootStep | 巨型敌人脚步 | 0.2-0.4 | 0.1s |
| S06 | Parry | 成功弹反 | 0.6-1.0 | 0.2s |
| S07 | Critical | 暴击/处决 | 1.0-1.5 | 0.3s |
| S08 | Ambient | 环境震动 | 0.1-0.3 | 持续 |

### 震动参数详解

**通用参数：**
| 参数 | 说明 |
|------|------|
| Amplitude | 震动幅度（位置偏移量） |
| Frequency | 震动频率（次/秒） |
| Duration | 持续时间 |
| BlendInTime | 淡入时间 |
| BlendOutTime | 淡出时间 |

**示例：玩家受到重击**
```
事件：On Player Damaged
  ↓
Trigger Camera Shake
  ├─ Type: S01_Damage
  ├─ Intensity: DamageAmount / MaxHealth  (根据伤害比例)
  └─ Duration: 0 (使用默认)
```

## 5.3 缩放修改器（Z01-Z05）

### 缩放类型说明

| ID | 名称 | 效果 | 使用场景 |
|----|------|------|----------|
| Z01 | HitFreeze | FOV收缩+时间暂停感 | 重击命中瞬间 |
| Z02 | SpeedLines | FOV扩张 | 高速冲刺 |
| Z03 | FocusPull | FOV缓慢收缩 | 瞄准/专注 |
| Z04 | ImpactPunch | FOV快速脉冲 | 受击反馈 |
| Z05 | Dramatic | FOV大幅变化 | 戏剧性时刻 |

**蓝图触发：**
```
Trigger FOV Pulse
├─ Zoom Type: (选择类型)
├─ FOV Change: -15 到 +20 (负=收缩，正=扩张)
├─ Duration: 持续时间
└─ Intensity: 效果强度
```

## 5.4 反应修改器（R01-R05）

### 反应类型说明

| ID | 名称 | 效果 | 使用场景 |
|----|------|------|----------|
| R01 | HitReaction | 向受击方向偏移 | 玩家受击 |
| R02 | Recoil | 向后退缩 | 攻击后坐力 |
| R03 | Flinch | 快速抖动 | 受到惊吓 |
| R04 | Stagger | 摇晃失衡 | 体力耗尽 |
| R05 | KnockBack | 大幅后退 | 被击飞 |

**蓝图触发：**
```
Trigger Hit Reaction
├─ Direction: 受击方向向量 (世界空间)
├─ Intensity: 0.0 - 2.0
└─ Reaction Type: (选择类型)
```

**示例：受击反应**
```
事件：On Damage Received (DamageDirection, DamageAmount)
  ↓
Trigger Hit Reaction
  ├─ Direction: DamageDirection
  ├─ Intensity: DamageAmount / 100
  └─ Type: R01_HitReaction
```

## 5.5 过场修改器（C01-C05）

### 过场类型说明

| ID | 名称 | 使用场景 | 特点 |
|----|------|----------|------|
| C01 | Execution | 处决动画 | 绕角色旋转 |
| C02 | Backstab | 背刺动画 | 快速切换视角 |
| C03 | BossIntro | Boss登场 | 宏大展示 |
| C04 | BossPhaseChange | Boss阶段转换 | 戏剧性停顿 |
| C05 | Death | 死亡动画 | 慢速下坠 |

**蓝图触发：**
```
Trigger Cinematic Camera
├─ Cinematic Type: (选择类型)
├─ Target Actor: 目标Actor（如Boss）
├─ Duration: 持续时间
└─ Override Transform: 可选的相机变换覆盖
```

## 5.6 效果修改器（E01-E03）

### 效果类型说明

| ID | 名称 | 效果 | 触发条件 |
|----|------|------|----------|
| E01 | LowHealth | 红色暗角+心跳脉冲 | 血量<30% |
| E02 | StatusAilment | 颜色偏移+画面扭曲 | 中毒/诅咒 |
| E03 | FocusMode | 轻微暗角+饱和度提升 | 锁定目标 |

**E01 低血量效果控制：**
```
Update Low Health Effect
├─ Health Percent: 当前血量百分比 (0-1)
└─ 效果自动根据血量调整强度

建议在血量变化时调用：
事件：On Health Changed
  ↓
Update Low Health Effect
  └─ Health Percent: CurrentHealth / MaxHealth
```

**E02 状态异常效果控制：**
```
Trigger Status Effect
├─ Status Type:  Poison / Curse / Bleed
├─ Intensity: 效果强度 (0-1)
└─ Duration: 0 = 持续到手动停止

Stop Status Effect
└─ 停止当前状态效果
```

---

# 6. 碰撞系统配置

## 6.1 碰撞系统概述

碰撞系统防止相机穿过墙壁和障碍物。

```
碰撞处理流程：
检测(D) → 响应(RS) → 遮挡(OC) → 恢复(RC) → 特殊(SP)
```

## 6.2 碰撞配置参数

### 基础设置

| 参数 | 类型 | 说明 |
|------|------|------|
| **bUseSphereTrace** | bool | 使用球体检测（推荐true） |
| **SphereTraceRadius** | float | 检测球体半径（8-20cm） |
| **MinDistanceFromTarget** | float | 最小距离限制 |

### 响应设置

| 参数 | 类型 | 说明 |
|------|------|------|
| **PullInSpeed** | float | 拉近速度（值越大响应越快） |
| **bAllowSliding** | bool | 允许沿表面滑动 |
| **bAllowRotation** | bool | 允许旋转避开障碍 |

### 恢复设置

| 参数 | 类型 | 说明 |
|------|------|------|
| **RecoverySpeed** | float | 恢复到原位的速度 |
| **RecoveryDelay** | float | 开始恢复前的延迟（秒） |
| **bSmoothRecovery** | bool | 平滑恢复（推荐true） |

## 6.3 碰撞策略说明

### 检测策略（D01-D04）

| ID | 名称 | 特点 | 推荐场景 |
|----|------|------|----------|
| D01 | SingleRay | 单射线，最快 | 简单环境 |
| D02 | SphereSweep | 球体扫描，默认 | 通用 |
| D03 | MultiRay | 多射线锥形 | 复杂角落 |
| D04 | MultiSphereSweep | 多球体 | 大型相机 |

### 响应策略（RS01-RS05）

| ID | 名称 | 行为 | 优先级 |
|----|------|------|--------|
| RS01 | PullIn | 拉近相机 | 100（默认） |
| RS02 | Slide | 沿表面滑动 | 110 |
| RS03 | Orbit | 绕目标旋转 | 105 |
| RS04 | FOVCompensate | FOV补偿 | 95 |
| RS05 | InstantSnap | 立即吸附 | 120（紧急） |

### 遮挡策略（OC01-OC04）

| ID | 名称 | 效果 | 使用场景 |
|----|------|------|----------|
| OC01 | FadeOut | 渐隐遮挡物 | 透明墙壁 |
| OC02 | Hide | 隐藏遮挡物 | 可破坏物 |
| OC03 | PullInFurther | 进一步拉近 | 无法透视时 |
| OC04 | DitherFade | 抖动淡出 | 风格化效果 |

## 6.4 碰撞调试

**启用碰撞调试显示：**
```
蓝图节点：
Set Collision Debug Enabled
├─ Enable: true
└─ 效果：显示碰撞检测射线和安全位置
```

**调试显示说明：**
- 🔴 红色球体：碰撞点
- 🟡 黄色球体：安全位置
- 🟢 绿色线：无碰撞路径
- 🟠 橙色线：有碰撞，显示调整

---

# 7. 预设与风格

## 7.1 游戏风格预设

系统提供四种类魂游戏风格预设：

### 黑暗之魂风格（Dark Souls）
```
特点：沉稳、审慎
├─ 距离：450cm（中远）
├─ FOV：75°（略窄）
├─ 响应速度：慢（Lag 7-10）
└─ 感受：沉重、压迫感
```

### 血源诅咒风格（Bloodborne）
```
特点：快速、激进
├─ 距离：320cm（较近）
├─ FOV：95°（较宽）
├─ 响应速度：快（Lag 12-14）
└─ 感受：紧凑、速度感
```

### 只狼风格（Sekiro）
```
特点：灵敏、精确
├─ 距离：350cm（中等）
├─ FOV：85°（标准）
├─ 响应速度：极快（Lag 18-20）
└─ 感受：灵动、精准
```

### 艾尔登法环风格（Elden Ring）
```
特点：开阔、探索
├─ 距离：500cm（远）
├─ FOV：90°（标准偏宽）
├─ 响应速度：平衡（Lag 10-12）
└─ 感受：宏大、自由
```

## 7.2 应用预设

**蓝图节点：**
```
Apply Camera Style Preset
├─ Style: DarkSouls / Bloodborne / Sekiro / EldenRing
└─ 效果：自动应用该风格的所有参数
```

**或在DataTable中选择：**
```
在FCameraStateConfig中：
├─ 选择 "使用预设"
└─ 选择预设类型
```

## 7.3 自定义风格

基于预设进行微调：

```
步骤：
1. 选择最接近的预设作为基础
2. 在DataTable中覆盖特定参数
3. 测试并迭代调整
```

---

# 8. 调试工具

## 8.1 调试组件

将 `SoulsCameraDebugComponent` 添加到角色：

```
添加组件：
角色蓝图 → Add Component → Souls Camera Debug Component
```

## 8.2 调试标志

| 标志 | 显示内容 |
|------|----------|
| **bShowFocusPoint** | 焦点球体 |
| **bShowCameraPath** | 相机到焦点连线 |
| **bShowCollisionProbes** | 碰撞检测可视化 |
| **bShowTargetInfo** | 锁定目标信息 |
| **bShowStateInfo** | 当前状态信息 |
| **bShowModuleInfo** | 激活模块列表 |
| **bShowModifierInfo** | 激活修改器列表 |
| **bShowPerformanceStats** | 性能统计 |
| **bShowOnScreen** | 屏幕文字信息 |

## 8.3 调试命令

**蓝图调试节点：**
```
Get Camera Debug String
└─ 返回当前相机状态的调试字符串

Print Camera State
└─ 在屏幕上打印当前状态

Validate Camera System
└─ 验证系统完整性（返回报告）
```

## 8.4 性能监控

```
调试组件属性：
├─ AverageFrameTime：平均帧时间
├─ PeakFrameTime：峰值帧时间
└─ PerformanceSampleInterval：采样间隔
```

---

# 9. 常见问题解答

## Q1: 相机感觉太迟钝/太灵敏

**解决方案：**
```
调整Lag参数：
├─ 太迟钝：增加 PositionLagSpeed / RotationLagSpeed
├─ 太灵敏：减少 PositionLagSpeed / RotationLagSpeed
└─ 推荐范围：探索8-12，战斗15-20
```

## Q2: 相机穿墙

**解决方案：**
```
检查碰撞设置：
├─ 确认 bEnableCollision = true
├─ 增加 SphereTraceRadius（12-20cm）
├─ 检查碰撞通道设置
└─ 确认墙壁有正确的碰撞设置
```

## Q3: Boss战时看不全Boss

**解决方案：**
```
调整Boss状态参数：
├─ 增加 BaseDistance（500-600cm）
├─ 增加 BaseFOV（95-100°）
├─ 调整 MidPointBias（0.3-0.4）
└─ 增加 TargetSizeDistanceScale（1.3-1.5）
```

## Q4: 锁定目标时相机抖动

**解决方案：**
```
调整锁定参数：
├─ 增加 LookAtInterpSpeed（8-12）
├─ 减少 SoftLockStrength（0.6-0.7）
└─ 增加 RotationLagSpeed
```

## Q5: 受击反馈不明显

**解决方案：**
```
增强反馈效果：
├─ 增加震动 Intensity（0.8-1.2）
├─ 增加 ImpactFOVAmount（12-15）
├─ 添加 Hit Reaction 修改器
└─ 考虑添加时间膨胀效果
```

## Q6: 过场动画不流畅

**解决方案：**
```
调整过场参数：
├─ 增加 BlendInTime / BlendOutTime
├─ 选择合适的曲线类型（EaseInOut推荐）
└─ 检查关键帧时间间隔
```

## Q7: 骑乘时视野不好

**解决方案：**
```
调整骑乘状态：
├─ 增加 MountVerticalOffset（80-120cm）
├─ 增加 BaseDistance（550-650cm）
├─ 增加 BaseFOV（95-100°）
└─ 调整 MaxPitch 允许更多向下看
```

---

# 10. 参数速查表

## 10.1 状态基础参数（A组）

| 参数 | 探索 | 战斗 | Boss | 瞄准 | 单位 |
|------|------|------|------|------|------|
| BaseDistance | 450 | 350 | 500 | 300 | cm |
| MinDistance | 100 | 80 | 150 | 80 | cm |
| MaxDistance | 700 | 500 | 800 | 400 | cm |
| BaseFOV | 90 | 85 | 95 | 70 | ° |
| MinFOV | 70 | 70 | 80 | 60 | ° |
| MaxFOV | 110 | 100 | 110 | 80 | ° |
| PositionLagSpeed | 10 | 18 | 12 | 20 | - |
| RotationLagSpeed | 10 | 18 | 12 | 20 | - |
| ShoulderOffset | 50 | 40 | 0 | 80 | cm |
| BlendInTime | 0.5 | 0.3 | 0.8 | 0.15 | s |

## 10.2 模块参数（B组）

| 参数 | 推荐值 | 范围 | 说明 |
|------|--------|------|------|
| PredictionTime | 0.15 | 0-0.5 | 位置预测 |
| SoftLockStrength | 0.7 | 0-1 | 软锁定 |
| MidPointBias | 0.35 | 0-1 | 双目标中点 |
| SpeedFOVIncrease | 12 | 0-30 | 速度FOV |
| ImpactFOVAmount | 10 | 0-30 | 受击FOV |
| ImpactDuration | 0.15 | 0.05-0.5 | 受击时长 |

## 10.3 震动强度参考

| 事件 | 推荐强度 | 持续时间 |
|------|----------|----------|
| 轻击受伤 | 0.3-0.5 | 0.15s |
| 重击受伤 | 0.7-1.0 | 0.3s |
| 命中敌人 | 0.2-0.4 | 0.1s |
| 重击命中 | 0.5-0.7 | 0.2s |
| 爆炸 | 1.0-1.5 | 0.5s |
| 落地 | 0.4-0.6 | 0.2s |
| 弹反成功 | 0.6-0.8 | 0.2s |
| 处决 | 0.8-1.2 | 0.4s |

## 10.4 碰撞参数推荐

| 参数 | 推荐值 | 说明 |
|------|--------|------|
| SphereTraceRadius | 15 | 碰撞半径 |
| MinDistanceFromTarget | 50 | 最小距离 |
| PullInSpeed | 15 | 拉近速度 |
| RecoverySpeed | 8 | 恢复速度 |
| RecoveryDelay | 0.2 | 恢复延迟 |

---

# 附录

## A. 相机状态枚举

```cpp
UENUM(BlueprintType)
enum class ECameraState : uint8
{
    Exploration = 0,    // 探索
    Combat = 1,         // 战斗
    BossLockOn = 2,     // Boss锁定
    Dialogue = 3,       // 对话
    Cinematic = 4,      // 过场
    Aiming = 5,         // 瞄准
    Mounted = 6,        // 骑乘
    Swimming = 7        // 游泳
};
```

## B. 修改器类型枚举

```cpp
UENUM(BlueprintType)
enum class EModifierCategory : uint8
{
    Shake = 0,      // 震动 (S01-S08)
    Zoom = 1,       // 缩放 (Z01-Z05)
    Reaction = 2,   // 反应 (R01-R05)
    Cinematic = 3,  // 过场 (C01-C05)
    Effect = 4      // 效果 (E01-E03)
};
```

## C. 快捷键（调试用）

| 快捷键 | 功能 |
|--------|------|
| F1 | 切换调试显示 |
| F2 | 显示/隐藏碰撞 |
| F3 | 打印当前状态 |
| F4 | 切换性能统计 |

---

**文档结束**

如有问题，请联系技术团队。