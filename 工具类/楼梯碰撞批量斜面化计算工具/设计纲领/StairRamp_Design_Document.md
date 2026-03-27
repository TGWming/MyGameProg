# 🏗️ 楼梯碰撞斜面化工具 — 完整设计纲领

> **项目：** soul (UE4.27)
> **模块：** soulEditor (Editor-Only)
> **文档日期：** 2026-03-27
> **状态：** 架构设计完成，未开始编码

---

## 一、核心概念

### 1.1 目标

把楼梯的 **阶梯状碰撞（L形）** 变成 **一个光滑斜面（三角形填充）**，角色在上面跑就像在坡道上一样顺畅。

```
之前（阶梯碰撞）：              之后（斜面碰撞）：

    ┌─┐                         ╱─┐
   ┌┘ │                       ╱  │
  ┌┘  │          →          ╱   │
 ┌┘   │                  ╱    │
┌┘    │                ╱─────┘
└─────┘
                    填充的三角形区域
```

**关键理解：我们不是"替换"碰撞，而是在原始 Complex Collision 的阶梯凹槽上方"叠加"一个斜面碰撞体，把 L 形的凹陷抹平。原始碰撞保持不动。**

### 1.2 设计哲学

参考类魂游戏的做法：直接放弃楼梯的精细阶梯碰撞，用一个斜面覆盖整段楼梯。角色走在斜面上，感受到的是一个坡道，而不是一��一级的台阶。

### 1.3 名词表

| 名词 | 英文 | 说明 |
|------|------|------|
| 踏步面 | Tread | 每一级楼梯的水平面 |
| 踢面 | Riser | 每一级楼梯的垂直面 |
| 步高 | Step Height | 单级台阶的高度（通常 15-20cm） |
| 步深 | Step Depth / Run | 单级台阶的水平深度（通常 25-30cm） |
| 总升 | Total Rise | 楼梯从底到顶的总高度差 |
| 总进 | Total Run | 楼梯从底到顶的总水平距离 |
| 坡度角 | Slope Angle | 斜面与水平面的夹角 |
| 行走面 | Walkable Surface | UE 中角色可以站立/行走的面（法线 Z 分量 ≥ cos(WalkableFloorAngle)） |
| BoundingBox | AABB | 轴对齐包围盒，包裹 Mesh 的最小矩形 |
| Complex Collision | — | 使用原始三角形网格作为碰撞体 |
| Convex Hull | 凸包 | 包裹一组点的最小凸多面体 |
| 螺旋线 | Helix | 旋转楼梯的路径���线 |
| 螺距 | Pitch (P) | 螺旋线旋转一圈上升的高度 |
| 法线 | Normal | 三角形面的朝向向量 |
| 聚类 | Clustering | 把相近的数据点归为一组 |

---

## 二、三种楼梯类型的数学原理

### 2.1 Type A：直梯（Straight Staircase）

最简单的情况，一个倾斜的长方体就能搞定。

#### 几何模型

```
侧视图：
                    B (顶端)
                   ╱│
                 ╱  │
               ╱    │ Rise (总升 H)
             ╱      │
           ╱        │
    A ───╱──────────┘
         Run (总进 L)

A = 楼梯底部起点
B = 楼梯顶部终点
H = B.Z - A.Z（总高度差）
L = 沿楼梯前进方向的水平距离
W = 楼梯宽度（垂直于前进方向）
```

#### 数学计算

```
输入：
  H = 楼梯总高度 (BoundingBox.Max.Z - BoundingBox.Min.Z)
  L = 楼梯总长度 (沿楼梯前进方向的 BoundingBox 尺寸)
  W = 楼梯宽度 (垂直于前进方向的 BoundingBox 尺寸)

计算：
  θ = atan2(H, L)              // 坡度角（弧度）
  θ_deg = θ × 180 / π          // 坡度角（度）
  S = √(H² + L²)              // 斜面长度（勾股定理）

生成的 Box Collision：
  尺寸（Half Extents）= (S/2, W/2, 薄厚度)
  薄厚度建议 = 2.5cm ~ 5cm（确保角色不穿透）
  
  位置 = 楼梯 BoundingBox 的中心点
         具体为：((A.X+B.X)/2, (A.Y+B.Y)/2, (A.Z+B.Z)/2)
  
  旋转 = 绕 Pitch 轴（Y轴）旋转 -θ_deg 度
         注意：需要先对齐楼梯的朝向（Yaw），再叠加 Pitch
```

