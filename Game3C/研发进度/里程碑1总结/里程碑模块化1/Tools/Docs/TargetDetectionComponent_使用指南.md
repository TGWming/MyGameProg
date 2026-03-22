# 目标检测组件 (TargetDetectionComponent) 使用指南

## 概述

`TargetDetectionComponent` 是一个独立的目标检测组件，已成功从 `MyCharacter` 中分离出来，提供模块化的目标搜索、验证、排序和尺寸分析功能。

## 功能特性

### 1. 核心目标检测功能
- **FindLockOnCandidates()**: 查找可锁定目标
- **IsValidLockOnTarget()**: 检查目标是否有效
- **IsTargetStillLockable()**: 检查目标是否仍然可以保持锁定
- **GetBestTargetFromList()**: 从目标列表中获取最佳目标
- **GetBestSectorLockTarget()**: 获取扇形区域内的最佳锁定目标
- **SortCandidatesByDirection()**: 按方向角度排序候选目标

### 2. 新增的敌人尺寸分析功能
- **GetTargetSizeCategory()**: 获取目标的尺寸分类
- **GetTargetsBySize()**: 根据尺寸分类获取目标列表
- **GetNearestTargetBySize()**: 获取指定尺寸分类中最近的目标
- **GetSizeCategoryStatistics()**: 获取所有尺寸分类统计信息

### 3. 敌人尺寸分类
- **Small**: 小型敌人 (边界盒 ≤ 150单位)
- **Medium**: 中型敌人 (边界盒 ≤ 400单位)
- **Large**: 大型敌人 (边界盒 > 400单位)
- **Unknown**: 未知/未分析尺寸

### 4. 事件委托系统
- **FOnTargetsUpdated**: 目标列表更新时触发
- **FOnValidTargetFound**: 发现有效目标时触发

## 性能优化

### 搜索间隔控制
- **目标搜索间隔**: 0.2秒 (5fps)
- **尺寸更新间隔**: 1.0秒 (1fps)

### 缓存系统
- **敌人尺寸缓存**: `TMap<AActor*, EEnemySizeCategory>`
- **自动清理**: 定期清理无效目标的缓存

## 配置参数

### LockOnSettings
```cpp
float LockOnRange = 2000.0f;           // 锁定范围
float LockOnAngle = 120.0f;            // 锁定角度
float SectorLockAngle = 60.0f;         // 扇形锁定角度
float EdgeDetectionAngle = 90.0f;      // 边缘检测角度
```

### AdvancedCameraSettings
```cpp
float SmallEnemySizeThreshold = 150.0f;    // 小型敌人阈值
float LargeEnemySizeThreshold = 400.0f;    // 大型敌人阈值
```

## 使用示例

### 在MyCharacter中的集成
```cpp
// 1. 头文件中包含组件
#include "TargetDetectionComponent.h"

// 2. 声明组件
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UTargetDetectionComponent* TargetDetectionComponent;

// 3. 构造函数中创建组件
TargetDetectionComponent = CreateDefaultSubobject<UTargetDetectionComponent>(TEXT("TargetDetectionComponent"));

// 4. BeginPlay中配置组件
TargetDetectionComponent->SetLockOnDetectionSphere(LockOnDetectionSphere);
```

### 调用目标检测功能
```cpp
// 查找目标
if (TargetDetectionComponent)
{
    TargetDetectionComponent->FindLockOnCandidates();
    LockOnCandidates = TargetDetectionComponent->GetLockOnCandidates();
}

// 获取不同尺寸的敌人
TArray<AActor*> SmallEnemies = GetTargetsBySize(EEnemySizeCategory::Small);
TArray<AActor*> LargeEnemies = GetTargetsBySize(EEnemySizeCategory::Large);

// 获取最近的大型敌人
AActor* NearestBoss = GetNearestTargetBySize(EEnemySizeCategory::Large);
```

### 调试功能
```cpp
// 显示目标尺寸分析信息
void AMyCharacter::DebugDisplayTargetSizes()
{
    // 获取统计信息
    TMap<EEnemySizeCategory, int32> Statistics = GetSizeCategoryStatistics();
    
    // 显示每个分类的数量和最近目标
    for (auto& Pair : Statistics)
    {
        FString CategoryName = UEnum::GetValueAsString(Pair.Key);
        UE_LOG(LogTemp, Warning, TEXT("- %s: %d enemies"), *CategoryName, Pair.Value);
    }
}
```

## 调试命令

### 输入绑定
- **F键**: `DebugInputTest()` - 一般调试信息
- **G键**: `DebugDisplayTargetSizes()` - 敌人尺寸分析调试

### 调试输出示例
```
=== TARGET SIZE ANALYSIS DEBUG ===
Enemy Count by Size Category:
- Small: 3 enemies
- Medium: 2 enemies  
- Large: 1 enemies
- Unknown: 0 enemies

Nearest Targets by Category:
- Small: Enemy_BP_C_1 (Distance: 456.7)
- Medium: Enemy_BP_C_2 (Distance: 678.9)
- Large: Boss_BP_C_1 (Distance: 1234.5)
===================================
```

## 验证清单

### ? 编译成功
- [x] TargetDetectionComponent.h 编译通过
- [x] TargetDetectionComponent.cpp 编译通过
- [x] MyCharacter.h 集成成功
- [x] MyCharacter.cpp 集成成功

### ? 功能完整性
- [x] 所有原始目标检测函数已迁移
- [x] 新增敌人尺寸分析功能
- [x] 事件委托系统实现
- [x] 性能优化机制
- [x] 调试接口完善

### ? 接口兼容性
- [x] MyCharacter所有原始函数接口保持不变
- [x] 蓝图接口完全兼容
- [x] 调试功能可正常使用

## 扩展建议

### 1. 增强尺寸分类
```cpp
// 可以基于敌人类型进一步细分
enum class EEnemyType : uint8
{
    Soldier,    // 士兵类
    Elite,      // 精英类
    Boss,       // Boss类
    Flying      // 飞行类
};
```

### 2. 动态配置
```cpp
// 运行时调整检测参数
UFUNCTION(BlueprintCallable)
void UpdateDetectionSettings(const FLockOnSettings& NewSettings);
```

### 3. 智能优先级
```cpp
// 基于威胁等级和距离的综合评分
float CalculateThreatScore(AActor* Target) const;
```

## 总结

目标检测组件已成功模块化，提供了：
- 完整的目标检测功能
- 新增的敌人尺寸分析系统
- 优化的性能表现
- 灵活的配置选项
- 完善的调试工具

所有功能已验证编译成功，可以立即在游戏中使用。