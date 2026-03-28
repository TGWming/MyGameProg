# Debug Console Plugin for UE4

## 概述

DebugConsolePlugin 是一个完全独立的 UE4 调试控制台插件，可以在独立窗口中显示所有 `UE_LOG` 输出，并在游戏退出后保持窗口开启，方便查看日志信息。

## 主要特性

- ✅ **完全独立** - 游戏代码无需任何修改
- ✅ **即插即用** - 放入 Plugins 文件夹即可使用
- ✅ **自动初始化** - 无需手动调用初始化代码
- ✅ **高度可配置** - 通过配置文件或编辑器设置控制所有功能
- ✅ **彩色输出** - 根据日志级别显示不同颜色
- ✅ **退出等待** - 游戏退出后自动等待按键再关闭
- ✅ **线程安全** - 支持多线程日志输出
- ✅ **仅开发版本** - Shipping 版本自动排除
- ✅ **仅 Windows 平台** - 其他平台自动跳过

## 安装步骤

1. 将 `DebugConsolePlugin` 文件夹复制到项目的 `Plugins` 目录
2. 右键项目的 `.uproject` 文件，选择 "Generate Visual Studio project files"
3. 重新编译项目
4. 在编辑器中打开项目，确认插件已启用（默认自动启用）

## 配置方式

### 方法 1：通过编辑器设置（推荐）

1. 打开 UE4 编辑器
2. 菜单：**Edit → Project Settings**
3. 找到：**Plugins → Debug Console Settings**
4. 修改相关配置项

### 方法 2：通过配置文件

编辑项目的 `Config/DefaultDebugConsole.ini` 文件（如不存在会自动创建）：

```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
bEnableDebugConsole=True
bWaitForKeyOnExit=True
ConsoleTitle=Game Debug Console
bShowTimestamp=True
bShowCategory=True
bShowVerbosity=True
MinVerbosity=Log
```

## 配置选项说明

| 选项 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `bEnableDebugConsole` | bool | True | 是否启用调试控制台 |
| `bWaitForKeyOnExit` | bool | True | 游戏退出后是否等待按键 |
| `ConsoleTitle` | string | "Game Debug Console" | 控制台窗口标题 |
| `bShowTimestamp` | bool | True | 是否显示时间戳 |
| `bShowCategory` | bool | True | 是否显示日志类别 |
| `bShowVerbosity` | bool | True | 是否显示日志级别 |
| `CategoryFilter` | array | [] | 日志类别过滤（空=显示全部） |
| `MinVerbosity` | enum | Log | 最低日志级别 |

## 日志级别

从高到低：
- **Fatal** - 致命错误（红底）
- **Error** - 错误（红色）
- **Warning** - 警告（黄色）
- **Display** - 显示（青色）
- **Log** - 日志（白色）
- **Verbose** - 详细（白色）
- **VeryVerbose** - 非常详细（白色）

## 使用示例

### 基本使用

插件安装后，直接运行游戏即可。所有 `UE_LOG` 输出会自动显示在控制台窗口。

```cpp
// 游戏代码中，无需任何修改
UE_LOG(LogTemp, Warning, TEXT("This is a warning message"));
UE_LOG(LogTemp, Error, TEXT("This is an error message"));
```

### 高级用法：直接输出到控制台

虽然不推荐，但如果需要直接向控制台输出文本：

```cpp
#include "DebugConsoleManager.h"

// 检查插件是否可用
if (FDebugConsoleManager::Get().IsInitialized())
{
    FDebugConsoleManager::Get().PrintToConsole(TEXT("Custom console message"));
}
```

### 过滤特定类别

如果只想显示特定类别的日志：

```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
CategoryFilter=(LogTemp,LogMyGame,LogAI)
```

## 禁用插件

### 方法 1：通过配置禁用
```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
bEnableDebugConsole=False
```

### 方法 2：在 .uplugin 中禁用
```json
{
    "EnabledByDefault": false
}
```

### 方法 3：在编辑器中禁用
**Edit → Plugins → Developer Tools → Debug Console Plugin** → 取消勾选

## 技术细节

### 架构设计

- **FDebugConsolePluginModule** - 插件模块，负责自动启动
- **FDebugConsoleManager** - 单例管理器，控制控制台生命周期
- **FDebugConsoleOutputDevice** - 输出设备，拦截并格式化日志
- **UDebugConsoleSettings** - 配置类，管理所有可配置项

### 加载时机

- **LoadingPhase: PostConfigInit** - 在配置系统初始化后立即加载
- 确保能捕获引擎早期的日志输出

### 平台支持

- ✅ Windows (Win64, Win32)
- ❌ 其他平台（自动跳过，不影响编译）

### 构建配置

- ✅ Development - 启用
- ✅ DebugGame - 启用
- ❌ Shipping - 自动排除（模块类型为 Developer）

## 常见问题

### Q: 控制台窗口没有出现？
A: 检查以下几点：
1. 确认配置中 `bEnableDebugConsole=True`
2. 确认不是 Shipping 构建
3. 确认是 Windows 平台
4. 查看输出日志确认插件已加载

### Q: 如何在 Shipping 版本中完全移除插件？
A: 插件已自动配置为 Developer 类型，Shipping 版本会自动排除，无需额外操作。

### Q: 日志输出乱码？
A: 插件已自动设置控制台为 UTF-8 编码，如仍有问题，检查系统区域设置。

### Q: 能否在运行时动态开关控制台？
A: 目前不支持运行时开关，需要在启动前通过配置文件设置。

## 许可证

本插件采用 MIT 许可证，可自由用于商业和非商业项目。

## 技术支持

如有问题或建议，请联系开发者。

---

**版本**: 1.0  
**最低 UE 版本**: 4.20+  
**支持平台**: Windows (Win64, Win32)  
**模块类型**: Developer
