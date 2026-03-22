# MyCharacter 系统检查点

**创建日期**: 2024-12-19  
**版本**: v1.0.0  
**项目**: Soul Project  

## 系统概述

这是一个完整的第三人称角色控制系统，实现了类似《黑魂3》的锁定机制，包含智能目标切换、相机管理、UI系统和移动控制。

## 核心功能模块

### 1. 锁定系统 (Lock-On System)
- **扇形锁定区域**: 前方±50度范围内的优先锁定
- **边缘检测**: ±60度范围内的边缘目标检测
- **智能目标选择**: 基于距离和角度的评分算法
- **目标有效性检查**: 距离、视线遮挡检测
- **自动失锁**: 目标距离过远或失效时自动取消锁定

### 2. 相机管理系统
- **平滑跟踪**: 锁定时相机平滑跟随目标
- **自动修正**: 边缘目标的相机自动调整
- **平滑重置**: 取消锁定时相机平滑回到角色后方
- **输入检测**: 玩家输入时停止自动相机控制
- **角度阈值**: 小角度切换时减少相机移动

### 3. 目标切换系统
- **方向排序**: 目标按左右方向自动排序
- **平滑切换**: 大角度切换时的平滑过渡
- **摇杆控制**: 右摇杆左右切换目标
- **键盘支持**: 左右箭头键切换目标
- **防误触**: 摇杆阈值和冷却机制

### 4. UI系统
- **Widget组件**: 每个目标的3D UI显示
- **屏幕空间UI**: 备用的2D UI系统
- **智能隐藏**: 自动隐藏非当前目标的UI
- **强制清理**: 多重保险机制确保UI正确显示

### 5. 移动控制
- **双模式移动**: 自由移动和锁定移动
- **速度控制**: 锁定状态下的移动速度调整
- **身体转向**: 锁定时角色朝向目标
- **移动检测**: 玩家移动时的状态切换

### 6. Socket投射系统
- **精确定位**: 基于目标Socket的精确UI定位
- **屏幕投射**: 3D位置到屏幕坐标的转换
- **偏移控制**: 可配置的Socket偏移
- **兼容性**: 向后兼容现有系统

## 技术架构

### 组件结构
```cpp
// 相机组件
USpringArmComponent* CameraBoom
UCameraComponent* FollowCamera

// 检测组件
USphereComponent* LockOnDetectionSphere

// UI组件
TSubclassOf<UUserWidget> LockOnWidgetClass
UUserWidget* LockOnWidgetInstance
```

### 状态管理
```cpp
// 锁定状态
bool bIsLockedOn
AActor* CurrentLockOnTarget
TArray<AActor*> LockOnCandidates

// 相机控制状态
bool bShouldCameraFollowTarget
bool bShouldCharacterRotateToTarget
bool bPlayerIsMoving

// 平滑切换状态
bool bIsSmoothSwitching
bool bShouldSmoothSwitchCamera
bool bShouldSmoothSwitchCharacter

// 相机修正状态
bool bIsCameraAutoCorrection
AActor* DelayedCorrectionTarget

// 相机重置状态
bool bIsSmoothCameraReset
```

### 常量配置
```cpp
// 性能常量
static constexpr float TARGET_SEARCH_INTERVAL = 0.2f;
static constexpr float CAMERA_INTERP_SPEED = 5.0f;
static constexpr float CHARACTER_ROTATION_SPEED = 10.0f;

// 锁定区域常量
static constexpr float SECTOR_LOCK_ANGLE = 100.0f;
static constexpr float EDGE_DETECTION_ANGLE = 120.0f;
static constexpr float CAMERA_AUTO_CORRECTION_SPEED = 7.0f;

// 切换控制常量
static constexpr float TARGET_SWITCH_ANGLE_THRESHOLD = 20.0f;
static constexpr float TARGET_SWITCH_SMOOTH_SPEED = 3.0f;
static constexpr float THUMBSTICK_THRESHOLD = 0.5f;
```

## 核心算法

### 1. 目标评分算法
```cpp
float AngleFactor = DotProduct;
float DistanceFactor = 1.0f - NormalizedDistance;
float Score = (AngleFactor * 0.7f) + (DistanceFactor * 0.3f);
```

### 2. 扇形检测算法
```cpp
float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
bool bInSectorZone = AngleDegrees <= (SECTOR_LOCK_ANGLE * 0.5f);
```

### 3. 平滑插值系统
```cpp
FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, 
    DeltaTime, InterpolationSpeed);
```

## 输入绑定

### 基础控制
- `MoveForward` / `MoveRight`: WASD移动
- `Turn` / `LookUp`: 鼠标相机控制
- `Jump`: 空格跳跃

### 锁定控制
- `LockOn`: 右摇杆按下/鼠标中键
- `RightStickX`: 右摇杆水平轴（目标切换）
- `SwitchTargetLeft` / `SwitchTargetRight`: 左右箭头键

### 调试
- `DebugInput`: F键调试信息

## 性能优化

### 1. 目标搜索优化
- 定时搜索: 每0.2秒更新一次候选目标
- 早期退出: 无效目标立即跳过
- 范围限制: 球体碰撞检测减少计算量

### 2. 状态管理优化
- 条件检查: 只在必要时执行更新
- 状态缓存: 避免重复计算
- 时间控制: 基于时间的状态切换

### 3. UI优化
- 按需创建: 只有锁定时才显示UI
- 批量隐藏: 一次性隐藏所有无关UI
- 强制清理: 多重保险确保内存释放

## 已知特性

### 1. 智能相机行为
- 玩家移动时立即停止自动相机控制
- 小角度切换时不移动相机，减少晕眩
- 相机重置采用平滑插值，体验友好

### 2. 鲁棒性设计
- 多重UI清理机制防止遗留显示
- 输入检测机制防止与玩家操作冲突
- 目标有效性检查防止锁定失效目标

### 3. 扩展性考虑
- Socket投射系统为将来的精确UI定位做准备
- 模块化设计便于功能扩展
- 参数化配置支持运行时调整

## 测试要点

### 1. 基础功能测试
- [ ] 锁定/取消锁定正常工作
- [ ] 目标切换响应及时
- [ ] 相机跟踪平滑自然
- [ ] UI显示隐藏正确

### 2. 边界条件测试
- [ ] 目标距离过远时自动失锁
- [ ] 视线遮挡时的处理
- [ ] 快速移动时的相机响应
- [ ] 多目标环境下的切换

### 3. 性能测试
- [ ] 大量目标时的帧率稳定性
- [ ] 内存泄漏检查
- [ ] UI资源正确释放

## 文件结构

### 头文件 (MyCharacter.h)
- 类声明和成员变量定义
- 常量定义和前置声明
- 公共接口函数声明

### 实现文件 (MyCharacter.cpp)
- 构造函数和初始化
- 核心系统实现
- 输入处理和事件响应
- UI管理和相机控制

## 编译环境

- **C++标准**: C++14
- **引擎版本**: Unreal Engine (推测4.x或5.x)
- **平台**: Windows (基于PowerShell工具)

## 下一步开发建议

1. **Socket投射系统完善**: 完成精确UI定位功能
2. **动画系统集成**: 添加锁定状态的角色动画
3. **音效系统**: 锁定和切换目标的音效反馈
4. **配置系统**: 运行时参数调整界面
5. **网络同步**: 多人游戏的锁定状态同步

---

**注意**: 此检查点记录了当前系统的完整状态，包括所有已实现的功能和已知的特性。在进行后续开发时，请参考此文档以确保系统的一致性和稳定性。