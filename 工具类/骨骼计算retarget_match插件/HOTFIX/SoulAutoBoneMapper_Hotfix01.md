# 热修复 1: 修复 CreatePackage API 调用

```
===== 热修复 1: 修复 CreatePackage API 调用 =====

目标: 修复 BatchRetargeter.cpp 中 CreatePackage 的废弃 API 调用

操作类型: 仅修改已有文件（1 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 BatchRetargeter.cpp 中的指定行
- 禁止删除或修改其他函数
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BatchRetargeter.cpp

找到:

            UAnimSequence* DuplicatedAnim = DuplicateObject<UAnimSequence>(
                SourceAnim, CreatePackage(nullptr, *NewAssetPath), *NewAssetName);

替换为:

            UAnimSequence* DuplicatedAnim = DuplicateObject<UAnimSequence>(
                SourceAnim, CreatePackage(*NewAssetPath), *NewAssetName);

说明: UE4.27 的 CreatePackage 新 API 移除了第一个 Outer 参数，
不再接受 nullptr 作为第一个参数。

完成后汇报: 确认 CreatePackage 调用是否已修复

===== 热修复 1 结束 =====
```