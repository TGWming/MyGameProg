# Souls-like 相机系统 - 完整文件清单

## 📋 文档信息

| 项目 | 内容 |
|-----|------|
| 系统名称 | Souls-like Camera System |
| 目标引擎 | Unreal Engine 4. 27 |
| 编程语言 | C++ |
| 文件总数 | 47 个文件 |
| 创建日期 | 2025-11-29 |

---

## 📁 目录结构

```
Source/[ProjectName]/
└── Camera/
    ├── Core/                           [核心系统 - 11个文件]
    │   ├── SoulsCameraTypes.h
    │   ├── SoulsCameraModule.h
    │   ├── SoulsCameraModule.cpp
    │   ├── SoulsCameraModifier. h
    │   ├── SoulsCameraModifier.cpp
    │   ├── SoulsCameraStateMachine.h
    │   ├── SoulsCameraStateMachine.cpp
    │   ├── SoulsCameraCollisionResolver.h
    │   ├── SoulsCameraCollisionResolver.cpp
    │   ├── SoulsCameraPipeline.h
    │   └── SoulsCameraPipeline.cpp
    │
    ├── Modules/                        [功能模块 - 22个文件]
    │   ├── Position/
    │   │   ├── SoulsModule_FollowTarget.h
    │   │   ├── SoulsModule_FollowTarget.cpp
    │   │   ├── SoulsModule_OrbitPosition.h
    │   │   └── SoulsModule_OrbitPosition.cpp
    │   │
    │   ├── Rotation/
    │   │   ├── SoulsModule_InputRotation.h
    │   │   ├── SoulsModule_InputRotation.cpp
    │   │   ├── SoulsModule_LockOnRotation.h
    │   │   ├── SoulsModule_LockOnRotation.cpp
    │   │   ├── SoulsModule_AutoRotation.h
    │   │   └── SoulsModule_AutoRotation.cpp
    │   │
    │   ├── Distance/
    │   │   ├── SoulsModule_BaseDistance.h
    │   │   ├── SoulsModule_BaseDistance.cpp
    │   │   ├── SoulsModule_TargetSizeDistance.h
    │   │   ├── SoulsModule_TargetSizeDistance.cpp
    │   │   ├── SoulsModule_CombatDistance.h
    │   │   └── SoulsModule_CombatDistance.cpp
    │   │
    │   └── FOV/
    │       ├── SoulsModule_BaseFOV.h
    │       ├── SoulsModule_BaseFOV.cpp
    │       ├── SoulsModule_SpeedFOV.h
    │       ├── SoulsModule_SpeedFOV.cpp
    │       ├── SoulsModule_CombatFOV.h
    │       └── SoulsModule_CombatFOV.cpp
    │
    ├── Modifiers/                      [效果修改器 - 8个文件]
    │   ├── SoulsModifier_HitReaction.h
    │   ├── SoulsModifier_HitReaction.cpp
    │   ├── SoulsModifier_Execution.h
    │   ├── SoulsModifier_Execution.cpp
    │   ├── SoulsModifier_Death.h
    │   ├── SoulsModifier_Death.cpp
    │   ├── SoulsModifier_ScreenShake.h
    │   └── SoulsModifier_ScreenShake.cpp
    │
    └── Utils/                          [工具类 - 6个文件]
        ├── SoulsCameraMath. h
        ├── SoulsCameraMath.cpp
        ├── SoulsCameraDebug.h
        ├── SoulsCameraDebug.cpp
        ├── SoulsCameraFunctionLibrary.h
        └── SoulsCameraFunctionLibrary.cpp
```

---

## 📦 第一部分：核心系统（Core）

### 1.1 类型定义

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 1 | **SoulsCameraTypes.h** | 所有枚举、结构体、数据类型定义 |

**包含内容：**
- `ESoulsCameraState` - 相机状态枚举（Explore/LockOn/Boss/Execution/Death/Cinematic）
- `FSoulsCameraContext` - 相机上下文结构体（角色状态、输入、目标引用）
- `FSoulsCameraOutput` - 相机输出结构体（位置、旋转、距离、FOV）
- `FSoulsCameraStateConfig` - 状态配置结构体
- `FSoulsCameraTransitionRule` - 状态转换规则
- `FModifierActivationParams` - 修改器激活参数

---

### 1.2 模块基类

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 2 | **SoulsCameraModule.h** | 模块基类声明 |
| 3 | **SoulsCameraModule. cpp** | 模块基类实现 |