#### 确定"前进方向"的方法

**方案 1：用 Actor 的 Forward Vector（简单）**
- 大多数楼梯 Mesh 沿 X 轴建模
- 直接用 Actor 的 ForwardVector 作为前进方向
- L = BoundingBox 在 Forward 方向的投影长度

**方案 2：分析踏步面（精确）**
- 找最低踏步面和最高踏步面的中心点
- 连线方向投影到 XY 平面 = 前进方向
- 这个方法适用于楼梯 Actor 旋转过的情况

#### 示例计算

```
假设一段楼梯：
  10 级台阶，每级步高 18cm，步深 28cm
  
  H = 10 × 18cm = 180cm
  L = 10 × 28cm = 280cm
  W = 120cm（楼梯宽度）
  
  θ = atan2(180, 280) = 32.7°
  S = √(180² + 280²) = √(32400 + 78400) = √110800 ≈ 332.9cm
  
  Box Half Extents = (166.5, 60, 2.5) cm
  Box Rotation = Pitch -32.7°
```

---

### 2.2 Type B：L 形 / 拐角楼梯（L-Shaped / Multi-Flight）

由多段直梯 + 中间平台组成。

#### 几何模型

```
俯视图：
    ┌──────┐
    │ 第2段 │ ↑ 前进方向2
    │      │
    ├──────┤ ← 平台（Landing，水平）
    │      │
    │ 第1段 │ ↑ 前进方向1
    └──────┘

侧视图展开：

              B2
             ╱│
    B1─────╱ │    第2段
   ╱│  平台   │
  ╱ │         │
 A  │         │    第1段
    
平台是水平的，不需要 Ramp
第1段和第2段各自独立生成 Ramp
```

#### 处理算法

```
算法：L-Shaped Stair Processing

1. 获取 Mesh 所有三角形
2. 用 DetectStairSteps() 筛选所有踏步面
3. 识别"平台"：
   - 面积明显大于普通踏步面的水平面（面积 > 平均踏步面积 × 2）
   - 或者：连续多级踏步面高度相同（ΔZ ≈ 0）
4. 用平台把楼梯分成多个"段"（Flight）
5. 每段独立处理：
   
   For each Flight:
     H_flight = 该段最高踏步面.Z - 该段最低踏步面.Z
     
     // 前进方向 = 最低踏步面中心 → 最高踏步面中心（XY投影）
     Direction = (HighestStep.Center - LowestStep.Center)
     Direction.Z = 0
     Direction.Normalize()
     
     L_flight = |HighestStep.Center - LowestStep.Center| 在 Direction 上的投影
     W_flight = 楼梯宽度（从踏步面的横向尺寸推算）
     
     θ = atan2(H_flight, L_flight)
     S = √(H_flight² + L_flight²)
     
     生成 Box Collision：
       尺寸 = (S/2, W_flight/2, 薄厚度)
       位置 = 该段中心
       旋转 = Yaw(朝向 Direction) + Pitch(-θ)
```

#### 平台处理

```
平台不生成 Ramp，保持原始碰撞即可。
平台本身就是水平面，角色走在上面不会有问题。
```

---

### 2.3 Type C：旋转楼梯（Spiral Staircase）

旋转楼梯的路径是一条 **螺旋线（Helix）**。

#### 螺旋线参数方程

```
三维螺旋线的参数方程：

  x(t) = R × cos(t)
  y(t) = R × sin(t)
  z(t) = P × t / (2π)

参数说明：
  R = 旋转半径（楼梯中心到踏步面外缘的距离）
  P = 螺距（旋转一圈 360° 上升的高度）
  t = 角度参数（弧度），从 0 到 TotalAngle

导出参数：
  TotalAngle = 楼梯总旋转角度（如 360° = 2π, 540° = 3π）
  TotalHeight = P × TotalAngle / (2π)
  
  反推螺距：
  P = TotalHeight × (2π) / TotalAngle
```

#### 几何模型

```
俯视图：                    侧视图展开：
      ╱──╲                      ╱
    ╱      ╲                  ╱
   │   中心  │              ╱   ← 螺旋展开后就是一个斜面
    ╲      ╱              ╱
      ╲──╱              ╱

3D 视图（多段小 Ramp 拼接）：

   每段 Ramp 覆盖 15-30° 弧度
   段越多，近似越精确
   
        ╱▓▓╲         ▓ = 单段 Ramp
      ╱▓▓▓▓▓▓╲
     │▓▓ 中心 ▓▓│
      ╲▓▓▓▓▓▓╱
        ╲▓▓╱
```

