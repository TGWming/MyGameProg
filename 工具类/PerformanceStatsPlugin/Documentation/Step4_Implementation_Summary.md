# Step 4: 帧性能数据采集 + 定时刷新 - 实现总结

## 完成状态 ?

所有Step 4相关文件已成功实现并编译通过。

---

## 实现的文件

### 1. PerformanceDataCollector.h
**位置**: `Plugins/PerformanceStatsPlugin/Source/PerformanceStatsPlugin/Public/PerformanceDataCollector.h`

**功能**:
- 定义了 `ECostLevel` 枚举（Green/Yellow/Red）用于表示性能开销等级
- 定义了 `FFramePerformanceData` 结构体，包含：
  - GameThreadTime（游戏线程时间）
  - RenderThreadTime（渲染线程时间）
  - GPUTime（GPU时间）
  - TotalFrameTime（总帧时间）
  - FPS（帧率）
  - 各线程的成本等级（GameThreadCost, RenderThreadCost, GPUCost）
- 定义了 `FPerformanceDataCollector` 单例类，负责采集性能数据

**关键特性**:
- 使用 `USTRUCT(BlueprintType)` 使数据可以暴露给蓝图
- 使用 `UENUM(BlueprintType)` 使枚举可以在蓝图中使用
- 采用单例模式便于全局访问

### 2. PerformanceDataCollector.cpp
**位置**: `Plugins/PerformanceStatsPlugin/Source/PerformanceStatsPlugin/Private/PerformanceDataCollector.cpp`

**功能**:
- 实现了帧性能数据的采集逻辑
- 使用 `FApp::GetDeltaTime()` 获取帧时间
- 计算FPS（1000ms / 帧时间）
- 根据阈值计算性能等级：
  - 绿色：< 16.67ms (>60 FPS)
  - 黄色：16.67ms ~ 33.33ms (30-60 FPS)
  - 红色：>= 33.33ms (<30 FPS)

**注意事项**:
- 当前版本使用总帧时间作为各线程时间的基准
- RenderThreadTime 和 GPUTime 使用估算值（分别为总帧时间的 80% 和 70%）
- 后续步骤可以通过UE的Stats系统获取更精确的线程数据

### 3. PerformanceConsoleManager.h
**位置**: `Plugins/PerformanceStatsPlugin/Source/PerformanceStatsPlugin/Public/PerformanceConsoleManager.h`

**功能**:
- 定义了控制台性能数据管理器类
- 管理性能数据的定时刷新和显示
- Windows平台支持彩色控制台输出

**关键方法**:
- `Initialize()` - 初始化管理器，注册Tick回调
- `Shutdown()` - 清理资源，注销回调
- `SetEnabled(bool)` / `IsEnabled()` - 控制启用/禁用
- `SetPaused(bool)` / `IsPaused()` - 控制暂停/恢复
- `Tick(float)` - 每帧更新，定时刷新显示
- `RefreshDisplay()` - 刷新控制台显示
- `PrintFrameData()` - 打印帧数据

**成员变量**:
- `UpdateInterval` - 刷新间隔（默认1秒）
- `TimeSinceLastUpdate` - 距离上次刷新的时间
- `bEnabled` - 是否启用
- `bPaused` - 是否暂停
- `ConsoleHandle` - Windows控制台句柄（仅Windows平台）

### 4. PerformanceConsoleManager.cpp
**位置**: `Plugins/PerformanceStatsPlugin/Source/PerformanceStatsPlugin/Private/PerformanceConsoleManager.cpp`

**功能**:
- 实现了控制台管理器的所有方法
- 使用 `FCoreDelegates::OnEndFrame` 注册Tick回调
- Windows平台使用 `SetConsoleTextAttribute` 设置控制台颜色
- 根据性能等级显示不同颜色：
  - Green → 绿色
  - Yellow → 黄色
  - Red → 红色

**工作流程**:
1. `Initialize()` 时获取控制台句柄并注册Tick回调
2. 每帧 `Tick()` 被调用，累加时间
3. 达到刷新间隔时调用 `RefreshDisplay()`
4. `RefreshDisplay()` 采集数据并调用 `PrintFrameData()`
5. `PrintFrameData()` 使用 `UE_LOG` 输出数据，并设置相应颜色

---

## 模块集成

### PerformanceStatsPluginModule.cpp
已更新为在引擎初始化后自动启动性能监控：

```cpp
void FPerformanceStatsPluginModule::StartupModule()
{
    FCoreDelegates::OnPostEngineInit.AddLambda([]()
    {
        FPerformanceConsoleManager::Get().Initialize();
        UE_LOG(LogTemp, Log, TEXT("PerformanceStatsPlugin: Module started (delayed init)"));
    });
}

void FPerformanceStatsPluginModule::ShutdownModule()
{
    FPerformanceConsoleManager::Get().Shutdown();
    UE_LOG(LogTemp, Log, TEXT("PerformanceStatsPlugin: Module shutdown"));
}
```

