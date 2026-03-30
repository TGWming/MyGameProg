# Spline 自动路径 — Step 4：修改 CreateStairSpline 集成自动路径

## PROMPT

```
任务：修改 Source/soulEditor/Private/StairRampGeneratorBFL.cpp
仅修改 CreateStairSpline 函数。

安全约束：
  - 不修改 Source/soul/ 下的文件
  - 不修改 .uproject / .Target.cs / .Build.cs
  - 不删除 Intermediate 或 Binaries
  - 不修改 GenerateRampFromSpline、CreateRampSegment、
    ExtractMeshTriangles、DetectStairSteps、
    GenerateStraightStairRamp、ComputeLayerCentroids、DouglasPeucker
  - 不修改 .h 头文件
  - 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径读取
  - 编译配置：Development Editor + Win64
  - 编译失败只报告，不自行修复

背景信息 — 当前 CreateStairSpline 的结构（行号供参考）：

  行 180-187: Step 1 参数校验
  行 189-202: Step 2-3 BoundingBox + BottomPoint/TopPoint
  行 204-205: Step 3.5 ActorLocation
  行 207-214: Step 4 StairWidth 自动估算（使用 FMath::Max）
  行 216-235: Step 5 SpawnActor + SetActorLabel + Tag
  行 237-244: Step 6-6.5 SplineComponent 创建 + SetActorLocation
  行 246-257: Step 7-8 设置 2 个 Spline 控制点 + 线性
  行 259-263: Step 9 Spline 可视化
  行 265-346: Step 9.5 Z 值提取 + StepHeight 检测 + Tag 存储
  行 348-353: Step 10-10.5 StairWidth Tag + StairActorName Tag
  行 355-368: Step 11-12 日志 + return

目标：
  在 Step 7（设置 Spline 控制点）之前，插入自动路径检测逻辑。
  如果检测到 >= 3 个质心（说明楼梯有拐弯），用自动路径替代原有 2 点。
  如果质心 < 3 个，保持原有 2 点逻辑作为 fallback。

具体修改：

═══════════════════════════════════════
修改 1：在 Step 6.5 之后、Step 7 之前，插入自动路径检测
═══════════════════════════════════════

在 SplineActor->SetActorLocation(ActorLocation); 之后（行 244 之后），
在 SplineComp->ClearSplinePoints(false); 之前（行 247 之前），
插入以下代码块：

    // ---- 自动路径检测 ----
    // 提取 Mesh 顶点（世界坐标），用于计算楼梯路径
    TArray<FVector> PathWorldVertices;
    UStaticMeshComponent* PathMeshComp = StairActor->FindComponentByClass<UStaticMeshComponent>();
    if (PathMeshComp && PathMeshComp->GetStaticMesh())
    {
        UStaticMesh* PathMesh = PathMeshComp->GetStaticMesh();
        if (PathMesh->GetRenderData() && PathMesh->GetRenderData()->LODResources.Num() > 0)
        {
            FStaticMeshLODResources& PathLOD = PathMesh->GetRenderData()->LODResources[0];
            FPositionVertexBuffer& PathVB = PathLOD.VertexBuffers.PositionVertexBuffer;
            FTransform MeshTransform = PathMeshComp->GetComponentTransform();
            uint32 PathNumVerts = PathVB.GetNumVertices();
            PathWorldVertices.Reserve(PathNumVerts);
            
            for (uint32 vi = 0; vi < PathNumVerts; vi++)
            {
                FVector WorldPos = MeshTransform.TransformPosition(PathVB.VertexPosition(vi));
                PathWorldVertices.Add(WorldPos);
            }
        }
    }
    
    // 计算层质心 → 简化路径
    TArray<FVector> AutoPathPoints;
    bool bUseAutoPath = false;
    
    if (PathWorldVertices.Num() > 10 && DetectedStepHeight > 0.0f)
    {
        ... // 见下文
    }

注意！这里有一个问题：DetectedStepHeight 在 Step 9.5 才计算（行 287），
但自动路径需要在 Step 7 之前使用它。

解决方案：将 Step 9.5 的整段代码（行 265-346）移动到 Step 6.5 之后、
自动路径检测之前。即执行顺序变为：

  Step 6.5  SetActorLocation
  Step 9.5  Z 值提取 + StepHeight 检测（原位置删除，移到这里）
  新增      自动路径���测
  Step 7    设置 Spline 控制点

移动时注意：
  - Step 9.5 中使用的 MeshComp 变量名改为 StepMeshComp（避免和 PathMeshComp 冲突）
  - 或者复用 PathMeshComp（直接让路径检测和 StepHeight 检测共用同一个 MeshComp）
  
推荐方案：将 StepHeight 检测中的 MeshComp 改名为 StepMeshComp，
保持两段代码独立，减少互相干扰的风险。

═══════════════════════════════════════
修改 2：自动路径计算逻辑（紧接 PathWorldVertices 提取之后）
═══════════════════════════════════════

    if (PathWorldVertices.Num() > 10 && DetectedStepHeight > 0.0f)
    {
        // 计算层质心
        TArray<FVector> Centroids = ComputeLayerCentroids(
            PathWorldVertices, DetectedStepHeight,
            ActorBounds.Min.Z, ActorBounds.Max.Z);
        
        UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: AutoPath raw centroids=%d"),
            Centroids.Num());
        
        if (Centroids.Num() >= 3)
        {
            // Douglas-Peucker 简化
            // Tolerance 使用 BBox 较短边 × 0.3
            float ShortSide = FMath::Min(
                ActorBounds.Max.X - ActorBounds.Min.X,
                ActorBounds.Max.Y - ActorBounds.Min.Y);
            float DPTolerance = FMath::Max(ShortSide * 0.3f, 30.0f);
            // 下限 30，防止过度简化
            
            AutoPathPoints = DouglasPeucker(Centroids, DPTolerance);
            
            UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: AutoPath simplified %d -> %d points (tolerance=%.1f)"),
                Centroids.Num(), AutoPathPoints.Num(), DPTolerance);
            
            if (AutoPathPoints.Num() >= 3)
            {
                bUseAutoPath = true;
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: AutoPath simplified to < 3 points, using default 2-point"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: AutoPath centroids < 3, using default 2-point"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: AutoPath skipped (vertices=%d, StepHeight=%.1f)"),
            PathWorldVertices.Num(), DetectedStepHeight);
    }

═══════════════════════════════════════
修改 3：修改 Step 7（设置 Spline 控制点）为条件分支
═══════════════════════════════════════

将当前 Step 7 的代码：

    // 7. 设置 Spline 控制点（使用相对于 ActorLocation 的本地坐标）
    SplineComp->ClearSplinePoints(false);
    SplineComp->AddSplinePoint(BottomPoint - ActorLocation, ESplineCoordinateSpace::Local, false);
    SplineComp->AddSplinePoint(TopPoint - ActorLocation, ESplineCoordinateSpace::Local, false);
    SplineComp->UpdateSpline();

替换为：

    // 7. 设置 Spline 控制点
    SplineComp->ClearSplinePoints(false);
    
    if (bUseAutoPath)
    {
        // 使用自动路径点（世界坐标 → 本地坐标）
        for (int32 pi = 0; pi < AutoPathPoints.Num(); pi++)
        {
            FVector LocalPoint = AutoPathPoints[pi] - ActorLocation;
            SplineComp->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, false);
        }
        UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: Using auto path with %d control points"),
            AutoPathPoints.Num());
    }
    else
    {
        // fallback：原有 2 点逻辑
        SplineComp->AddSplinePoint(BottomPoint - ActorLocation, ESplineCoordinateSpace::Local, false);
        SplineComp->AddSplinePoint(TopPoint - ActorLocation, ESplineCoordinateSpace::Local, false);
        UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: Using default 2-point path (bottom + top)"));
    }
    SplineComp->UpdateSpline();

═══════════════════════════════════════
修改 4：Step 11 日志更新
═══════════════════════════════════════

将 Step 11 的日志中 Spline 点数改为实际数量（已经是动态的 GetNumberOfSplinePoints，无需修改）。

新增一条 Tag 存储自动路径点数：

    // 在 Step 10.5 之后添加
    SplineActor->Tags.Add(FName(*FString::Printf(TEXT("SplineAutoPoints_%d"),
        SplineComp->GetNumberOfSplinePoints())));

═══════════════════════════════════════
完整修改清单
═══════════════════════════════════════

1. 将 Step 9.5（行 265-346 的 Z 值提取 + StepHeight 检测）
   移动到 Step 6.5 之后、自动路径检测之前
   变量名 MeshComp → StepMeshComp（避免冲突）

2. 在移动后的 Step 9.5 之后插入自动路径检测代码
   （PathWorldVertices 提取 + ComputeLayerCentroids + DouglasPeucker）

3. Step 7 的 2 点逻辑改为条件分支
   （bUseAutoPath → 自动路径 / else → 原有 2 点）

4. Step 10.5 之后新增 "SplineAutoPoints_X" Tag

请输出修改后的完整 CreateStairSpline 函数。
不要修改其他函数。
编译失败只报告，不自行修复。
```

## 验证

```
完成后检查：
  1. CreateStairSpline 函数中执行顺序为：
     Step 1    参数校验
     Step 2-3  BoundingBox + 端点
     Step 3.5  ActorLocation
     Step 4    StairWidth 估算
     Step 5    SpawnActor
     Step 6    SplineComponent 创建
     Step 6.5  SetActorLocation
     Step 9.5  StepHeight 检测（从原位置移过来的）
     新增      自动路径检测
     Step 7    Spline 控制点（条件分支）
     Step 8    线性设置
     Step 9    可视化
     Step 10   Tags 存储
     Step 11   日志
     Step 12   return
  
  2. Step 9.5 在原位置已被删除（不能出现两次）
  3. 其他函数全部未修改
  4. 编译通过
```