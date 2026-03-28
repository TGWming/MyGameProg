# 控制台冲突检测说明

## 问题背景

### Windows 控制台限制
一个 Windows 进程只能拥有**一个控制台窗口**。当多个插件都尝试创建或使用控制台时，会发生冲突。

### 插件冲突场景

当同时启用以下插件时会发生冲突：

1. **DebugConsolePlugin**
   - 目的：显示 `UE_LOG` 日志输出
   - 方式：创建独立控制台窗口，注册 `OutputDevice` 拦截日志
   - 标题：`"Debug Console - [ProjectName]"`

2. **PerformanceStatsPlugin**
   - 目的：显示实时性能监控数据
   - 方式：创建控制台窗口，使用 `printf()` 刷新数据
   - 标题：`"Performance Stats Monitor"`

### 冲突后果

如果两个插件共享同一个控制台窗口：

? **显示混乱**
- DebugConsole 不断输出日志文本（向下滚动）
- PerformanceStats 每帧将光标移到顶部 (`SetConsoleCursorPosition(0,0)`) 并覆盖内容
- 结果：屏幕闪烁，内容互相覆盖，完全无法使用

? **数据丢失**
- PerformanceStats 会覆盖 DebugConsole 的日志输出
- DebugConsole 的新日志会破坏 PerformanceStats 的格式化显示

## 解决方案

### 自动冲突检测

PerformanceStatsPlugin 现在会在初始化时自动检测是否存在 DebugConsolePlugin：

```cpp
// 检查控制台窗口标题
TCHAR ExistingTitle[256];
if (::GetConsoleTitleW(ExistingTitle, 256) > 0)
{
    FString TitleStr(ExistingTitle);
    if (TitleStr.Contains(TEXT("Debug Console")))
    {
        // 检测到 DebugConsole，中止初始化
        UE_LOG(LogTemp, Warning, TEXT("Detected DebugConsolePlugin window!"));
        UE_LOG(LogTemp, Warning, TEXT("Cannot share console - outputs will conflict!"));
        UE_LOG(LogTemp, Warning, TEXT("Please disable one of the console plugins."));
        return; // 中止初始化
    }
}
```

### 日志输出

当检测到冲突时，会在编辑器的 Output Log 中显示警告：

```
LogTemp: Warning: PerformanceConsoleManager: Detected DebugConsolePlugin window!
LogTemp: Warning: PerformanceConsoleManager: Cannot share console with DebugConsole - outputs will conflict!
LogTemp: Warning: PerformanceConsoleManager: Please disable one of the console plugins.
LogTemp: Warning: PerformanceConsoleManager: Initialization aborted to avoid conflicts.
```

### 使用建议

**选项 1：禁用 DebugConsolePlugin**
- 如果你主要需要性能监控，禁用 DebugConsole
- 在项目设置中：`Edit` → `Plugins` → 搜索 `Debug Console` → 取消勾选
- 重启编辑器

**选项 2：禁用 PerformanceStatsPlugin**
- 如果你主要需要查看日志，禁用 PerformanceStats
- 在项目设置中：`Edit` → `Plugins` → 搜索 `Performance Stats` → 取消勾选
- 重启编辑器

**选项 3：通过配置文件动态控制**
- DebugConsole 配置：`Config/DefaultDebugConsole.ini`
  ```ini
  [/Script/DebugConsolePlugin.DebugConsoleSettings]
  bEnableDebugConsole=False
  ```

- PerformanceStats 配置：`Config/DefaultPerformanceStats.ini`
  ```ini
  [/Script/PerformanceStatsPlugin.PerformanceStatsSettings]
  bEnablePerformanceStats=False
  ```

**选项 4：分别用于不同场景**
- **开发调试阶段**：启用 DebugConsole，禁用 PerformanceStats
  - 查看详细日志和错误信息
- **性能优化阶段**：禁用 DebugConsole，启用 PerformanceStats
  - 专注于帧率、线程时间、资源消耗

## 技术细节

### 为什么不能合并两个窗口？

理论上可以，但实现复杂：

1. **输出方式不同**
   - DebugConsole 使用 `OutputDevice` + `GLog`（日志流）
   - PerformanceStats 使用 `printf()` + `SetConsoleCursorPosition()`（直接屏幕控制）

2. **更新频率不同**
   - DebugConsole：事件驱动（有日志就输出）
   - PerformanceStats：定时刷新（每秒 N 次）

3. **显示区域冲突**
   - PerformanceStats 占用整个屏幕（固定布局）
   - DebugConsole 需要滚动显示日志

### 未来改进方向

可能的改进方案：

1. **分屏显示**
   - 上半部分：PerformanceStats（固定区域）
   - 下半部分：DebugConsole（滚动区域）
   - 需要重构两个插件的输出逻辑

2. **独立窗口**
   - 使用 Win32 API 创建独立的非控制台窗口
   - 需要更复杂的 GUI 编程

3. **编辑器内嵌显示**
   - 使用 UE4 的 Slate UI 系统
   - 在编辑器内创建停靠窗口
   - 不依赖控制台窗口

## 测试步骤

### 验证冲突检测

1. **同时启用两个插件**
   ```
   Edit → Plugins
   - [x] Debug Console Plugin
   - [x] Performance Stats Plugin
   ```

2. **重启编辑器**

3. **查看 Output Log**
   - 应该看到 PerformanceStats 的警告消息
   - PerformanceStats 不会创建控制台窗口

4. **启动 PIE**
   - 只有 DebugConsole 窗口出现
   - PerformanceStats 不会干扰

### 验证独立运行

1. **只启用 PerformanceStatsPlugin**
   - 禁用 DebugConsole
   - 重启编辑器
   - 启动 PIE
   - 应该看到 Performance Stats Monitor 窗口

2. **只启用 DebugConsolePlugin**
   - 禁用 PerformanceStats
   - 重启编辑器
   - 启动 PIE
   - 应该看到 Debug Console 窗口

## 常见问题

### Q1: 我想同时看到日志和性能数据怎么办？

**A:** 目前有两种方案：
1. 使用 DebugConsole 查看日志，使用编辑器内置的 `stat fps`、`stat unit` 命令查看性能
2. 等待未来版本支持分屏或独立窗口

### Q2: 可以用两个显示器分别显示吗？

**A:** 不可以。Windows 控制台窗口的限制是进程级别的，即使有多个显示器也只能有一个控制台。

### Q3: 检测逻辑会误判吗？

**A:** 当前检测基于窗口标题，只要标题包含 "Debug Console" 就会触发。如果你有其他使用类似标题的程序，可能会误判。可以通过修改 `DebugConsoleSettings.h` 中的 `ConsoleTitle` 来避免。

### Q4: 我手动关闭了 DebugConsole 窗口，PerformanceStats 能启动吗？

**A:** 可以，但需要重新启动 PIE。因为 PerformanceStats 的冲突检测只在初始化时进行一次。

## 修改记录

- **2024-12-03**: 添加 DebugConsole 冲突检测
  - 检测窗口标题避免冲突
  - 输出警告信息指导用户
  - 自动中止初始化防止显示混乱