#### 参数估算方法

```
1. 检测旋转楼梯特征：
   - BoundingBox 在 XY 平面近似正方形（Width ≈ Depth）
   - 高度 H 远大于 Width/2
   
2. 估算旋转半径 R：
   R = BoundingBox.Width / 2
   （或更精确：找所有踏步面中心到 BoundingBox 中心的平均距离）
   
3. 分析踏步面确定旋转方向和总角度：
   a. 找所有踏步面的中心点
   b. 投影到 XY 平面
   c. 计算每个踏步面相对于 BoundingBox XY 中心的角度：
      angle_i = atan2(Step_i.Center.Y - BB.Center.Y, 
                      Step_i.Center.X - BB.Center.X)
   d. 按 Z 高度排序踏步面
   e. 按排序后的角度序列判断旋转方向（顺时针/逆时针）
   f. 累加角度差 = TotalAngle
   
4. 计算螺距 P：
   P = TotalHeight × (2π) / TotalAngle
```

#### 分段生成算法

```
算法：Spiral Stair Ramp Generation

输入：R（半径）, P（螺距）, TotalAngle, TotalHeight
      StairWidth（楼梯宽度）, SegmentAngle（每段弧度，建议 15°-30°）
      Center（螺旋中心 XY 坐标）, BaseZ（楼梯底部 Z）

NumSegments = ceil(TotalAngle / SegmentAngle)

For i = 0 to NumSegments - 1:
    // 这一段的起止角度
    t0 = i × SegmentAngle
    t1 = min((i + 1) × SegmentAngle, TotalAngle)
    
    // 起点坐标
    Start.X = Center.X + R × cos(t0)
    Start.Y = Center.Y + R × sin(t0)
    Start.Z = BaseZ + P × t0 / (2π)
    
    // 终点坐标
    End.X = Center.X + R × cos(t1)
    End.Y = Center.Y + R × sin(t1)
    End.Z = BaseZ + P × t1 / (2π)
    
    // 这一段的方向向量
    Direction = End - Start
    
    // 水平分量和垂直分量
    HorizontalDir = Direction
    HorizontalDir.Z = 0
    SegmentRun = HorizontalDir.Length()    // 水平距离
    SegmentRise = End.Z - Start.Z          // 垂直距离
    SegmentLength = Direction.Length()      // 斜面长度
    
    // 这一段的坡度角
    θ_segment = atan2(SegmentRise, SegmentRun)
    
    // 这一段的朝向（Yaw）
    Yaw_segment = atan2(HorizontalDir.Y, HorizontalDir.X)
    
    // 生成 Box Collision
    Box 位置 = (Start + End) / 2
    Box 旋转 = FRotator(-θ_segment_deg, Yaw_segment_deg, 0)
    Box 尺寸 = (SegmentLength/2, StairWidth/2, 薄厚度)
    
    SpawnRampCollision(位置, 旋转, 尺寸)
```

#### 精度分析

```
每段弧度 vs 近似误差：

  30° 一段 → 12 段/圈 → 误差较明显，弧线处有棱角
  15° 一段 → 24 段/圈 → 误差小，基本光滑
  10° 一段 → 36 段/圈 → 非常精确，几乎无感

推荐：15° 一段，平衡精度和碰撞体数量

角色感知：
  角色胶囊体宽度通常 34-42cm
  15° 弧段在 R=150cm 的螺旋梯上，弧长 ≈ 39cm
  接近角色宽度，走起来几乎感觉不到段落接缝
```

---

## 三、核心算法详解

### 3.1 从 Mesh 提取三角形 — ExtractMeshTriangles

```
函数：ExtractMeshTriangles(UStaticMesh* Mesh)
返回：TArray<FTriangle> （每个三角形包含 3 个顶点 + 法线）

实现路径（UE4.27）：

方案 A — 从 RenderData 提取（编辑器模式可用）：
  1. Mesh->GetRenderData()->LODResources[0]
  2. 读取 PositionVertexBuffer → 所有顶点位置
  3. 读取 IndexBuffer → 三角形索引
  4. 每 3 个索引 = 1 个三角形
  5. 计算法线：Normal = Cross(v1-v0, v2-v0).Normalize()

方案 B — 从 BodySetup 提取碰撞三角形：
  1. Mesh->GetBodySetup()
  2. BodySetup->TriMeshes → 物理三角形网格
  3. 遍历 PxTriangleMesh 的顶点和索引

方案 C — 从 Complex Collision 提取：
  1. UBodySetup* BS = Mesh->GetBodySetup()
  2. BS->GetCookInfo() 或直接访问 BS->TriMeshBodies
  3. 需要 PhysX API 访问权限

推荐：方案 A（RenderData），在 Editor 模式下最稳定
```

