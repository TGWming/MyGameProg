# Spline 自动路径 — Step 1：新增 ComputeLayerCentroids 函数

## PROMPT

```
任务：修改 Source/soulEditor/Private/StairRampGeneratorBFL.cpp
仅新增一个静态函数 ComputeLayerCentroids。

安全约束：
  - 不修改 Source/soul/ 下的文件
  - 不修改 .uproject / .Target.cs / .Build.cs
  - 不删除 Intermediate 或 Binaries
  - 不修改任何已有函数（CreateStairSpline、GenerateRampFromSpline、
    CreateRampSegment、ExtractMeshTriangles、DetectStairSteps、
    GenerateStraightStairRamp）
  - 不修改 .h 头文件（本 Step 不改头文件）
  - 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径读取
  - 编译配置：Development Editor + Win64
  - 编译失败只报告，不自行修复

目标：
  新增一个静态函数，将世界坐标顶点按 Z 高度分层，
  每层计算 XY 质心，输出按 Z 从低到高排序的质心列表。

函数位置：
  在 CreateStairSpline 函数之前（行 180 之前）插入新函数。

函数签名：
  static TArray<FVector> ComputeLayerCentroids(
      const TArray<FVector>& WorldVertices,
      float StepHeight,
      float BBoxMinZ,
      float BBoxMaxZ)

实现逻辑：

1. 计算分层间距和层数：
   float LayerHeight = StepHeight * 2.0f;
   // 每 2 级台阶一层，平衡精度和噪声
   
   if (LayerHeight < 10.0f)
       LayerHeight = 10.0f; // 安全下限
   
   int32 NumLayers = FMath::CeilToInt((BBoxMaxZ - BBoxMinZ) / LayerHeight);
   if (NumLayers < 1) NumLayers = 1;
   if (NumLayers > 100) NumLayers = 100; // 安全上限

2. 按 Z 高度将顶点分配到各层：
   TArray<TArray<FVector>> Layers;
   Layers.SetNum(NumLayers);
   
   for (const FVector& Vert : WorldVertices)
   {
       int32 LayerIndex = FMath::FloorToInt((Vert.Z - BBoxMinZ) / LayerHeight);
       LayerIndex = FMath::Clamp(LayerIndex, 0, NumLayers - 1);
       Layers[LayerIndex].Add(Vert);
   }

3. 每层计算 XY 质心：
   TArray<FVector> Centroids;
   
   for (int32 i = 0; i < NumLayers; i++)
   {
       // 跳过顶点太少的层（噪声、栏杆碎片等）
       if (Layers[i].Num() < 10)
       {
           UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: Layer %d skipped (%d vertices, need >= 10)"),
               i, Layers[i].Num());
           continue;
       }
       
       FVector Sum = FVector::ZeroVector;
       for (const FVector& V : Layers[i])
       {
           Sum += V;
       }
       Sum /= Layers[i].Num();
       
       // Z 值用该层范围的中心（而非顶点平均 Z，避免偏向某侧台阶）
       Sum.Z = BBoxMinZ + LayerHeight * (i + 0.5f);
       
       Centroids.Add(Sum);
       
       UE_LOG(LogTemp, Log, TEXT("StairRampGenerator: Layer %d centroid=(%.1f, %.1f, %.1f) from %d vertices"),
           i, Sum.X, Sum.Y, Sum.Z, Layers[i].Num());
   }

4. 返回：
   return Centroids;

请在 CreateStairSpline 之前插入这个完整函数。
不要修改任��已有函数。
编译失败只报告，不自行修复。
```

## 验证

```
完成后检查：
  1. 新函数位于 CreateStairSpline 之前
  2. 所有已有函数未被修改
  3. 函数可被同文件内其他函数调用（static 文件级函数）
```