**核心功能：**
- 模块激活状态管理（ActiveStates）
- 权重系统（Weight）
- 优先级排序（Priority）
- 虚函数接口：`Initialize()`, `Compute()`, `IsActiveInState()`

---

### 1.3 修改器基类

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 4 | **SoulsCameraModifier.h** | 修改器基类声明 |
| 5 | **SoulsCameraModifier. cpp** | 修改器基类实现 |

**核心功能：**
- 激活/停用控制
- 生命周期管理
- 虚函数接口：`Activate()`, `Deactivate()`, `Update()`, `IsFinished()`

---

### 1.4 状态机

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 6 | **SoulsCameraStateMachine.h** | 状态机声明 |
| 7 | **SoulsCameraStateMachine.cpp** | 状态机实现 |

**核心功能：**
- 状态转换规则管理
- 转换条件评估
- 混合过渡（Blend Transition）
- 默认转换规则：
  - Explore ↔ LockOn（锁定/解锁目标）
  - LockOn ↔ Boss（Boss 检测）
  - Any → Execution（处决触发）
  - Any → Death（死亡触发）

---

### 1.5 碰撞处理器

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 8 | **SoulsCameraCollisionResolver.h** | 碰撞处理器声明 |
| 9 | **SoulsCameraCollisionResolver.cpp** | 碰撞处理器实现 |

**核心功能：**
- 多射线检测（Multi-Trace）
- 球形扫描（Sphere Sweep）
- 渐进恢复（Progressive Recovery）
- 透明物体忽略
- 碰撞通道配置

---

### 1.6 主 Pipeline

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 10 | **SoulsCameraPipeline.h** | 主 Pipeline 组件声明 |
| 11 | **SoulsCameraPipeline.cpp** | 主 Pipeline 组件实现 |

**核心功能 - 7 阶段处理流程：**
```
Phase 1: UpdateContext()         - 收集角色状态、输入、目标信息
Phase 2: StateEvaluation()       - 状态机更新，评估状态转换
Phase 3: PositionCalculation()   - 执行位置模块，计算焦点和相机位置
Phase 4: RotationCalculation()   - 执行旋转模块，计算相机朝向
Phase 5: DistanceCalculation()   - 执行距离模块，计算相机距离
Phase 6: FOVCalculation()        - 执行 FOV 模块，计算视场角
Phase 7: CollisionResolution()   - 碰撞检测和处理
Phase 8: FinalBlend()            - 状态混合，应用修改器
Phase 9: ApplyToCamera()         - 输出到实际相机
```

---

## 📦 第二部分：功能模块（Modules）

### 2.1 位置计算模块（Position）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 12 | **SoulsModule_FollowTarget.h** | 跟随目标模块 - 头文件 |
| 13 | **SoulsModule_FollowTarget.cpp** | 跟随目标模块 - 源文件 |

**功能：** 计算相机焦点位置，跟随角色并应用偏移

**关键参数：**
- `FocusOffset` - 焦点偏移（默认：0, 0, 80）
- `PositionLagSpeed` - 位置平滑速度（默认：8. 0）
- `bUseVelocityPrediction` - 是否使用速度预测

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 14 | **SoulsModule_OrbitPosition.h** | 轨道位置模块 - 头文件 |
| 15 | **SoulsModule_OrbitPosition.cpp** | 轨道位置模块 - 源文件 |

**功能：** 计算相机环绕位置，基于球面坐标系

**关键参数：**
- `ArmLength` - 相机臂长度
- `PitchMin/PitchMax` - 俯仰角限制
- `bUseSpringArm` - 是否使用弹簧效果

---

### 2.2 旋转计算模块（Rotation）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 16 | **SoulsModule_InputRotation.h** | 输入旋转模块 - 头文件 |
| 17 | **SoulsModule_InputRotation.cpp** | 输入旋转模块 - 源文件 |

**功能：** 处理玩家摇杆/鼠标输入，控制相机旋转

**关键参数：**
- `SensitivityX/Y` - 灵敏度
- `DeadZone` - 死区
- `AccelerationCurve` - 加速曲线
- `InputSmoothSpeed` - 输入平滑速度

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 18 | **SoulsModule_LockOnRotation.h** | 锁定旋转模块 - 头文件 |
| 19 | **SoulsModule_LockOnRotation.cpp** | 锁定旋转模块 - 源文件 |

**功能：** 锁定状态下自动追踪目标

