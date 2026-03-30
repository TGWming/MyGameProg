# Spline 自动路径 — Step 2：新增 DouglasPeucker 函数

## PROMPT

```
任务：修改 Source/soulEditor/Private/StairRampGeneratorBFL.cpp
仅新增一个静态函数 DouglasPeucker。

安全约束：
  - 不修改 Source/soul/ 下的文件
  - 不修改 .uproject / .Target.cs / .Build.cs
  - 不删除 Intermediate 或 Binaries
  - 不修改任何已有函数（CreateStairSpline、GenerateRampFromSpline、
    CreateRampSegment、ExtractMeshTriangles、DetectStairSteps、
    GenerateStraightStairRamp、ComputeLayerCentroids）
  - 不修改 .h 头文件（本 Step 不改头文件）
  - 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径读取
  - 编译配置：Development Editor + Win64
  - 编译失败只报告，不自行修复

目标：
  新增 Douglas-Peucker 路径简化函数。
  输入一组有序路径点，删除"几乎在直线上"的中间点，
  只保留方向发生明显变化的关键拐点。

函数位置：
  在 ComputeLayerCentroids 函数之后、CreateStairSpline 函数之前插入。

函数签名：
  static TArray<FVector> DouglasPeucker(
      const TArray<FVector>& Path,
      float Tolerance)

实现逻辑：

1. 边界条件：
   if (Path.Num() <= 2)
       return Path; // 2 个点以下无需简化

2. 找到离首尾连线最远的中间点：
   FVector LineStart = Path[0];
   FVector LineEnd = Path.Last();
   FVector LineDir = (LineEnd - LineStart).GetSafeNormal();
   float LineLength = FVector::Dist(LineStart, LineEnd);
   
   float MaxDist = 0.0f;
   int32 MaxIndex = 0;
   
   for (int32 i = 1; i < Path.Num() - 1; i++)
   {
       // 计算点到直线的距离
       FVector PointToStart = Path[i] - LineStart;
       FVector Projection = FVector::DotProduct(PointToStart, LineDir) * LineDir;
       FVector Perpendicular = PointToStart - Projection;
       float Dist = Perpendicular.Size();
       
       if (Dist > MaxDist)
       {
           MaxDist = Dist;
           MaxIndex = i;
       }
   }

3. 递归或保留：
   if (MaxDist > Tolerance)
   {
       // 最远点超出容差 → 保留该点，递归处理两半
       TArray<FVector> LeftHalf;
       for (int32 i = 0; i <= MaxIndex; i++)
           LeftHalf.Add(Path[i]);
       
       TArray<FVector> RightHalf;
       for (int32 i = MaxIndex; i < Path.Num(); i++)
           RightHalf.Add(Path[i]);
       
       TArray<FVector> SimplifiedLeft = DouglasPeucker(LeftHalf, Tolerance);
       TArray<FVector> SimplifiedRight = DouglasPeucker(RightHalf, Tolerance);
       
       // 合并（去掉右半段的第一个点，避免重复）
       TArray<FVector> Result = SimplifiedLeft;
       for (int32 i = 1; i < SimplifiedRight.Num(); i++)
           Result.Add(SimplifiedRight[i]);
       
       return Result;
   }
   else
   {
       // 所有中间点都在容差内 → 只保留首尾
       TArray<FVector> Result;
       Result.Add(Path[0]);
       Result.Add(Path.Last());
       return Result;
   }

4. 日志（在递归最外层不加日志，避免递归时刷屏）：
   不在此函数内加 UE_LOG。
   调用方在调用后打印简化结果。

请在 ComputeLayerCentroids 之后、CreateStairSpline 之前插入这个完整函数。
不要修改任何已有函数。
编译失败只报告，不自行修复。
```

## 验证

```
完成后检查：
  1. 新函数位于 ComputeLayerCentroids 之后、CreateStairSpline 之前
  2. 递归逻辑正确：始终保留首尾点
  3. 所有已有函数未被修改
```