# 创建测试敌人 - 快速指南

## 方法1：修改现有敌人

如果场景中已经有敌人Actor：

1. **选中敌人Actor**
2. **在Details面板中：**
   - 找到 `Actor` 分类
   - 展开 `Tags`
   - 点击 `+` 添加新标签
   - 输入: `Enemy`
   - 按Enter确认

3. **检查碰撞设置：**
   - 找到 `Collision` 分类
   - `Collision Presets` → Custom 或 Pawn
   - 确保 `Object Type` 是 `Pawn` 或 `WorldDynamic`
   - `Collision Enabled` → Query and Physics 或 Query Only

完成！敌人现在应该可以被锁定了。

## 方法2：创建新的测试敌人

### 步骤1：创建简单的敌人蓝图

1. **Content Browser** → 右键 → Blueprint Class
2. 选择 **Character** 作为父类
3. 命名为 `BP_TestEnemy`

### 步骤2：配置蓝图

打开 `BP_TestEnemy`:

#### Class Defaults设置：
1. 点击工具栏的 **Class Defaults** 按钮
2. 在Details面板中：
   - **Tags** → 添加 `Enemy`

#### 组件设置：
1. 选择 **Mesh (Inherited)** 组件
2. 可选：设置一个临时Mesh用于可视化（如Cube或Sphere）
   - 如果没有mesh，角色仍然可以工作，只是看不见

3. 选择 **CapsuleComponent (Inherited)**
   - 这是碰撞检测的主要组件
   - 默认设置通常就可以了

#### 可选：添加LockOn Socket（推荐）

如果您想要精确的锁定位置：

1. 选择 **Mesh** 组件
2. 如果有Skeletal Mesh：
   - 在Skeleton中添加Socket
   - 命名为 `LockOnSocket`
   - 位置：通常在头部或身体中心上方

3. 如果只是测试，可以跳过此步骤
   - 系统会自动使用Capsule中心

### 步骤3：放置到场景

1. 将 `BP_TestEnemy` 拖放到关卡中
2. 确保敌人在玩家角色附近（距离 < 2000单位）
3. 确保敌人在玩家前方（摄像机视野内）

### 步骤4：测试

1. 运行游戏（Alt+P 或点击Play）
2. 靠近敌人
3. 按 **鼠标中键** 或 **右摇杆点击** 锁定
4. 查看 **Output Log** 确认锁定是否成功

## 方法3：快速视觉调试

### 添加简单的可视化

如果敌人看不见，添加一个简单的可视化：

1. 打开 `BP_TestEnemy`
2. 添加组件 → Static Mesh Component
3. 选择一个简单的mesh（如 Cube 或 Sphere）
4. 调整Transform让它在Capsule内部
5. 设置Material为明显的颜色（如红色）

### 测试用敌人的完整设置示例

```
BP_TestEnemy (Character Blueprint)
│
├─ CapsuleComponent (根组件)
│   └─ Collision: Pawn
│
├─ Mesh (SkeletalMeshComponent)
│   └─ (可选) Socket: "LockOnSocket"
│
├─ (可选) DebugMesh (StaticMeshComponent)
│   ├─ Mesh: SM_Cube
│   └─ Material: 红色
│
└─ Tags
    └─ "Enemy"
```

## C++敌人类（可选）

如果您想创建C++敌人类：

```cpp
// EnemyCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class MYPROJECT_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

protected:
    virtual void BeginPlay() override;
};

// EnemyCharacter.cpp
#include "EnemyCharacter.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // 添加Enemy标签
    Tags.Add(FName("Enemy"));
    
    // 设置碰撞
    GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
}
```

## 验证敌人设置

运行游戏后，在控制台输入：
```
ShowDebug COLLISION
```

这会显示所有碰撞体，帮助您确认敌人是否正确设置。

## 问题排查

### 问题：找不到敌人
- 检查敌人是否有"Enemy"标签（大小写敏感！）
- 检查距离（应该 < 2000单位）
- 确保敌人在摄像机前方

### 问题：可以锁定但看不到敌人
- 添加一个DebugMesh或设置Mesh可见
- 检查敌人是否在地面下方

### 问题：锁定后Widget不显示
- 参考 LOCKON_DEBUG_GUIDE.md
- 检查UIManagerComponent的LockOnWidgetClass设置

---

**快速检查命令：**
选中敌人 → Details面板 → 搜索"tags" → 确认有"Enemy"标签