**关键参数：**
- `TargetScreenX` - 目标屏幕 X 位置（默认：0. 618 黄金分割）
- `TargetScreenY` - 目标屏幕 Y 位置（默认：0.382）
- `RotationLagSpeed` - 旋转平滑速度
- `MaxAutoRotationSpeed` - 最大自动旋转速度
- `bAllowManualAdjustment` - 是否允许手动微调

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 20 | **SoulsModule_AutoRotation.h** | 自动旋转模块 - 头文件 |
| 21 | **SoulsModule_AutoRotation.cpp** | 自动旋转模块 - 源文件 |

**功能：** 角色移动时相机自动跟随移动方向

**关键参数：**
- `AutoRotationDelay` - 自动旋转延迟
- `AutoRotationSpeed` - 自动旋转速度
- `MinSpeedForAutoRotation` - 触发自动旋转的最小速度
- `bOnlyRotateYaw` - 是否只旋转 Yaw

---

### 2.3 距离计算模块（Distance）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 22 | **SoulsModule_BaseDistance.h** | 基础距离模块 - 头文件 |
| 23 | **SoulsModule_BaseDistance.cpp** | 基础距离模块 - 源文件 |

**功能：** 根据当前状态读取配置的基础距离

**关键参数：**
- 从 `FSoulsCameraStateConfig` 读取各状态的 `BaseDistance`

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 24 | **SoulsModule_TargetSizeDistance.h** | 目标尺寸距离模块 - 头文件 |
| 25 | **SoulsModule_TargetSizeDistance.cpp** | 目标尺寸距离模块 - 源文件 |

**功能：** 根据 Boss 体型动态调整相机距离

**关键参数：**
- `SmallBossThreshold` - 小型 Boss 阈值（包围盒半径）
- `MediumBossThreshold` - 中型 Boss 阈值
- `LargeBossThreshold` - 大型 Boss 阈值
- `SmallBossDistance` - 小型 Boss 距离
- `MediumBossDistance` - 中型 Boss 距离
- `LargeBossDistance` - 大型 Boss 距离
- `DistanceTransitionSpeed` - 距离过渡速度

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 26 | **SoulsModule_CombatDistance.h** | 战斗距离模块 - 头文件 |
| 27 | **SoulsModule_CombatDistance.cpp** | 战斗距离模块 - 源文件 |

**功能：** 战斗状态下调整相机距离

**关键参数：**
- `CombatDistanceMultiplier` - 战斗距离倍数
- `CombatEntrySpeed` - 进入战斗过渡速度
- `CombatExitSpeed` - 退出战斗过渡速度

---

### 2.4 FOV 计算模块（FOV）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 28 | **SoulsModule_BaseFOV.h** | 基础 FOV 模块 - 头文件 |
| 29 | **SoulsModule_BaseFOV.cpp** | 基础 FOV 模块 - 源文件 |

**功能：** 根据当前状态读取配置的基础 FOV

**关键参数：**
- 从 `FSoulsCameraStateConfig` 读取各状态的 `BaseFOV`

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 30 | **SoulsModule_SpeedFOV.h** | 速度 FOV 模块 - 头文件 |
| 31 | **SoulsModule_SpeedFOV.cpp** | 速度 FOV 模块 - 源文件 |

**功能：** 根据角色移动速度调整 FOV，冲刺时增加速度感

**关键参数：**
- `BaseFOV` - 基础 FOV（默认：70. 0）
- `SprintFOVBonus` - 冲刺 FOV 增量（默认：8.0）
- `SpeedThreshold` - 速度阈值
- `FOVChangeSpeed` - FOV 变化速度

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 32 | **SoulsModule_CombatFOV.h** | 战斗 FOV 模块 - 头文件 |
| 33 | **SoulsModule_CombatFOV.cpp** | 战斗 FOV 模块 - 源文件 |

**功能：** 战斗状态下 FOV 调整

**关键参数：**
- `CombatFOVBonus` - 战斗 FOV 增量
- `LockOnFOVReduction` - 锁定时 FOV 减少量

---

## 📦 第三部分：效果修改器（Modifiers）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 34 | **SoulsModifier_HitReaction.h** | 受击反馈修改器 - 头文件 |
| 35 | **SoulsModifier_HitReaction.cpp** | 受击反馈修改器 - 源文件 |

**功能：** 角色受击时的相机反馈效果

