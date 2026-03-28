# DebugConsolePlugin 安装指南

## 状态

✅ 插件文件已全部创建完成！

## 下一步操作

### 1. 重新生成项目文件

**方法A：通过右键菜单（推荐）**
1. 在 Windows 文件资源管理器中找到 `MyProject.uproject`
2. 右键点击该文件
3. 选择 **"Generate Visual Studio project files"**
4. 等待完成

**方法B：通过命令行**
```powershell
cd "H:\codeCam\MyProject"
& "C:\Program Files\Epic Games\UE_4.27\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "H:\codeCam\MyProject\MyProject.uproject"
```

### 2. 编译项目

**方法A：通过 Visual Studio**
1. 打开 `MyProject.sln`
2. 选择 `Development Editor` 配置
3. 右键 `MyProject` 项目 → Build
4. 等待编译完成

**方法B：通过 UE4 编辑器**
1. 双击打开 `MyProject.uproject`
2. 如果提示需要重新编译，点击 "Yes"
3. 等待编译完成

### 3. 验证插件

1. 打开 UE4 编辑器
2. 菜单：**Edit → Plugins**
3. 搜索 "Debug Console"
4. 确认 **Debug Console Plugin** 显示为已启用 ✓

### 4. 配置插件（可选）

1. 菜单：**Edit → Project Settings**
2. 找到：**Plugins → Debug Console Settings**
3. 根据需要调整配置

### 5. 测试插件

1. 点击 **Play** 按钮启动游戏（或使用 Standalone Game）
2. 应该会弹出一个独立的控制台窗口，显示：
   ```
   ╔══════════════════════════════════════════════════════════╗
   ║           DEBUG CONSOLE INITIALIZED                      ║
   ║   This window will remain open after game exits          ║
   ╚══════════════════════════════════════════════════════════╝
   ```
3. 所有 `UE_LOG` 输出会显示在该窗口中
4. 退出游戏后，控制台窗口会提示按任意键关闭

## 插件文件结构

```
H:\codeCam\MyProject\Plugins\DebugConsolePlugin\
├── DebugConsolePlugin.uplugin
├── README.md
├── STRUCTURE.md
├── Config\
│   └── DefaultDebugConsole.ini
└── Source\
    └── DebugConsolePlugin\
        ├── DebugConsolePlugin.Build.cs
        ├── Public\
        │   ├── DebugConsolePluginModule.h
        │   ├── DebugConsoleManager.h
        │   ├── DebugConsoleOutputDevice.h
        │   └── DebugConsoleSettings.h
        └── Private\
            ├── DebugConsolePluginModule.cpp
            ├── DebugConsoleManager.cpp
            ├── DebugConsoleOutputDevice.cpp
            └── DebugConsoleSettings.cpp
```

## 常见问题

### Q: 生成项目文件失败？
A: 确保：
- UE 4.27 引擎已正确安装
- .uproject 文件关联正确
- 以管理员权限运行

### Q: 编译失败？
A: 检查：
- Visual Studio 版本（推荐 VS 2019）
- Windows SDK 已安装
- 查看编译错误信息

### Q: 控制台窗口没有出现？
A: 确认：
- 使用 **Development** 或 **DebugGame** 配置（不是 Shipping）
- 插件已启用
- `bEnableDebugConsole=True` 在配置中

### Q: 如何在打包的游戏中使用？
A: 
1. 使用 **Development** 配置打包
2. Shipping 配置会自动排除此插件

## 配置示例

### 只显示错误和警告
```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
MinVerbosity=Warning
```

### 只显示特定类别
```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
CategoryFilter=(LogTemp,LogMyGame)
```

### 简化输出格式
```ini
[/Script/DebugConsolePlugin.DebugConsoleSettings]
bShowTimestamp=False
bShowCategory=False
```

## 技术支持

如有问题，请查看：
- `README.md` - 完整文档
- `STRUCTURE.md` - 文件结构说明
- UE4 Output Log - 查看插件加载信息

---

**创建时间**: {DateTime}  
**插件版本**: 1.0  
**UE 版本**: 4.27  
**平台**: Windows (Win64)
