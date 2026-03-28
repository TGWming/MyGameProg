# 控制台窗口修复说明

## 问题描述
监控窗口只在第一次启动 PIE（Play in Editor）时弹出，之后再次启动 PIE 时不再弹出。

## 根本原因
1. **Windows 限制**：在同一个进程中，`AllocConsole()` 只能成功调用一次
2. **生命周期问题**：
   - 第一次启动 PIE：成功创建控制台窗口
   - 停止 PIE：`Shutdown()` 调用 `FreeConsole()` 释放控制台
   - 第二次启动 PIE：再次调用 `AllocConsole()` 失败（错误代码 5 - ERROR_ACCESS_DENIED）
3. **单例持久化**：`FPerformanceConsoleManager` 是单例，在编辑器进程中一直存活

## 解决方案

### 1. 改进 `AllocateConsole()` 函数
- **检测现有控制台句柄**：优先尝试重用现有的控制台句柄
- **处理 ERROR_ACCESS_DENIED 错误**：当 `AllocConsole()` 返回错误代码 5 时，尝试获取现有句柄
- **多重回退策略**：
  1. 检查是否已有有效句柄
  2. 尝试附加到父进程控制台（PIE 模式）
  3. 尝试分配新控制台
  4. 如果失败且错误为 ERROR_ACCESS_DENIED，重新获取句柄

### 2. 修改 `FreeConsoleWindow()` 函数
- **保持控制台窗口存活**：不再调用 `::FreeConsole()`
- **只标记状态**：仅将 `bConsoleAllocated` 设为 `false`
- **保留句柄**：保持 `ConsoleHandle` 不变，以便下次重用
- **好处**：
  - 控制台窗口在 PIE 会话之间保持打开
  - 避免重复创建和销毁窗口
  - 用户可以持续监控性能数据

## 修改的文件
- `Plugins\PerformanceStatsPlugin\Source\PerformanceStatsPlugin\Private\PerformanceConsoleManager.cpp`
  - `AllocateConsole()` - 添加句柄重用逻辑和错误处理
  - `FreeConsoleWindow()` - 不再释放控制台窗口

## 测试方法
1. 启动 UE4 编辑器
2. 第一次启动 PIE - 控制台窗口应该弹出
3. 停止 PIE - 控制台窗口保持打开
4. 第二次启动 PIE - 控制台窗口继续更新数据（不会消失或重新弹出）
5. 多次重复步骤 3-4，验证窗口始终工作正常

## 注意事项
- 控制台窗口现在会在整个编辑器会话期间保持打开
- 如果需要关闭窗口，用户可以手动关闭（但下次启动 PIE 时会重新创建）
- 这种设计更符合监控工具的预期行为：持续可见，无需重复打开
