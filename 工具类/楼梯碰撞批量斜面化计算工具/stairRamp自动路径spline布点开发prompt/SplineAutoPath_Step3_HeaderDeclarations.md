# Spline 自动路径 — Step 3：头文件新增函数声明

## PROMPT

```
任务：修改 Source/soulEditor/Public/StairRampGeneratorBFL.h
仅在 private 区域新增 2 个静态函数声明。

安全约束：
  - 不修改 Source/soul/ 下的文件
  - 不修改 .uproject / .Target.cs / .Build.cs
  - 不删除 Intermediate 或 Binaries
  - 不修改任何已有的函数声明或成员变量
  - 不修改 .cpp 源文件（本 Step 只改头文件）
  - 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径读取
  - 编译配置：Development Editor + Win64
  - 编译失败只报告，不自行修复

目标：
  在 UStairRampGeneratorBFL 类的 private 区域中，
  在已有的 CreateRampSegment 声明之后，
  新增 2 个静态函数声明。

具体新增内容：

在 private: 区域中 CreateRampSegment 声明之后添加：

    /** 按 Z 高度将世界坐标顶点分层，每层计算 XY 质心 */
    static TArray<FVector> ComputeLayerCentroids(
        const TArray<FVector>& WorldVertices,
        float StepHeight,
        float BBoxMinZ,
        float BBoxMaxZ);

    /** Douglas-Peucker 路径简化算法，删除近似共线的中间点 */
    static TArray<FVector> DouglasPeucker(
        const TArray<FVector>& Path,
        float Tolerance);

注意事项：
  - 确保在已有的 private: 区域中添加，不要创建新的 private: 块
  - 不修改已有的 CreateRampSegment 声明
  - 不修改 public 区域的任何声明
  - 不修改 FStairStep 结构体
  - 不添加新的 #include

请输出修改后的完整头文件。
编译失败只报告，不自行修复。
```

## 验证

```
完成后检查：
  1. private 区域中有 3 个函数声明：
     - CreateRampSegment（原有）
     - ComputeLayerCentroids（新增）
     - DouglasPeucker（新增）
  2. public 区域的 5 个函数声明未被修改
  3. FStairStep 结构体未被修改
  4. #include 列表未被修改
```