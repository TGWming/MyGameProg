# StairRamp Generator — 完整 Prompt 步骤

> 按顺序执行以下 Prompt 可从零构建完整插件。  
> 已排除错误路径，每个 Step 都是验证通过的最终版本。

---

## 通用安全约束（每个 Prompt 都适用）

```
安全约束：
  - 不修改 Source/soul/ 下的文件
  - 不修改 .uproject / .Target.cs / .Build.cs
  - 不删除 Intermediate 或 Binaries
  - 编译配置：Development Editor + Win64
  - 编译失败只报告，不自行修复
```

---

## Step 1：创建 Editor 模块 soulEditor

```
任务：创建 UE4.27 Editor 模块 soulEditor

需要创建的文件：

1. Source/soulEditor/soulEditor.Build.cs
   - Type = Editor
   - PublicDependencyModuleNames:
     Core, CoreUObject, Engine, UnrealEd, InputCore,
     EditorScriptingUtilities, UMG, Blutility

2. Source/soulEditor/Private/soulEditorModule.cpp
   - 实现 IModuleInterface
   - IMPLEMENT_MODULE(FsoulEditorModule, soulEditor)

注意：
  - 不修改 soul.uproject 的 Modules 数组
    （Editor 模块通过 .Target.cs 加载，不需要写入 .uproject）
  - 确保 soulEditorTarget.cs 中包含 "soulEditor"
```

---

## Step 2：创建 StairRampGeneratorBFL 头文件

```
任务：创建 Source/soulEditor/Public/StairRampGeneratorBFL.h

类：UStairRampGeneratorBFL : public UBlueprintFunctionLibrary

UFUNCTION(BlueprintCallable) 静态函数：

1. CreateStairSpline
   参数：UObject* WorldContextObject, AActor* StairActor
   返回：AActor*
   说明：从楼梯 Mesh Actor 生成 Spline 路径

2. GenerateRampFromSpline
   参数：UObject* WorldContextObject, AActor* SplineActor,
         float RampWidth = 0.0f, float RampThickness = 5.0f,
         bool bDisableOriginalCollision = false,
         AActor* OriginalStairActor = nullptr
   返回：AActor*
   说明：沿 Spline 生成分段斜面碰撞体

3. CreateRampSegment (private helper)
   参数：UWorld* World, AActor* ParentActor,
         FVector SegStart, FVector SegEnd,
         float Width, float Thickness
   返回：UBoxComponent*

辅助函数（如需要）：
  - ExtractMeshTriangles
  - DetectStairSteps
```

---

## Step 3：实现 CreateStairSpline

```
任务：实现 CreateStairSpline 函数

实现要点：
1. 从 StairActor 获取 BoundingBox
2. 计算 BottomPoint（底部中心）和 TopPoint（顶部中心）
3. ActorLocation = StairActor->GetActorLocation()
4. SpawnActor<AActor>（不设置 SpawnParams.Name，让 UE4 自动命名）
5. 创建 USplineComponent，ClearSplinePoints，添加 2 个点
6. RegisterComponent → SetRootComponent → AddInstanceComponent
7. SplineActor->SetActorLocation(ActorLocation)
   （必须在 SetRootComponent 之后调用）
8. Spline 点使用本地坐标：BottomPoint - ActorLocation, TopPoint - ActorLocation
9. 宽度 = Min(BBox.SizeX, BBox.SizeY)，下限 50，默认 200
10. Tags 存储宽度和楼梯 Actor 名字：
    "StairWidth_XXX.X"
    "StairActorName:XXX"
11. SetActorLabel("StairSpline")（WITH_EDITOR）

#include：
  - Components/SplineComponent.h
  - Engine/StaticMeshActor.h
  - EngineUtils.h
```

---

## Step 4：实现 GenerateRampFromSpline

