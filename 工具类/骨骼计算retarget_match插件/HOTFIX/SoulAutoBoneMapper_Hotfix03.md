# 热修复 3: 修复 UE_LOG 大小写错误

```
===== 热修复 3: 修复 UE_LOG 大小写错误 =====

目标: 修复所有源文件中可能存在的 UE_Log 大小写错误

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改包含错误的文件
- 禁止删除或修改无关代码
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/ 目录下的所有 .cpp 文件中，
搜索所有 UE_Log（注意小写 g）并替换为 UE_LOG（全大写）。

同时搜索所有不带括号的 Log 标识符错误用法。

具体检查以下文件:
  - Private/BoneScanner.cpp
  - Private/BoneMatcher.cpp
  - Private/BoneAliasTable.cpp
  - Private/MappingPreset.cpp
  - Private/RetargetApplier.cpp
  - Private/BatchRetargeter.cpp
  - Private/BoneHighlighter.cpp
  - Private/SBoneMappingWidget.cpp

所有 UE_LOG 调用应为以下格式:
  UE_LOG(LogTemp, Log, TEXT("..."), ...);
  UE_LOG(LogTemp, Warning, TEXT("..."), ...);
  UE_LOG(LogTemp, Verbose, TEXT("..."), ...);

确保:
  - UE_LOG 全部大写（不是 UE_Log 或 UE_log）
  - 第二个参数 Log / Warning / Verbose 首字母大写
  - 如果发现 "Log" 作为未声明标识符的错误，检查是否有裸的 Log 
    被误用（应该是 UE_LOG 宏的第二个参数，不是独立标识符）

完成后汇报: 
  - 列出修改了哪些文件的哪些行
  - 确认所有 UE_LOG 拼写正确

===== 热修复 3 结束 =====
```