#### 代码结构

```cpp
struct FStairTriangle
{
    FVector V0, V1, V2;    // 三个顶点（世界坐标）
    FVector Normal;         // 面法线
    float Area;             // 三角形面积
    FVector Center;         // 三角形中心 = (V0+V1+V2)/3
};

TArray<FStairTriangle> ExtractMeshTriangles(
    UStaticMeshComponent* MeshComp)
{
    // 1. 获取 StaticMesh
    UStaticMesh* Mesh = MeshComp->GetStaticMesh();
    FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
    
    // 2. 读取顶点
    FPositionVertexBuffer& VertexBuffer = LOD.VertexBuffers.PositionVertexBuffer;
    uint32 NumVertices = VertexBuffer.GetNumVertices();
    
    // 3. 读取索引
    FRawStaticIndexBuffer& IndexBuffer = LOD.IndexBuffer;
    TArray<uint32> Indices;
    IndexBuffer.GetCopy(Indices);
    
    // 4. 组装三角形（转换到世界坐标）
    FTransform WorldTransform = MeshComp->GetComponentTransform();
    TArray<FStairTriangle> Triangles;
    
    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        FStairTriangle Tri;
        Tri.V0 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i]));
        Tri.V1 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i+1]));
        Tri.V2 = WorldTransform.TransformPosition(VertexBuffer.VertexPosition(Indices[i+2]));
        
        FVector Edge1 = Tri.V1 - Tri.V0;
        FVector Edge2 = Tri.V2 - Tri.V0;
        FVector CrossProduct = FVector::CrossProduct(Edge1, Edge2);
        
        Tri.Area = CrossProduct.Size() * 0.5f;
        Tri.Normal = CrossProduct.GetSafeNormal();
        Tri.Center = (Tri.V0 + Tri.V1 + Tri.V2) / 3.0f;
        
        Triangles.Add(Tri);
    }
    
    return Triangles;
}
```

---

### 3.2 踏步面检测 — DetectStairSteps

```
函数：DetectStairSteps(TArray<FStairTriangle>& Triangles)
返回：TArray<FStairStep> 按高度排序的踏步面列表

算法步骤：

Step 1: 筛选近似水平的三角形
  条件：Normal.Z > 0.95（法线几乎朝上 = 水平面）
  数学原理：
    完全水平面的法线 = (0, 0, 1)，Normal.Z = 1.0
    Normal.Z > 0.95 对应倾斜角 < acos(0.95) ≈ 18.2°
    这个容差允许轻微不平的踏步面通过

Step 2: 按 Z 高度聚类
  容差 = 2cm（同一级踏步面上的三角形 Z 可能有微小差异）
  
  算法：
    Sort candidates by Center.Z ascending
    clusters = []
    current_cluster = [candidates[0]]
    
    For i = 1 to N-1:
      If |candidates[i].Center.Z - current_cluster.last.Center.Z| < 2cm:
        current_cluster.Add(candidates[i])
      Else:
        clusters.Add(current_cluster)
        current_cluster = [candidates[i]]
    clusters.Add(current_cluster)

Step 3: 为每个聚类生成 StairStep 信息
  struct FStairStep:
    float Height           // 该级踏步面的平均 Z 高度
    FVector Center         // 该级所有三角形中心的平均值
    float TotalArea        // 该级所有三角形面积之和
    int32 TriangleCount    // 三角形数量
    bool bIsPlatform       // 是否是平台（大面积）
  
  For each cluster:
    Step.Height = cluster 所有三角形 Center.Z 的平均值
    Step.Center = cluster 所有三角形 Center 的平均值
    Step.TotalArea = cluster 所有三角形面积之和
    Step.TriangleCount = cluster.Num()

Step 4: 过滤噪声
  - 面积太小的聚类（< 100cm²）→ 不是踏步面，可能是装饰细节，丢弃
  
Step 5: 识别平台
  AverageStepArea = 所有 Step 的面积平均值
  For each Step:
    If Step.TotalArea > AverageStepArea × 2.0:
      Step.bIsPlatform = true

Step 6: 按高度排序输出
  Sort Steps by Height ascending
```

