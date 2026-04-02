# 任务 3: 创建模块头文件

```
===== 任务 3: 创建模块头文件 =====

目标: 创建插件模块的公共头文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/SoulAutoBoneMapper.h 内容如下:

#pragma once

#include "Modules/ModuleManager.h"

class FSoulAutoBoneMapperModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

完成后汇报: 确认头文件是否已创建

===== 任务 3 结束 =====
```