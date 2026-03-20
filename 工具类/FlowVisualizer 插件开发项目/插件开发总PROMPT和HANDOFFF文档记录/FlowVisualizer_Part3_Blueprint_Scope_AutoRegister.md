# FlowVisualizer 插件开发文档 — Part 3：蓝图信号 + Scope 层级 + 自动注册

## 概述

Part 3 涵盖三大功能模块：
1. **P7**：蓝图函数库（FlowSignal / FlowScopeBegin / FlowScopeEnd）
2. **P8**：Scope 层级嵌套（EnterScope / ExitScope / 面包屑导航）
3. **B-Auto**：蓝图信号自动注册到 Blueprint_Auto 管线 + 时序推断边

---

## P7：蓝图函数库

### 任务背景
让蓝图开发者无需写 C++ 即可在蓝图中发出 Flow Signal，
同时支持 Scope Begin/End 来标记逻辑块的开始和结束。

### 实现思路
- 创建 `UFlowSignalBlueprintLibrary` 继承 `UBlueprintFunctionLibrary`
- 三个静态函数暴露给蓝图：
  - `FlowSignal(NodeID, Data, Scope)` — 发出普通信号
  - `FlowScopeBegin(ScopeName)` — 标记作用域开始
  - `FlowScopeEnd(ScopeName)` — 标记作用域结束
- 内部调用 `FFlowTracer::Get().Signal()`

### Prompt

```
【蓝图函数库】FlowSignalBlueprintLibrary — 蓝图可用的 Flow Signal

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizer/Public/FlowSignalBlueprintLibrary.h

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlowSignalBlueprintLibrary.generated.h"

UCLASS()
class FLOWVISUALIZER_API UFlowSignalBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * 发出一个 Flow Signal 信号
     * @param NodeID  节点唯一标识（如 "BP_DodgeStart"）
     * @param Data    附加数据（如 "Damage=50"）
     * @param Scope   所属作用域（如 "BP_Player/CombatLogic"）用于层级嵌套
     */
    UFUNCTION(BlueprintCallable, Category = "FlowVisualizer",
              meta = (DisplayName = "Flow Signal"))
    static void FlowSignal(
        const FString& NodeID,
        const FString& Data = TEXT(""),
        const FString& Scope = TEXT(""));

    /**
     * 标记一个 Scope 开始
     * @param ScopeName 作用域名称（如 "CombatLogic"）
     */
    UFUNCTION(BlueprintCallable, Category = "FlowVisualizer",
              meta = (DisplayName = "Flow Scope Begin"))
    static void FlowScopeBegin(const FString& ScopeName);

    /**
     * 标记一个 Scope 结束
     * @param ScopeName 作用域名称（如 "CombatLogic"）
     */
    UFUNCTION(BlueprintCallable, Category = "FlowVisualizer",
              meta = (DisplayName = "Flow Scope End"))
    static void FlowScopeEnd(const FString& ScopeName);
};

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizer/Private/FlowSignalBlueprintLibrary.cpp

#include "FlowSignalBlueprintLibrary.h"
#include "FlowTracer.h"

void UFlowSignalBlueprintLibrary::FlowSignal(
    const FString& NodeID,
    const FString& Data,
    const FString& Scope)
{
    if (NodeID.IsEmpty()) return;
    FFlowTracer::Get().Signal(FName(*NodeID), Data, Scope);
}

void UFlowSignalBlueprintLibrary::FlowScopeBegin(const FString& ScopeName)
{
    if (ScopeName.IsEmpty()) return;
    FString NodeID = FString::Printf(TEXT("ScopeBegin_%s"), *ScopeName);
    FFlowTracer::Get().Signal(
        FName(*NodeID),
        TEXT("ScopeBegin"),
        ScopeName);
}

void UFlowSignalBlueprintLibrary::FlowScopeEnd(const FString& ScopeName)
{
    if (ScopeName.IsEmpty()) return;
    FString NodeID = FString::Printf(TEXT("ScopeEnd_%s"), *ScopeName);
    FFlowTracer::Get().Signal(
        FName(*NodeID),
        TEXT("ScopeEnd"),
        ScopeName);
}

禁止执行任何终端/PowerShell 命令。
```