#### 数据结构

```cpp
struct FStairStep
{
    float Height;           // 平均 Z 高度
    FVector Center;         // 中心位置（世界坐标）
    float TotalArea;        // 总面积 (cm²)
    int32 TriangleCount;    // 三角形数量
    bool bIsPlatform;       // 是否是平台
};
```

---

### 3.3 楼梯类型识别 — ClassifyStairType

```
函数：ClassifyStairType(TArray<FStairStep>& Steps)
返回：EStairType { Straight, LShaped, Spiral }

算法：基于踏步面中心点在 XY 平面上的角度变化

Step 1: 计算所有踏步面的 XY 中心
  BB_Center = 所有 Step.Center 的 BoundingBox 中心（XY）

Step 2: 计算每个踏步面相对于 BB_Center 的角度
  For each Step (按 Height 排序):
    dx = Step.Center.X - BB_Center.X
    dy = Step.Center.Y - BB_Center.Y
    angle_i = atan2(dy, dx) × 180 / π    // 角度制

Step 3: 分析角度变化模式
  
  // 计算相邻踏步面的角度差
  TArray<float> AngleDeltas;
  For i = 1 to N-1:
    delta = angle[i] - angle[i-1]
    // 处理 -180°/180° 跳变
    While delta > 180: delta -= 360
    While delta < -180: delta += 360
    AngleDeltas.Add(delta)
  
  TotalAngleChange = Sum(AngleDeltas)
  MaxSingleDelta = Max(|AngleDeltas|)
  
  // 判断规则：
  If |TotalAngleChange| < 10°:
    // 角度几乎没变 → 直梯
    return EStairType::Straight
    
  Else If |TotalAngleChange| > 90° AND 没有大面积平台:
    // 角度持续变化超过 90° 且没有平台 → 旋转楼梯
    return EStairType::Spiral
    
  Else If 存在大面积平台 (bIsPlatform == true):
    // 有平台 → L形/多段楼梯
    return EStairType::LShaped
    
  Else:
    // 默认当直梯处理
    return EStairType::Straight
```

#### 枚举定义

```cpp
UENUM(BlueprintType)
enum class EStairType : uint8
{
    Straight    UMETA(DisplayName = "Straight"),
    LShaped     UMETA(DisplayName = "L-Shaped"),
    Spiral      UMETA(DisplayName = "Spiral"),
    Unknown     UMETA(DisplayName = "Unknown")
};
```

---

### 3.4 坡度角计算 — CalculateSlopeAngle

```
函数：CalculateSlopeAngle(FVector Start, FVector End)
返回：float 角度（度）

计算：
  Direction = End - Start
  
  // 水平分量
  HorizontalDistance = √(Direction.X² + Direction.Y²)
  
  // 垂直分量
  VerticalDistance = |Direction.Z|
  
  // 坡度角
  SlopeRadians = atan2(VerticalDistance, HorizontalDistance)
  SlopeDegrees = SlopeRadians × 180 / π
  
  return SlopeDegrees

注意：
  UE4 的 WalkableFloorAngle 默认为 44.765°
  如果计算出的坡度 > WalkableFloorAngle，角色将无法行走
  一般楼梯坡度在 25°-40° 之间，不会超过限制
```

---

### 3.5 Ramp 碰撞体生成 — SpawnRampCollision

```
函数：SpawnRampCollision(UWorld* World,
                          FVector Location,
                          FRotator Rotation,
                          FVector BoxHalfExtents,
                          AActor* ParentStairActor)
返回：生成的 Ramp Actor

Step 1: 在关卡中生成一个空 Actor
  AActor* RampActor = World->SpawnActor<AActor>(Location, Rotation)
  
Step 2: 添加 SceneComponent 作为根
  USceneComponent* Root = NewObject<USceneComponent>(RampActor)
  RampActor->SetRootComponent(Root)
  Root->RegisterComponent()
  
Step 3: 添加 BoxComponent
  UBoxComponent* Box = NewObject<UBoxComponent>(RampActor)
  Box->SetupAttachment(Root)
  Box->SetBoxExtent(BoxHalfExtents)
  Box->SetCollisionProfileName(TEXT("BlockAll"))
  Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)
  Box->SetVisibility(false)  // 不可见
  Box->RegisterComponent()
  
Step 4: 设置标签便于管理
  RampActor->Tags.Add(FName("StairRamp_Auto"))
  
  // 可选：记录关联的楼梯 Actor
  // 用于后续删除/更新时找到对应关系
  
Step 5: 碰撞响应设置
  Box->SetCollisionResponseToAllChannels(ECR_Block)
  // 确保对 Pawn 阻挡
  Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block)
  // 对 Visibility 通道忽略（不影响射线检测视线）
  Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore)
```

