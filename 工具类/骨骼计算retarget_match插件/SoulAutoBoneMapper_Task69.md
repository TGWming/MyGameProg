# 任务 69: 实现 BoneHighlighter — GetBoneRefPoseWorldPosition + DrawDebugBoneMarker

```
===== 任务 69: 实现 BoneHighlighter — 辅助方法 =====

目标: 创建 BoneHighlighter.cpp，实现骨骼位置获取和 Debug 绘制

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneHighlighter.cpp 内容如下:

#include "BoneHighlighter.h"
#include "Animation/Skeleton.h"
#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Engine/World.h"

bool FBoneHighlighter::GetBoneRefPoseWorldPosition(
    const USkeleton* Skeleton,
    const FName& BoneName,
    FVector& OutPosition)
{
    if (!Skeleton)
    {
        return false;
    }

    const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
    const int32 BoneIndex = RefSkel.FindBoneIndex(BoneName);

    if (BoneIndex == INDEX_NONE)
    {
        return false;
    }

    // 累乘局部 Transform 得到世界空间坐标
    const TArray<FTransform>& RefPose = RefSkel.GetRawRefBonePose();

    FTransform WorldTransform = RefPose[BoneIndex];
    int32 ParentIdx = RefSkel.GetParentIndex(BoneIndex);

    while (ParentIdx != INDEX_NONE)
    {
        WorldTransform = WorldTransform * RefPose[ParentIdx];
        ParentIdx = RefSkel.GetParentIndex(ParentIdx);
    }

    // 注意：这是 Component Space 坐标，不含 Actor Transform
    // 但作为标记参考已足够
    OutPosition = WorldTransform.GetLocation();
    return true;
}

void FBoneHighlighter::DrawDebugBoneMarker(
    const FVector& Position,
    const FColor& Color,
    float Radius,
    const FString& Label)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return;
    }

    // 绘制球体标记（持续 5 秒）
    DrawDebugSphere(World, Position, Radius, 12, Color, false, 5.0f);

    // 绘制文本标签（持续 5 秒）
    DrawDebugString(World, Position + FVector(0, 0, Radius + 2.f), Label, nullptr, Color, 5.0f);
}

完成后汇报: 确认 BoneHighlighter.cpp 是否已创建，包含:
  1. GetBoneRefPoseWorldPosition 完整实现
  2. DrawDebugBoneMarker 完整实现
  注意: public 方法将在下一个任务中追加

===== 任务 69 结束 =====
```