### 蓝图中的使用方式

```
Event BeginPlay
    ↓
Flow Signal
    NodeID: "BP_EnemySpawn"
    Data: "Type=Skeleton"
    Scope: ""（留空表示顶层）
    ↓
Flow Scope Begin
    ScopeName: "CombatLogic"
    ↓
Flow Signal
    NodeID: "BP_ComboCheck"
    Data: "Combo=1"
    Scope: "CombatLogic"
    ↓
Flow Signal
    NodeID: "BP_ComboHit"
    Data: "Damage=50"
    Scope: "CombatLogic"
    ↓
Flow Scope End
    ScopeName: "CombatLogic"
```

---

## P8：Scope 层级嵌套

### 任务背景
当管线节点过多时，需要将相关节点分组到 Scope 中。
用户点击 Scope 节点可以"进入"查看内部节点，右键返回上级。

### 实现思路
- 在 Panel 中维护 `CurrentScopeStack`（作用域栈）
- `IsScopeNode(NodeID)` 判断节点是否是 Scope 入口
- `EnterScope(ScopeName)` 推入栈，过滤只显示该 Scope 内的节点
- `ExitScope()` 弹出栈，返回上级
- `RebuildFilteredView()` 根据当前 Scope 重建显示列表
- OnPaint 中 Scope 节点显示 `>` 箭头，面包屑导航显示在顶部

### Prompt