---

## 蓝图函数库集成

`PerformanceStatsFunctionLibrary.cpp` 中的以下函数已可以正常工作：

- `EnablePerformanceMonitor()` - 启用性能监控
- `DisablePerformanceMonitor()` - 禁用性能监控
- `TogglePerformanceMonitor()` - 切换启用状态
- `IsPerformanceMonitorEnabled()` - 检查是否启用
- `PausePerformanceMonitor()` - 暂停监控
- `ResumePerformanceMonitor()` - 恢复监控
- `TogglePausePerformanceMonitor()` - 切换暂停状态
- `IsPerformanceMonitorPaused()` - 检查是否暂停

---

## 编译结果

? **编译成功** - 所有文件无错误无警告

---

## 使用方法

### 1. 启动编辑器
插件会在引擎初始化后自动启动性能监控。

### 2. 查看控制台输出
在编辑器的 Output Log 中每秒会看到如下输出：

```
LogTemp: === Frame Performance ===
LogTemp: FPS: 60.00
LogTemp: GameThread: 16.00 ms
LogTemp: RenderThread: 12.80 ms
LogTemp: GPU: 11.20 ms
LogTemp: TotalFrame: 16.00 ms
LogTemp: ========================
```

### 3. 在蓝图中控制
可以在蓝图中调用以下节点：
- Enable Performance Monitor
- Disable Performance Monitor
- Toggle Performance Monitor
- Pause Performance Monitor
- Resume Performance Monitor

### 4. 在C++中控制
```cpp
// 启用
FPerformanceConsoleManager::Get().SetEnabled(true);

// 禁用
FPerformanceConsoleManager::Get().SetEnabled(false);

// 暂停
FPerformanceConsoleManager::Get().SetPaused(true);

// 恢复
FPerformanceConsoleManager::Get().SetPaused(false);
```

---

## 颜色编码（Windows平台）

在Windows控制台中，性能数据会根据开销等级显示不同颜色：

- **绿色**: 性能良好（< 16.67ms, 即 >60 FPS）
- **黄色**: 性能警告（16.67ms ~ 33.33ms, 即 30-60 FPS）
- **红色**: 性能严重（>= 33.33ms, 即 <30 FPS）

---

## 已知限制和后续改进

### 当前限制:
1. **线程时间数据不精确**: 当前使用总帧时间的估算值作为各线程时间
   - GameThreadTime = TotalFrameTime
   - RenderThreadTime = TotalFrameTime * 0.8
   - GPUTime = TotalFrameTime * 0.7

2. **仅支持Windows彩色输出**: 其他平台的控制台颜色功能未实现

3. **固定刷新间隔**: 当前固定为1秒，未从配置文件读取

### 后续改进计划（Step 5及以后）:
1. 使用UE Stats系统获取真实的线程时间数据
2. 从 `UPerformanceStatsSettings` 读取配置参数
3. 支持其他平台的彩色输出
4. 添加历史数据记录和趋势分析
5. 实现资源性能追踪（粒子、材质、骨骼网格等）

---

## 测试建议

### 基础功能测试:
1. 启动编辑器，验证控制台是否每秒输出性能数据
2. 播放场景，观察FPS变化
3. 在蓝图中调用 Toggle Performance Monitor 节点，验证启用/禁用功能
4. 调用 Pause Performance Monitor，验证输出是否停止

### 性能等级测试:
1. 在空场景中应该看到绿色的性能等级（高FPS）
2. 添加大量对象降低帧率，观察颜色变化：
   - 降到30-60 FPS → 黄色
   - 降到30 FPS以下 → 红色

### 压力测试:
1. 在复杂场景中验证插件是否影响性能
2. 长时间运行验证是否有内存泄漏

---

## 文件清单

| 文件 | 大小 | 状态 |
|------|------|------|
| PerformanceDataCollector.h | ~2.0 KB | ? 完成 |
| PerformanceDataCollector.cpp | ~1.5 KB | ? 完成 |
| PerformanceConsoleManager.h | ~1.2 KB | ? 完成 |
| PerformanceConsoleManager.cpp | ~3.5 KB | ? 完成 |
| PerformanceStatsPluginModule.cpp | ~0.5 KB | ? 更新 |

---

## 总结

Step 4 的所有功能已成功实现：
- ? 帧性能数据采集
- ? 定时刷新显示
- ? 性能等级分类（绿/黄/红）
- ? 控制台彩色输出（Windows）
- ? 蓝图函数库集成
- ? 启用/禁用/暂停控制

可以继续进行下一步开发或测试验证。