---

## 四、建筑内嵌楼梯的特殊处理

### 4.1 问题描述

部分资产是一整栋建筑，楼梯是建筑的一部分。使用 Complex Collision As Simple 时，楼梯已经有精准的阶梯碰撞。

```
整栋建筑 Mesh：
┌─────────────────┐
│  2F             │
│  ┌─┐            │
│ ┌┘ │ ← 楼梯部分 │
│┌┘  │            │
││   │            │
│└───┘            │
│  1F             │
└─────────────────┘
```

### 4.2 处理原则

```
1. 不动原始 Mesh 的碰撞设置（不能关闭整栋建筑的碰撞）
2. 只在楼梯区域上方叠加 Ramp Collision
3. Ramp Collision 的设置：
   - Collision Response: Block for Pawn/Character
   - 厚度 ≥ 5cm（确保角色不穿透 Ramp 碰到下面的台阶）
4. 角色踩在 Ramp 上 → 不会碰到原始阶梯碰撞
   （因为 Ramp 在上方，角色被 Ramp 支撑住了）
```

### 4.3 效果示意

```
叠加 Ramp 后：
┌─────────────────┐
│  2F             │
│  ╱─┐            │
│ ╱  │ ← Ramp 覆盖在楼梯上方
│╱   │            │
│    │            │
│    │            │
│  1F             │
└─────────────────┘

角色走在 Ramp 上，感觉是斜面
原始楼梯碰撞仍在，但角色碰不到（被 Ramp 挡住了）
```

### 4.4 工作流

```
对于建筑内嵌楼梯：
1. 用户手动选中建筑 Actor
2. 工具扫描 Mesh 三角形，检测楼梯区域
3. 自动在楼梯区域上方生成 Ramp
4. 建筑其他部分的碰撞完全不受影响
```

---

## 五、整体架构

### 5.1 模块位置

```
Source/soulEditor/
├── Public/
│   ├── FolderColorMarkerBFL.h     （已有）
│   └── StairRampGeneratorBFL.h    （新增）
├── Private/
│   ├── soulEditorModule.cpp       （已有）
│   ├── FolderColorMarkerBFL.cpp   （已有）
│   └── StairRampGeneratorBFL.cpp  （新增）
└── soulEditor.Build.cs            （已有，可能需添加依赖）
```

### 5.2 类架构图

```
┌─────────────────────────────────────────────────┐
│              StairRampGenerator                  │
│         (Editor Utility in soulEditor)          │
├─────────────────────────────────────────────────┤
│                                                  │
│  ┌──────────────┐    ┌────────────────────┐     │
│  │  入口函数      │    │  楼梯类型识别器      │     │
│  │ GenerateRamps │───→│ ClassifyStairType  │     │
│  │ ForSelected() │    │                    │     │
│  └──────┬───────┘    └────────┬───────────┘     │
│         │                     │                  │
│         │         ┌───────────┼───────────┐     │
│         │         ▼           ▼           ▼     │
│         │  ┌──────────┐ ┌──────────┐ ┌────────┐│
│         │  │ Straight  │ │ L-Shaped │ │ Spiral ││
│         │  │ Generator │ │ Generator│ │Generator││
│         │  └────┬─────┘ └────┬─────┘ └───┬────┘│
│         │       │            │            │     │
│         │       ▼            ▼            ▼     │
│         │  ┌────────────────────────────────┐   │
│         │  │     Ramp Collision Spawner      │   │
│         │  │  - 生成 BoxCollision Actor      │   │
│         │  │  - 设置位置/旋转/大小            │   │
│         │  │  - 不动原始楼梯碰撞              │   │
│         │  └────────────��───────────────────┘   │
│         │                                        │
│  ┌──────┴───────┐                               │
│  │ 共用工具函数   │                               │
│  │              │                               │
│  │ - ExtractMeshTriangles()  从 Mesh 提取三角形  │
│  │ - DetectStairSteps()      识别踏步面          │
│  │ - ClassifyStairType()     判断直梯/L形/旋转    │
│  │ - CalculateSlopeAngle()   计算斜面角度        │
│  │ - SpawnRampCollision()    生成碰撞体          │
│  │ - RemoveAutoRamps()       删除自动生成的 Ramp  │
│  └──────────────┘                               │
│                                                  │
└─────────────────────────────────────────────────┘
```

