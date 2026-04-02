# 热修复 2: 修复 USkeletalMesh::Skeleton 直接访问

```
===== 热修复 2: 修复 USkeletalMesh::Skeleton 直接访问 =====

目标: 修复 BoneScanner.cpp 中直接访问 Mesh->Skeleton 的废弃用法

操作类型: 仅修改已有文件（1 行改动）
引擎版本: UE4.27

安全约束:
- 仅修改 BoneScanner.cpp 中的指定行
- 禁止删除或修改其他函数
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

修改文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneScanner.cpp

找到:

        return Mesh->Skeleton;

替换为:

        return Mesh->GetSkeleton();

说明: UE4.27 已废弃直接访问 USkeletalMesh::Skeleton 成员，
应使用 GetSkeleton() 访问器。

完成后汇报: 确认 Mesh->Skeleton 是否已替换为 Mesh->GetSkeleton()

===== 热修复 2 结束 =====
```