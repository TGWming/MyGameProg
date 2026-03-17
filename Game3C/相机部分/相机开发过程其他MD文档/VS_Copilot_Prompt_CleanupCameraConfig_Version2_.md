# 任务：清理 CameraSetupConfig 并重构相机架构

## 目标
移除 C++ 中的相机配置和创建代码，改为从蓝图中查找相机组件的引用模式。

---

## 任务计划

### 第一步：删除 CameraSetupConfig
1. 删除 `CameraSetupConfig.h` 文件
2. 移除所有对该文件的 `#include` 引用
3.  移除 `FCameraSetupConfig` 相关的属性声明和使用

### 第二步：重构相机组件为引用模式
1. 删除构造函数中的 `CreateDefaultSubobject` 相机创建代码
2. 删除构造函数中的相机参数设置代码
3. 将相机组件指针重命名为 `CameraBoomRef` 和 `FollowCameraRef`
4. 构造函数中初始化为 `nullptr`

### 第三步：添加 BeginPlay 查找逻辑
在 `BeginPlay()` 中使用 `FindComponentByClass` 查找蓝图中添加的 `USpringArmComponent` 和 `UCameraComponent`，并添加日志确认。

### 第四步：更新所有引用
将项目中所有使用旧变量名的代码更新为新变量名。

---

## 注意
- 保留 `LockOnConfig.  h`（与本次任务无关）
- 相机参数将改为在蓝图组件上设置