**关键参数：**
- `ShakeIntensity` - 震动强度（默认：5.0）
- `ShakeDuration` - 震动持续时间（默认：0.2）
- `ShakeFrequency` - 震动频率
- `FOVPunchAmount` - FOV 冲击量（默认：3.0）
- `DirectionalShake` - 是否根据受击方向震动

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 36 | **SoulsModifier_Execution.h** | 处决相机修改器 - 头文件 |
| 37 | **SoulsModifier_Execution.cpp** | 处决相机修改器 - 源文件 |

**功能：** 处决动画时的相机控制

**关键参数：**
- `ExecutionDistance` - 处决时相机距离
- `ExecutionFOV` - 处决时 FOV
- `ExecutionOffset` - 处决时相机偏移
- `BlendInTime` - 进入混合时间
- `BlendOutTime` - 退出混合时间
- `bUseCinematicBars` - 是否使用电影黑边

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 38 | **SoulsModifier_Death.h** | 死亡相机修改器 - 头文件 |
| 39 | **SoulsModifier_Death.cpp** | 死亡相机修改器 - 源文件 |

**功能：** 角色死亡时的相机行为

**关键参数：**
- `DeathCameraMode` - 死亡相机模式（固定/环绕/俯视）
- `DeathDistance` - 死亡时相机距离
- `DeathPitch` - 死亡时俯仰角
- `OrbitSpeed` - 环绕速度（如使用环绕模式）
- `SlowMotionFactor` - 慢动作系数

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 40 | **SoulsModifier_ScreenShake.h** | 通用屏幕震动修改器 - 头文件 |
| 41 | **SoulsModifier_ScreenShake.cpp** | 通用屏幕震动修改器 - 源文件 |

**功能：** 可配置的通用屏幕震动效果

**关键参数：**
- `ShakePattern` - 震动模式（正弦/随机/柏林噪声）
- `Amplitude` - 振幅
- `Frequency` - 频率
- `Duration` - 持续时间
- `FalloffExponent` - 衰减指数
- `bAffectPosition` - 是否影响位置
- `bAffectRotation` - 是否影响旋转
- `bAffectFOV` - 是否影响 FOV

---

## 📦 第四部分：工具类（Utils）

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 42 | **SoulsCameraMath.h** | 数学工具函数 - 头文件 |
| 43 | **SoulsCameraMath.cpp** | 数学工具函数 - 源文件 |

**包含函数：**
```cpp
// 平滑插值
static float SmoothDamp(float Current, float Target, float& Velocity, float SmoothTime, float DeltaTime);
static FVector SmoothDampVector(const FVector& Current, const FVector& Target, FVector& Velocity, float SmoothTime, float DeltaTime);
static FRotator SmoothDampRotator(const FRotator& Current, const FRotator& Target, FRotator& Velocity, float SmoothTime, float DeltaTime);

// 弹簧阻尼
static float SpringDamp(float Current, float Target, float& Velocity, float Stiffness, float Damping, float DeltaTime);

// 角度工具
static float NormalizeAngle(float Angle);
static float ShortestAngleDelta(float From, float To);

// 曲线采样
static float EvaluateAccelerationCurve(float Input, float Exponent);

// 屏幕空间计算
static FVector2D WorldToScreenPosition(const FVector& WorldLocation, APlayerController* PC);
static FVector ScreenToWorldDirection(const FVector2D& ScreenPosition, APlayerController* PC);
```

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 44 | **SoulsCameraDebug.h** | 调试工具 - 头文件 |
| 45 | **SoulsCameraDebug.cpp** | 调试工具 - 源文件 |

**包含功能：**
```cpp
// 可视化绘制
static void DrawFocusPoint(UWorld* World, const FVector& FocusPoint);
static void DrawCameraFrustum(UWorld* World, const FSoulsCameraOutput& Output);
static void DrawCollisionProbes(UWorld* World, const TArray<FHitResult>& Hits);
static void DrawTargetIndicator(UWorld* World, AActor* Target);
static void DrawStateInfo(UCanvas* Canvas, const FSoulsCameraContext& Context);

// 控制台命令
static void RegisterDebugCommands();
```

---

| 序号 | 文件名 | 说明 |
|:---:|--------|------|
| 46 | **SoulsCameraFunctionLibrary.h** | 蓝图函数库 - 头文件 |
| 47 | **SoulsCameraFunctionLibrary.cpp** | 蓝图函数库 - 源文件 |

