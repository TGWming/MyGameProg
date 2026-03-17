# 锁定系统调试指南

## 问题诊断步骤

### 1. 检查敌人设置
确保场景中的敌人Actor有以下设置：
- **标签（Tag）**: 在敌人Actor的Tags数组中添加 "Enemy" 标签
  - 选中敌人Actor → Details面板 → Actor → Tags → 添加 "Enemy"
- **碰撞设置**: 敌人的Collision Object Type应该是 Pawn

### 2. 检查玩家Character蓝图设置
在您的玩家Character蓝图中（基于AMycharacter）：

#### UIManagerComponent设置（非常重要！）
1. 选择 UIManagerComponent
2. 在Details面板中找到以下属性并设置：

**关键设置：**
- **Lock On Widget Class**: 
  - 点击下拉菜单选择：`UI_LockOnWidget`
  - 或者点击浏览按钮，导航到：
    ```
    /Game/LockOnTS/Widgets/UI_LockOnWidget
    ```
  - **这是最重要的设置！如果没设置，Widget不会显示！**

- **Current UI Display Mode**: 选择 `ScreenSpace` 或 `SocketProjection`
  - ScreenSpace: 2D屏幕空间UI（推荐用于测试）
  - SocketProjection: 3D投影到屏幕
  - Traditional3D: 3D世界空间（需要敌人有WidgetComponent）

- **Enable Size Adaptive UI**: 可选，建议先设为 False 测试

#### TargetDetectionComponent设置
- **Lock On Range**: 2000.0（默认值，可根据需要调整）
- **Lock On Angle**: 120.0（视野角度）
- **Sector Lock Angle**: 60.0（扇形锁定角度）

### 3. 输入设置检查
输入绑定已自动添加到 `Config/DefaultInput.ini`:

**键盘+鼠标：**
- **锁定/解锁**: 鼠标中键 (MiddleMouseButton)
- **切换目标左**: Q
- **切换目标右**: E

**手柄：**
- **锁定/解锁**: 按下右摇杆 (Gamepad_RightThumbstick)
- **切换目标**: 左右摇动右摇杆或D-Pad左右

### 4. 测试步骤

#### 第一步：检查日志
1. 在编辑器中运行游戏（PIE）
2. 打开Output Log窗口（Window → Developer Tools → Output Log）
3. 按下锁定键（鼠标中键）
4. 查看日志输出，应该看到：

```
LogTemp: Warning: ===== ToggleLockOn Called =====
LogTemp: Warning: Current bIsLockedOn: FALSE
LogTemp: Warning: TargetDetectionComponent is valid, finding candidates...
LogTemp: Warning: Found X lock-on candidates
LogTemp: Warning: Best target found: BP_Enemy_C_0
LogTemp: Warning: ===== StartLockOn =====
LogTemp: Warning: Target: BP_Enemy_C_0
LogTemp: Warning: UIManagerComponent is valid, showing widget...
LogTemp: Warning: LockOnWidgetClass is SET
LogTemp: Warning: ===== UIManagerComponent::ShowLockOnWidget =====
LogTemp: Warning: LockOnWidgetClass: UI_LockOnWidget_C
LogTemp: Warning: Creating widget...
LogTemp: Warning: Widget created successfully!
LogTemp: Warning: Widget added to viewport
```

#### 第二步：根据日志诊断

**如果看到 "Found 0 lock-on candidates":**
- 检查敌人是否在锁定范围内（默认2000单位）
- 检查敌人是否有 "Enemy" 标签
- 检查敌人的碰撞设置（Object Type应该是Pawn）

**如果看到 "TargetDetectionComponent is NULL!":**
- 检查玩家Character是否正确继承自AMycharacter
- 重新编译C++代码并刷新蓝图

**如果看到 "LockOnWidgetClass is NULL":**
- **这是最常见的问题！**
- 在玩家Character蓝图中，选择UIManagerComponent
- 设置LockOnWidgetClass为UI_LockOnWidget

**如果看到 "Failed to create widget instance!":**
- 检查Widget路径是否正确
- 尝试在Content Browser中打开UI_LockOnWidget，确保它能正常打开
- 检查Widget类是否有编译错误

**如果锁定成功但看不到UI:**
- 检查CurrentUIDisplayMode设置（应该是ScreenSpace）
- 检查Widget的可见性设置
- 在Widget蓝图中添加测试图形（如一个简单的Image或Text）

### 5. Widget蓝图检查

打开 `/Game/LockOnTS/Widgets/UI_LockOnWidget`:

1. 确保有可见的UI元素（Image、Border等）
2. 检查Canvas Panel的层级结构
3. 确保UI元素的Visibility设置为Visible
4. 建议添加一个简单的彩色Image用于测试，确保能看到

**最小测试Widget结构：**
```
Canvas Panel
  └─ Image (设置颜色为红色，大小为100x100)
```

### 6. 常见问题解决方案

#### 问题：按锁定键没反应
**解决方案：**
- 检查输入绑定（Config/DefaultInput.ini）
- 确认在Character的SetupPlayerInputComponent中绑定了LockOn
- 尝试不同的按键

#### 问题：找不到敌人
**解决方案：**
- 给敌人添加"Enemy"标签
- 增加LockOnRange（在TargetDetectionComponent中）
- 确保敌人在相机前方

#### 问题：Widget不显示
**解决方案：**
1. **最重要**：在UIManagerComponent中设置LockOnWidgetClass
2. 设置CurrentUIDisplayMode为ScreenSpace
3. 检查Widget蓝图是否有可见的UI元素
4. 查看Output Log中的错误信息

#### 问题：锁定后立即失去目标
**解决方案：**
- 检查Extended Lock Range Multiplier（应该>1.0）
- 确保敌人不会立即移出范围
- 检查视线检查（Line of Sight）设置

### 7. 快速测试代码

如果仍然有问题，可以在玩家Character蓝图的BeginPlay事件中添加测试：

1. 打开玩家Character蓝图
2. 在Event Graph中添加：
   - Event BeginPlay
   - 连接到 Print String
   - String: "UIManager Widget Class: {LockOnWidgetClass}"（从UIManagerComponent获取）

这样可以确认Widget类是否正确设置。

### 8. 最终检查清单

- [ ] 敌人有"Enemy"标签
- [ ] UIManagerComponent的LockOnWidgetClass已设置为UI_LockOnWidget
- [ ] CurrentUIDisplayMode设置为ScreenSpace
- [ ] Widget蓝图中有可见的UI元素
- [ ] 输入绑定正确（按中键或右摇杆）
- [ ] 敌人在锁定范围内（2000单位）
- [ ] 查看Output Log确认无错误
- [ ] Character正确继承自AMycharacter类

## 调试命令

在游戏运行时，按 `~` 打开控制台，输入以下命令：

```
ShowDebug COLLISION  // 显示碰撞调试信息
Stat FPS            // 显示FPS
```

## 需要更多帮助？

如果按照以上步骤仍然无法解决问题，请：
1. 截图Output Log的完整输出
2. 截图Character蓝图中UIManagerComponent的设置
3. 确认Widget资源的完整路径
4. 说明具体的错误信息

---

**最常见的问题总结：**
90%的问题是因为**没有在Character蓝图的UIManagerComponent中设置LockOnWidgetClass**！
请务必检查这一设置！