```
【Scope 层级】EnterScope / ExitScope / 面包屑导航

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/SFlowVisualizerPanel.h

在已有成员变量区域添加：

    // ---- Scope 层级 ----
    /** 当前作用域栈 */
    TArray<FString> CurrentScopeStack;

    /** 当前 Scope 下的过滤后节点 */
    TArray<FFlowNode> FilteredNodes;

    /** 当前 Scope 下的过滤后边 */
    TArray<FFlowEdge> FilteredEdges;

在已有函数声明区域添加：

    /** 判断节点是否是 Scope 入口 */
    bool IsScopeNode(FName NodeID) const;

    /** 进入子 Scope */
    void EnterScope(const FString& ScopeName);

    /** 退出当前 Scope（返回上级） */
    void ExitScope();

    /** 根据当前 Scope 栈重建 FilteredNodes/FilteredEdges */
    void RebuildFilteredView();

不要删除任何已有声明。

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

### 添加 IsScopeNode 函数

bool SFlowVisualizerPanel::IsScopeNode(FName NodeID) const
{
    // 检查是否有子节点的 Scope 与该 NodeID 匹配
    const FFlowPipeline* Pipeline =
        FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
    if (!Pipeline) return false;

    FString ScopeName = NodeID.ToString();
    for (const FFlowNode& Node : Pipeline->Nodes)
    {
        // 检查信号历史中是否有该 Scope 的记录
        // 简化方案：检查是否有 ScopeBegin_ 前缀的节点
        FString CheckName = FString::Printf(TEXT("ScopeBegin_%s"), *ScopeName);
        if (Node.NodeID == FName(*CheckName))
            return true;
    }
    return false;
}

### 添加 EnterScope 函数

void SFlowVisualizerPanel::EnterScope(const FString& ScopeName)
{
    CurrentScopeStack.Add(ScopeName);
    RebuildFilteredView();
}

### 添加 ExitScope 函数

void SFlowVisualizerPanel::ExitScope()
{
    if (CurrentScopeStack.Num() > 0)
    {
        CurrentScopeStack.Pop();
        RebuildFilteredView();
    }
}

### 添加 RebuildFilteredView 函数

void SFlowVisualizerPanel::RebuildFilteredView()
{
    FilteredNodes.Empty();
    FilteredEdges.Empty();

    const FFlowPipeline* Pipeline =
        FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
    if (!Pipeline) return;

    if (CurrentScopeStack.Num() == 0)
    {
        // 顶层：显示所有节点
        FilteredNodes = Pipeline->Nodes;
        FilteredEdges = Pipeline->Edges;
        return;
    }

    FString CurrentScope = CurrentScopeStack.Last();

    // 过滤：只保留 Scope 匹配的信号节点
    TSet<FName> VisibleNodeIDs;
    for (const FFlowNode& Node : Pipeline->Nodes)
    {
        // 检查该节点的信号是否在当前 Scope 内
        // 需要查信号历史（简化：通过节点名前缀判断）
        FString NodeStr = Node.NodeID.ToString();

        // ScopeBegin_X / ScopeEnd_X 节点始终在其父级可见
        if (NodeStr.StartsWith(TEXT("ScopeBegin_")) ||
            NodeStr.StartsWith(TEXT("ScopeEnd_")))
        {
            continue; // Scope 标记节点不在子视图中显示
        }

        // TODO: 需要 FFlowTracer 记录每个节点的 Scope 信息
        // 当前简化方案：全部显示
        FilteredNodes.Add(Node);
        VisibleNodeIDs.Add(Node.NodeID);
    }

    // 过滤边
    for (const FFlowEdge& Edge : Pipeline->Edges)
    {
        if (VisibleNodeIDs.Contains(Edge.SourceNodeID) &&
            VisibleNodeIDs.Contains(Edge.TargetNodeID))
        {
            FilteredEdges.Add(Edge);
        }
    }
}

### 修改 OnMouseButtonDown — 添加 Scope 交互

在 OnMouseButtonDown 函数中：

FReply SFlowVisualizerPanel::OnMouseButtonDown(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    FVector2D LocalPos = MyGeometry.AbsoluteToLocal(
        MouseEvent.GetScreenSpacePosition());
    FVector2D PanelSize = MyGeometry.GetLocalSize();

    // 左键：检查 Scope 节点点击
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        const FFlowPipeline* Pipeline =
            FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
        if (Pipeline)
        {
            for (const FFlowNode& Node : Pipeline->Nodes)
            {
                FVector2D NodePixel = NodeRelativeToPixel(
                    Node.RelativePosition, PanelSize);
                float HitRadius = 60.0f;
                if (FVector2D::Distance(LocalPos, NodePixel) < HitRadius)
                {
                    if (IsScopeNode(Node.NodeID))
                    {
                        EnterScope(Node.NodeID.ToString());
                        return FReply::Handled();
                    }
                }
            }
        }
    }

    // 右键：返回上级 Scope
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (CurrentScopeStack.Num() > 0)
        {
            ExitScope();
            return FReply::Handled();
        }
    }

    return FReply::Unhandled();
}

### 修改 OnPaint — Scope 箭头和面包屑

在 OnPaint 的节点绘制循环中，节点名称绘制之后添加 Scope 箭头：

    // Scope 箭头指示
    if (IsScopeNode(Node.NodeID))
    {
        FSlateDrawElement::MakeText(
            OutDrawElements, LayerId + 3,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(NodePixelPos.X + 55.0f, NodePixelPos.Y - 8.0f),
                FVector2D(20.0f, 20.0f)),
            FText::FromString(TEXT(">")),
            FlowVisStyle::NodeFont(),
            ESlateDrawEffect::None,
            FlowVisStyle::ScopeArrow());
    }

在 OnPaint 的状态栏绘制之前，添加面包屑导航：

    // 面包屑导航
    if (CurrentScopeStack.Num() > 0)
    {
        FString BreadcrumbText = TEXT("< Back | Scope: ");
        for (int32 i = 0; i < CurrentScopeStack.Num(); i++)
        {
            if (i > 0) BreadcrumbText += TEXT(" / ");
            BreadcrumbText += CurrentScopeStack[i];
        }

        FSlateDrawElement::MakeText(
            OutDrawElements, LayerId + 4,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(10.0f, 30.0f),
                FVector2D(500.0f, 20.0f)),
            FText::FromString(BreadcrumbText),
            FlowVisStyle::NodeFont(),
            ESlateDrawEffect::None,
            FlowVisStyle::BreadcrumbText());
    }

只修改以上 2 个文件。
不要删除已有的绘制代码。
禁止执行任何终端/PowerShell 命令。
```

---

## B-Auto 系列：蓝图信号自动注册

### 任务背景
蓝图发出的 Flow Signal 不在任何预定义管线中。
需要自动将未知信号注册到 `Blueprint_Auto` 管线，
并根据时序推断节点之间的边。

