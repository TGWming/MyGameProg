# Soul Performance Profiler System

## 概述
Soul性能分析器是一个专为soul项目设计的性能监控系统，可以帮助开发者分析和优化代码性能。

## 主要组件

### 1. UPerformanceProfiler 子系统
- **功能**: 核心性能数据收集和分析
- **类型**: UGameInstanceSubsystem
- **位置**: `Source/soul/PerformanceProfiler.h/cpp`

### 2. FSoulPerformanceScope 结构体
- **功能**: 自动性能测量作用域
- **位置**: `DebugManager.h` 和 `PerformanceProfiler.h`

### 3. 性能监控宏
- **SOUL_PERFORMANCE_SCOPE(FunctionName)**: 基本性能监控宏
- **SOUL_PERFORMANCE_SCOPE_CONDITIONAL(FunctionName, Condition)**: 条件性能监控宏

## 核心功能

### 性能数据结构 (FPerformanceData)
```cpp
struct FPerformanceData
{
    FString FunctionName;    // 函数名称
    float AverageTime;       // 平均执行时间（毫秒）
    float MaxTime;          // 最大执行时间（毫秒）
    float MinTime;          // 最小执行时间（毫秒）
    int32 CallCount;        // 调用次数
    float TotalTime;        // 总执行时间（毫秒）
};
```

### 主要接口函数
- `RecordFunctionTime(FunctionName, ElapsedTime)`: 记录函数执行时间
- `GetPerformanceReport()`: 获取性能报告
- `ResetPerformanceData()`: 重置性能数据
- `PrintPerformanceReport()`: 打印性能报告到日志
- `GetFunctionPerformanceData(FunctionName)`: 获取特定函数性能数据

## 使用方法

### 1. 基本用法 - 手动记录
```cpp
// 获取性能分析器实例
UPerformanceProfiler* Profiler = UPerformanceProfiler::GetPerformanceProfiler(this);
if (Profiler)
{
    // 手动记录性能数据
    Profiler->RecordFunctionTime(TEXT("MyFunction"), 5.0f); // 5毫秒
}
```

### 2. 自动测量 - 使用作用域宏
```cpp
void MyClass::SomeFunction()
{
    // 自动测量整个函数执行时间
    SOUL_PERFORMANCE_SCOPE(TEXT("SomeFunction"));
    
    // 你的代码逻辑
    // ...
} // 函数结束时自动记录执行时间
```

### 3. 条件性能监控
```cpp
void MyClass::ConditionalFunction()
{
    bool bShouldProfile = GetSomeCondition();
    
    // 只在满足条件时进行性能监控
    SOUL_PERFORMANCE_SCOPE_CONDITIONAL(TEXT("ConditionalFunction"), bShouldProfile);
    
    // 你的代码逻辑
    // ...
}
```

### 4. 获取和打印性能报告
```cpp
// 获取性能分析器
UPerformanceProfiler* Profiler = UPerformanceProfiler::GetPerformanceProfiler(this);
if (Profiler)
{
    // 打印完整性能报告
    Profiler->PrintPerformanceReport();
    
    // 获取特定函数的性能数据
    FPerformanceData Data = Profiler->GetFunctionPerformanceData(TEXT("MyFunction"));
    if (Data.CallCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyFunction: Avg=%.3fms, Max=%.3fms, Calls=%d"), 
            Data.AverageTime, Data.MaxTime, Data.CallCount);
    }
}
```

## 示例性能报告输出

```
=== SOUL PERFORMANCE REPORT ===
Generated at: 2024-10-04 14:30:25
Total functions tracked: 5

Function Name                            | Avg (ms)   | Max (ms)   | Min (ms)   | Calls
----------------------------------------|------------|------------|------------|--------
SimulateSlowFunction                    |     15.423 |     18.234 |     12.156 |       10
SimulateMediumFunction                  |      5.234 |      6.123 |      4.567 |       15
TestFunction_Variable                   |      5.123 |      9.876 |      1.234 |        5
SimulateFastFunction                    |      0.156 |      0.234 |      0.123 |       20
TestFunction_Fast                       |      0.500 |      0.500 |      0.500 |        1

SUMMARY:
- Total execution time: 482.345ms
- Total function calls: 51
- Slowest function: SimulateSlowFunction (avg: 15.423ms)
==============================
```

## 测试组件

### UPerformanceTestComponent
- **位置**: `Source/soul/PerformanceTestComponent.h/cpp`
- **功能**: 演示和测试性能分析器的各种功能
- **使用方法**: 
  1. 将此组件添加到任何Actor上
  2. 组件会自动运行性能测试
  3. 查看输出日志了解性能分析器工作情况

### 测试函数
- `TestBasicPerformanceMonitoring()`: 测试基本性能监控
- `TestPerformanceScopeMacros()`: 测试性能作用域宏
- `TestPerformanceReporting()`: 测试性能报告生成
- `RunFullPerformanceTestSuite()`: 运行完整测试套件

## 配置和控制

### 启用/禁用性能监控
```cpp
UPerformanceProfiler* Profiler = UPerformanceProfiler::GetPerformanceProfiler(this);
if (Profiler)
{
    // 启用性能监控
    Profiler->SetPerformanceMonitoringEnabled(true);
    
    // 检查是否启用
    bool bEnabled = Profiler->IsPerformanceMonitoringEnabled();
}
```

### 重置性能数据
```cpp
// 清除所有已收集的性能数据
Profiler->ResetPerformanceData();
```

## 最佳实践

### 1. 合理使用性能监控
- 不要在每个小函数中都使用性能监控
- 重点监控关键路径和性能敏感的函数
- 在发布版本中可以考虑禁用性能监控

### 2. 函数命名
- 使用描述性的函数名称
- 避免使用太长的名称（报告中会截断）
- 可以使用类名::函数名的格式

### 3. 监控范围
```cpp
void MyClass::ComplexFunction()
{
    SOUL_PERFORMANCE_SCOPE(TEXT("MyClass::ComplexFunction"));
    
    // 第一部分逻辑
    {
        SOUL_PERFORMANCE_SCOPE(TEXT("MyClass::ComplexFunction_Part1"));
        // ...
    }
    
    // 第二部分逻辑
    {
        SOUL_PERFORMANCE_SCOPE(TEXT("MyClass::ComplexFunction_Part2"));
        // ...
    }
}
```

## 注意事项

1. **性能开销**: 性能监控本身会带来小的性能开销，建议在开发和调试阶段使用
2. **内存使用**: 性能数据会保存在内存中，定期调用`ResetPerformanceData()`清理
3. **线程安全**: 当前实现假设在主线程中使用，多线程环境需要额外考虑
4. **持久化**: 性能数据不会自动保存到文件，如需持久化请手动实现

## 扩展功能

系统设计允许轻松扩展，可以添加：
- 内存使用监控
- CPU使用率监控
- 自定义性能指标
- 性能数据导出功能
- 实时性能图表显示

## 故障排除

### 1. 性能分析器实例为空
- 确保在有效的World上下文中调用
- 检查GameInstance是否存在

### 2. 宏不工作
- 确保包含了正确的头文件
- 检查性能监控是否已启用

### 3. 报告为空
- 确保调用了性能监控函数
- 检查是否调用了`ResetPerformanceData()`

## 版本信息

- **创建时间**: 2024-10-04
- **适用版本**: Unreal Engine 4.27
- **兼容性**: C++14
- **模块**: soul