### 5.3 公开接口（Blueprint Callable）

```cpp
UCLASS()
class SOULEDITOR_API UStairRampGeneratorBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 为编辑器中当前选中的 Actor 生成楼梯 Ramp
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void GenerateRampsForSelected();

    // 删除关卡中所有自动生成的 Ramp
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void RemoveAllAutoRamps();

    // 为选中的 Actor 生成 Ramp（自定义厚度）
    UFUNCTION(BlueprintCallable, Category = "StairRamp", meta = (CallInEditor = "true"))
    static void GenerateRampsWithThickness(float Thickness = 5.0f);
};
```

---

## 六、数据流详解

```
完整数据流：

Step 1: 用户在编辑器中选中一个或多个楼梯 Actor
         │
         ▼
Step 2: GenerateRampsForSelected()
         │
         ├─ 获取编辑器当前选中的 Actor 列表
         │  GEditor->GetSelectedActors()
         │
         ▼
Step 3: For each selected Actor:
         │
         ├─ 获取 StaticMeshComponent
         │  Actor->FindComponentByClass<UStaticMeshComponent>()
         │
         ├─ 获取 StaticMesh
         │  MeshComp->GetStaticMesh()
         │
         ▼
Step 4: ExtractMeshTriangles(MeshComp)
         │
         ├─ StaticMesh->GetRenderData()->LODResources[0]
         ├─ 读取 PositionVertexBuffer → 顶点位置数组
         ├─ 读取 IndexBuffer → 三角形索引数组
         ├─ 转换到世界坐标（MeshComp->GetComponentTransform()）
         ├─ 计算每个三角形的法线和面积
         │
         ▼ 输出：TArray<FStairTriangle>
         
Step 5: DetectStairSteps(Triangles)
         │
         ├─ 筛选 Normal.Z > 0.95 的三角形
         ├─ 按 Center.Z 排序
         ├─ Z 高度聚类（容差 2cm）
         ├─ 计算每级踏步面的 Height, Center, Area
         ├─ 过滤面积 < 100cm² 的噪声
         ├─ 识别平台（面积 > 平均值 × 2）
         │
         ▼ 输出：TArray<FStairStep>（按高度排序）
         
Step 6: ClassifyStairType(Steps)
         │
         ├─ 计算每级踏步面相对中心的 XY 角度
         ├─ 分析角度变化序列
         │
         ├─ 角度变化 < 10° → Straight
         ├─ 有平�� + 角度跳变 → LShaped  
         ├─ 角度持续变化 > 90° → Spiral
         │
         ▼ 输出：EStairType
         
Step 7: 根据类型分发
         │
         ├─ Straight → StraightStairGenerator(Steps)
         │   ├─ H = MaxStep.Z - MinStep.Z
         │   ├─ Direction = MinStep.Center → MaxStep.Center (XY)
         │   ├─ L = 水平距离
         │   ├─ θ = atan2(H, L)
         │   ├─ S = √(H² + L²)
         │   └─ 生成 1 个 Box: (S/2, W/2, Thickness/2), Pitch=-θ
         │
         ├─ LShaped → LShapedStairGenerator(Steps)
         │   ├─ 用平台分割 Steps 为多个 Flight
         │   ├─ 每个 Flight 独立按直梯算法处理
         │   └─ 生成 N 个 Box
         │
         └─ Spiral → SpiralStairGenerator(Steps)
             ├─ 估算 R, P, TotalAngle
             ├─ 分段（每 15° 一段）
             ├─ 每段计算起止点、方向、坡度
             └─ 生成 N 个 Box
         │
         ▼
Step 8: SpawnRampCollision() （每个 Box 调用一次）
         │
         ├─ World->SpawnActor()
         ├─ 添加 UBoxComponent
         ├─ 设置碰撞为 BlockAll（对 Pawn 阻挡）
         ├─ SetVisibility(false)
         ├─ 添加 Tag "StairRamp_Auto"
         │
         ▼ 输出：关卡中生成的不可见 Ramp Actor
```

---

## 七、可行性与风险评估

| 项目 | 直梯 | L形 | 旋转梯 | 建筑内嵌 |
|------|------|------|--------|---------|
| 自动检测 | ✅ 简单 | ⚠️ 中等 | ⚠️ 中等 | ❌ 需手动选区域 |
| 角度计算精度 | ✅ 精准 | ✅ 分段后精准 | ⚠️ 近似（分段越多越精准） | ✅ 精准 |
| 实现难度 | ⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| 提取 Mesh 三角形 | ⚠️ UE4.27 API 需验证 | 同左 | 同左 | 同左 |