```
任务：实现 GenerateRampFromSpline 函数

实现要点：

A. 获取 SplineComponent：
   SplineActor->FindComponentByClass<USplineComponent>()

B. 从 Tag 读取楼梯 Actor + 提取 Mesh 顶点：
   1. 遍历 SplineActor->Tags 查找 "StairActorName:" 前缀
   2. �� TActorIterator 在关卡中查找该 Actor
   3. 获取 StaticMeshComponent → StaticMesh
   4. 从 RenderData->LODResources[0]->VertexBuffers.PositionVertexBuffer
      提取所有顶点，转换为世界坐标
   5. bool bUsePerSegmentWidth = (MeshVertices.Num() > 0)

C. 从 Tag 读取默认宽度：
   查找 "StairWidth_" 前缀 → RightChop(11) → Atof

D. Spawn 父 Actor：
   与 CreateStairSpline 相同方式（不设 Name）
   SetActorLabel("StairRamp")

E. 遍历 Spline 每段：
   for (int32 i = 0; i < NumPoints - 1; i++)
   {
       FVector SegStart = GetLocationAtSplinePoint(i, World);
       FVector SegEnd = GetLocationAtSplinePoint(i+1, World);

       // 逐段宽度计算
       float SegmentWidth = RampWidth;
       if (bUsePerSegmentWidth)
       {
           FVector SegDir = (SegEnd - SegStart).GetSafeNormal();
           FVector Right = CrossProduct(SegDir, UpVector).GetSafeNormal();
           float MinZ/MaxZ = 该段高度范围 ± 50 容差
           float SegLength = Dist(SegStart, SegEnd)
           
           遍历 MeshVertices：
             筛选 Z 范围 + 前后投影范围
             投影到 Right 方向 → 取 Min/Max
           
           if (ValidCount > 10)
               SegmentWidth = Clamp(MaxRight - MinRight, 50, 2000)
           else
               SegmentWidth = RampWidth（fallback）
       }

       CreateRampSegment(World, RampActor, SegStart, SegEnd,
                         SegmentWidth, RampThickness);
   }
```

---

## Step 5：实现 CreateRampSegment

```
任务：实现 CreateRampSegment 函数

实现要点：
1. Center = (SegStart + SegEnd) / 2
2. Length = |SegEnd - SegStart|
3. Dir = (SegEnd - SegStart).GetSafeNormal()
4. Rotation = FRotationMatrix::MakeFromX(Dir).Rotator()
5. NewObject<UBoxComponent>(ParentActor)
6. BoxExtent = (Length/2, Width/2, Thickness/2)
7. SetWorldLocation(Center)
8. SetWorldRotation(Rotation)
9. SetCollisionProfileName("BlockAll")
10. SetGenerateOverlapEvents(false)

编辑器可视化：
   BoxComp->ShapeColor = FColor(0, 255, 128, 255);
   BoxComp->bIsEditorOnly = false;
   #if WITH_EDITOR
   BoxComp->bVisualizeComponent = true;
   #endif

11. RegisterComponent → AttachToComponent(Root)
12. ParentActor->AddInstanceComponent(BoxComp)
```

---

## Step 6：创建 Editor Widget 蓝图

```
手动操作（非代码 Prompt）：

1. Content Browser → 右键 → Editor Utilities → Editor Widget
   命名：WBP_StairsRampTester

2. 添加 3 个 Button：
   - Button_0: "Create Stair Spline"
   - Button_1: "Generate Ramp From Spline"
   - Button_2: "Delete Ramp"（预留）

3. Button_0 OnClicked 连线：
   → Get Selected Level Actors
   → GET (a copy) Index=0
   → CreateStairSpline(Self, SelectedActor)
   → Print String(GetDisplayName(ReturnValue))

4. Button_1 OnClicked 连线：
   → Get Selected Level Actors
   → GET (a copy) Index=0
   → GenerateRampFromSpline(Self, SelectedActor, 0.0, 5.0, false, None)
   → Print String(GetDisplayName(ReturnValue))

注意：OriginalStairActor 引脚留空 None
```

---

## 技术修复备忘

### 修复 1：SpawnActor 位置丢失
```
症状：Spline 生成在 (0,0,0) 世界原点
原因：SpawnActor<AActor> 的 Location 参数对无 RootComponent 的 Actor 无效
修复：创建 SplineComponent → SetRootComponent 后调用 SetActorLocation()
教训：纯 AActor 在 SpawnActor 时没有 RootComponent 承载 Transform
```

### 修复 2：重名崩溃
```
症状：重复点击 "Create Stair Spline" 导致 Fatal Error 闪退
原因：SpawnParams.Name 指定固定名字，UE4 不允许同名 Actor
修复：不设置 SpawnParams.Name → UE4 自动生成唯一名
      用 SetActorLabel() 设置人类可读名称
教训：MakeUniqueObjectName 在某些边��情况下也可能冲突
```

### 修复 3：逐段宽度不生效
```
症状：所有段使用统一宽度 554.3，日志无 "auto width" 输出
原因：RampWidth=0 → 先从 Tag 读取 StairWidth=554.3
      → 到逐段计算时条件 (RampWidth <= 0) 不满足 → 跳过
修复：用 bool bUsePerSegmentWidth 标记是否有顶点数据
      有顶点就优先逐段计算，不受 RampWidth 值影响
教训：注意代码执行顺序，变量可能在中途被修改
```

### 修复 4：编辑器可视化不显示
```
症状：不选中 Actor 时看不到碰撞体线框
原因：仅设置 ShapeColor 不够，需要同时启用 bVisualizeComponent
修复：三合一设置：
      ShapeColor = 绿色
      bIsEditorOnly = false
      bVisualizeComponent = true（WITH_EDITOR 守卫）
教训：UE4 编辑器可视化需要多个属性配合
```