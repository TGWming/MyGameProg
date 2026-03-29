# StairRamp Generator 插件 — 技术文档

> UE4.27 Editor 工具插件，用于在楼梯 Mesh 上自动生成 Spline 引导的斜面碰撞体（Ramp Collision），  
> 替代传统 Simple Collision，实现更精确的楼梯行走体验。

---

## 一、插件架构

```
soul.uproject
├── Source/
│   ├── soul/                    ← 运行时模块（不修改）
│   └── soulEditor/              ← 编辑器模块
│       ├── soulEditor.Build.cs
│       ├── Public/
│       │   └── StairRampGeneratorBFL.h
│       └── Private/
│           ├── soulEditorModule.cpp
│           └── StairRampGeneratorBFL.cpp
└── Content/
    └── blueprint/
        └── WBP_StairsRampTester.uasset  ← Editor Widget 蓝图
```

### 模块依赖

```
soulEditor.Build.cs:
  PublicDependencyModuleNames:
    - Core
    - CoreUObject
    - Engine
    - UnrealEd
    - InputCore
    - EditorScriptingUtilities
    - UMG
    - Blutility
```

---

## 二、核心类：UStairRampGeneratorBFL

继承自 `UBlueprintFunctionLibrary`，提供 3 个 `UFUNCTION(BlueprintCallable)` 静态函数：

### 2.1 CreateStairSpline

```
输入：WorldContextObject, StairActor
输出：AActor*（包含 SplineComponent 的 Actor）

功能：选中楼梯 Mesh → 生成一条 Spline 路径
```

**计算原理：**

```
                    ┌─── TopPoint (BBox 顶部中心)
                   /
                  /    ← Spline 路径
                 /
                /
 BottomPoint ──┘  (BBox 底部中心)

 1. 获取楼梯 Actor 的 BoundingBox
 2. BottomPoint = BBox 底面 XY 中心, Z = BBox.Min.Z
 3. TopPoint   = BBox 顶面 XY 中心, Z = BBox.Max.Z
 4. ActorLocation = StairActor->GetActorLocation()
 5. SpawnActor<AActor>（不设置 Name，UE4 自动命名）
 6. 创建 USplineComponent → SetRootComponent
 7. SplineActor->SetActorLocation(ActorLocation)
    （必须在 RootComponent 设置之后调用，否则位置丢失）
 8. Spline 控制点使用本地坐标：
      Point0 = BottomPoint - ActorLocation
      Point1 = TopPoint - ActorLocation
 9. 宽度 = Min(BBox.SizeX, BBox.SizeY)
10. Tags 存储：
      "StairWidth_XXX.X"      ← 宽度
      "StairActorName:XXX"    ← 楼梯 Actor 名字
11. SetActorLabel("StairSpline") ← 编辑器显示名
```

### 2.2 GenerateRampFromSpline

```
输入：WorldContextObject, SplineActor, RampWidth(0=自动),
      RampThickness(5.0), bDisableOriginalCollision, OriginalStairActor(可选)
输出：AActor*（包含所有 Ramp 碰撞段的父 Actor）

功能：沿 Spline 路径生成分段斜面碰撞体
```

**逐段宽度自动计算原理：**

```
俯视图 — L 形楼梯：

  SegDir₁ →          SegDir₂ ↑
  ┌──────────┐       ┌────┐
  │ 下层楼梯  │──────→│平台│
  │ 宽度 A   │       │宽B │
  └──────────┘       └──┬─┘
                        │ SegDir₃ ↑
                     ┌──┴─┐
                     │上层│
                     │宽C │
                     └────┘

每段独立计算宽度：
  1. 从 Spline Tag 读取 StairActorName
  2. 在关卡中 FindActor → 获取 StaticMeshComponent
  3. 从 LOD0 的 PositionVertexBuffer 提取所有顶点（世界坐标）
  4. 对每段 Spline：
     a. SegDir = (SegEnd - SegStart).Normalize()
     b. Right  = SegDir × UpVector
     c. 筛选顶点：Z 在 [MinZ-50, MaxZ+50] 范围内
                   前后投影在 [-50, SegLength+50] 范围内
     d. 投影到 Right 方向 → 取 Max - Min = 宽度
     e. Clamp(50, 2000)
```

**图解 — 顶点投影计算宽度：**

```
侧视图（某一段）：

      Right 方向 →
  ←─────────────────→
  |  · · · · · · ·  |    ← 楼梯顶点（世界坐标）
  |  · · · · · · ·  |
  ←─────────────────→
  MinRight       MaxRight

  SegmentWidth = MaxRight - MinRight
```

### 2.3 CreateRampSegment

```
输入：World, Parent, SegStart, SegEnd, Width, Thickness
输出：UBoxComponent*

功能：创建单��斜面碰撞段
```

**计算原理：**