### 实现思路
- 在 `OnFlowEventReceived` 中检查信号 NodeID 是否已注册
- 如果不在任何管线中，自动添加到 `Blueprint_Auto`
- 节点按 5 列网格自动排列
- 500ms 内连续触发的信号自动连边
- EditorModule 启动时预创建 `Blueprint_Auto` 空管线

### B-Auto-1 + B-Auto-2 + B-Auto-3（合并为一个完整 Prompt）

```
【自动注册】蓝图信号自动注册到 Blueprint_Auto 管线

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/SFlowVisualizerPanel.h

在已有成员变量区域添加：

    // ---- 自动注册 ----
    /** Blueprint_Auto 管线 ID */
    FName AutoPipelineID = FName(TEXT("Blueprint_Auto"));

    /** 自动注册节点计数（用于网格排列） */
    int32 AutoNodeCount = 0;

    /** 上一个自动注册的信号节点 ID（用于时序推断边） */
    FName LastAutoSignalNodeID = NAME_None;

    /** 上一个自动注册信号的时间戳 */
    double LastAutoSignalTime = 0.0;

    /** 自动连边的时间窗口（秒） */
    static constexpr double AutoEdgeTimeWindow = 0.5;

不要删除任何已有声明。

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/FlowVisualizerEditorModule.cpp

在 StartupModule() 的管线注册代码之后，添加 Blueprint_Auto 空管线注册：

    // Blueprint_Auto 管线（用于自动注册蓝图信号）
    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("Blueprint_Auto");
        P.DisplayName = FText::FromString(TEXT("Blueprint (Auto)"));
        // 节点和边为空，运行时动态添加
        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

## 文件 3: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

修改 OnFlowEventReceived 函数。在现有代码（NodeLastActiveTime 和
EdgeLastActiveTime 更新）之后，添加自动注册逻辑：

    // =======================================================
    // === 自动注册蓝图信号节点到 Blueprint_Auto ===
    // =======================================================
    {
        // 检查该 NodeID 是否在当前 Pipeline 中已注册
        bool bNodeExists = false;
        {
            const FFlowPipeline* CurrentPipeline =
                FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
            if (CurrentPipeline)
            {
                for (const FFlowNode& Node : CurrentPipeline->Nodes)
                {
                    if (Node.NodeID == Event.NodeID)
                    {
                        bNodeExists = true;
                        break;
                    }
                }
            }
        }

        if (!bNodeExists)
        {
            // 计算网格位置（5 列排列）
            const int32 Columns = 5;
            int32 Col = AutoNodeCount % Columns;
            int32 Row = AutoNodeCount / Columns;
            float X = 0.08f + (float)Col * 0.18f;
            float Y = 0.10f + (float)Row * 0.20f;

            // 添加节点
            FFlowPipelineRegistry::Get().AddNodeToPipeline(
                AutoPipelineID,
                Event.NodeID,
                FText::FromName(Event.NodeID),
                FVector2D(X, Y));

            AutoNodeCount++;

            // 时序推断边（500ms 内连续信号自动连边）
            if (LastAutoSignalNodeID != NAME_None &&
                LastAutoSignalNodeID != Event.NodeID)
            {
                double TimeDiff = Event.Timestamp - LastAutoSignalTime;
                if (TimeDiff >= 0.0 && TimeDiff <= AutoEdgeTimeWindow)
                {
                    FFlowPipelineRegistry::Get().AddEdgeToPipeline(
                        AutoPipelineID,
                        LastAutoSignalNodeID,
                        Event.NodeID);
                }
            }

            LastAutoSignalNodeID = Event.NodeID;
            LastAutoSignalTime = Event.Timestamp;
        }
        else
        {
            // 已存在的节点也要更新时序追踪（用于边推断）
            LastAutoSignalNodeID = Event.NodeID;
            LastAutoSignalTime = Event.Timestamp;
        }
    }

只修改以上 3 个文件。
不要删除已有的信号处理代码。
不要删除已有的管线注册代码。
禁止执行任何终端/PowerShell 命令。
```

### 自动注册的工作流程