**包含函数：**
```cpp
UCLASS()
class USoulsCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
    // 获取相机 Pipeline
    UFUNCTION(BlueprintPure, Category = "Souls Camera")
    static USoulsCameraPipeline* GetSoulsCameraPipeline(AActor* Actor);
    
    // 设置锁定目标
    UFUNCTION(BlueprintCallable, Category = "Souls Camera")
    static void SetLockOnTarget(AActor* Owner, AActor* Target);
    
    // 清除锁定
    UFUNCTION(BlueprintCallable, Category = "Souls Camera")
    static void ClearLockOnTarget(AActor* Owner);
    
    // 触发受击反馈
    UFUNCTION(BlueprintCallable, Category = "Souls Camera")
    static void TriggerHitReaction(AActor* Owner, FVector HitDirection, float Intensity);
    
    // 触发屏幕震动
    UFUNCTION(BlueprintCallable, Category = "Souls Camera")
    static void TriggerScreenShake(AActor* Owner, float Intensity, float Duration);
    
    // 获取当前状态
    UFUNCTION(BlueprintPure, Category = "Souls Camera")
    static ESoulsCameraState GetCurrentCameraState(AActor* Owner);
};
```

---

## 📊 文件统计汇总

| 分类 | 头文件 (. h) | 源文件 (.cpp) | 合计 |
|-----|:-----------:|:-------------:|:----:|
| 核心系统 | 6 | 5 | 11 |
| 位置模块 | 2 | 2 | 4 |
| 旋转模块 | 3 | 3 | 6 |
| 距离模块 | 3 | 3 | 6 |
| FOV 模块 | 3 | 3 | 6 |
| 效果修改器 | 4 | 4 | 8 |
| 工具类 | 3 | 3 | 6 |
| **总计** | **24** | **23** | **47** |

---

## 🔗 文件依赖关系图

```
┌─────────────────────────────────────────────────────────────────────┐
│                        SoulsCameraTypes.h                           │
│              (枚举、结构体、数据类型 - 无依赖)                          │
└─────────────────────────────────────────────────────────────────────┘
                                    │
            ┌───────────────────────┼───────────────────────┐
            │                       │                       │
            ▼                       ▼                       ▼
┌───────────────────┐   ┌───────────────────┐   ┌───────────────────┐
│ SoulsCameraMath   │   │ SoulsCameraModule │   │SoulsCameraModifier│
│   (数学工具)       │   │   (模块基类)       │   │  (修改器基类)      │
└───────────────────┘   └───────────────────┘   └───────────────────┘
            │                       │                       │
            │           ┌───────────┴───────────┐           │
            │           │                       │           │
            │           ▼                       ▼           │
            │   ┌───────────────┐       ┌───────────────┐   │
            │   │ Position模块  │       │ Rotation模块  │   │
            │   │ FollowTarget │       │InputRotation │   │
            │   │ OrbitPosition│       │LockOnRotation│   │
            │   └───────────────┘       │ AutoRotation │   │
            │           │               └───────────────┘   │
            │           │                       │           │
            │           ▼                       ▼           │
            │   ┌───────────────┐       ┌───────────────┐   │
            │   │ Distance模块  │       │   FOV模块     │   │
            │   │ BaseDistance │       │   BaseFOV    │   │
            │   │TargetSize   │       │  SpeedFOV    │   │
            │   │CombatDistance│       │  CombatFOV   │   │
            │   └───────────────┘       └───────────────┘   │
            │                                               │
            │                                               ▼
            │                               ┌───────────────────┐
            │                               │   Modifiers       │
            │                               │  HitReaction     │
            │                               │  Execution       │
            │                               │  Death           │
            │                               │  ScreenShake     │
            │                               └───────────────────┘
            │                                               │
            └───────────────────────┬───────────────────────┘
                                    │
                                    ▼
                    ┌───────────────────────────┐
                    │ SoulsCameraStateMachine   │
                    │      (状态机)              │
                    └───────────────────────────┘
                                    │
                                    ▼
                    ┌───────────────────────────┐
                    │SoulsCameraCollisionResolver│
                    │     (碰撞处理器)            │
                    └───────────────────────────┘
                                    │
                                    ▼
                    ┌───────────────────────────┐
                    │   SoulsCameraPipeline     │
                    │   (主组件 - 引用所有)       │
                    └───────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
                    ▼                               ▼
        ┌───────────────────┐           ┌───────────────────────┐
        │ SoulsCameraDebug  │           │SoulsCameraFunctionLib │
        │   (调试工具)       │           │    (蓝图函数库)        │
        └───────────────────┘           └───────────────────────┘
```

---

## 📋 推荐实现顺序

