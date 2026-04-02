# 任务 36: 注册编辑器窗口 — 修改模块头文件

```
===== 任务 36: 注册编辑器窗口 — 修改模块头文件 =====

目标: 修改 SoulAutoBoneMapper.h，增加窗口注册所需的成员声明

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SoulAutoBoneMapper.h，禁止修改其他文件
- 禁止删除已有代码
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/SoulAutoBoneMapper.h

将整个文件内容替换为:

#pragma once

#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"

class FSoulAutoBoneMapperModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** 注册菜单项 */
    void RegisterMenuExtension();

    /** 菜单点击回调 */
    void OnMenuButtonClicked();

    /** 生成窗口 Tab 内容 */
    TSharedRef<SDockTab> OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs);

    /** 菜单扩展句柄 */
    TSharedPtr<FExtensibilityManager> ExtensionManager;
};

完成后汇报: 确认 SoulAutoBoneMapper.h 是否已更新，包含:
  1. RegisterMenuExtension 声明
  2. OnMenuButtonClicked 声明
  3. OnSpawnTab 声明

===== 任务 36 结束 =====
```