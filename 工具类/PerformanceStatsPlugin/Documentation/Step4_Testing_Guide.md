# Step 4 快速测试指南

## 快速验证步骤

### 1. 编译确认
? 项目已成功编译，无错误无警告

### 2. 启动编辑器测试

#### 步骤：
1. 打开 Unreal Editor
2. 等待编辑器完全加载
3. 打开 **Window → Developer Tools → Output Log**
4. 观察日志输出

#### 预期结果：
你应该能看到类似以下的日志输出（每秒更新一次）：

```
LogTemp: === Frame Performance ===
LogTemp: FPS: 60.00
LogTemp: GameThread: 16.67 ms
LogTemp: RenderThread: 13.34 ms
LogTemp: GPU: 11.67 ms
LogTemp: TotalFrame: 16.67 ms
LogTemp: ========================
```

### 3. 测试控制功能

#### 在编辑器中测试：

**方式1: 使用控制台命令（如果已添加）**
- 打开控制台（` 键）
- 输入命令测试

**方式2: 创建蓝图测试**
1. 创建一个新的 Actor Blueprint
2. 在 Event Graph 中添加以下节点：

##### 测试启用/禁用：
- 添加 **Event BeginPlay** 节点
- 连接到 **Disable Performance Monitor** 节点
- 延迟3秒后
- 连接到 **Enable Performance Monitor** 节点

##### 测试暂停/恢复：
- 按下某个键（如 P）
- 连接到 **Toggle Pause Performance Monitor** 节点

3. 将 Actor 放入关卡中
4. 点击 Play 测试

#### 预期结果：
- 禁用时：Output Log 中不再显示性能数据
- 启用时：Output Log 恢复显示性能数据
- 暂停时：性能数据停止更新
- 恢复时：性能数据继续更新

### 4. 测试性能等级颜色（Windows平台）

如果你在 Windows 平台上运行，并且从 Visual Studio 启动编辑器：

#### 步骤：
1. 确保在 Visual Studio Output 窗口中查看
2. 打开编辑器
3. 观察性能数据的颜色

#### 预期结果：
- **绿色文字**：表示性能良好（>60 FPS）
- **黄色文字**：表示性能警告（30-60 FPS）
- **红色文字**：表示性能差（<30 FPS）

### 5. 压力测试

#### 步骤：
1. 创建一个空关卡
2. 观察性能数据（应该是绿色，高FPS）
3. 添加大量静态网格物体（如 500+ 个）
4. 观察性能数据变化

#### 预期结果：
- 随着物体增加，FPS 应该下降
- 当 FPS 降到 30-60 之间，颜色变为黄色
- 当 FPS 降到 30 以下，颜色变为红色

---

## 常见问题排查

### Q1: 看不到性能数据输出
**检查项**：
- 确认 Output Log 窗口已打开（Window → Developer Tools → Output Log）
- 检查日志级别过滤器，确保 "Log" 级别已启用
- 确认插件已成功加载（在 Edit → Plugins 中搜索 "PerformanceStatsPlugin"）

### Q2: 性能数据一直不更新
**检查项**：
- 调用了 Pause 功能但没有 Resume
- 调用了 Disable 功能但没有 Enable

### Q3: 没有看到颜色
**可能原因**：
- 非 Windows 平台（当前只有 Windows 支持彩色输出）
- 在编辑器的 Output Log 中查看（应该在 Visual Studio Output 或 Windows Console 中查看）
- 控制台不支持 ANSI 颜色

### Q4: FPS 数值不准确
**说明**：
- Step 4 的实现使用 `FApp::GetDeltaTime()` 获取帧时间
- 这是正常的简化实现
- 在后续步骤中会使用更精确的 Stats 系统

---

## 性能验证

### 插件开销测试

#### 测试方法：
1. 在空关卡中记录 FPS（未启用插件）
2. 启用插件后记录 FPS
3. 对比差异

#### 预期结果：
- 插件开销应该非常小（<0.5% 性能影响）
- 主要开销来自每秒一次的日志输出

### 长时间运行测试

#### 测试方法：
1. 启动编辑器
2. 让其运行 30 分钟以上
3. 观察内存使用和性能变化

#### 预期结果：
- 无内存泄漏
- 性能稳定，无降级

---

## C++ 代码测试示例

如果你想在 C++ 中直接测试，可以在任意 Actor 的 Tick 函数中添加：

```cpp
void AMyTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 获取性能数据
    FFramePerformanceData PerfData = FPerformanceDataCollector::Get().CollectFrameData();
    
    // 使用数据
    if (PerfData.FPS < 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Low FPS detected: %.2f"), PerfData.FPS);
    }
    
    // 控制监控器
    if (SomeCondition)
    {
        FPerformanceConsoleManager::Get().SetPaused(true);
    }
}
```

---

## 下一步

Step 4 完成后，可以继续进行：

- **Step 5**: 实现更精确的线程时间采集（使用 Stats 系统）
- **Step 6**: 添加资源性能追踪（粒子系统、材质、骨骼网格）
- **Step 7**: 实现配置文件集成
- **Step 8**: 添加性能历史记录和分析

---

## 成功标志

如果你能看到以下所有现象，说明 Step 4 实现成功：

? 编辑器启动后自动开始性能监控  
? Output Log 每秒显示性能数据  
? 蓝图节点可以控制启用/禁用/暂停  
? 性能数据包含 FPS、线程时间等信息  
? 在 Windows 平台上显示颜色编码  
? 插件对整体性能影响极小  

恭喜！Step 4 已成功完成！??
