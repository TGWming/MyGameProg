# 输入问题已修复 - 测试指南

## ✅ 问题已解决

**问题原因：** 输入绑定名称不匹配
- C++代码期望：`"LockOn"`
- 项目设置中：`"SetLockon"`

**解决方案：** 现在代码同时支持两种名称！

---

## 🎮 测试步骤

### 1. 准备测试环境

#### 设置敌人（重要！）
1. 在场景中放置一个Character或Pawn
2. 选中敌人
3. Details面板 → Actor → Tags
4. 添加标签：`Enemy` （大小写敏感！）
5. 确保敌人距离玩家 **< 2000 单位**

#### 确认玩家设置
打开玩家Character蓝图：
1. 选择 **UIManagerComponent**
2. 在Details中设置：
   - **Lock On Widget Class** = `UI_LockOnWidget`
   - **Current UI Display Mode** = `ScreenSpace`

### 2. 运行测试

1. **启动PIE**（Alt+P）
2. **打开Output Log**（Window → Developer Tools → Output Log）
3. **按下锁定键**：
   - 🖱️ **鼠标中键** (Middle Mouse Button)
   - 🎮 **手柄右摇杆按下** (Right Thumbstick Button)

### 3. 观察日志输出

**成功锁定时应该看到：**
```
LogTemp: Warning: ===== ToggleLockOn Called =====
LogTemp: Warning: Current bIsLockedOn: FALSE
LogTemp: Warning: TargetDetectionComponent is valid, finding candidates...
LogTemp: Warning: Found 1 lock-on candidates
LogTemp: Warning: Best target found: BP_Enemy_C_0
LogTemp: Warning: ===== StartLockOn =====
LogTemp: Warning: Target: BP_Enemy_C_0
LogTemp: Warning: UIManagerComponent is valid, showing widget...
LogTemp: Warning: LockOnWidgetClass is SET
LogTemp: Warning: ===== UIManagerComponent::ShowLockOnWidget =====
LogTemp: Warning: Widget created successfully!
LogTemp: Warning: Widget added to viewport
```

**再次按锁定键解锁：**
```
LogTemp: Warning: ===== ToggleLockOn Called =====
LogTemp: Warning: Current bIsLockedOn: TRUE
LogTemp: Log: Lock-on cancelled
```

### 4. 可能遇到的问题

#### ❌ "Found 0 lock-on candidates"
**原因：**
- 敌人没有"Enemy"标签
- 敌人距离 > 2000单位
- 敌人不在视野范围内（±60度）

**解决：**
- 给敌人添加"Enemy"标签
- 移动敌人到玩家附近
- 确保敌人在摄像机前方

#### ❌ "LockOnWidgetClass is NULL"
**原因：** Character蓝图中未设置Widget类

**解决：**
1. 打开Character蓝图
2. 选择UIManagerComponent
3. 设置Lock On Widget Class = UI_LockOnWidget

#### ❌ 按键没反应，日志没输出
**检查：**
1. 确认您的Character蓝图是基于 `AMycharacter` C++类
2. 检查Project Settings → Input中是否有以下任一：
   - `LockOn` 动作映射
   - `SetLockon` 动作映射
3. 重新编译C++代码

---

## 🔧 输入配置检查

### 在Project Settings中应该有：

**Action Mappings:**
```
SetLockon (或 LockOn)
  ├─ Middle Mouse Button
  └─ Gamepad Right Thumbstick (Button)
```

### 手柄支持
- **锁定/解锁**：按下右摇杆
- **切换目标**：左右推动右摇杆

---

## 📊 当前配置

### 锁定系统参数
- **检测范围**：2000 单位
- **视野角度**：±60度（总共120度）
- **检测类型**：Pawn类（包括Character）
- **需要标签**：不强制，但建议添加"Enemy"
- **视线检查**：启用

### 输入绑定（已修复）
现在同时支持：
- ✅ `LockOn` 
- ✅ `SetLockon`

两个名称都可以工作！

---

## 🎯 快速验证清单

测试前确认：
- [ ] 敌人是Pawn/Character类型
- [ ] 敌人有"Enemy"标签（可选但推荐）
- [ ] 敌人在2000单位范围内
- [ ] 敌人在摄像机前方
- [ ] UIManagerComponent的Widget类已设置
- [ ] 项目已重新编译
- [ ] Output Log已打开

---

## 🐛 调试命令

游戏运行时按 `~` 打开控制台：

```
// 显示碰撞调试信息
ShowDebug COLLISION

// 显示输入调试
ShowDebug INPUT

// 查看当前玩家状态
GetAll Mycharacter bIsLockedOn
```

---

## 📝 预期行为

### 锁定成功后：
1. ✅ 角色移动速度变为LockedOnSpeed（400）
2. ✅ 角色不再自动朝向移动方向
3. ✅ 屏幕上显示锁定UI（如果Widget配置正确）
4. ✅ Output Log显示"Locked on to: [敌人名]"

### 解锁后：
1. ✅ 角色移动速度恢复
2. ✅ 角色恢复自动朝向移动方向
3. ✅ 锁定UI消失
4. ✅ Output Log显示"Lock-on cancelled"

---

**现在去测试吧！按中键或右摇杆应该可以正常锁定了！** 🎮