### 风险详情

```
风险 1: UE4.27 Mesh 数据访问
  问题：从 StaticMesh 提取三角形数据的 API 在 Editor 模式下可能有限制
  缓解：优先使用 RenderData 路径，fallback 到 BodySetup
  验证方法：Phase 1 开始时先写一个测试函数提取三角形并打印数量

风险 2: BoundingBox 包含装饰元素
  问题：楼梯 Mesh 可能包含扶手、栏杆等装饰，影响 BoundingBox 精度
  缓解：使用踏步面检测而不是 BoundingBox 来确定楼梯范围
  备选：提供手动调整 Ramp 位置/旋转的功能

风险 3: 角色穿透 Ramp
  问题：如果 Ramp 太薄，高速移动的角色可能穿透
  缓解：默认厚度 5cm，提供可调参数

风险 4: 多个碰撞体重叠
  问题：L形和旋转楼梯的分段 Ramp 之间可能有缝隙或重叠
  缓解：相邻段之间有少量重叠（1-2cm），确保无缝隙
```

---

## 八、建议的开发顺序

```
Phase 1: 直梯 Ramp（选中 → 按钮 → 生成）
  目标：验证整个流程可行
  内容：
    - ExtractMeshTriangles()
    - DetectStairSteps()（基础版）
    - StraightStairGenerator()
    - SpawnRampCollision()
    - RemoveAllAutoRamps()
  预计：1-2 次 Agent 会话
         ↓
Phase 2: 建筑内嵌楼梯（选中建筑 → 自动检测楼梯区域 → 生成叠加 Ramp）
  目标：处理楼梯是建筑一部分的情况
  内容：
    - 复用 Phase 1 的 Ramp 生成逻辑
    - 增加楼梯区域自动检测
    - 不关闭原始碰撞
  预计：1 次 Agent 会话
         ↓
Phase 3: L形楼梯（自动分段）
  目标：支持有拐角和平台的楼梯
  内容：
    - ClassifyStairType()
    - 平台识别
    - 分段 Ramp 生成
  预计：1-2 次 Agent 会话
         ↓
Phase 4: 旋转楼梯（螺旋线拟合）
  目标：支持螺旋楼梯
  内容：
    - 旋转参数估算
    - 螺旋线分段算法
    - 多段小 Ramp 拼接
  预计：2 次 Agent 会话
```

---

## 九、UI 设计

### Editor Utility Widget 按钮

```
在现有的 FolderColorTools Widget 中追加，或新建 StairRampTools Widget：

┌─────────────────────────────┐
│  Stair Ramp Generator       │
│                             │
│  [Generate Ramp]            │  ← 为选中的楼梯生成 Ramp
│  [Remove All Ramps]         │  ← 删除所有自动生成的 Ramp
│                             │
│  Thickness: [5.0] cm        │  ← 可选：Ramp 厚度参数
│  Segment Angle: [15] °      │  ← 可选：旋转楼梯分段角度
│                             │
└─────────────────────────────┘
```

### 操作流程

```
1. 在场景中选中一个或多个楼梯 Actor
2. 运行 StairRampTools Widget
3. 点击 "Generate Ramp"
4. 查看 Output Log 确认生成结果
5. 运行游戏测试角色行走
6. 如不满意，点击 "Remove All Ramps" 清除，调整参数后重新生成
```

---

## 十、测试验证计划

```
测试 1: 直梯基础功能
  - 放置一个简单的直梯 Mesh
  - 选中 → Generate Ramp
  - 验证 Ramp 角度和位置正确
  - 运行游戏，角色能平滑走上去

测试 2: 多个直梯批量处理
  - 选中场景中 5+ 个直梯
  - 一次性 Generate
  - 所有梯子都有 Ramp

测试 3: 角色行走体验
  - 角色从底部跑到顶部，无卡顿
  - 角色从顶部跑到底部，无弹跳
  - AI 敌人能正常寻路通过

测试 4: Remove All Ramps
  - 清除所有 Ramp
  - 确认关卡中无残留

测试 5: 建筑内嵌楼梯
  - 选中含楼梯的建筑
  - 验证只在楼梯区域生成 Ramp
  - 建筑其他部分碰撞不受影响
```