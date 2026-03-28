# DebugConsolePlugin - 快速参考卡

## 🚀 快速开始（3步）

```bash
# 1. 重新生成项目
右键 MyProject.uproject → Generate Visual Studio project files

# 2. 编译项目
打开 MyProject.sln → Build (Development Editor)

# 3. 运行游戏
点击 Play → 观察独立控制台窗口
```

---

## 📍 文件位置

```
H:\codeCam\MyProject\Plugins\DebugConsolePlugin\
```

---

## ⚙️ 快速配置

### 编辑器配置
```
Edit → Project Settings → Plugins → Debug Console Settings
```

### .ini 配置
```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
bEnableDebugConsole=True          # 主开关
bWaitForKeyOnExit=True            # 退出等待
ConsoleTitle=My Console           # 窗口标题
bShowTimestamp=True               # 时间戳
bShowCategory=True                # 类别
bShowVerbosity=True               # 级别
MinVerbosity=Log                  # 最低级别
```

---

## 🎨 日志级别颜色

| 级别 | 颜色 | 用途 |
|------|------|------|
| Fatal | 🔴 红底 | 致命错误 |
| Error | 🔴 红色 | 错误 |
| Warning | 🟡 黄色 | 警告 |
| Display | 🔵 青色 | 显示 |
| Log | ⚪ 白色 | 日志 |
| Verbose | ⚪ 白色 | 详细 |

---

## 📝 常用代码

### 基本输出
```cpp
UE_LOG(LogTemp, Log, TEXT("Message"));
UE_LOG(LogTemp, Warning, TEXT("Warning: %s"), *Message);
UE_LOG(LogTemp, Error, TEXT("Error: %d"), ErrorCode);
```

### 格式化输出
```cpp
// 数值
UE_LOG(LogTemp, Log, TEXT("Score: %d, HP: %.1f"), Score, Health);

// 字符串
UE_LOG(LogTemp, Log, TEXT("Name: %s"), *PlayerName);

// Vector
UE_LOG(LogTemp, Log, TEXT("Pos: %s"), *Location.ToString());
```

### 检查插件
```cpp
#include "DebugConsoleManager.h"

if (FDebugConsoleManager::Get().IsInitialized())
{
    FDebugConsoleManager::Get().PrintToConsole(TEXT("Custom"));
}
```

---

## 🎯 常见配置场景

### 只显示错误和警告
```ini
MinVerbosity=Warning
```

### 只显示特定类别
```ini
CategoryFilter=(LogTemp,LogMyGame,LogAI)
```

### 简化输出
```ini
bShowTimestamp=False
bShowCategory=False
bShowVerbosity=False
```

### 禁用等待
```ini
bWaitForKeyOnExit=False
```

### 完全禁用
```ini
bEnableDebugConsole=False
```

---

## 🐛 故障排除

| 问题 | 解决方法 |
|------|----------|
| 窗口不出现 | 检查 Development 配置 + 插件已启用 |
| 编译错误 | 重新生成项目文件 |
| 乱码 | 已支持 UTF-8，检查系统设置 |
| Shipping 包含 | 正常，Developer 类型自动排除 |

---

## 📚 文档索引

| 文档 | 内容 |
|------|------|
| `README.md` | 完整使用文档 |
| `INSTALLATION.md` | 详细安装步骤 |
| `SUMMARY.md` | 技术总结 |
| `STRUCTURE.md` | 文件结构 |
| `Examples/` | 示例代码 |

---

## 🔍 关键信息

- **版本**: 1.0
- **引擎**: UE 4.27
- **平台**: Windows (Win64/Win32)
- **类型**: Developer（Shipping 自动排除）
- **加载**: PostConfigInit（自动）

---

## ✅ 检查清单

安装：
- [ ] 文件已复制到 Plugins/DebugConsolePlugin/
- [ ] 项目文件已重新生成
- [ ] 项目已成功编译

验证：
- [ ] 插件在编辑器中显示为已启用
- [ ] 运行游戏时控制台窗口弹出
- [ ] 日志正确显示在控制台中
- [ ] 退出时等待按键提示出现

---

## 💡 小技巧

1. **快速禁用**: 配置中设置 `bEnableDebugConsole=False`
2. **过滤噪音**: 使用 `MinVerbosity=Warning` 只看警告和错误
3. **类别过滤**: `CategoryFilter` 只显示关心的类别
4. **测试代码**: 参考 `Examples/TestDebugConsole.cpp`
5. **打包游戏**: Development 配置会包含，Shipping 自动排除

---

## 🆘 获取帮助

1. 查看 Output Log（UE4 编辑器）
2. 检查编译错误信息
3. 验证文件完整性（见 SUMMARY.md）
4. 参考完整文档（README.md）

---

**快速参考卡 v1.0** | DebugConsolePlugin | UE 4.27 | Windows
