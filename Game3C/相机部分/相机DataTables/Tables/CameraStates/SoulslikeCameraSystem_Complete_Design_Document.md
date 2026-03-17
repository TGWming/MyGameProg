# 类魂游戏第三人称相机系统完整设计方案

## UE4.27 落地实现文档

**版本**: 1.0  
**日期**: 2025-11-26  
**对标作品**: 血源诅咒 / 匹诺曹的谎言 / 艾尔登法环 / 黑神话：悟空 / 渊虚之羽

---

# 目录

- [第一部分：理论基础与设计原则](#第一部分理论基础与设计原则)
  - [1. 1 Mark Haigh-Hutchinson 核心原则应用](#11-mark-haigh-hutchinson-核心原则应用)
  - [1.2 知乎文章核心理论整合](#12-知乎文章核心理论整合)
  - [1.3 对标作品分析](#13-对标作品分析)
- [第二部分：系统架构设计](#第二部分系统架构设计)
  - [2.1 整体Pipeline架构](#21-整体pipeline架构)
  - [2.2 核心类结构设计](#22-核心类结构设计)
  - [2. 3 状态机设计](#23-状态机设计)
  - [2.4 模块系统设计](#24-模块系统设计)
  - [2.5 修改器系统](#25-修改器系统modifier-system)
  - [2.6 碰撞处理系统](#26-碰撞处理系统)
  - [2.7 主Pipeline实现](#27-主pipeline实现)
- [第三部分：状态详细配置](#第三部分状态详细配置)
- [第四部分：数据结构定义](#第四部分数据结构定义)
- [第五部分：接口定义](#第五部分接口定义)
- [第六部分：调试系统](#第六部分调试系统)
- [第七部分：迁移指南](#第七部分迁移指南)
- [第八部分：性能优化建议](#第八部分性能优化建议)
- [第九部分：蓝图暴露](#第九部分蓝图暴露)
- [第十部分：总结](#第十部分总结)

---

# 第一部分：理论基础与设计原则

## 1.1 Mark Haigh-Hutchinson 核心原则应用

基于《Real-Time Cameras》的核心理论，结合类魂游戏特性，确立以下设计原则：

### 1.1.1 控制参考系选择（Control Reference Frames）

| 参考系类型 | 类魂游戏应用场景 | 理论依据 |
|-----------|-----------------|---------|
| **World-Relative** | 默认探索、非锁定战斗 | 操作感最佳，玩家能精准控制移动方向 |
| **Camera-Relative** | 高速奔跑、骑乘 | 平衡代入感与操作感，避免高速移动撞墙 |
| **Target-Relative** | 锁定战斗（类魂特有） | 相机围绕"玩家-敌人"中点构建，而非纯跟随玩家 |

**类魂游戏特殊考量**：
- 不同于一般动作游戏，类魂游戏的锁定系统需要**软锁定（Soft Lock）**而非硬锁定
- 玩家在锁定状态下仍可通过右摇杆微调视角
- 参考：血源诅咒、艾尔登法环的锁定手感

### 1.1.2 时间连贯性原则（Temporal Coherence）

```
核心公式（知乎文章1）：
StepPerFrame = InertiaCoeff × DeltaTime × Distance

应用规则：
1. 任何相机参数变化都必须经过插值，绝不允许跳跃
2. 位置插值使用指数衰减（Exponential Decay）
3. 旋转插值使用球面插值（Slerp）
4. 位置和旋转必须使用不同的插值速度
```

### 1.1. 3 约束满足系统（Constraint Satisfaction）

类魂游戏相机需要同时满足的约束优先级：

```
Priority 0: 碰撞安全（不穿墙）        [硬约束-不可违反]
Priority 1: 目标可见性（锁定时）      [硬约束-不可违反]
Priority 2: 玩家可见性               [硬约束-不可违反]
Priority 3: Pitch角度限制            [软约束-可弹性处理]
Priority 4: 理想构图位置             [软约束-可弹性处理]
Priority 5: 美学偏好（黄金分割等）    [软约束-可弹性处理]
```

### 1.1.4 预测性定位（Predictive Positioning）

```cpp
// 基于角色速度预测相机目标位置
FVector PredictedFocusPoint = CurrentFocusPoint + CharacterVelocity * PredictionTime;

// 类魂游戏建议值
PredictionTime = 0. 1s ~ 0.2s  // 轻微预测，不宜过大
```

---

## 1. 2 知乎文章核心理论整合

### 1.2. 1 镜头描述的最简参数集（文章1）

```
相机状态 = {
    FocusPoint,      // 焦点位置（角色身上某点）
    Distance,        // 焦距（相机到焦点的距离）
    Pitch,           // 俯仰角
    Yaw,             // 偏航角
    FOV,             // 视场角
    ScreenOffset     // 屏幕空间焦点偏移
}
```

**设计原则**：参数越少越好，减少后续每一步需要考虑的复杂度。

### 1. 2.2 双空间偏移系统（文章1核心发现）

| 偏移类型 | 作用 | 类魂游戏应用 |
|---------|------|-------------|
| **屏幕空间偏移** | 让焦点稳定在屏幕固定位置，相机旋转时比例不变 | 过肩视角、构图控制 |
| **三维空间偏移** | 改变相机旋转轴心 | 蹲伏、骑乘时焦点下移 |

### 1.2. 3 Yaw自动旋转的两种方案（文章1）

**方案A：速度-差值函数法**
```cpp
// 可定制曲线，但实现复杂
YawSpeed = CustomCurve. Evaluate(YawDiff);
CurrentYaw += YawSpeed * DeltaTime;
```

**方案B：差值百分比渐进法**
```cpp
// 简单高效，推荐用于类魂游戏
float Alpha = 1.0f - FMath::Exp(-LagSpeed * DeltaTime);
CurrentYaw = FMath::Lerp(CurrentYaw, TargetYaw, Alpha);
```

**类魂游戏选择**：方案B为主，特殊场景（Boss入场）使用方案A的自定义曲线。

### 1. 2.4 动态FOV公式（文章1）

```cpp
// 基础公式
FOV = BaseFOV + SpeedCoefficient * CharacterSpeed;

// 推荐参数（类魂游戏）
BaseFOV = 70. 0f;           // 基础视野
SpeedCoefficient = 0.5f;   // 每1m/s增加0.5度
MaxFOV = 85.0f;            // 上限（奔跑时）
MinFOV = 55.0f;            // 下限（瞄准时，如黑神话弓箭）
```

### 1.2.5 相机系统组成（文章2）

```
┌─────────────────────────────────────────┐
│            CameraManager                 │
│  ┌─────────────────────────────────┐    │
│  │  Camera List (多相机管理)        │    │
│  │  Active Camera Selection        │    │
│  │  Blend/Transition Control       │    │
│  │  Input Routing                  │    │
│  └─────────────────────────────────┘    │
│                  │                       │
│     ┌────────────┼────────────┐         │
│     ▼            ▼            ▼         │
│  ┌──────┐   ┌──────┐    ┌──────┐       │
│  │Camera│   │Camera│    │Camera│       │
│  │  A   │   │  B   │    │  C   │       │
│  └──┬───┘   └──────┘    └──────┘       │
│     │                                   │
│  ┌──▼───────────────┐                  │
│  │   SpringArm      │                  │
│  │  ├─ TargetOffset │                  │
│  │  ├─ SocketOffset │                  │
│  │  ├─ ArmLength    │                  │
│  │  └─ Collision    │                  │
│  └──────────────────┘                  │
└─────────────────────────────────────────┘
```

---

## 1. 3 对标作品分析

### 1.3. 1 血源诅咒（Bloodborne）

| 特性 | 参数/实现 |
|-----|----------|
| **基础距离** | 约350-400单位 |
| **锁定距离** | 动态，根据敌人体型调整 |
| **锁定构图** | 玩家偏左1/3，敌人偏右1/3 |
| **解锁过渡** | 快速但平滑，约0.3s |
| **特点** | 快节奏战斗，相机反应迅速，Pitch自由度高 |

### 1.3. 2 艾尔登法环（Elden Ring）

| 特性 | 参数/实现 |
|-----|----------|
| **基础距离** | 约400-500单位（开阔世界更远） |
| **骑马距离** | 约600-700单位 |
| **多目标锁定** | 支持右摇杆切换目标 |
| **大型Boss** | 相机大幅拉远，可达800+ |
| **特点** | 开放世界适配，远景展示，骑乘系统深度集成 |

### 1.3. 3 匹诺曹的谎言（Lies of P）

| 特性 | 参数/实现 |
|-----|----------|
| **基础距离** | 约300-350单位（偏近） |
| **过肩偏移** | 明显的右侧偏移 |
| **锁定紧凑度** | 高，相机积极跟随 |
| **室内适配** | 优秀的狭窄空间处理 |
| **特点** | 线性关卡优化，相机更"紧"，打击感强化 |

### 1.3.4 黑神话：悟空

| 特性 | 参数/实现 |
|-----|----------|
| **基础距离** | 约400单位 |
| **棍法连招** | 相机轻微后拉+FOV增加 |
| **变身系统** | 大型变身时相机大幅拉远 |
| **法术瞄准** | 过肩视角+FOV缩小 |
| **特点** | 动作幅度大，相机配合招式节奏，电影感强 |

### 1.3. 5 渊虚之羽（Phantom Blade Zero预览）

| 特性 | 参数/实现 |
|-----|----------|
| **基础距离** | 约350单位 |
| **时停/弹刀** | 相机短暂定格+微缩放 |
| **连招流畅** | 相机延迟跟随，不打断动作节奏 |
| **特点** | 极快节奏，相机"稳"但不"死"，处决镜头精致 |

### 1.3. 6 综合参数提取

```cpp
// 类魂游戏通用参数范围
namespace SoulslikeCameraDefaults
{
    // 距离
    const float ExploreDistance = 400.0f;      // 探索
    const float CombatDistance = 350.0f;       // 战斗
    const float LockOnDistance = 300.0f;       // 锁定（基础）
    const float BossDistance_Min = 500.0f;     // Boss战最小
    const float BossDistance_Max = 800.0f;     // Boss战最大（根据体型）
    const float MountDistance = 600.0f;        // 骑乘
    
    // FOV
    const float BaseFOV = 70.0f;
    const float SprintFOV = 80.0f;
    const float AimFOV = 55.0f;
    const float BossFOV = 75.0f;
    
    // Pitch限制
    const float MinPitch = -60.0f;
    const float MaxPitch = 70.0f;
    
    // 过渡时间
    const float StateBlendTime = 0.3f;         // 状态切换
    const float LockOnBlendTime = 0.2f;        // 锁定切换
    const float TargetSwitchBlendTime = 0.15f; // 目标切换
    
    // 惯性系数
    const float PositionLagSpeed = 8.0f;       // 位置跟随
    const float RotationLagSpeed = 12.0f;      // 旋转跟随
}
```

---

# 第二部分：系统架构设计

## 2. 1 整体Pipeline架构

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              SOULSLIKE CAMERA PIPELINE                               │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                      │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │    INPUT     │   │    STATE     │   │   MODULE     │   │   BLEND &    │         │
│  │   CONTEXT    │──▶│   MACHINE    │──▶│   COMPUTE    │──▶│    SOLVE     │──┐      │
│  │   GATHER     │   │   UPDATE     │   │   STACK      │   │   CONFLICTS  │  │      │
│  └──────────────┘   └──────────────┘   └──────────────┘   └──────────────┘  │      │
│                                                                              │      │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐   ┌──────────────┐  │      │
│  │   RENDER     │◀──│    POST      │◀──│  COLLISION   │◀──│   OUTPUT     │◀─┘      │
│  │   CAMERA     │   │   PROCESS    │   │  RESOLVER    │   │  TRANSFORM   │         │
│  │   APPLY      │   │   EFFECTS    │   │              │   │              │         │
│  └──────────────┘   └──────────────┘   └──────────────┘   └──────────────┘         │
│                                                                                      │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### 2.1.1 Pipeline阶段详解

| 阶段 | 职责 | 输入 | 输出 |
|-----|------|-----|------|
| **Input Context** | 收集当前帧所有相关数据 | 玩家输入、角色状态、目标信息、环境数据 | FCameraInputContext |
| **State Machine** | 决定当前相机状态 | Context + 上一帧状态 | ECameraState + 过渡信息 |
| **Module Compute** | 各模块独立计算 | Context + State | 各模块的Raw输出 |
| **Blend & Solve** | 混合模块输出，解决冲突 | 所有模块输出 | 统一的相机参数 |
| **Output Transform** | 生成最终Transform | 混合后参数 | FTransform + FCameraParams |
| **Collision Resolver** | 碰撞检测与修正 | Transform | 修正后Transform |
| **Post Process** | 应用后处理效果 | Transform | 最终Transform + 震屏等 |
| **Render Apply** | 应用到实际相机 | 最终数据 | 更新SpringArm/Camera |

---

## 2. 2 核心类结构设计

### 2.2.1 UE4.27 类继承图

```
UObject
├── USoulsCameraSubsystem              // 全局相机子系统
│
├── USoulsCameraPipeline               // 主Pipeline（组件形式）
│   └── UActorComponent
│
├── USoulsCameraState                  // 状态基类
│   ├── USoulsCameraState_Explore      // 探索状态
│   ├── USoulsCameraState_Combat       // 战斗状态（未锁定）
│   ├── USoulsCameraState_LockOn       // 锁定状态
│   ├── USoulsCameraState_Boss         // Boss战状态
│   ├── USoulsCameraState_Execution    // 处决状态
│   ├── USoulsCameraState_Death        // 死亡状态
│   ├── USoulsCameraState_Cinematic    // 过场状态
│   └── USoulsCameraState_Mount        // 骑乘状态
│
├── USoulsCameraModule                 // 模块基类
│   ├── USoulsModule_Position          // 位置计算
│   │   ├── USoulsModule_FollowTarget
│   │   ├── USoulsModule_Orbit
│   │   └── USoulsModule_Predictive
│   ├── USoulsModule_Rotation          // 旋转计算
│   │   ├── USoulsModule_LookAt
│   │   ├── USoulsModule_PlayerInput
│   │   └── USoulsModule_AutoOrient
│   ├── USoulsModule_Distance          // 距离计算
│   │   ├── USoulsModule_BaseDistance
│   │   ├── USoulsModule_TargetSize
│   │   └── USoulsModule_EnvironmentAware
│   ├── USoulsModule_FOV               // FOV计算
│   │   ├── USoulsModule_SpeedFOV
│   │   ├── USoulsModule_CombatFOV
│   │   └── USoulsModule_AimFOV
│   └── USoulsModule_PostProcess       // 后处理
│       ├── USoulsModule_Shake
│       ├── USoulsModule_Impact
│       └── USoulsModule_Smoothing
│
├── USoulsCameraModifier               // 修改器基类（叠加效果）
│   ├── USoulsModifier_BossZoom        // Boss战缩放
│   ├── USoulsModifier_ExecutionCam    // 处决相机
│   ├── USoulsModifier_HitReaction     // 受击反应
│   └── USoulsModifier_Parry           // 弹反特写
│
└── USoulsCameraBlender                // 混合器
    ├── USoulsBlender_Linear           // 线性混合
    ├── USoulsBlender_SmoothStep       // 平滑混合
    └── USoulsBlender_CustomCurve      // 自定义曲线
```

---

## 2.3 状态机设计

### 2. 3.1 状态枚举与优先级

```cpp
UENUM(BlueprintType)
enum class ESoulsCameraState : uint8
{
    // 探索层 (Priority 0-9)
    Explore         = 0   UMETA(DisplayName = "Explore"),
    Sprint          = 1   UMETA(DisplayName = "Sprint"),
    Mount           = 2   UMETA(DisplayName = "Mount"),
    Swim            = 3   UMETA(DisplayName = "Swim"),
    Climb           = 4   UMETA(DisplayName = "Climb"),
    
    // 战斗层 (Priority 10-29)
    Combat          = 10  UMETA(DisplayName = "Combat"),
    LockOn          = 11  UMETA(DisplayName = "LockOn"),
    LockOn_Multi    = 12  UMETA(DisplayName = "LockOn Multi-Target"),
    Aim             = 13  UMETA(DisplayName = "Aim/Ranged"),
    
    // Boss战层 (Priority 30-39)
    Boss_Medium     = 30  UMETA(DisplayName = "Boss Medium"),
    Boss_Large      = 31  UMETA(DisplayName = "Boss Large"),
    Boss_Huge       = 32  UMETA(DisplayName = "Boss Huge"),
    Boss_Phase      = 33  UMETA(DisplayName = "Boss Phase Transition"),
    
    // 动作反馈层 (Priority 40-49)
    Dodge           = 40  UMETA(DisplayName = "Dodge"),
    Parry           = 41  UMETA(DisplayName = "Parry"),
    Execution       = 42  UMETA(DisplayName = "Execution"),
    BackStab        = 43  UMETA(DisplayName = "BackStab"),
    
    // 过场层 (Priority 100+)
    Cinematic       = 100 UMETA(DisplayName = "Cinematic"),
    Death           = 101 UMETA(DisplayName = "Death"),
    BossIntro       = 102 UMETA(DisplayName = "Boss Intro"),
    Bonfire         = 103 UMETA(DisplayName = "Bonfire/Checkpoint"),
    
    MAX             = 255
};
```

### 2.3.2 状态转换规则

```cpp
// 状态转换规则表
struct FSoulsCameraTransitionRule
{
    ESoulsCameraState FromState;
    ESoulsCameraState ToState;
    float BlendTime;
    EBlendType BlendType;
    bool bCanInterrupt;
    TFunction<bool(const FCameraContext&)> Condition;
};

// 核心转换规则示例
const TArray<FSoulsCameraTransitionRule> TransitionRules = {
    // 探索 <-> 战斗
    { Explore,  Combat,     0.3f, SmoothStep, true,  [](auto& C){ return C.bInCombat; } },
    { Combat,   Explore,    0.5f, SmoothStep, true,  [](auto& C){ return !C. bInCombat; } },
    
    // 战斗 <-> 锁定
    { Combat,   LockOn,     0.2f, SmoothStep, true,  [](auto& C){ return C.bHasLockTarget; } },
    { LockOn,   Combat,     0.25f, SmoothStep, true, [](auto& C){ return ! C.bHasLockTarget; } },
    
    // 锁定 -> Boss
    { LockOn,   Boss_Large, 0.4f, CustomCurve, false, [](auto& C){ return C. TargetSize > 500. f; } },
    
    // 任意 -> 处决（高优先级）
    { Any,      Execution,  0.15f, Cut, false, [](auto& C){ return C.bExecutionTriggered; } },
    
    // 任意 -> 死亡（最高优先级）
    { Any,      Death,      0.1f, Cut, false, [](auto& C){ return C.bPlayerDead; } },
};
```

### 2.3. 3 状态机核心逻辑

```cpp
UCLASS()
class USoulsCameraStateMachine : public UObject
{
    GENERATED_BODY()
    
public:
    void Update(float DeltaTime, const FCameraInputContext& Context);
    
    ESoulsCameraState GetCurrentState() const { return CurrentState; }
    ESoulsCameraState GetPreviousState() const { return PreviousState; }
    float GetBlendAlpha() const { return BlendAlpha; }
    bool IsTransitioning() const { return bIsTransitioning; }
    
private:
    ESoulsCameraState EvaluateDesiredState(const FCameraInputContext& Context);
    bool CanTransitionTo(ESoulsCameraState NewState, const FCameraInputContext& Context);
    void BeginTransition(ESoulsCameraState NewState, const FSoulsCameraTransitionRule& Rule);
    void UpdateTransition(float DeltaTime);
    
private:
    UPROPERTY()
    ESoulsCameraState CurrentState = ESoulsCameraState::Explore;
    
    UPROPERTY()
    ESoulsCameraState PreviousState = ESoulsCameraState::Explore;
    
    UPROPERTY()
    ESoulsCameraState DesiredState = ESoulsCameraState::Explore;
    
    float BlendAlpha = 1.0f;
    float BlendDuration = 0. 0f;
    float BlendElapsed = 0. 0f;
    bool bIsTransitioning = false;
    
    UPROPERTY()
    UCurveFloat* CurrentBlendCurve = nullptr;
};
```

---

## 2.4 模块系统设计

### 2.4. 1 模块基类

```cpp
UCLASS(Abstract, Blueprintable)
class USoulsCameraModule : public UObject
{
    GENERATED_BODY()
    
public:
    // 模块初始化
    virtual void Initialize(USoulsCameraPipeline* Pipeline);
    
    // 每帧更新，返回该模块的计算结果
    virtual FSoulsModuleOutput Compute(
        float DeltaTime,
        const FCameraInputContext& Context,
        const FSoulsModuleOutput& PreviousOutput
    ) PURE_VIRTUAL(, return FSoulsModuleOutput(););
    
    // 模块是否在当前状态下激活
    virtual bool IsActiveInState(ESoulsCameraState State) const;
    
    // 获取模块优先级（用于冲突解决）
    virtual int32 GetPriority() const { return Priority; }
    
    // 获取混合策略
    virtual EModuleBlendPolicy GetBlendPolicy() const { return BlendPolicy; }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Module")
    int32 Priority = 0;
    
    UPROPERTY(EditAnywhere, Category = "Module")
    EModuleBlendPolicy BlendPolicy = EModuleBlendPolicy::Additive;
    
    UPROPERTY(EditAnywhere, Category = "Module")
    float Weight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Module")
    TArray<ESoulsCameraState> ActiveStates;
    
    UPROPERTY()
    USoulsCameraPipeline* OwnerPipeline = nullptr;
};
```

### 2.4.2 模块输出结构

```cpp
USTRUCT(BlueprintType)
struct FSoulsModuleOutput
{
    GENERATED_BODY()
    
    // 位置相关
    FVector PositionOffset = FVector::ZeroVector;
    FVector PositionOverride = FVector::ZeroVector;
    bool bOverridePosition = false;
    
    // 旋转相关
    FRotator RotationOffset = FRotator::ZeroRotator;
    FRotator RotationOverride = FRotator::ZeroRotator;
    bool bOverrideRotation = false;
    
    // 距离相关
    float DistanceMultiplier = 1.0f;
    float DistanceOffset = 0. 0f;
    float DistanceOverride = 0.0f;
    bool bOverrideDistance = false;
    
    // FOV相关
    float FOVOffset = 0.0f;
    float FOVOverride = 0.0f;
    bool bOverrideFOV = false;
    
    // 元数据
    float OutputWeight = 1.0f;
    bool bIsValid = true;
};
```

### 2.4.3 混合策略

```cpp
UENUM(BlueprintType)
enum class EModuleBlendPolicy : uint8
{
    Additive,       // 加法叠加（震屏、偏移）
    Override,       // 完全覆盖（过场镜头）
    Multiplicative, // 乘法叠加（距离系数）
    Maximum,        // 取最大值（碰撞限制）
    Minimum,        // 取最小值（距离限制）
    Blend           // 权重混合（状态切换）
};
```

### 2.4. 4 位置跟随模块

```cpp
UCLASS()
class USoulsModule_FollowTarget : public USoulsCameraModule
{
    GENERATED_BODY()
    
public:
    virtual FSoulsModuleOutput Compute(
        float DeltaTime,
        const FCameraInputContext& Context,
        const FSoulsModuleOutput& PreviousOutput) override
    {
        FSoulsModuleOutput Output;
        
        // 计算目标焦点位置
        FVector TargetFocusPoint = CalculateFocusPoint(Context);
        
        // 应用预测性定位
        if (bUsePrediction && Context.CharacterVelocity.SizeSquared() > 100.0f)
        {
            TargetFocusPoint += Context.CharacterVelocity * PredictionTime;
        }
        
        // 应用惯性跟随（指数衰减插值）
        float Alpha = 1.0f - FMath::Exp(-PositionLagSpeed * DeltaTime);
        FVector SmoothedFocusPoint = FMath::Lerp(
            PreviousOutput.bIsValid ? LastFocusPoint : TargetFocusPoint,
            TargetFocusPoint,
            Alpha
        );
        
        // 分离XZ平面和Y轴的惯性（知乎文章1建议）
        if (bSeparateVerticalLag)
        {
            float VerticalAlpha = 1. 0f - FMath::Exp(-VerticalLagSpeed * DeltaTime);
            SmoothedFocusPoint.Z = FMath::Lerp(
                LastFocusPoint. Z,
                TargetFocusPoint.Z,
                VerticalAlpha
            );
        }
        
        LastFocusPoint = SmoothedFocusPoint;
        Output.PositionOverride = SmoothedFocusPoint;
        Output.bOverridePosition = true;
        
        return Output;
    }
    
private:
    FVector CalculateFocusPoint(const FCameraInputContext& Context)
    {
        // 基础焦点 = 角色位置 + 三维空间偏移
        FVector FocusPoint = Context.CharacterLocation;
        
        // 应用骨骼偏移（如果配置）
        if (bUseSocketOffset && Context.CharacterMesh)
        {
            FocusPoint = Context.CharacterMesh->GetSocketLocation(FocusSocketName);
        }
        else
        {
            FocusPoint += FocusOffset;
        }
        
        return FocusPoint;
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Follow")
    FVector FocusOffset = FVector(0, 0, 80.0f);  // 默认看向胸口
    
    UPROPERTY(EditAnywhere, Category = "Follow")
    bool bUseSocketOffset = false;
    
    UPROPERTY(EditAnywhere, Category = "Follow")
    FName FocusSocketName = "spine_02";
    
    UPROPERTY(EditAnywhere, Category = "Lag")
    float PositionLagSpeed = 8.0f;
    
    UPROPERTY(EditAnywhere, Category = "Lag")
    bool bSeparateVerticalLag = true;
    
    UPROPERTY(EditAnywhere, Category = "Lag")
    float VerticalLagSpeed = 4.0f;  // 垂直方向更"松"
    
    UPROPERTY(EditAnywhere, Category = "Prediction")
    bool bUsePrediction = true;
    
    UPROPERTY(EditAnywhere, Category = "Prediction")
    float PredictionTime = 0.1f;
    
private:
    FVector LastFocusPoint = FVector::ZeroVector;
};
```

### 2.4. 5 锁定旋转模块

```cpp
UCLASS()
class USoulsModule_LockOnRotation : public USoulsCameraModule
{
    GENERATED_BODY()
    
public:
    virtual FSoulsModuleOutput Compute(
        float DeltaTime,
        const FCameraInputContext& Context,
        const FSoulsModuleOutput& PreviousOutput) override
    {
        FSoulsModuleOutput Output;
        
        if (!Context.LockOnTarget)
        {
            Output.bIsValid = false;
            return Output;
        }
        
        // 计算玩家到目标的方向
        FVector PlayerToTarget = Context.LockOnTargetLocation - Context. CharacterLocation;
        PlayerToTarget.Z = 0;  // 先只考虑水平方向
        PlayerToTarget.Normalize();
        
        // 计算理想的相机Yaw（在玩家身后，面向目标）
        FRotator LookAtRotation = PlayerToTarget. Rotation();
        float IdealYaw = LookAtRotation. Yaw;
        
        // 计算理想的相机Pitch
        FVector CameraToTarget = Context.LockOnTargetLocation - 
            (Context.CharacterLocation + FVector(0, 0, 100.0f)); // 假设相机高度
        float IdealPitch = FMath::Clamp(
            FMath::Atan2(CameraToTarget.Z, CameraToTarget.Size2D()) * 
            (180.0f / PI),
            MinPitch,
            MaxPitch
        );
        
        // 应用软锁定：允许玩家输入微调
        if (bAllowPlayerAdjustment)
        {
            IdealYaw += Context.PlayerYawInput * PlayerAdjustmentStrength;
            IdealPitch += Context.PlayerPitchInput * PlayerAdjustmentStrength;
        }
        
        // 平滑旋转（使用Slerp思想，但分离处理Yaw和Pitch）
        float YawAlpha = 1.0f - FMath::Exp(-YawLagSpeed * DeltaTime);
        float PitchAlpha = 1. 0f - FMath::Exp(-PitchLagSpeed * DeltaTime);
        
        float SmoothedYaw = FMath::Lerp(
            FRotator::NormalizeAxis(LastYaw),
            FRotator::NormalizeAxis(IdealYaw),
            YawAlpha
        );
        float SmoothedPitch = FMath::Lerp(LastPitch, IdealPitch, PitchAlpha);
        
        LastYaw = SmoothedYaw;
        LastPitch = SmoothedPitch;
        
        Output. RotationOverride = FRotator(SmoothedPitch, SmoothedYaw, 0);
        Output.bOverrideRotation = true;
        
        return Output;
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "LockOn")
    float YawLagSpeed = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "LockOn")
    float PitchLagSpeed = 8.0f;
    
    UPROPERTY(EditAnywhere, Category = "LockOn")
    float MinPitch = -30.0f;
    
    UPROPERTY(EditAnywhere, Category = "LockOn")
    float MaxPitch = 45.0f;
    
    UPROPERTY(EditAnywhere, Category = "Soft Lock")
    bool bAllowPlayerAdjustment = true;
    
    UPROPERTY(EditAnywhere, Category = "Soft Lock")
    float PlayerAdjustmentStrength = 0.3f;  // 软锁定强度
    
private:
    float LastYaw = 0.0f;
    float LastPitch = 0. 0f;
};
```

### 2.4.6 目标体型距离模块

```cpp
UCLASS()
class USoulsModule_TargetSizeDistance : public USoulsCameraModule
{
    GENERATED_BODY()
    
public:
    virtual FSoulsModuleOutput Compute(
        float DeltaTime,
        const FCameraInputContext& Context,
        const FSoulsModuleOutput& PreviousOutput) override
    {
        FSoulsModuleOutput Output;
        
        if (!Context.LockOnTarget)
        {
            Output. DistanceMultiplier = 1.0f;
            return Output;
        }
        
        // 获取目标体型
        float TargetSize = GetTargetSize(Context. LockOnTarget);
        
        // 根据体型曲线计算距离系数
        float DistanceMultiplier = TargetSizeCurve ?  
            TargetSizeCurve->GetFloatValue(TargetSize) : 
            CalculateDefaultMultiplier(TargetSize);
        
        // 考虑玩家与目标的实际距离
        float ActualDistance = FVector::Dist(
            Context.CharacterLocation, 
            Context.LockOnTargetLocation
        );
        
        // 距离过近时额外拉远，保证构图
        if (ActualDistance < MinFramingDistance)
        {
            float ExtraMultiplier = MinFramingDistance / FMath::Max(ActualDistance, 1.0f);
            DistanceMultiplier *= FMath::Min(ExtraMultiplier, MaxDistanceMultiplier);
        }
        
        // 平滑过渡
        float Alpha = 1.0f - FMath::Exp(-DistanceLagSpeed * DeltaTime);
        SmoothedMultiplier = FMath::Lerp(SmoothedMultiplier, DistanceMultiplier, Alpha);
        
        Output.DistanceMultiplier = SmoothedMultiplier;
        
        return Output;
    }
    
private:
    float GetTargetSize(AActor* Target)
    {
        // 尝试从接口获取
        if (auto* SizeInterface = Cast<ICameraSizeInterface>(Target))
        {
            return SizeInterface->GetCameraTargetSize();
        }
        
        // 回退：使用碰撞体估算
        if (auto* CapsuleComp = Target->FindComponentByClass<UCapsuleComponent>())
        {
            return CapsuleComp->GetScaledCapsuleHalfHeight() * 2.0f;
        }
        
        return DefaultTargetSize;
    }
    
    float CalculateDefaultMultiplier(float Size)
    {
        // 默认曲线：体型越大，距离越远
        // Size 100 -> 1.0x
        // Size 300 -> 1. 5x
        // Size 500 -> 2.0x
        // Size 800+ -> 2.5x
        
        if (Size <= 100.0f) return 1.0f;
        if (Size <= 300.0f) return 1.0f + (Size - 100.0f) / 400.0f;
        if (Size <= 500.0f) return 1. 5f + (Size - 300.0f) / 400.0f;
        return FMath::Min(2.0f + (Size - 500.0f) / 600.0f, 2.5f);
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Size")
    UCurveFloat* TargetSizeCurve = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Size")
    float DefaultTargetSize = 180.0f;  // 人形敌人默认高度
    
    UPROPERTY(EditAnywhere, Category = "Framing")
    float MinFramingDistance = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Framing")
    float MaxDistanceMultiplier = 2.5f;
    
    UPROPERTY(EditAnywhere, Category = "Smooth")
    float DistanceLagSpeed = 4.0f;
    
private:
    float SmoothedMultiplier = 1.0f;
};
```

### 2.4. 7 速度FOV模块

```cpp
UCLASS()
class USoulsModule_SpeedFOV : public USoulsCameraModule
{
    GENERATED_BODY()
    
public:
    virtual FSoulsModuleOutput Compute(
        float DeltaTime,
        const FCameraInputContext& Context,
        const FSoulsModuleOutput& PreviousOutput) override
    {
        FSoulsModuleOutput Output;
        
        // 获取角色水平速度
        FVector HorizontalVelocity = Context.CharacterVelocity;
        HorizontalVelocity.Z = 0;
        float Speed = HorizontalVelocity.Size();
        
        // 计算目标FOV偏移（知乎文章1公式）
        float TargetFOVOffset = 0.0f;
        
        if (Speed > SpeedThreshold)
        {
            TargetFOVOffset = (Speed - SpeedThreshold) * FOVPerSpeed;
            TargetFOVOffset = FMath::Clamp(TargetFOVOffset, 0.0f, MaxFOVOffset);
        }
        
        // 平滑过渡
        float Alpha = 1.0f - FMath::Exp(-FOVLagSpeed * DeltaTime);
        SmoothedFOVOffset = FMath::Lerp(SmoothedFOVOffset, TargetFOVOffset, Alpha);
        
        Output.FOVOffset = SmoothedFOVOffset;
        
        return Output;
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Speed FOV")
    float SpeedThreshold = 300.0f;  // 开始增加FOV的速度阈值
    
    UPROPERTY(EditAnywhere, Category = "Speed FOV")
    float FOVPerSpeed = 0.02f;  // 每单位速度增加的FOV
    
    UPROPERTY(EditAnywhere, Category = "Speed FOV")
    float MaxFOVOffset = 15.0f;  // 最大FOV增量
    
    UPROPERTY(EditAnywhere, Category = "Speed FOV")
    float FOVLagSpeed = 5.0f;
    
private:
    float SmoothedFOVOffset = 0.0f;
};
```

---

## 2.5 修改器系统（Modifier System）

修改器与模块的区别：
- **模块（Module）**：持续运行，计算相机基础参数
- **修改器（Modifier）**：事件触发，临时叠加效果

### 2.5.1 修改器基类

```cpp
UCLASS(Abstract, Blueprintable)
class USoulsCameraModifier : public UObject
{
    GENERATED_BODY()
    
public:
    // 激活修改器
    virtual void Activate(const FModifierActivationParams& Params);
    
    // 停用修改器
    virtual void Deactivate();
    
    // 每帧更新，返回是否仍然活跃
    virtual bool Update(float DeltaTime);
    
    // 修改相机输出
    virtual void ModifyOutput(FSoulsCameraOutput& InOutOutput);
    
    // 是否活跃
    bool IsActive() const { return bIsActive; }
    
    // 获取优先级
    int32 GetPriority() const { return Priority; }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Modifier")
    int32 Priority = 0;
    
    UPROPERTY(EditAnywhere, Category = "Modifier")
    float Duration = 0.0f;  // 0 = 无限
    
    UPROPERTY(EditAnywhere, Category = "Modifier")
    UCurveFloat* BlendInCurve = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Modifier")
    UCurveFloat* BlendOutCurve = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Modifier")
    float BlendInTime = 0.1f;
    
    UPROPERTY(EditAnywhere, Category = "Modifier")
    float BlendOutTime = 0.2f;
    
    bool bIsActive = false;
    float ElapsedTime = 0.0f;
    float CurrentWeight = 0.0f;
};
```

### 2.5. 2 受击反应修改器

```cpp
UCLASS()
class USoulsModifier_HitReaction : public USoulsCameraModifier
{
    GENERATED_BODY()
    
public:
    virtual void Activate(const FModifierActivationParams& Params) override
    {
        Super::Activate(Params);
        
        // 从参数获取受击方向和强度
        HitDirection = Params.Direction;
        HitIntensity = Params.Intensity;
        
        // 计算震动方向（屏幕空间）
        CalculateShakeDirection();
    }
    
    virtual void ModifyOutput(FSoulsCameraOutput& InOutOutput) override
    {
        if (!bIsActive) return;
        
        // 计算当前震动强度
        float ShakeStrength = HitIntensity * CurrentWeight;
        
        // 应用位置震动
        float NoiseTime = ElapsedTime * ShakeFrequency;
        FVector PositionShake;
        PositionShake. X = FMath::PerlinNoise1D(NoiseTime) * ShakeStrength * PositionShakeScale;
        PositionShake.Y = FMath::PerlinNoise1D(NoiseTime + 100.0f) * ShakeStrength * PositionShakeScale;
        PositionShake. Z = FMath::PerlinNoise1D(NoiseTime + 200.0f) * ShakeStrength * PositionShakeScale * 0.5f;
        
        InOutOutput.Position += PositionShake;
        
        // 应用旋转震动
        FRotator RotationShake;
        RotationShake. Pitch = FMath::PerlinNoise1D(NoiseTime + 300.0f) * ShakeStrength * RotationShakeScale;
        RotationShake. Yaw = FMath::PerlinNoise1D(NoiseTime + 400.0f) * ShakeStrength * RotationShakeScale * 0.5f;
        RotationShake.Roll = FMath::PerlinNoise1D(NoiseTime + 500. 0f) * ShakeStrength * RotationShakeScale * 0.3f;
        
        InOutOutput. Rotation += RotationShake;
        
        // 应用FOV冲击（可选）
        if (bApplyFOVPunch)
        {
            float FOVPunch = HitIntensity * FOVPunchScale * CurrentWeight;
            InOutOutput.FOV -= FOVPunch;  // 受击时FOV短暂缩小
        }
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Hit Reaction")
    float ShakeFrequency = 30.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hit Reaction")
    float PositionShakeScale = 5.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hit Reaction")
    float RotationShakeScale = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hit Reaction")
    bool bApplyFOVPunch = true;
    
    UPROPERTY(EditAnywhere, Category = "Hit Reaction")
    float FOVPunchScale = 3.0f;
    
private:
    FVector HitDirection;
    float HitIntensity;
    FVector ShakeDirection;
    
    void CalculateShakeDirection();
};
```

### 2.5.3 处决相机修改器

```cpp
UCLASS()
class USoulsModifier_Execution : public USoulsCameraModifier
{
    GENERATED_BODY()
    
public:
    virtual void Activate(const FModifierActivationParams& Params) override
    {
        Super::Activate(Params);
        
        ExecutionTarget = Params.Target;
        ExecutionType = Params.ExecutionType;
        
        // 计算处决相机的理想位置
        CalculateExecutionCameraPosition();
    }
    
    virtual void ModifyOutput(FSoulsCameraOutput& InOutOutput) override
    {
        if (!bIsActive || ! ExecutionTarget) return;
        
        // 根据处决类型应用不同的相机行为
        switch (ExecutionType)
        {
            case EExecutionType::Front:
                ApplyFrontExecutionCamera(InOutOutput);
                break;
            case EExecutionType::Back:
                ApplyBackstabCamera(InOutOutput);
                break;
            case EExecutionType::Riposte:
                ApplyRiposteCamera(InOutOutput);
                break;
        }
        
        // 应用时间缩放效果的相机响应
        if (bUseSlowMotionCompensation)
        {
            // 在慢动作时，相机移动也相应减慢，但保持视觉流畅
            float TimeScale = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
            // 相机可以比实际时间快一点，增强戏剧性
            InOutOutput.bUseRawDeltaTime = true;
        }
    }
    
private:
    void ApplyFrontExecutionCamera(FSoulsCameraOutput& InOutOutput)
    {
        // 计算玩家和目标的中点
        FVector MidPoint = (PlayerLocation + TargetLocation) * 0.5f;
        
        // 相机位于侧面，同时看到双方
        FVector CameraOffset = FVector::CrossProduct(
            (TargetLocation - PlayerLocation).GetSafeNormal(),
            FVector::UpVector
        ) * SideOffset;
        
        FVector IdealPosition = MidPoint + CameraOffset + FVector(0, 0, HeightOffset);
        
        // 平滑过渡到处决相机位置
        InOutOutput.Position = FMath::Lerp(
            InOutOutput.Position,
            IdealPosition,
            CurrentWeight
        );
        
        // 看向动作中心点
        FRotator LookAtRotation = (MidPoint - IdealPosition). Rotation();
        InOutOutput.Rotation = FMath::Lerp(
            InOutOutput. Rotation,
            LookAtRotation,
            CurrentWeight
        );
        
        // 缩小FOV增强戏剧性
        InOutOutput. FOV = FMath::Lerp(InOutOutput.FOV, ExecutionFOV, CurrentWeight);
    }
    
protected:
    UPROPERTY(EditAnywhere, Category = "Execution")
    float SideOffset = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Execution")
    float HeightOffset = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Execution")
    float ExecutionFOV = 55.0f;
    
    UPROPERTY(EditAnywhere, Category = "Execution")
    bool bUseSlowMotionCompensation = true;
    
private:
    AActor* ExecutionTarget = nullptr;
    EExecutionType ExecutionType;
    FVector PlayerLocation;
    FVector TargetLocation;
    
    void ApplyBackstabCamera(FSoulsCameraOutput& InOutOutput);
    void ApplyRiposteCamera(FSoulsCameraOutput& InOutOutput);
    void CalculateExecutionCameraPosition();
};
```

---

## 2. 6 碰撞处理系统

### 2.6.1 碰撞检测策略

```cpp
UCLASS()
class USoulsCameraCollisionResolver : public UObject
{
    GENERATED_BODY()
    
public:
    FSoulsCameraOutput ResolveCollision(
        const FSoulsCameraOutput& DesiredOutput,
        const FCameraInputContext& Context,
        float DeltaTime
    );
    
private:
    // 多射线检测（知乎文章1推荐）
    bool PerformMultiTrace(
        const FVector& Start,
        const FVector& End,
        float ProbeRadius,
        FHitResult& OutHit
    );
    
    // 遮挡检测
    bool CheckOcclusion(
        const FVector& CameraLocation,
        const FVector& TargetLocation,
        TArray<AActor*>& OutOccludingActors
    );
    
    // 计算安全位置
    FVector CalculateSafePosition(
        const FVector& DesiredPosition,
        const FVector& FocusPoint,
        const FHitResult& Hit
    );
    
    // 处理遮挡Actor（透明化或隐藏）
    void HandleOccludingActors(const TArray<AActor*>& Actors);
    
    // 恢复碰撞后的相机距离
    void UpdateCollisionRecovery(float DeltaTime);
    
private:
    // 碰撞配置
    UPROPERTY(EditAnywhere, Category = "Collision")
    float ProbeRadius = 12.0f;
    
    UPROPERTY(EditAnywhere, Category = "Collision")
    TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Camera;
    
    UPROPERTY(EditAnywhere, Category = "Collision")
    float MinDistanceFromSurface = 10.0f;
    
    // 恢复配置
    UPROPERTY(EditAnywhere, Category = "Recovery")
    float RecoverySpeed = 3.0f;  // 碰撞恢复速度
    
    UPROPERTY(EditAnywhere, Category = "Recovery")
    float RecoveryDelay = 0. 2f;  // 恢复延迟
    
    // 多射线配置
    UPROPERTY(EditAnywhere, Category = "Multi Trace")
    bool bUseMultiTrace = true;
    
    UPROPERTY(EditAnywhere, Category = "Multi Trace")
    int32 TraceCount = 5;  // 射线数量
    
    UPROPERTY(EditAnywhere, Category = "Multi Trace")
    float TraceSpreadAngle = 15.0f;  // 射线扩散角度
    
    // 遮挡处理配置
    UPROPERTY(EditAnywhere, Category = "Occlusion")
    bool bFadeOccludingActors = true;
    
    UPROPERTY(EditAnywhere, Category = "Occlusion")
    float FadeSpeed = 5.0f;
    
    UPROPERTY(EditAnywhere, Category = "Occlusion")
    float MinFadeAlpha = 0. 3f;
    
    // 状态
    float CurrentCollisionDistance = 0.0f;
    float DesiredCollisionDistance = 0.0f;
    float TimeSinceLastCollision = 0. 0f;
    TArray<TWeakObjectPtr<AActor>> CurrentlyFadedActors;
};
```

### 2.6. 2 多射线检测实现

```cpp
bool USoulsCameraCollisionResolver::PerformMultiTrace(
    const FVector& Start,
    const FVector& End,
    float ProbeRadius,
    FHitResult& OutHit)
{
    if (! bUseMultiTrace)
    {
        // 单射线回退
        return GetWorld()->SweepSingleByChannel(
            OutHit, Start, End, FQuat::Identity,
            CollisionChannel,
            FCollisionShape::MakeSphere(ProbeRadius),
            FCollisionQueryParams(NAME_None, false, Context. OwnerCharacter)
        );
    }
    
    // 多射线检测
    FVector Direction = (End - Start).GetSafeNormal();
    float Distance = FVector::Dist(Start, End);
    
    // 构建射线起点（围绕中心点的圆形分布）
    TArray<FVector> TraceStarts;
    TraceStarts.Add(Start);  // 中心射线
    
    FVector Right = FVector::CrossProduct(Direction, FVector::UpVector). GetSafeNormal();
    FVector Up = FVector::CrossProduct(Right, Direction).GetSafeNormal();
    
    float AngleStep = 360.0f / (TraceCount - 1);
    float SpreadDistance = FMath::Tan(FMath::DegreesToRadians(TraceSpreadAngle)) * Distance;
    
    for (int32 i = 1; i < TraceCount; ++i)
    {
        float Angle = FMath::DegreesToRadians(AngleStep * (i - 1));
        FVector Offset = (Right * FMath::Cos(Angle) + Up * FMath::Sin(Angle)) * SpreadDistance * 0.1f;
        TraceStarts.Add(Start + Offset);
    }
    
    // 执行所有射线检测，取最近的碰撞点
    float NearestDistance = Distance;
    bool bHasHit = false;
    
    for (const FVector& TraceStart : TraceStarts)
    {
        FHitResult Hit;
        FVector TraceEnd = TraceStart + Direction * Distance;
        
        if (GetWorld()->SweepSingleByChannel(
            Hit, TraceStart, TraceEnd, FQuat::Identity,
            CollisionChannel,
            FCollisionShape::MakeSphere(ProbeRadius),
            FCollisionQueryParams(NAME_None, false, Context.OwnerCharacter)))
        {
            if (Hit.Distance < NearestDistance)
            {
                NearestDistance = Hit.Distance;
                OutHit = Hit;
                bHasHit = true;
            }
        }
    }
    
    return bHasHit;
}
```

---


## 2.7 主Pipeline实现

### 2.7. 1 Pipeline组件定义

```cpp
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class USoulsCameraPipeline : public UActorComponent
{
    GENERATED_BODY()
    
public:
    USoulsCameraPipeline();
    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;
    
    // 公共接口
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetLockOnTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ClearLockOnTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ActivateModifier(TSubclassOf<USoulsCameraModifier> ModifierClass, 
        const FModifierActivationParams& Params);
    
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ForceState(ESoulsCameraState State, float Duration = -1.0f);
    
    // 获取当前相机数据
    UFUNCTION(BlueprintPure, Category = "Camera")
    FSoulsCameraOutput GetCurrentCameraOutput() const { return CurrentOutput; }
    
protected:
    // Pipeline阶段
    void GatherInputContext(float DeltaTime);
    void UpdateStateMachine(float DeltaTime);
    void ComputeModules(float DeltaTime);
    void BlendAndSolve(float DeltaTime);
    void ResolveCollisions(float DeltaTime);
    void ApplyPostProcess(float DeltaTime);
    void ApplyToCamera(float DeltaTime);
    
    // 模块管理
    void InitializeModules();
    TArray<USoulsCameraModule*> GetActiveModules() const;
    
protected:
    // 状态机
    UPROPERTY()
    USoulsCameraStateMachine* StateMachine;
    
    // 模块列表
    UPROPERTY(EditAnywhere, Instanced, Category = "Modules")
    TArray<USoulsCameraModule*> PositionModules;
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Modules")
    TArray<USoulsCameraModule*> RotationModules;
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Modules")
    TArray<USoulsCameraModule*> DistanceModules;
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Modules")
    TArray<USoulsCameraModule*> FOVModules;
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Modules")
    TArray<USoulsCameraModule*> PostProcessModules;
    
    // 修改器栈
    UPROPERTY()
    TArray<USoulsCameraModifier*> ActiveModifiers;
    
    // 碰撞解析器
    UPROPERTY()
    USoulsCameraCollisionResolver* CollisionResolver;
    
    // 当前帧数据
    FCameraInputContext CurrentContext;
    FSoulsCameraOutput CurrentOutput;
    FSoulsCameraOutput PreviousOutput;
    
    // 模块输出缓存
    TArray<FModuleOutputEntry> ModuleOutputCache;
    
    // 引用
    UPROPERTY()
    ACharacter* OwnerCharacter;
    
    UPROPERTY()
    USpringArmComponent* SpringArm;
    
    UPROPERTY()
    UCameraComponent* Camera;
    
    // 当前锁定目标
    UPROPERTY()
    AActor* CurrentLockOnTarget;
    
    // 配置
    UPROPERTY(EditAnywhere, Category = "Config")
    FSoulsCameraConfig DefaultConfig;
    
    // 平滑配置
    UPROPERTY(EditAnywhere, Category = "Smoothing")
    bool bApplyFinalSmoothing = true;
    
    UPROPERTY(EditAnywhere, Category = "Smoothing")
    float FinalSmoothingSpeed = 20.0f;
    
    // FOV限制
    UPROPERTY(EditAnywhere, Category = "FOV")
    float MinFOV = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "FOV")
    float MaxFOV = 90.0f;
};
```

### 2.7. 2 Pipeline核心更新循环

```cpp
void USoulsCameraPipeline::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (! OwnerCharacter || !SpringArm || !Camera)
    {
        return;
    }
    
    // 保存上一帧输出
    PreviousOutput = CurrentOutput;
    
    // === STAGE 1: 收集输入上下文 ===
    GatherInputContext(DeltaTime);
    
    // === STAGE 2: 更新状态机 ===
    UpdateStateMachine(DeltaTime);
    
    // === STAGE 3: 计算所有模块 ===
    ComputeModules(DeltaTime);
    
    // === STAGE 4: 混合与冲突解决 ===
    BlendAndSolve(DeltaTime);
    
    // === STAGE 5: 碰撞检测与修正 ===
    ResolveCollisions(DeltaTime);
    
    // === STAGE 6: 应用后处理效果 ===
    ApplyPostProcess(DeltaTime);
    
    // === STAGE 7: 应用到实际相机 ===
    ApplyToCamera(DeltaTime);
}
```

### 2.7.3 输入上下文收集

```cpp
void USoulsCameraPipeline::GatherInputContext(float DeltaTime)
{
    CurrentContext.DeltaTime = DeltaTime;
    CurrentContext.TotalTime += DeltaTime;
    
    // 角色数据
    CurrentContext.CharacterLocation = OwnerCharacter->GetActorLocation();
    CurrentContext.CharacterRotation = OwnerCharacter->GetActorRotation();
    CurrentContext.CharacterVelocity = OwnerCharacter->GetVelocity();
    CurrentContext.CharacterMesh = OwnerCharacter->GetMesh();
    
    // 玩家输入
    if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        float YawInput, PitchInput;
        PC->GetInputMouseDelta(YawInput, PitchInput);
        CurrentContext.PlayerYawInput = YawInput;
        CurrentContext.PlayerPitchInput = PitchInput;
        
        // 手柄输入
        CurrentContext.RightStickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
        CurrentContext.RightStickY = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
    }
    
    // 锁定目标数据
    CurrentContext.LockOnTarget = CurrentLockOnTarget;
    if (CurrentLockOnTarget)
    {
        CurrentContext.LockOnTargetLocation = CurrentLockOnTarget->GetActorLocation();
        
        // 获取目标锁定点（如果有接口）
        if (auto* LockInterface = Cast<ICameraLockOnInterface>(CurrentLockOnTarget))
        {
            CurrentContext. LockOnTargetLocation = LockInterface->GetLockOnLocation();
            CurrentContext. TargetSize = LockInterface->GetCameraTargetSize();
        }
        
        CurrentContext.bHasLockTarget = true;
    }
    else
    {
        CurrentContext.bHasLockTarget = false;
    }
    
    // 战斗状态（需要根据项目实际组件调整）
    if (auto* CombatComp = OwnerCharacter->FindComponentByClass<UCombatComponent>())
    {
        CurrentContext.bInCombat = CombatComp->IsInCombat();
        CurrentContext.bIsAttacking = CombatComp->IsAttacking();
        CurrentContext.bIsDodging = CombatComp->IsDodging();
    }
    
    // 角色状态
    CurrentContext.bIsSprinting = OwnerCharacter->GetCharacterMovement()->IsSprinting();
    CurrentContext.bIsCrouching = OwnerCharacter->bIsCrouched;
    CurrentContext.bIsInAir = OwnerCharacter->GetCharacterMovement()->IsFalling();
    
    // 当前相机状态
    CurrentContext.CurrentCameraState = StateMachine->GetCurrentState();
    CurrentContext.PreviousCameraState = StateMachine->GetPreviousState();
    CurrentContext.StateBlendAlpha = StateMachine->GetBlendAlpha();
}
```

### 2.7.4 模块计算

```cpp
void USoulsCameraPipeline::ComputeModules(float DeltaTime)
{
    ESoulsCameraState CurrentState = StateMachine->GetCurrentState();
    
    // 清空模块输出缓存
    ModuleOutputCache.Empty();
    
    // 计算位置模块
    for (USoulsCameraModule* Module : PositionModules)
    {
        if (Module && Module->IsActiveInState(CurrentState))
        {
            FSoulsModuleOutput Output = Module->Compute(DeltaTime, CurrentContext, PreviousOutput);
            if (Output.bIsValid)
            {
                ModuleOutputCache.Add(FModuleOutputEntry{
                    Module->GetPriority(),
                    Module->GetBlendPolicy(),
                    EModuleCategory::Position,
                    Output
                });
            }
        }
    }
    
    // 计算旋转模块
    for (USoulsCameraModule* Module : RotationModules)
    {
        if (Module && Module->IsActiveInState(CurrentState))
        {
            FSoulsModuleOutput Output = Module->Compute(DeltaTime, CurrentContext, PreviousOutput);
            if (Output.bIsValid)
            {
                ModuleOutputCache.Add(FModuleOutputEntry{
                    Module->GetPriority(),
                    Module->GetBlendPolicy(),
                    EModuleCategory::Rotation,
                    Output
                });
            }
        }
    }
    
    // 计算距离模块
    for (USoulsCameraModule* Module : DistanceModules)
    {
        if (Module && Module->IsActiveInState(CurrentState))
        {
            FSoulsModuleOutput Output = Module->Compute(DeltaTime, CurrentContext, PreviousOutput);
            if (Output.bIsValid)
            {
                ModuleOutputCache.Add(FModuleOutputEntry{
                    Module->GetPriority(),
                    Module->GetBlendPolicy(),
                    EModuleCategory::Distance,
                    Output
                });
            }
        }
    }
    
    // 计算FOV模块
    for (USoulsCameraModule* Module : FOVModules)
    {
        if (Module && Module->IsActiveInState(CurrentState))
        {
            FSoulsModuleOutput Output = Module->Compute(DeltaTime, CurrentContext, PreviousOutput);
            if (Output.bIsValid)
            {
                ModuleOutputCache. Add(FModuleOutputEntry{
                    Module->GetPriority(),
                    Module->GetBlendPolicy(),
                    EModuleCategory::FOV,
                    Output
                });
            }
        }
    }
}
```

### 2.7.5 混合与冲突解决

```cpp
void USoulsCameraPipeline::BlendAndSolve(float DeltaTime)
{
    // 按优先级排序
    ModuleOutputCache.Sort([](const FModuleOutputEntry& A, const FModuleOutputEntry& B) {
        return A.Priority < B.Priority;
    });
    
    // 初始化输出（从配置获取默认值）
    FSoulsCameraOutput BaseOutput = GetDefaultOutputForState(StateMachine->GetCurrentState());
    
    // === 位置处理 ===
    FVector AccumulatedPosition = BaseOutput.FocusPoint;
    FVector PositionOverride = FVector::ZeroVector;
    bool bHasPositionOverride = false;
    
    for (const FModuleOutputEntry& Entry : ModuleOutputCache)
    {
        if (Entry. Category != EModuleCategory::Position) continue;
        
        const FSoulsModuleOutput& Output = Entry.Output;
        
        if (Output. bOverridePosition)
        {
            PositionOverride = Output.PositionOverride;
            bHasPositionOverride = true;
        }
        else
        {
            AccumulatedPosition += Output. PositionOffset * Output.OutputWeight;
        }
    }
    
    CurrentOutput. FocusPoint = bHasPositionOverride ? PositionOverride : AccumulatedPosition;
    
    // === 旋转处理 ===
    FRotator AccumulatedRotation = BaseOutput. Rotation;
    FRotator RotationOverride = FRotator::ZeroRotator;
    bool bHasRotationOverride = false;
    
    for (const FModuleOutputEntry& Entry : ModuleOutputCache)
    {
        if (Entry.Category != EModuleCategory::Rotation) continue;
        
        const FSoulsModuleOutput& Output = Entry.Output;
        
        if (Output.bOverrideRotation)
        {
            RotationOverride = Output.RotationOverride;
            bHasRotationOverride = true;
        }
        else
        {
            AccumulatedRotation += Output.RotationOffset * Output. OutputWeight;
        }
    }
    
    CurrentOutput.Rotation = bHasRotationOverride ? RotationOverride : AccumulatedRotation;
    
    // === 距离处理 ===
    float BaseDistance = GetBaseDistanceForState(StateMachine->GetCurrentState());
    float DistanceMultiplier = 1.0f;
    float DistanceOffset = 0.0f;
    float DistanceOverride = 0.0f;
    bool bHasDistanceOverride = false;
    
    for (const FModuleOutputEntry& Entry : ModuleOutputCache)
    {
        if (Entry.Category != EModuleCategory::Distance) continue;
        
        const FSoulsModuleOutput& Output = Entry.Output;
        
        if (Output.bOverrideDistance)
        {
            DistanceOverride = Output. DistanceOverride;
            bHasDistanceOverride = true;
        }
        else
        {
            DistanceMultiplier *= Output. DistanceMultiplier;
            DistanceOffset += Output. DistanceOffset;
        }
    }
    
    CurrentOutput.Distance = bHasDistanceOverride ? 
        DistanceOverride : 
        (BaseDistance * DistanceMultiplier + DistanceOffset);
    
    // === FOV处理 ===
    float BaseFOV = GetBaseFOVForState(StateMachine->GetCurrentState());
    float FOVOffset = 0.0f;
    float FOVOverride = 0. 0f;
    bool bHasFOVOverride = false;
    
    for (const FModuleOutputEntry& Entry : ModuleOutputCache)
    {
        if (Entry.Category != EModuleCategory::FOV) continue;
        
        const FSoulsModuleOutput& Output = Entry. Output;
        
        if (Output. bOverrideFOV)
        {
            FOVOverride = Output.FOVOverride;
            bHasFOVOverride = true;
        }
        else
        {
            FOVOffset += Output. FOVOffset * Output.OutputWeight;
        }
    }
    
    CurrentOutput.FOV = bHasFOVOverride ?  FOVOverride : (BaseFOV + FOVOffset);
    CurrentOutput.FOV = FMath::Clamp(CurrentOutput.FOV, MinFOV, MaxFOV);
    
    // === 状态切换混合 ===
    if (StateMachine->IsTransitioning())
    {
        float BlendAlpha = StateMachine->GetBlendAlpha();
        
        FSoulsCameraOutput PreviousStateOutput = GetOutputForState(
            StateMachine->GetPreviousState(), 
            CurrentContext
        );
        
        // 混合位置
        CurrentOutput.FocusPoint = FMath::Lerp(
            PreviousStateOutput. FocusPoint,
            CurrentOutput. FocusPoint,
            BlendAlpha
        );
        
        // 混合旋转（使用Slerp）
        CurrentOutput. Rotation = FQuat::Slerp(
            PreviousStateOutput. Rotation. Quaternion(),
            CurrentOutput.Rotation. Quaternion(),
            BlendAlpha
        ). Rotator();
        
        // 混合距离
        CurrentOutput.Distance = FMath::Lerp(
            PreviousStateOutput.Distance,
            CurrentOutput.Distance,
            BlendAlpha
        );
        
        // 混合FOV
        CurrentOutput.FOV = FMath::Lerp(
            PreviousStateOutput.FOV,
            CurrentOutput.FOV,
            BlendAlpha
        );
    }
    
    // === 计算最终相机位置 ===
    CalculateFinalCameraTransform();
}

void USoulsCameraPipeline::CalculateFinalCameraTransform()
{
    // 从焦点位置、旋转、距离计算相机世界位置
    FVector ArmDirection = CurrentOutput. Rotation.Vector() * -1.0f;
    
    CurrentOutput.Position = CurrentOutput.FocusPoint + ArmDirection * CurrentOutput.Distance;
    
    // 应用Socket偏移（屏幕空间偏移）
    FVector SocketOffset = GetSocketOffsetForState(StateMachine->GetCurrentState());
    FVector WorldSocketOffset = CurrentOutput. Rotation.RotateVector(SocketOffset);
    CurrentOutput.Position += WorldSocketOffset;
}
```

### 2.7.6 碰撞解决与后处理

```cpp
void USoulsCameraPipeline::ResolveCollisions(float DeltaTime)
{
    if (! CollisionResolver) return;
    
    CurrentOutput = CollisionResolver->ResolveCollision(
        CurrentOutput,
        CurrentContext,
        DeltaTime
    );
}

void USoulsCameraPipeline::ApplyPostProcess(float DeltaTime)
{
    // 更新所有活跃的修改器
    for (int32 i = ActiveModifiers.Num() - 1; i >= 0; --i)
    {
        USoulsCameraModifier* Modifier = ActiveModifiers[i];
        
        if (! Modifier->Update(DeltaTime))
        {
            Modifier->Deactivate();
            ActiveModifiers.RemoveAt(i);
        }
        else
        {
            Modifier->ModifyOutput(CurrentOutput);
        }
    }
    
    // 应用后处理模块
    for (USoulsCameraModule* Module : PostProcessModules)
    {
        if (Module && Module->IsActiveInState(StateMachine->GetCurrentState()))
        {
            FSoulsModuleOutput Output = Module->Compute(DeltaTime, CurrentContext, PreviousOutput);
            
            CurrentOutput. Position += Output. PositionOffset;
            CurrentOutput. Rotation += Output. RotationOffset;
        }
    }
    
    // 最终平滑
    if (bApplyFinalSmoothing)
    {
        float SmoothAlpha = 1.0f - FMath::Exp(-FinalSmoothingSpeed * DeltaTime);
        
        CurrentOutput.Position = FMath::Lerp(PreviousOutput.Position, CurrentOutput.Position, SmoothAlpha);
        CurrentOutput.Rotation = FMath::Lerp(PreviousOutput.Rotation, CurrentOutput. Rotation, SmoothAlpha);
        CurrentOutput.FOV = FMath::Lerp(PreviousOutput.FOV, CurrentOutput.FOV, SmoothAlpha);
    }
}

void USoulsCameraPipeline::ApplyToCamera(float DeltaTime)
{
    if (!SpringArm || !Camera) return;
    
    // 应用到SpringArm
    SpringArm->TargetArmLength = CurrentOutput.Distance;
    SpringArm->SetWorldLocation(CurrentOutput.FocusPoint);
    SpringArm->SetWorldRotation(CurrentOutput.Rotation);
    
    // 应用Socket偏移
    FVector SocketOffset = GetSocketOffsetForState(StateMachine->GetCurrentState());
    SpringArm->SocketOffset = SocketOffset;
    
    // 应用FOV
    Camera->SetFieldOfView(CurrentOutput.FOV);
    
    // 应用后处理设置
    if (CurrentOutput.bApplyPostProcessSettings)
    {
        Camera->PostProcessSettings = CurrentOutput.PostProcessSettings;
        Camera->PostProcessBlendWeight = CurrentOutput.PostProcessBlendWeight;
    }
}
```

---

# 第三部分：状态详细配置

## 3.1 探索状态配置

```cpp
USTRUCT(BlueprintType)
struct FSoulsExploreStateConfig
{
    GENERATED_BODY()
    
    // 距离配置
    UPROPERTY(EditAnywhere, Category = "Distance")
    float BaseDistance = 400.0f;
    
    UPROPERTY(EditAnywhere, Category = "Distance")
    float MinDistance = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Distance")
    float MaxDistance = 600.0f;
    
    // FOV配置
    UPROPERTY(EditAnywhere, Category = "FOV")
    float BaseFOV = 70.0f;
    
    UPROPERTY(EditAnywhere, Category = "FOV")
    float SprintFOV = 80.0f;
    
    // 偏移配置
    UPROPERTY(EditAnywhere, Category = "Offset")
    FVector FocusOffset = FVector(0, 0, 80.0f);
    
    UPROPERTY(EditAnywhere, Category = "Offset")
    FVector SocketOffset = FVector(0, 50.0f, 0);
    
    // Pitch限制
    UPROPERTY(EditAnywhere, Category = "Rotation")
    float MinPitch = -60.0f;
    
    UPROPERTY(EditAnywhere, Category = "Rotation")
    float MaxPitch = 70.0f;
    
    // 惯性配置
    UPROPERTY(EditAnywhere, Category = "Lag")
    float PositionLagSpeed = 8.0f;
    
    UPROPERTY(EditAnywhere, Category = "Lag")
    float RotationLagSpeed = 12.0f;
    
    UPROPERTY(EditAnywhere, Category = "Lag")
    float VerticalLagSpeed = 4.0f;
    
    // 自动校正
    UPROPERTY(EditAnywhere, Category = "Auto Correct")
    bool bEnableAutoCorrect = true;
    
    UPROPERTY(EditAnywhere, Category = "Auto Correct")
    float AutoCorrectDelay = 3.0f;
    
    UPROPERTY(EditAnywhere, Category = "Auto Correct")
    float AutoCorrectSpeed = 2.0f;
};
```

## 3.2 锁定状态配置

```cpp
USTRUCT(BlueprintType)
struct FSoulsLockOnStateConfig
{
    GENERATED_BODY()
    
    // 距离配置
    UPROPERTY(EditAnywhere, Category = "Distance")
    float BaseDistance = 350.0f;
    
    UPROPERTY(EditAnywhere, Category = "Distance")
    float MinDistanceToTarget = 150.0f;
    
    UPROPERTY(EditAnywhere, Category = "Distance")
    float MaxDistanceToTarget = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Distance")
    UCurveFloat* DistanceByTargetSizeCurve = nullptr;
    
    // FOV配置
    UPROPERTY(EditAnywhere, Category = "FOV")
    float BaseFOV = 65.0f;
    
    // 构图配置
    UPROPERTY(EditAnywhere, Category = "Framing")
    FVector2D PlayerScreenPosition = FVector2D(0. 35f, 0. 5f);
    
    UPROPERTY(EditAnywhere, Category = "Framing")
    FVector2D TargetScreenPosition = FVector2D(0.65f, 0.5f);
    
    UPROPERTY(EditAnywhere, Category = "Framing")
    float FramingWeight = 0.7f;
    
    // 软锁定配置
    UPROPERTY(EditAnywhere, Category = "Soft Lock")
    bool bEnableSoftLock = true;
    
    UPROPERTY(EditAnywhere, Category = "Soft Lock")
    float SoftLockStrength = 0. 3f;
    
    UPROPERTY(EditAnywhere, Category = "Soft Lock")
    float SoftLockReturnSpeed = 5.0f;
    
    // Pitch配置
    UPROPERTY(EditAnywhere, Category = "Rotation")
    float MinPitch = -30.0f;
    
    UPROPERTY(EditAnywhere, Category = "Rotation")
    float MaxPitch = 45.0f;
    
    UPROPERTY(EditAnywhere, Category = "Rotation")
    bool bAutoAdjustPitchForHeight = true;
    
    // 目标切换
    UPROPERTY(EditAnywhere, Category = "Target Switch")
    float TargetSwitchBlendTime = 0.15f;
    
    UPROPERTY(EditAnywhere, Category = "Target Switch")
    UCurveFloat* TargetSwitchCurve = nullptr;
    
    // 跟随速度
    UPROPERTY(EditAnywhere, Category = "Follow")
    float YawLagSpeed = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Follow")
    float PitchLagSpeed = 8.0f;
    
    UPROPERTY(EditAnywhere, Category = "Follow")
    float PositionLagSpeed = 12.0f;
};
```

## 3.3 Boss战状态配置

```cpp
USTRUCT(BlueprintType)
struct FSoulsBossStateConfig
{
    GENERATED_BODY()
    
    // Boss体型分类阈值
    UPROPERTY(EditAnywhere, Category = "Size Classification")
    float MediumBossThreshold = 300.0f;
    
    UPROPERTY(EditAnywhere, Category = "Size Classification")
    float LargeBossThreshold = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Size Classification")
    float HugeBossThreshold = 800.0f;
    
    // 中型Boss配置
    UPROPERTY(EditAnywhere, Category = "Medium Boss")
    float MediumBossDistance = 450.0f;
    
    UPROPERTY(EditAnywhere, Category = "Medium Boss")
    float MediumBossFOV = 70.0f;
    
    UPROPERTY(EditAnywhere, Category = "Medium Boss")
    float MediumBossPitchOffset = 5.0f;
    
    // 大型Boss配置
    UPROPERTY(EditAnywhere, Category = "Large Boss")
    float LargeBossDistance = 600.0f;
    
    UPROPERTY(EditAnywhere, Category = "Large Boss")
    float LargeBossFOV = 75.0f;
    
    UPROPERTY(EditAnywhere, Category = "Large Boss")
    float LargeBossPitchOffset = 10.0f;
    
    // 巨型Boss配置
    UPROPERTY(EditAnywhere, Category = "Huge Boss")
    float HugeBossDistance = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Huge Boss")
    float HugeBossFOV = 80.0f;
    
    UPROPERTY(EditAnywhere, Category = "Huge Boss")
    float HugeBossPitchOffset = 15.0f;
    
    UPROPERTY(EditAnywhere, Category = "Huge Boss")
    FVector HugeBossFocusOffset = FVector(0, 0, -100.0f);
    
    // 阶段转换
    UPROPERTY(EditAnywhere, Category = "Phase Transition")
    float PhaseTransitionZoomOut = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Phase Transition")
    float PhaseTransitionFOVIncrease = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Phase Transition")
    float PhaseTransitionDuration = 2. 0f;
    
    // 攻击响应
    UPROPERTY(EditAnywhere, Category = "Attack Response")
    float AttackZoomIntensity = 30.0f;
    
    UPROPERTY(EditAnywhere, Category = "Attack Response")
    float AttackShakeIntensity = 1.5f;
};
```

## 3.4 处决状态配置

```cpp
USTRUCT(BlueprintType)
struct FSoulsExecutionStateConfig
{
    GENERATED_BODY()
    
    // 相机位置
    UPROPERTY(EditAnywhere, Category = "Position")
    float SideOffset = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Position")
    float HeightOffset = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Position")
    float DistanceFromMidpoint = 250.0f;
    
    // FOV
    UPROPERTY(EditAnywhere, Category = "FOV")
    float ExecutionFOV = 55.0f;
    
    // 时间
    UPROPERTY(EditAnywhere, Category = "Timing")
    float BlendInTime = 0. 15f;
    
    UPROPERTY(EditAnywhere, Category = "Timing")
    float BlendOutTime = 0.3f;
    
    // 不同处决类型配置
    UPROPERTY(EditAnywhere, Category = "Execution Types")
    TMap<EExecutionType, FExecutionCameraPreset> ExecutionPresets;
    
    // 慢动作补偿
    UPROPERTY(EditAnywhere, Category = "Slow Motion")
    bool bCompensateForSlowMotion = true;
    
    UPROPERTY(EditAnywhere, Category = "Slow Motion")
    float SlowMotionCameraSpeedMultiplier = 1.5f;
};

USTRUCT(BlueprintType)
struct FExecutionCameraPreset
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    FVector RelativeCameraOffset = FVector(-200, 150, 50);
    
    UPROPERTY(EditAnywhere)
    float FOV = 55.0f;
    
    UPROPERTY(EditAnywhere)
    bool bLookAtMidpoint = true;
    
    UPROPERTY(EditAnywhere)
    FVector LookAtOffset = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    UCurveFloat* BlendCurve = nullptr;
};
```

---

# 第四部分：数据结构定义

## 4.1 输入上下文

```cpp
USTRUCT(BlueprintType)
struct FCameraInputContext
{
    GENERATED_BODY()
    
    // === 时间 ===
    float DeltaTime = 0.0f;
    float TotalTime = 0.0f;
    
    // === 角色数据 ===
    FVector CharacterLocation = FVector::ZeroVector;
    FRotator CharacterRotation = FRotator::ZeroRotator;
    FVector CharacterVelocity = FVector::ZeroVector;
    
    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh = nullptr;
    
    // === 玩家输入 ===
    float PlayerYawInput = 0.0f;
    float PlayerPitchInput = 0.0f;
    float RightStickX = 0.0f;
    float RightStickY = 0.0f;
    float TimeSinceLastInput = 0.0f;
    
    // === 锁定目标 ===
    UPROPERTY()
    AActor* LockOnTarget = nullptr;
    
    FVector LockOnTargetLocation = FVector::ZeroVector;
    float TargetSize = 180.0f;
    bool bHasLockTarget = false;
    
    // === 战斗状态 ===
    bool bInCombat = false;
    bool bIsAttacking = false;
    bool bIsDodging = false;
    bool bIsParrying = false;
    bool bExecutionTriggered = false;
    
    // === 角色状态 ===
    bool bIsSprinting = false;
    bool bIsCrouching = false;
    bool bIsInAir = false;
    bool bIsMounting = false;
    bool bPlayerDead = false;
    
    // === 相机状态 ===
    ESoulsCameraState CurrentCameraState = ESoulsCameraState::Explore;
    ESoulsCameraState PreviousCameraState = ESoulsCameraState::Explore;
    float StateBlendAlpha = 1.0f;
    
    // === 环境数据 ===
    bool bInTightSpace = false;
    float EnvironmentDistanceLimit = 0.0f;
};
```

## 4.2 相机输出

```cpp
USTRUCT(BlueprintType)
struct FSoulsCameraOutput
{
    GENERATED_BODY()
    
    // === 核心Transform ===
    FVector Position = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    FVector FocusPoint = FVector::ZeroVector;
    float Distance = 400.0f;
    
    // === 视野 ===
    float FOV = 70.0f;
    
    // === SpringArm参数 ===
    FVector SocketOffset = FVector::ZeroVector;
    FVector TargetOffset = FVector::ZeroVector;
    
    // === 碰撞状态 ===
    bool bIsColliding = false;
    float CollisionAdjustedDistance = 0.0f;
    FVector CollisionNormal = FVector::ZeroVector;
    
    // === 后处理 ===
    bool bApplyPostProcessSettings = false;
    FPostProcessSettings PostProcessSettings;
    float PostProcessBlendWeight = 1.0f;
    
    // === 元数据 ===
    bool bUseRawDeltaTime = false;
    ESoulsCameraState ActiveState = ESoulsCameraState::Explore;
};
```

## 4.3 全局配置

```cpp
USTRUCT(BlueprintType)
struct FSoulsCameraConfig
{
    GENERATED_BODY()
    
    // === 状态配置 ===
    UPROPERTY(EditAnywhere, Category = "States")
    FSoulsExploreStateConfig ExploreConfig;
    
    UPROPERTY(EditAnywhere, Category = "States")
    FSoulsLockOnStateConfig LockOnConfig;
    
    UPROPERTY(EditAnywhere, Category = "States")
    FSoulsBossStateConfig BossConfig;
    
    UPROPERTY(EditAnywhere, Category = "States")
    FSoulsExecutionStateConfig ExecutionConfig;
    
    // === 全局参数 ===
    UPROPERTY(EditAnywhere, Category = "Global")
    float GlobalFOVMin = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Global")
    float GlobalFOVMax = 90.0f;
    
    UPROPERTY(EditAnywhere, Category = "Global")
    float GlobalDistanceMin = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "Global")
    float GlobalDistanceMax = 1000.0f;
    
    // === 碰撞配置 ===
    UPROPERTY(EditAnywhere, Category = "Collision")
    float CollisionProbeRadius = 12.0f;
    
    UPROPERTY(EditAnywhere, Category = "Collision")
    TEnumAsByte<ECollisionChannel> CameraCollisionChannel = ECC_Camera;
    
    UPROPERTY(EditAnywhere, Category = "Collision")
    float CollisionRecoverySpeed = 3.0f;
    
    // === 平滑配置 ===
    UPROPERTY(EditAnywhere, Category = "Smoothing")
    bool bEnableFinalSmoothing = true;
    
    UPROPERTY(EditAnywhere, Category = "Smoothing")
    float FinalSmoothingSpeed = 20.0f;
};
```

## 4.4 模块输出缓存条目

```cpp
USTRUCT()
struct FModuleOutputEntry
{
    GENERATED_BODY()
    
    int32 Priority = 0;
    EModuleBlendPolicy BlendPolicy = EModuleBlendPolicy::Additive;
    EModuleCategory Category = EModuleCategory::Position;
    FSoulsModuleOutput Output;
};

UENUM()
enum class EModuleCategory : uint8
{
    Position,
    Rotation,
    Distance,
    FOV,
    PostProcess
};
```

---

# 第五部分：接口定义

## 5.1 锁定目标接口

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UCameraLockOnInterface : public UInterface
{
    GENERATED_BODY()
};

class ICameraLockOnInterface
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    FVector GetLockOnLocation() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    float GetCameraTargetSize() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    bool CanBeLocked() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    int32 GetLockOnPriority() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    FSoulsTargetCameraOverride GetCameraOverride() const;
};
```

## 5.2 相机事件接口

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UCameraEventInterface : public UInterface
{
    GENERATED_BODY()
};

class ICameraEventInterface
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Events")
    void OnExecutionStarted(AActor* Target, EExecutionType Type);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Events")
    void OnExecutionEnded();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Events")
    void OnBossPhaseChanged(int32 NewPhase);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Events")
    void OnHitReceived(const FVector& HitDirection, float Intensity);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Events")
    void OnParrySuccess(AActor* ParriedActor);
};
```

## 5.3 目标相机覆盖结构

```cpp
USTRUCT(BlueprintType)
struct FSoulsTargetCameraOverride
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverrideDistance = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceOverride = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverrideFOV = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FOVOverride = 70.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverridePitchLimits = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinPitchOverride = -30. 0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxPitchOverride = 45. 0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LockOnPointOffset = FVector::ZeroVector;
};
```

---

# 第六部分：调试系统

## 6.1 调试组件

```cpp
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class USoulsCameraDebugComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
    
    // 调试开关
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugInfo = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawCameraPath = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawCollisionProbes = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawFocusPoint = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawTargetFraming = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bLogStateChanges = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bLogModuleOutputs = false;
    
private:
    void DrawDebugHUD();
    void DrawCameraPath();
    void DrawCollisionProbes();
    void DrawFocusPoint();
    void DrawTargetFraming();
    
    TArray<FVector> CameraPathHistory;
    static const int32 MaxPathHistoryLength = 120;
    
    UPROPERTY()
    USoulsCameraPipeline* Pipeline;
};
```

## 6.2 调试HUD绘制

```cpp
void USoulsCameraDebugComponent::DrawDebugHUD()
{
    if (! bShowDebugInfo || !Pipeline) return;
    
    FSoulsCameraOutput Output = Pipeline->GetCurrentCameraOutput();
    ESoulsCameraState State = Pipeline->GetStateMachine()->GetCurrentState();
    
    FString DebugText = FString::Printf(
        TEXT("=== Souls Camera Debug ===\n")
        TEXT("State: %s\n")
        TEXT("Position: %s\n")
        TEXT("Rotation: %s\n")
        TEXT("Distance: %. 1f\n")
        TEXT("FOV: %.1f\n")
        TEXT("Is Colliding: %s\n")
        TEXT("Focus Point: %s\n"),
        *UEnum::GetValueAsString(State),
        *Output.Position.ToString(),
        *Output.Rotation.ToString(),
        Output.Distance,
        Output.FOV,
        Output. bIsColliding ? TEXT("Yes") : TEXT("No"),
        *Output.FocusPoint.ToString()
    );
    
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
}
```

---

# 第七部分：迁移指南

## 7.1 从现有代码迁移

### 7.1.1 替换映射表

| 现有代码 | 新系统对应 | 迁移策略 |
|---------|-----------|---------|
| `CameraControlComponent::UpdateLockOnCamera()` | `USoulsModule_LockOnRotation` + `USoulsModule_TargetSizeDistance` | 逻辑拆分到模块 |
| `bIsSmoothSwitching` | `StateMachine->IsTransitioning()` | 状态机管理 |
| `bIsCameraAutoCorrection` | `ExploreConfig. bEnableAutoCorrect` | 配置驱动 |
| `AdjustSpringArmForSizeDistance()` | `USoulsModule_TargetSizeDistance` | 模块化 |
| `ApplyPostProcessing()` | `USoulsModifier_*` 系列 | 修改器系统 |
| `TargetDetectionComponent` | 保留，通过接口与Pipeline交互 | 无需迁移 |

### 7. 1.2 迁移步骤

```
Phase 1: 并行运行（1周）
├── 在现有系统旁边添加Pipeline组件
├── Pipeline只读运行，不应用到相机
├── 对比两套系统的输出差异
└── 调试和验证

Phase 2: 逐步切换（1周）
├── 先切换探索状态
├── 验证稳定后切换战斗状态
├── 最后切换特殊状态（处决、Boss等）
└── 回归测试

Phase 3: 清理旧代码（1周）
├── 移除旧的相机控制逻辑
├── 保留必要的接口适配
├── 文档更新
└── 性能优化
```

### 7.1. 3 适配层代码

```cpp
UCLASS()
class UCameraSystemAdapter : public UObject
{
    GENERATED_BODY()
    
public:
    // 从旧的锁定系统获取目标
    static AActor* GetLockOnTargetFromLegacy(UTargetDetectionComponent* DetectionComp)
    {
        if (!DetectionComp) return nullptr;
        return DetectionComp->GetCurrentTarget();
    }
    
    // 将旧的状态标志转换为新的状态枚举
    static ESoulsCameraState ConvertLegacyState(
        bool bIsLocked,
        bool bInCombat,
        bool bIsSprinting,
        AActor* Target)
    {
        if (bIsLocked && Target)
        {
            if (auto* BossComp = Target->FindComponentByClass<UBossComponent>())
            {
                float Size = BossComp->GetBossSize();
                if (Size > 800.0f) return ESoulsCameraState::Boss_Huge;
                if (Size > 500.0f) return ESoulsCameraState::Boss_Large;
                if (Size > 300.0f) return ESoulsCameraState::Boss_Medium;
            }
            return ESoulsCameraState::LockOn;
        }
        
        if (bInCombat) return ESoulsCameraState::Combat;
        if (bIsSprinting) return ESoulsCameraState::Sprint;
        
        return ESoulsCameraState::Explore;
    }
    
    // 触发旧系统的震屏效果
    static void TriggerLegacyShake(
        USoulsCameraPipeline* Pipeline,
        float Intensity,
        const FVector& Direction)
    {
        FModifierActivationParams Params;
        Params. Intensity = Intensity;
        Params. Direction = Direction;
        
        Pipeline->ActivateModifier(USoulsModifier_HitReaction::StaticClass(), Params);
    }
};
```

---

# 第八部分：性能优化建议

## 8.1 计算优化

```cpp
// 1. 模块计算频率控制
UPROPERTY(EditAnywhere, Category = "Performance")
int32 ModuleUpdateFrequency = 1;  // 每N帧更新一次非关键模块

// 2. 碰撞检测优化
UPROPERTY(EditAnywhere, Category = "Performance")
bool bUseAsyncCollisionQueries = true;

// 3. 距离检测优化
UPROPERTY(EditAnywhere, Category = "Performance")
float CollisionCheckDistanceThreshold = 50.0f;  // 距离变化小于此值时跳过检测

// 4. 模块输出缓存
TMap<TSubclassOf<USoulsCameraModule>, FSoulsModuleOutput> ModuleOutputCache;
float CacheValidDuration = 0.016f;  // 缓存有效期
```

## 8.2 内存优化

```cpp
// 1. 对象池用于修改器
UPROPERTY()
TMap<TSubclassOf<USoulsCameraModifier>, TArray<USoulsCameraModifier*>> ModifierPool;

// 2. 路径历史使用环形缓冲区
TCircularBuffer<FVector> CameraPathHistory{120};

// 3. 模块预分配
void USoulsCameraPipeline::PreallocateModules()
{
    // 预创建所有可能用到的模块实例，避免运行时分配
}
```

---

# 第九部分：蓝图暴露

## 9.1 蓝图可调用函数

```cpp
UCLASS()
class USoulsCameraPipeline : public UActorComponent
{
public:
    // 锁定控制
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Lock On")
    void SetLockOnTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Lock On")
    void ClearLockOnTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Lock On")
    void SwitchToNextTarget(bool bRight = true);
    
    // 状态控制
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|State")
    void ForceState(ESoulsCameraState State, float Duration = -1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|State")
    void ReleaseForceState();
    
    UFUNCTION(BlueprintPure, Category = "Souls Camera|State")
    ESoulsCameraState GetCurrentState() const;
    
    // 修改器
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Modifiers")
    void TriggerCameraShake(float Intensity, FVector Direction);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Modifiers")
    void TriggerExecutionCamera(AActor* Target, EExecutionType Type);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Modifiers")
    void TriggerBossPhaseTransition();
    
    // 参数调整
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Config")
    void SetFOVOverride(float NewFOV, float BlendTime = 0.3f);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Config")
    void SetDistanceOverride(float NewDistance, float BlendTime = 0.3f);
    
    UFUNCTION(BlueprintCallable, Category = "Souls Camera|Config")
    void ClearOverrides(float BlendTime = 0.3f);
    
    // 事件
    UPROPERTY(BlueprintAssignable, Category = "Souls Camera|Events")
    FOnCameraStateChanged OnStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Souls Camera|Events")
    FOnLockOnTargetChanged OnLockOnTargetChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Souls Camera|Events")
    FOnCameraCollision OnCameraCollision;
};
```

## 9.2 事件委托定义

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCameraStateChanged, 
    ESoulsCameraState, OldState, 
    ESoulsCameraState, NewState
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnLockOnTargetChanged, 
    AActor*, OldTarget, 
    AActor*, NewTarget
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCameraCollision, 
    bool, bIsColliding, 
    FVector, CollisionNormal
);
```

---

# 第十部分：总结

## 10.1 设计原则总结

| 原则 | 来源 | 应用 |
|-----|------|------|
| **Pipeline架构** | 原始设计 | 7阶段清晰分离 |
| **模块化** | LLM1 | 可热插拔的Module/Modifier系统 |
| **状态机驱动** | 原始设计 + LLM1 | 26个状态，优先级驱动 |
| **时间连贯性** | Mark Haigh-Hutchinson | 所有变化必须插值 |
| **位置/旋转分离插值** | 知乎文章1 | 位置用Lerp，旋转用Slerp |
| **双空间偏移** | 知乎文章1 | 屏幕空间 vs 三维空间偏移 |
| **控制参考系** | Mark Haigh-Hutchinson + 知乎文章1 | World/Camera/Target-Relative |
| **软锁定** | 对标作品分析 | 锁定时仍允许玩家微调 |
| **惯性系统** | 知乎文章1 | 指数衰减插值，XZ/Y分离 |
| **动态FOV** | 知乎文章1 + 文章2 | FOV = Base + k * Speed |
| **碰撞处理** | 知乎文章1 + LLM2 | 多射线检测，遮挡拉近/剔除分离 |
| **修改器系统** | LLM1 | 事件触发的临时效果叠加 |

## 10.2 文件结构建议

```
Source/YourProject/Camera/
├── SoulsCameraTypes.h              // 所有枚举、结构体定义
├── SoulsCameraPipeline.h/cpp       // 主Pipeline组件
├── SoulsCameraStateMachine.h/cpp   // 状态机
├── SoulsCameraCollision.h/cpp      // 碰撞解析器
├── Modules/
│   ├── SoulsCameraModule.h         // 模块基类
│   ├── SoulsModule_FollowTarget.cpp
│   ├── SoulsModule_LockOnRotation.cpp
│   ├── SoulsModule_TargetSizeDistance.cpp
│   ├── SoulsModule_SpeedFOV.cpp
│   └── ... 
├── Modifiers/
│   ├── SoulsCameraModifier. h       // 修改器基类
│   ├── SoulsModifier_HitReaction.cpp
│   ├── SoulsModifier_Execution.cpp
│   └── ...
├── States/
│   ├── SoulsCameraState.h          // 状态基类（如需要）
│   └── ...
├── Config/
│   ├── SoulsCameraConfig. h         // 配置结构体
│   └── DA_DefaultCameraConfig.uasset
├── Debug/
│   └── SoulsCameraDebugComponent.h/cpp
└── Interfaces/
    ├── CameraLockOnInterface.h
    └── CameraEventInterface.h
```

## 10.3 核心公式速查

```cpp
// 惯性跟随（指数衰减）
float Alpha = 1.0f - FMath::Exp(-LagSpeed * DeltaTime);
CurrentValue = FMath::Lerp(CurrentValue, TargetValue, Alpha);

// 动态FOV
FOV = BaseFOV + SpeedCoefficient * CharacterSpeed;

// 目标体型距离系数
DistanceMultiplier = TargetSizeCurve->GetFloatValue(TargetSize);

// 旋转插值（球面）
FinalRotation = FQuat::Slerp(PreviousQuat, TargetQuat, Alpha). Rotator();

// 位置插值（线性）
FinalPosition = FMath::Lerp(PreviousPosition, TargetPosition, Alpha);
```

## 10.4 下一步建议

1. **白模工程验证**：先在干净的UE4. 27项目中实现核心Pipeline
2. **分阶段实现**：探索状态 → 锁定状态 → Boss状态 → 特殊状态
3. **对标测试**：与血源诅咒/艾尔登法环的相机行为对比
4. **性能测试**：确保在目标帧率下稳定运行
5. **迁移到主工程**：使用适配层逐步替换现有代码

---

# 附录：处决类型枚举

```cpp
UENUM(BlueprintType)
enum class EExecutionType : uint8
{
    Front       UMETA(DisplayName = "Front Execution"),
    Back        UMETA(DisplayName = "Backstab"),
    Riposte     UMETA(DisplayName = "Riposte/Parry Counter"),
    Plunge      UMETA(DisplayName = "Plunge Attack"),
    Special     UMETA(DisplayName = "Special/Boss Execution")
};
```

---

# 附录：修改器激活参数

```cpp
USTRUCT(BlueprintType)
struct FModifierActivationParams
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Direction = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Target = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EExecutionType ExecutionType = EExecutionType::Front;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0. 0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* CustomCurve = nullptr;
};
```

---

**文档结束**

版本: 1.0  
最后更新: 2025-11-26