### 阶段 1：基础框架（必须 - 可独立运行）

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 1 | SoulsCameraTypes.h | 无 |
| 2 | SoulsCameraMath.h/. cpp | Types |
| 3 | SoulsCameraModule.h/.cpp | Types |
| 4 | SoulsCameraStateMachine.h/.cpp | Types |
| 5 | SoulsCameraCollisionResolver.h/.cpp | Types, Math |
| 6 | SoulsCameraPipeline.h/.cpp | 以上所有 |

**完成后可测试：** 基础组件挂载，状态机运行

---

### 阶段 2：核心模块（必须 - 基础相机功能）

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 7 | SoulsModule_FollowTarget.h/.cpp | Module基类 |
| 8 | SoulsModule_OrbitPosition.h/.cpp | Module基类, Math |
| 9 | SoulsModule_InputRotation.h/.cpp | Module基类, Math |
| 10 | SoulsModule_BaseDistance.h/.cpp | Module基类 |
| 11 | SoulsModule_BaseFOV.h/.cpp | Module基类 |

**完成后可测试：** 基础第三人称相机跟随、旋转控制

---

### 阶段 3：锁定系统

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 12 | SoulsModule_LockOnRotation.h/.cpp | Module基类, Math |
| 13 | SoulsModule_TargetSizeDistance.h/.cpp | Module基类 |

**完成后可测试：** 目标锁定、Boss 体型适配

---

### 阶段 4：高级功能

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 14 | SoulsModule_AutoRotation.h/.cpp | Module基类, Math |
| 15 | SoulsModule_CombatDistance.h/.cpp | Module基类 |
| 16 | SoulsModule_SpeedFOV.h/.cpp | Module基类 |
| 17 | SoulsModule_CombatFOV. h/.cpp | Module基类 |

**完成后可测试：** 自动旋转、战斗距离、速度FOV

---

### 阶段 5：效果修改器

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 18 | SoulsCameraModifier. h/.cpp | Types |
| 19 | SoulsModifier_HitReaction.h/.cpp | Modifier基类, Math |
| 20 | SoulsModifier_ScreenShake.h/.cpp | Modifier基类, Math |
| 21 | SoulsModifier_Execution.h/. cpp | Modifier基类 |
| 22 | SoulsModifier_Death.h/.cpp | Modifier基类 |

**完成后可测试：** 受击反馈、屏幕震动、处决相机

---

### 阶段 6：工具和调试

| 顺序 | 文件 | 依赖 |
|:---:|------|------|
| 23 | SoulsCameraDebug.h/.cpp | Types, Pipeline |
| 24 | SoulsCameraFunctionLibrary. h/.cpp | Pipeline |

**完成后可测试：** 调试可视化、蓝图接口

---

## 🚀 最小可运行版本（MVP）

如需快速看到效果，只需实现以下 **14 个文件**：

| 序号 | 文件 | 代码行数估计 |
|:---:|------|:-----------:|
| 1 | SoulsCameraTypes.h | ~200 |
| 2 | SoulsCameraMath.h | ~50 |
| 3 | SoulsCameraMath.cpp | ~150 |
| 4 | SoulsCameraModule.h | ~60 |
| 5 | SoulsCameraModule.cpp | ~40 |
| 6 | SoulsCameraStateMachine. h | ~80 |
| 7 | SoulsCameraStateMachine.cpp | ~150 |
| 8 | SoulsCameraCollisionResolver. h | ~50 |
| 9 | SoulsCameraCollisionResolver.cpp | ~120 |
| 10 | SoulsCameraPipeline.h | ~120 |
| 11 | SoulsCameraPipeline. cpp | ~300 |
| 12 | SoulsModule_FollowTarget. h/. cpp | ~80 |
| 13 | SoulsModule_OrbitPosition.h/.cpp | ~100 |
| 14 | SoulsModule_InputRotation.h/.cpp | ~120 |
| **总计** | **14 个文件** | **~1620 行** |

---

## ✅ 确认清单

请确认以下内容：

- [ ] 文件结构是否符合预期？
- [ ] 是否需要增加其他模块？
- [ ] 是否需要删减某些模块？
- [ ] 是否从 MVP 版本开始实现？
- [ ] 是否需要调整实现顺序？

---

## 📝 下一步

确认清单后，请告诉我：

**"请生成 [文件名] 的完整代码"**

例如：
- "请生成 SoulsCameraTypes.h 的完整代码"
- "请生成阶段1的所有文件"
- "请生成 MVP 版本的所有文件"

我将按照确认的清单，为你生成完整的可编译代码。