# 任务 70: 实现 BoneHighlighter — 高亮方法 + 清除方法

```
===== 任务 70: 实现 BoneHighlighter — 高亮方法 + 清除方法 =====

目标: 在 BoneHighlighter.cpp 末尾追加所有 public 方法

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 BoneHighlighter.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneHighlighter.cpp 末尾追加以下代码:

void FBoneHighlighter::HighlightSourceBone(const USkeleton* Skeleton, const FName& BoneName)
{
    FVector Position;
    if (GetBoneRefPoseWorldPosition(Skeleton, BoneName, Position))
    {
        DrawDebugBoneMarker(Position, FColor::Green, 3.0f,
            FString::Printf(TEXT("[S] %s"), *BoneName.ToString()));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneHighlighter: Source bone '%s' not found"),
            *BoneName.ToString());
    }
}

void FBoneHighlighter::HighlightTargetBone(const USkeleton* Skeleton, const FName& BoneName)
{
    FVector Position;
    if (GetBoneRefPoseWorldPosition(Skeleton, BoneName, Position))
    {
        DrawDebugBoneMarker(Position, FColor::Cyan, 3.0f,
            FString::Printf(TEXT("[T] %s"), *BoneName.ToString()));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneHighlighter: Target bone '%s' not found"),
            *BoneName.ToString());
    }
}

void FBoneHighlighter::HighlightMappingPair(
    const USkeleton* SourceSkeleton,
    const USkeleton* TargetSkeleton,
    const FBoneMappingEntry& Entry)
{
    HighlightSourceBone(SourceSkeleton, Entry.SourceBoneName);

    if (Entry.TargetBoneName != NAME_None)
    {
        HighlightTargetBone(TargetSkeleton, Entry.TargetBoneName);

        // 在两个骨骼之间画一条连线
        FVector SourcePos, TargetPos;
        if (GetBoneRefPoseWorldPosition(SourceSkeleton, Entry.SourceBoneName, SourcePos) &&
            GetBoneRefPoseWorldPosition(TargetSkeleton, Entry.TargetBoneName, TargetPos))
        {
            UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
            if (World)
            {
                DrawDebugLine(World, SourcePos, TargetPos, FColor::Yellow, false, 5.0f, 0, 1.0f);
            }
        }
    }
}

void FBoneHighlighter::ClearAllHighlights()
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
    {
        FlushPersistentDebugLines(World);
    }
}

完成后汇报: 确认四个方法是否已追加到 BoneHighlighter.cpp 末尾，
且已有函数未被修改。至此 BoneHighlighter 全部方法实现完毕。

===== 任务 70 结束 =====
```