```
蓝图 Flow Signal 触发（NodeID = "BP_DodgeStart"）
                │
                ▼
FFlowTracer::Signal() → 广播 OnFlowEvent
                │
                ▼
SFlowVisualizerPanel::OnFlowEventReceived()
                │
    ┌───────────┴───────────┐
    │ 检查 NodeID 是否      │
    │ 在当前管线中已注册     │
    └───────────┬───────────┘
                │
        ┌───────┴───────┐
        │               │
    已存在           不存在
        │               │
  更新时序追踪     添加到 Blueprint_Auto
        │           │
        │       计算网格位置 (5列)
        │           │
        │       AddNodeToPipeline()
        │           │
        │       500ms 内? → AddEdgeToPipeline()
        │           │
        └─────┬─────┘
              │
    更新 LastAutoSignalNodeID/Time
              │
              ▼
    OnRegistryChanged.Broadcast()
              │
              ▼
    RefreshPipelineList() → 下拉框刷新
```

### 自动网格排列示意

```
5 列排列，每列间距 0.18，每行间距 0.20：

Col:    0        1        2        3        4
Row 0: [Node1] [Node2] [Node3] [Node4] [Node5]
Row 1: [Node6] [Node7] [Node8] [Node9] [Node10]
Row 2: [Node11] ...

坐标计算：
X = 0.08 + Col * 0.18
Y = 0.10 + Row * 0.20
```

---

## ClearVisualization 函数

### 任务背景
在 UI 中添加 Pause / Clear 按钮。Clear 清除所有运行时状态。

### Prompt（合并在 Construct 修改中）

```
【功能】添加 Pause / Clear 按钮

## 修改 SFlowVisualizerPanel.h

添加成员变量：
    bool bIsPaused = false;

添加函数声明：
    void ClearVisualization();

## 修改 SFlowVisualizerPanel.cpp

### Construct 中添加按钮（ComboBox 右侧）

在 ComboBox 的 SHorizontalBox 末尾追加：

    + SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .HAlign(HAlign_Right)
    .Padding(4, 0)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2, 0)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("Pause")))
            .OnClicked_Lambda([this]()
            {
                bIsPaused = !bIsPaused;
                return FReply::Handled();
            })
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2, 0)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("Clear")))
            .OnClicked_Lambda([this]()
            {
                ClearVisualization();
                return FReply::Handled();
            })
        ]
    ]

### 添加 ClearVisualization 函数

void SFlowVisualizerPanel::ClearVisualization()
{
    NodeLastActiveTime.Empty();
    NodeLastData.Empty();
    EdgeLastActiveTime.Empty();
    TestSignalNodeIndex = 0;
    TestSignalAccumulator = 0.0;
    ActiveSignalCount = 0;
    LastSignalElapsed = 0.0;
}

### 修改 OnFlowEventReceived（暂停支持）

在函数最开头添加：

    if (bIsPaused) return;

禁止执行任何终端/PowerShell 命令。
```

---

## Part 3 总结

| Prompt | 功能 | 文件数 | 核心改动 |
|--------|------|--------|---------|
| P7 | 蓝图函数库 | 2（新建） | FlowSignal / ScopeBegin / ScopeEnd |
| P8 | Scope 层级 | 2（修改） | EnterScope / ExitScope / 面包屑 |
| B-Auto | 自动注册 | 3（修改） | Blueprint_Auto 管线 + 时序边 |
| Clear | Pause/Clear | 2（修改） | 暂停接收 + 清除状态 |

### 关键设计决策

1. **为什么 Blueprint_Auto 要在 EditorModule 中预注册？**
   - 确保下拉框中始终可见，即使还没有信号触发
   - 避免首次信号触发时的竞态问题

2. **为什么用 500ms 时间窗口推断边？**
   - 游戏逻辑通常在几帧内连续触发相关信号
   - 500ms ≈ 30帧（60FPS），足够覆盖一个逻辑链
   - 太长会产生误连，太短会漏掉

3. **为什么 AutoNodeCount 不持久化？**
   - 每次 PIE 运行都是新的会话
   - Clear 可以重置所有状态
   - 持久化会增加复杂度且没有实际收益

---

*Part 4 将包含可视化增强的 Prompt（缩放平移、热力图）*