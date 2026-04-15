# UE4 打包排障记录

**项目：** soul (UE 4.27)
**日期：** 2026-04-15
**结果：** ✅ 打包成功（Windows 64-bit Shipping）

---

## 目录

1. [问题总览](#问题总览)
2. [问题 1：静态/动态组件附着冲突](#问题-1静态动态组件附着冲突)
3. [问题 2：FlowVisualizer 缺少 FMODStudio 依赖声明](#问题-2flowvisualizer-缺少-fmodstudio-依赖声明)
4. [问题 3：IncrediBuild 许可证未激活导致打包失败](#问题-3incredibuild-许可证未激活导致打包失败)
5. [问题 4：Easy_CombatFinisher 资产引用缺失](#问题-4easy_combatfinisher-资产引用缺失)
6. [问题 5：AudioZoneTrigger Shipping 编译错误](#问题-5audiozonetrigger-shipping-编译错误)
7. [IncrediBuild 恢复指南](#incredibuild-恢复指南)

---

## 问题总览

| # | 问题 | 原因 | 修复方式 | 涉及文件 |
|---|------|------|---------|---------|
| 1 | 静态/动态组件附着冲突 | `P_Steam_LitBoss_fogdoor` 动态组件附着到静态组件 | 修改蓝图中的附着设置 | 关卡蓝图 |
| 2 | FlowVisualizer 缺少依赖声明 | `.uplugin` 未声明 FMODStudio 依赖 | 添加 `"Plugins"` 数组 | `Plugins\FlowVisualizer\FlowVisualizer.uplugin` |
| 3 | IncrediBuild 许可证未激活 | `xgConsole.exe` 返回 ExitCode=1 | 重命名 `xgConsole.exe` 禁用 | `C:\Program Files (x86)\Incredibuild\xgConsole.exe` |
| 4 | BP_CinematicRig 资产引用缺失 | 资产包不完整，Cook 时找不到文件 | 添加到 Directories to never cook | Project Settings → Packaging |
| 5 | PostEditChangeProperty 编译错误 | Editor-only 函数在 Shipping 配置中不存在 | 添加 `#if WITH_EDITOR` 包裹 | `AudioZoneTrigger.h` / `.cpp` |

---

## 问题 1：静态/动态组件附着冲突

### 错误日志
```
LogSceneComponent: AttachTo: '/Game/Maps/xxx.xxx:PersistentLevel.P_Steam_LitBoss_fogdoor' is not static
```

### 原因
动态（Movable）组件尝试附着到静态（Static）组件上，UE4 不允许这种操作。

### 修复
在关卡蓝图中修改 `P_Steam_LitBoss_fogdoor` 的 Mobility 设置，确保附着关系中的父子组件 Mobility 类型一致。

---

## 问题 2：FlowVisualizer 缺少 FMODStudio 依赖声明

### 错误日志
```
Plugin 'FlowVisualizer' does not list plugin 'FMODStudio' as a dependency
```

### 修复文件
`Plugins\FlowVisualizer\FlowVisualizer.uplugin`

### 修改内容
在 `"Modules"` 数组后新增 `"Plugins"` 数组：

```json
"Plugins": [
    {
        "Name": "FMODStudio",
        "Enabled": true,
        "Optional": true
    }
]
```

---

## 问题 3：IncrediBuild 许可证未激活导致打包失败

### 错误日志
```
License not activated
Took 7.11s to run xgConsole.exe, ExitCode=1
BUILD FAILED: Command failed (Result:1): C:\Program Files (x86)\Incredibuild\xgConsole.exe
AutomationTool exiting with ExitCode=1 (Error_Unknown)
```

### 原因
IncrediBuild（分布式编译工具）的许可证未激活，`xgConsole.exe` 执行失败，UE4 将其包装为 `Error_Unknown`。

### 排查过程

#### 尝试 1（无效）：DefaultEngine.ini 添加 bAllowXGE=false
```ini
[BuildConfiguration]
bAllowXGE=false
```
**结果：** 无效。UE4.27 的编译系统不通过此 ini 控制 XGE。

#### 尝试 2（无效）：BuildConfiguration.xml 添加 bAllowXGE
文件路径：
```
%APPDATA%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml
```
修改内容：
```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
    <BuildConfiguration>
        <bAllowXGE>false</bAllowXGE>
    </BuildConfiguration>
</Configuration>
```
**结果：** 无效。UAT 有独立的 XGE 调用逻辑，绕过了此配置。

#### 尝试 3（成功）：重命名 xgConsole.exe
```cmd
ren "C:\Program Files (x86)\Incredibuild\xgConsole.exe" xgConsole.exe.bak
```
**结果：** ✅ 成功。UE4 找不到 xgConsole.exe 后自动回退到本地编译（Local Executor）。

---

## 问题 4：Easy_CombatFinisher 资产引用缺失

### 错误日志
```
Failed to load '/Game/Easy_CombatFinisher/Blueprints/Actors/BP_CinematicRig': Can't find file.
```

### 原因
`Easy_CombatFinisher`（Marketplace 处决动画资产包）内部引用了 `BP_CinematicRig`，但文件不完整。关卡中未直接使用该资产包，但打包时 UE4 会 Cook 所有 Content 目录下的资产。

### 修复
在 **Edit → Project Settings → Packaging → Directories to never cook** 中添加：
```
/Game/Easy_CombatFinisher
```

### 备注
项目中还有一个同名的 `BP_CinematicRig` 用于 Cutscene 影片，与 `Easy_CombatFinisher` 中的是不同资产。在 Content Browser 中搜索该名称无法找到 `Easy_CombatFinisher` 版本（文件缺失），只能在打包 Cook 阶段触发报错。

---

## 问题 5：AudioZoneTrigger Shipping 编译错误

### 错误日志
```
AudioZoneTrigger.h(58): error C3668: AAudioZoneTrigger::PostEditChangeProperty:
包含"override"说明符的方法没有重写任何基类方法

AudioZoneTrigger.cpp(124): error C2039: "PostEditChangeProperty": 不是"AActor"的成员
```

### 原因
`PostEditChangeProperty` 是 UE4 中仅限编辑器（Editor-only）的虚函数，只在 `WITH_EDITOR` 宏开启时可用。Shipping 配置下 `WITH_EDITOR=0`���该函数不存在于基类中，导致 `override` 找不到目标。

### 修复文件
- `Source\soul\Public\AudioZoneTrigger.h`
- `Source\soul\Private\AudioZoneTrigger.cpp`

### 修改内容

**头文件（.h）：**
```cpp
#if WITH_EDITOR
virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
```

**源文件（.cpp）：**
```cpp
#if WITH_EDITOR
void AAudioZoneTrigger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // ... 原有函数内容保持不变 ...
}
#endif
```

### 技术说明
UE4 中以下函数都是 Editor-only，在非编辑器配置中需要 `#if WITH_EDITOR` 包裹：
- `PostEditChangeProperty`
- `PostEditChangeChainProperty`
- `PreEditChange`
- `PostEditUndo`
- `CanEditChange`

---

## IncrediBuild 恢复指南

### 当前状态
`xgConsole.exe` 已被重命名为 `xgConsole.exe.bak`，IncrediBuild 被禁用。

### 为什么禁用
IncrediBuild 许可证未激活，每次打包都会调用 `xgConsole.exe` 并返回 `ExitCode=1`，导致 UE4 报 `Error_Unknown` 打包失败。通过 `BuildConfiguration.xml` 和 `.ini` 配置均无法阻止 UE4.27 的 UAT 调用 XGE。

### 恢复命令
如果未来 IncrediBuild 许可证已激活，需要恢复分布式编译，在 **管理员 CMD** 中执行：
```cmd
ren "C:\Program Files (x86)\Incredibuild\xgConsole.exe.bak" xgConsole.exe
```

### 验证恢复
```cmd
dir "C:\Program Files (x86)\Incredibuild\xgConsole.exe"
```
应显示 `xgConsole.exe`（不带 `.bak` 后缀）。

### 如需再次禁用
```cmd
ren "C:\Program Files (x86)\Incredibuild\xgConsole.exe" xgConsole.exe.bak
```

### 清理之前的无效配置（可选）

#### BuildConfiguration.xml
路径：`%APPDATA%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`

可以将内容恢复为��配置：
```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
</Configuration>
```

#### DefaultEngine.ini
如果之前添加了 `[BuildConfiguration]` 段落，可以移除：
```ini
[BuildConfiguration]
bAllowXGE=false
```
此设置对 UE4.27 无效，留着无害但也无用。

---

## 环境信息

| 项目 | 值 |
|------|-----|
| 引擎版本 | UE 4.27 |
| 操作系统 | Windows 10 (10.0.19045.6466) |
| 编译器 | Visual Studio 2022 14.44.35217 |
| Windows SDK | 10.0.26100.0 |
| IncrediBuild | 已安装但许可证未激活（已禁用） |
| 打包配置 | Win64 Shipping |
| 项目路径 | H:\soul\ |