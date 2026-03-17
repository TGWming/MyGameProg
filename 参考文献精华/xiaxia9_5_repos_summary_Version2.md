# xiaxia9 五个实战 Repo 完整总结

> 作者：xiaxia9（XiaWen / 爱吃菠萝不吃萝卜）
> 知乎：https://www.zhihu.com/people/xia-xia-93-23-65
> CSDN：https://blog.csdn.net/qq_38172320
> GitHub：https://github.com/xiaxia9
> 生成日期：2026-03-17

---

## 目录

1. [NewLocomotionSystem — UE5 运动系统项目](#1-newlocomotionsystem)
2. [AnimationProject — UE5 高级动画系统项目](#2-animationproject)
3. [ebook — 计算机专业书籍仓库](#3-ebook)
4. [XiaIM — C++ 即时通讯系统](#4-xiaim)
5. [blogs — 早期技术博客](#5-blogs)

---

## 1. NewLocomotionSystem

| 属性 | 信息 |
|---|---|
| **地址** | https://github.com/xiaxia9/NewLocomotionSystem |
| **类型** | UE5 C++ 第三人称角色运动系统项目 |
| **引擎** | UE5（源码编译版，EngineAssociation 为本地 GUID） |
| **分支** | `main` |
| **插件** | ModelingToolsEditorMode, EnhancedInput |
| **模块依赖** | Core, CoreUObject, Engine, InputCore, HeadMountedDisplay, EnhancedInput |

### 1.1 项目结构

```
NewLocomotionSystem/
├── Config/
├── Content/                                   ← UE 资产（.uasset，二进制不可读）
├── Source/
│   ├── NewLocomotionSystem.Target.cs
│   ├── NewLocomotionSystemEditor.Target.cs
│   └── NewLocomotionSystem/
│       ├── NewLocomotionSystem.Build.cs       ← 模块构建规则
│       ├── NewLocomotionSystem.h / .cpp       ← 模块入口
│       ├── NewLocomotionSystemCharacter.h/cpp ← ⭐ 角色类
│       └── NewLocomotionSystemGameMode.h/cpp  ← GameMode
└── NewLocomotionSystem.uproject
```

### 1.2 核心类：ANewLocomotionSystemCharacter

基于 UE5 第三人称模板，使用 **Enhanced Input System**。

**组件：**
- `CameraBoom` (USpringArmComponent) — 摇臂，TargetArmLength=400，碰撞拉近
- `FollowCamera` (UCameraComponent) — 跟随相机
- `DefaultMappingContext` (UInputMappingContext)
- `JumpAction / MoveAction / LookAction` (UInputAction)

**关键参数：**

| 参数 | 值 |
|---|---|
| 碰撞胶囊 | 半径42, 半高96 |
| bOrientRotationToMovement | true（角色面朝移动方向） |
| RotationRate | (0, 500, 0) |
| MaxWalkSpeed | 500 |
| JumpZVelocity | 700 |
| AirControl | 0.35 |
| BrakingDecelerationWalking | 2000 |

**输入处理：**
- **Move()** → 从 Controller Yaw 构造前进/右移方向 → AddMovementInput()
- **Look()** → AddControllerYawInput() + AddControllerPitchInput()
- **Jump** → 绑定 ACharacter::Jump / StopJumping

**BeginPlay：** 获取 PlayerController → 添加 DefaultMappingContext

### 1.3 GameMode

- 通过 FClassFinder 加载蓝图 `/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter` 作为默认 Pawn

### 1.4 评估

- **C++ 侧代码量**：约200行有效代码，标准 UE5 第三人称模板
- **核心运动逻辑**：在 Content/ 蓝图中（AnimBlueprint/BlendSpace/状态机），`.uasset` 无法通过 GitHub 读取
- **与笔记关联**：Game-Development-Notes 中运动系统（Locomotion System）的配套实战工程

---

## 2. AnimationProject

| 属性 | 信息 |
|---|---|
| **地址** | https://github.com/xiaxia9/AnimationProject |
| **类型** | UE5.1 C++ 高级动画系统项目（ALS 风格的完整 C++ 重写） |
| **引擎** | UE 5.1 |
| **分支** | `main` |
| **自定义插件** | PhysicsAnimationTools（物理动画工具，Runtime + Editor 两个模块） |
| **代码量** | **远超第一个项目**，CharacterBase.cpp ~61KB，AnimInstanceBase.cpp ~27KB |

### 2.1 项目结构

```
AnimationProject/
├── Config/
├── Content/
├── Plugins/
│   └── PhysicsAnimationTools/             ← ⭐ 自研插件
│       ├── PhysicsAnimationTools.uplugin
│       ├── Resources/
│       └── Source/
│           ├── PhysicsAnimationTools/     ← Runtime 模块
│           └── PhysicsAnimationToolsEditor/ ← Editor 模块
├── Source/AnimationProject/
│   ├── AnimationProject.Build.cs
│   ├── Character/
│   │   ├── CharacterBase.h/cpp            ← ⭐⭐ 核心角色类（11KB头文件 + 61KB实现）
│   │   ├── AnimInstanceBase.h/cpp         ← ⭐⭐ 动画实例（8KB头文件 + 27KB实现）
│   │   └── XXCharacterMovementComponent.h/cpp ← 自定义移动组件
│   ├── Common/
│   │   ├── CommonInterfaces.h/cpp         ← 接口定义（4个UInterface）
│   │   └── CommonUtilities.h/cpp
│   ├── GameModes/
│   │   └── AnimationProjectGameMode.h/cpp
│   ├── Locomotion/
│   │   └── LocomotionDefine.h             ← ⭐ 运动系统类型定义（6.8KB）
│   ├── Physics/
│   │   └── CollisionChannels.h            ← 碰撞通道定义
│   └── Player/
│       └── PlayerControllerBase.h/cpp
└── AnimationProject.uproject
```

### 2.2 运动系统枚举与数据结构 (LocomotionDefine.h)

**枚举类型：**

| 枚举 | 值 |
|---|---|
| `EGait` | Walking, Running, Sprinting |
| `EMovementState` | None, Grounded, InAir, Mantling, Ragdoll |
| `EMovementAction` | None, LowMantle, HighMantle, Rolling, GettingUp |
| `ERotationMode` | VelocityDirection, LookingDirection, Aiming |
| `EStance` | Standing, Crouching |
| `EViewMode` | ThirdPerson, FirstPerson |
| `EOverlayState` | Default, Masculine, Feminine, Injured, HandsTied, Rifle, Pistol1H, Pistol2H, Bow, Torch, Binoculars, Box, Barrel |
| `EMovementDirection` | Forward, Right, Left, Backward |
| `EHipsDirection` | F, B, RF, RB, LF, LB |
| `EMantleType` | HighMantle, LowMantle, FallingCatch |
| `EGroundedEntryState` | None, Roll |
| `EFootstepType` | Step, WalkOrRun, Jump, Land |
| `EAnimFeatureExample` | StrideBlending, AdditiveLeaning, SprintImpulse |

**关键数据结构：**
- `FMovementSettings` — Walk/Run/Sprint速度 + 移动曲线 + 旋转速率曲线
- `FMovementSettingsStance` — Standing + Crouching 各一套 FMovementSettings
- `FMovementSettingsState` — VelocityDirection / LookingDirection / Aiming 各一套
- `FVelocityBlend` — 前后左右(F/B/L/R)速度混合权重
- `FLeanAmount` — 左右(LR) + 前后(FB) 倾斜量
- `FMantleTraceSettings` — 攀爬检测参数（最大/最小高度、距离、射线半径）
- `FMantleAsset` — 攀爬动画资产（Montage + 位置修正曲线 + 高低位参数）
- `FMantleParams` — 运行时攀爬参数
- `FCameraSettings` — 摇臂长度/偏移/延迟/碰撞检测
- `FDynamicMontageParams` — 动态过渡蒙太奇参数
- `FTurnInPlaceAsset` — 原地转身资产
- `FRotateInPlaceAsset` — 原地旋转资产

### 2.3 接口系统 (CommonInterfaces.h)

四个 UInterface（CannotImplementInterfaceInBlueprint）：

| 接口 | 方法 | 用途 |
|---|---|---|
| **IAnimationInterface** | BPIJumped, BPISetGroundEntryState, BPISetOverlayOverrideState | 角色 → 动画实例通信 |
| **ICameraInterface** | BPIGetCameraParameters, BPIGetCameraTarget, BPIGet3PPivotTarget, BPIGet3PTraceParams | 相机系统查询 |
| **ICharacterInterface** | BPIGetCurrentStates, BPIGetEssentialValues, BPISetMovementState/Action/RotationMode/Gait/ViewMode/OverlayState | 动画实例 → 角色状态查询/设置 |
| **IControllerInterface** | BPIGetDebugInfo | 调试信息获取 |

### 2.4 核心角色类 (ACharacterBase)

**继承：** `ACharacter` + `ICharacterInterface` + `ICameraInterface`

**核心系统功能：**

| 功能模块 | 关键方法 |
|---|---|
| **状态管理** | OnMovementStateChanged, OnGaitChanged, OnRotationModeChanged, OnViewModeChanged, OnOverlayStateChanged |
| **Tick 主循环** | SetEssentialValues → 按MovementState分支(Grounded/InAir/Ragdoll) → CacheValues → DrawDebugShapes |
| **地面旋转** | UpdateGroundedRotation, SmoothCharacterRotation, LimitRotation, CalculateGroundedRotationRate |
| **空中逻辑** | UpdateInAirRotation, MantleCheck |
| **攀爬系统** | MantleCheck, MantleStart, MantleEnd, GetMantleAsset, Timeline驱动 |
| **布娃娃系统** | RagdollStart, RagdollUpdate, RagdollEnd, SetActorLocationDuringRagdoll |
| **步态计算** | GetAllowedGait, GetActualGait, UpdateDynamicMovementSettings, CanSprint, GetMappedSpeed |
| **手持物体** | UpdateHeldObject, ClearHeldObject, AttachToHand（13种OverlayState对应不同物体） |
| **颜色调试** | UpdateLayeringColors, SetDynamicMaterials（14个身体部位动态材质） |
| **翻滚/起身** | GetRollAnimation, GetGetUpAnimation（根据OverlayState和朝向选择对应Montage） |
| **数据表驱动** | MovementModelDT (UDataTable) → FMovementSettingsState |

### 2.5 动画实例 (UAnimInstanceBase)

**继承：** `UAnimInstance` + `IAnimationInterface`

**核心更新流程（NativeUpdateAnimation）：**

| 步骤 | 方法 | 说明 |
|---|---|---|
| 1 | UpdateCharacterInfo | 通过接口从角色获取所有状态和运动数据 |
| 2 | UpdateAimingValues | 平滑瞄准旋转、计算脊柱旋转、输入Yaw偏移 |
| 3 | UpdateLayerValues | 各身体部位动画层权重（通过Curve） |
| 4 | UpdateFootIK | 脚部IK：FootLocking + FootOffsets + PelvisIK |
| 5 | UpdateMovementValues | VelocityBlend、StrideBlend、WalkRunBlend、PlayRate等 |
| 6 | UpdateRotationValues | 移动方向计算（四象限） |
| 7 | UpdateInAirValues | 落地预测、空中倾斜 |
| 8 | UpdateRagdollValues | 布娃娃速率 |

**状态检查：**
- ShouldMoveCheck → 决定移动/静止状态
- RotateInPlaceCheck → 原地旋转（Aiming模式）
- TurnInPlaceCheck → 原地转身（静止时大角度偏转）
- DynamicTransitionCheck → 动态过渡

**AnimNotify 系统（20+个）：**
- 运动停止：NStopR, NStopL, CLFStop
- 翻滚/落地：RollIdle, LandIdle
- 武器切换：BowRelaxedReady/ReadyRelaxed, M4A1, Pistol1H, Pistol2H
- Pivot：方向切换检测
- Hips方向：HipsF/B/LF/LB/RF/RB

**脚部IK系统：**
- FootLocking（脚部锁定，防止滑步）
- FootOffsets（地面适应，坡面/台阶）
- PelvisIK（骨盆高度调整）
- 使用Curve驱动IK权重

**Turn In Place 资产（8组）：**
- Normal/CLF × Left/Right × 90°/180°

### 2.6 自研插件：PhysicsAnimationTools

| 属性 | 信息 |
|---|---|
| 名称 | PhysicsAnimationTools |
| 作者 | XiaWen |
| 版本 | 1.0 |
| 模块 | Runtime + Editor(UncookedOnly) |
| 内容 | CanContainContent = true |

### 2.7 评估

- **代码规模**：CharacterBase ~61KB + AnimInstanceBase ~27KB + LocomotionDefine ~7KB = **约100KB 纯 C++ 代码**
- **架构水平**：完整的 ALS (Advanced Locomotion System) 风格 C++ 重写，包含攀爬/布娃娃/原地转身/脚部IK/Overlay状态等全套系统
- **设计模式**：接口驱动（4个UInterface解耦角色/动画/相机/控制器）、数据表驱动（MovementModel）、曲线驱动（旋转/IK/混合）
- **与笔记关联**：直接对应 Game-Development-Notes 中动画系统相关章节

---

## 3. ebook

| 属性 | 信息 |
|---|---|
| **地址** | https://github.com/xiaxia9/ebook |
| **类型** | 计算机专业电子书收藏仓库 |
| **存储** | Git LFS（PDF 文件为 LFS 指针，133字节） |
| **总量** | 100+ 本 |

### 3.1 目录结构

```
ebook/
├── 01_programming/     ← 编程语言
│   ├── Android/ (4本)
│   ├── iOS/ (11本)
│   ├── C++/ (27本)     ← ⭐ 最多
│   ├── go/ (5本)
│   ├── python/ (11本)
│   └── other/ (2本)
├── 02_algorithm/ (13本)
├── 03_operating_system/ (18本)
├── 04_pattern/ (1本)
├── 05_reverse/ (4本)
├── 07_devops/ (2本)
├── 08_game/             ← ⭐⭐ 游戏开发（最高价值）
│   ├── engine/          ← 引擎相关子目录
│   ├── GPU Gems/        ← GPU Gems 系列
│   ├── gpu_pro/         ← GPU Pro 系列
│   ├── 3D数学基础/       ← 3D数学子目录
│   └── (20+ 本书)
├── 09_other/ (14本)
└── readme.md
```

### 3.2 游戏开发书籍清单 (08_game/)

**🎮 引擎架构：**
- 游戏引擎架构（第1版）
- 游戏引擎架构（第2版）
- 游戏引擎架构2 small

**🎮 实时渲染：**
- Real-Time Rendering, Third Edition
- Real-Time Rendering Fourth Edition (2018)
- 《Real-Time Rendering 3rd》提炼总结

**🎮 游戏编程精粹系列（Game Programming Gems）：**
- 游戏编程精粹 1~8（共8本）

**🎮 图形学：**
- Fundamentals of Computer Graphics 5th
- 3D数学基础 图形与游戏开发
- 计算机图形学导论
- Focus On 3D Terrain Programming

**🎮 GPU 编程：**
- GPU Gems 系列（子目录）
- GPU Pro 系列（子��录）
- GPU 精髓

**🎮 Shader：**
- Unity Shader 入门精要彩图
- The Unity Shaders Bible
- Unity 2021 Shaders and Effects Cookbook 4th

**🎮 Web 图形：**
- OpenGL Insights
- WebGL Insights
- WebGL 编程指南

### 3.3 C++ 书籍清单 (01_programming/C++/)

共27本，精选列表：
- Effective C++（第3版）/ More Effective C++
- C++ Primer Plus（第5版）
- The C++ Programming Language
- C++ Templates 中文版 / C++ 模板元编程
- 深度探索 C++ 对象模型
- C++ 沉思录 / C++ 编程思想（第二卷）
- C++ 性能优化指南
- C++ 设计新思维
- Exceptional C++
- C++ 游戏编程入门（第4版）
- C++ 编程规范 - 101条规则
- C++ 语言的设计和演化
- The C Programming Language（第2版）
- 深入理解 C 指针 / C 和指针 / C 专家编程
- C 陷阱与缺陷

### 3.4 评估

- PDF 内容无法通过 GitHub API 读取（LFS 指针）
- 书籍清单本身有参考价值，特别是 08_game/ 的游戏开发书单

---

## 4. XiaIM

| 属性 | 信息 |
|---|---|
| **地址** | https://github.com/xiaxia9/XiaIM |
| **类型** | 基于 C++ 的即时通讯系统 |
| **分支** | `master` |
| **客户端** | Windows (Qt 5.14.1, MSVC2017 64bit) |
| **服务端** | Linux |
| **参考博客** | https://blog.csdn.net/qq_38172320 |

### 4.1 项目结构

```
XiaIM/
├── IMClient/                                          ← Qt 客户端
├── IMServer/                                          ← Linux 服务端
├── build-IMClient-Desktop_Qt_5_14_1_MSVC2017_64bit-Debug/ ← 编译输出
└── README.md
```

### 4.2 架构说明

- **客户端**：基于 Qt 5.14 的 Windows 桌面 IM 应用
- **服务端**：Linux 环境，包含：
  - `gate_server` — 网关服务器（需配置内网/公网地址）
  - `login_server` — 登录服务器（需配置数据库连接）
- **技术栈**：C++ / Qt / 网络编程 / 数据库

### 4.3 评估

- 作者早期 C++ 网络编程项目
- 展示了客户端-服务端分离架构、网关模式、数据库连接等能力
- 与游戏开发主题关联较弱，但体现了 C++ 系统编程功底

---

## 5. blogs

| 属性 | 信息 |
|---|---|
| **地址** | https://github.com/xiaxia9/blogs |
| **类型** | 早期技术博客文章 |
| **分支** | `master` |
| **作者** | 知乎：爱吃菠萝不吃萝卜 / CSDN: qq_38172320 |

### 5.1 目录结构

```
blogs/
├── Android/     ← 1篇：基于《Android编程权威指南》前5章的 App
├── C++/
│   ├── C++.xmind        ← C++ 知识思维导图（32KB，二进制）
│   ├── 右值引用/         ← 右值引用相关文章
│   ├── 左值引用/         ← 左值引用相关文章
│   └── 虚函数/           ← 虚函数相关文章
├── IM/          ← 7篇：Spark/Openfire/XMPP/MINA 相关
├── Solve/
│   └── 安装UE4遇到的问题.md ← UE4 源码编译问题排查
└── README.md
```

### 5.2 UE4 编译问题排查 (Solve/安装UE4遇到的问题.md)

记录了从源码编译 UE4 遇到的常见问题：

| 问题 | 解决方法 |
|---|---|
| Setup.bat 执行后窗口消失 | 正常现象 |
| MSB3644 未找到 .NetFramework | VS Installer 勾选 .NET Framework 4.6.2 SDK + 目标包 |
| UE4Editor.exe 损坏映像 | 删除 Engine/Binaries/Win64 后重新生成 |
| D8049 命令行太长 | 修改安装路径长度 |
| C3859 PCH 虚拟内存范围 / C1076 堆限制 / C1002 堆空间不足 | 系统内存/虚拟内存相关配置 |

### 5.3 评估

- C++ 基础知识（左右值引用、虚函数）有一定学习价值
- IM 相关文章为 XMPP 协议方向
- UE4 编译问题排查有实用价值
- 整体内容零散且早期

---

## 总评

| Repo | 代码量 | 核心价值 | 推荐度 |
|---|---|---|---|
| **AnimationProject** | ~100KB C++ | ALS 风格完整运动/动画系统 C++ 重写，含攀爬/布娃娃/IK/原地转身/Overlay 系统 + 自研物理动画插件 | ⭐⭐⭐⭐⭐ |
| **NewLocomotionSystem** | ~200行 C++ | UE5 第三人称模板框架，核心逻辑在蓝图中 | ⭐⭐⭐ |
| **ebook** | 书单索引 | 游戏开发书籍收藏（引擎架构/RTR/GPG/GPU Gems/Shader 全系列） | ⭐⭐⭐ |
| **XiaIM** | 中等 | C++ 即时通讯系统，展示网络编程能力 | ⭐⭐ |
| **blogs** | 少量 | 早期 C++ 基础 + UE4 编译排查 | ⭐ |

> **关键发现**：`AnimationProject` 是最有价值的实战项目，代码量远超其他4个 Repo 总和，是一个完整的 ALS 风格高级角色动画系统的 C++ 实现。`NewLocomotionSystem` 更像是学习阶段的模板项目，而 `AnimationProject` 才是真正的进阶作品。