```
侧视图（一个碰撞段）：

  SegStart ·─────────────────· SegEnd
            ╲                ╱
             ╲  BoxComponent╱
              ╲    (倾斜)   ╱
               ╲──────────╱

  1. Center = (SegStart + SegEnd) / 2
  2. Length = |SegEnd - SegStart|
  3. BoxExtent = (Length/2, Width/2, Thickness/2)
  4. Rotation = 从 X 轴旋转到 (SegEnd - SegStart) 方向
  5. CollisionProfile = "BlockAll"
```

**编辑器可视化：**

```cpp
BoxComp->ShapeColor = FColor(0, 255, 128, 255);  // 绿色线框
BoxComp->bIsEditorOnly = false;
#if WITH_EDITOR
BoxComp->bVisualizeComponent = true;  // 无需选中即可看到线框
#endif
```

---

## 三、蓝图连线方法

### Widget 蓝图：WBP_StairsRampTester

包含 3 个按钮：

### Button 0 — Create Stair Spline（创建 Spline）

```
连线流程：

  [Button_0 OnClicked]
       │
       ▼
  [Get Selected Level Actors] → [GET (a copy) Index=0]
       │                              │
       ▼                              ▼
  [CreateStairSpline]          → StairActor 引脚
       │
       ├── WorldContextObject ← [Self]
       │
       ▼
  [Print String] ← [Get Display Name] ← ReturnValue
```

### Button 1 — Generate Ramp From Spline（生成 Ramp）

```
连线流程：

  [Button_1 OnClicked]
       │
       ▼
  [Get Selected Level Actors] → [GET (a copy) Index=0]
       │                              │
       ▼                              ▼
  [GenerateRampFromSpline]     → SplineActor 引脚
       │
       ├── WorldContextObject ← [Self]
       ├── RampWidth = 0.0（自动从 Mesh 顶点计算）
       ├── RampThickness = 5.0
       ├── bDisableOriginalCollision = false
       ├── OriginalStairActor = None（留空！从 Tag 自动查找）
       │
       ▼
  [Print String] ← [Get Display Name] ← ReturnValue
```

### Button 2 — Delete Ramp（删除 Ramp，如需实现）

```
预留按钮，目前未连线。
可用于一键清除旧 Ramp 功能。
```

**重要：OriginalStairActor 引脚留空（None）**，  
因为楼梯 Actor 名字已存储在 Spline Tag 中，代码会自动查找。

---

## 四、使用流程

```
Step 1: 选中楼梯 Mesh Actor
Step 2: 点击 "Create Stair Spline" → 生成 Spline
Step 3: 手动调整 Spline 控制点
        - 直梯：2 点（底部 + 顶部）
        - L 形：3 点（底部 + 拐弯 + 顶部）
        - U 形：4 点
        - 螺旋：6-12 点（每段弧度 < 90°）
Step 4: 选中 Spline Actor
Step 5: 点击 "Generate Ramp From Spline" → 生成碰撞体
Step 6: 视口中检查绿色线框是否贴合楼梯
Step 7: PIE 测试行走手感
```

---

## 五、支持的楼梯类型

```
┌─────────┬──────────┬────────────────────────┐
│ 类型     │ 控制点数  │ 备注                    │
├─────────┼──────────┼────────────────────────┤
│ 直梯     │ 2        │ 自动生成即可             │
│ L 形     │ 3        │ 拐弯处手动加 1 点        │
│ U 形     │ 3-4      │ 两个拐弯各加 1 点        │
│ 螺旋     │ 6-12     │ 每段弧度建议 < 90°       │
│ 多层复合  │ N        │ 按实际路径铺设            │
└─────────┴──────────┴────────────────────────┘
```

---

## 六、Tag 数据格式

```
SplineActor->Tags 中存储的数据：

  [0] "StairWidth_554.3"          ← BBox 估算的楼梯宽度
  [1] "StairActorName:SM_stairs_L_shaped_2"  ← 楼梯 Actor 内部名

解析方式：
  Width:     Tag.StartsWith("StairWidth_")  → RightChop(11) → FCString::Atof
  ActorName: Tag.StartsWith("StairActorName:") → RightChop(15)
```

---

## 七、关键技术细节

### 7.1 SpawnActor 位置丢失问题

```
问题：SpawnActor<AActor>(Location, ...) 对纯 AActor 无效
原因：AActor 默认无 RootComponent，Location 无处存放
解决：SpawnActor 后创建 SplineComponent → SetRootComponent
      → SetActorLocation(ActorLocation)
```

### 7.2 重名崩溃问题

```
问题：SpawnParams.Name 设置固定名字，重复创建导致 Fatal Error
解决：不设置 SpawnParams.Name，UE4 自动生成唯一名
      用 SetActorLabel("StairSpline") 设置编辑器显示名
```

### 7.3 编辑器线框可视化

```
需要同时设置：
  ShapeColor        → 线框颜色
  bIsEditorOnly=false → 确保组件存在
  bVisualizeComponent=true（WITH_EDITOR） → 无需选中即可显示
```

### 7.4 Mesh 顶点提取

```
从 StaticMesh->RenderData->LODResources[0] 获取 LOD0 顶点
通过 GetComponentTransform() 转换到世界坐标
用于逐段宽度计算
```