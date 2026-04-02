# 任务 1: 创建 SoulAutoBoneMapper 插件目录结构与 .uplugin 文件

```
===== 任务 1: 创建 SoulAutoBoneMapper 插件目录结构与 .uplugin 文件 =====

目标: 在项目 Plugins/ 目录下创建 SoulAutoBoneMapper 插件的目录结构和描述文件

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

1. 创建以下目录结构:
   Plugins/SoulAutoBoneMapper/
   Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Public/
   Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/
   Plugins/SoulAutoBoneMapper/Resources/

2. 创建文件 Plugins/SoulAutoBoneMapper/SoulAutoBoneMapper.uplugin 内容如下:

{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0",
    "FriendlyName": "Soul Auto Bone Mapper",
    "Description": "Automatic skeleton bone scanning, matching and retarget mapping tool for UE4.27",
    "Category": "Animation",
    "CreatedBy": "SoulDev",
    "EnabledByDefault": false,
    "CanContainContent": false,
    "IsBetaVersion": true,
    "Modules": [
        {
            "Name": "SoulAutoBoneMapper",
            "Type": "Editor",
            "LoadingPhase": "Default"
        }
    ]
}

完成后汇报: 确认目录和 .uplugin 文件是否已创建

===== 任务 1 结束 =====
```