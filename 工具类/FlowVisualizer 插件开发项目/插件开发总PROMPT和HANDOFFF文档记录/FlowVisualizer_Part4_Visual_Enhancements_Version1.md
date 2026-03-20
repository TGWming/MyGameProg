# FlowVisualizer 插件开发文档 — Part 4：可视化增强（缩放平移 + 热力图）

## 概述

Part 4 涵盖两个可视化增强模块：
1. **B-Zoom**：画布缩放（滚轮以鼠标为中心缩放）
2. **B-Pan**：画布平移（左键/中键拖拽）
3. **B-Heatmap**：节点信号频率统计 + 热力图颜色

---

## B-Zoom：画布缩放

### 任务背景
Blueprint_Auto 管线自动注册后节点数量可能很多（36+），
底部节点被窗口截断无法查看。需要滚轮缩放功能。

### 实现思路
- `CanvasZoom` 控制缩放倍率（0.2x ~ 3.0x）
- `CanvasOffset` 控制平移偏移
- `OnMouseWheel` 以鼠标位置为中心进行缩放
- `NodeRelativeToPixel` 应用缩放和偏移变换
- 所有绘制自动通过 `NodeRelativeToPixel` 适配缩放

### Prompt

```
【功能】SFlowVisualizerPanel 添加画布缩放和平移

## 文件 1：Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/SFlowVisualizerPanel.h

在已有成员变量区域添加：

    /** 画布缩放倍率，1.0 = 默认 */
    float CanvasZoom = 1.0f;

    /** 画布平移偏移（像素） */
    FVector2D CanvasOffset = FVector2D::ZeroVector;

    /** 是否正在拖拽平移 */
    bool bIsPanning = false;

    /** 拖拽起始位置 */
    FVector2D PanStartMousePos = FVector2D::ZeroVector;

    /** 拖拽起始时的 CanvasOffset */
    FVector2D PanStartOffset = FVector2D::ZeroVector;

    /** 缩放范围 */
    static constexpr float MinZoom = 0.2f;
    static constexpr float MaxZoom = 3.0f;

在已有函数声明区域添加（如果还没声明的话）：

    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

不要删除已有声明。

## 文件 2：Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

### 添加 OnMouseWheel（滚轮缩放）：

FReply SFlowVisualizerPanel::OnMouseWheel(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    float Delta = MouseEvent.GetWheelDelta();
    float OldZoom = CanvasZoom;

    // 缩放步进 10%
    CanvasZoom = FMath::Clamp(CanvasZoom + Delta * 0.1f, MinZoom, MaxZoom);

    // 以鼠标位置为中心缩放
    if (OldZoom != CanvasZoom)
    {
        FVector2D LocalMouse = MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition());
        float ZoomRatio = CanvasZoom / OldZoom;
        CanvasOffset = LocalMouse - (LocalMouse - CanvasOffset) * ZoomRatio;
    }

    return FReply::Handled();
}

### 修改 OnMouseButtonDown（添加中键拖拽平移）：

在现有 OnMouseButtonDown 函数的开头（左键逻辑之前）添加：

    // 中键开始平移
    if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
    {
        bIsPanning = true;
        PanStartMousePos = MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition());
        PanStartOffset = CanvasOffset;
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }

### 添加 OnMouseButtonUp：

FReply SFlowVisualizerPanel::OnMouseButtonUp(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton && bIsPanning)
    {
        bIsPanning = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}

### 添加 OnMouseMove：

FReply SFlowVisualizerPanel::OnMouseMove(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    if (bIsPanning)
    {
        FVector2D CurrentMouse = MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition());
        CanvasOffset = PanStartOffset + (CurrentMouse - PanStartMousePos);
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

### 修改 NodeRelativeToPixel 函数：

把原来的实现改为应用缩放和偏移：

FVector2D SFlowVisualizerPanel::NodeRelativeToPixel(
    FVector2D RelativePos, FVector2D InPanelSize) const
{
    FVector2D BasePos = RelativePos * InPanelSize;
    return BasePos * CanvasZoom + CanvasOffset;
}

### 修改 OnMouseButtonDown 中的点击检测：

在 Scope 节点点击检测中，将 HitRadius 改为随缩放变化：

    float HitRadius = 60.0f * CanvasZoom;

## 安全约束
- 只修改以上 2 个文件
- 不要删除 OnPaint 中的任何绘制逻辑
- 不要删除已有的 OnMouseButtonDown 逻辑（左键和右键）
- 禁止执行任何终端/PowerShell 命令
```

### 缩放数学原理

```
以鼠标位置为中心缩放的关键公式：

设：
  M = 鼠标在 Widget 局部坐标中的位置
  O = 当前 CanvasOffset
  Z_old = 旧缩放
  Z_new = 新缩放
  R = Z_new / Z_old

则新的 CanvasOffset：
  O_new = M - (M - O) * R

直觉理解：
  (M - O) 是鼠标相对于画布原点的偏移
  乘以 R 缩放这个偏移
  再从鼠标位置减回去，保持鼠标下方的内容不动
```

---

## B-Pan：左键拖拽平移

### 任务背景
中键平移已经实现，但用户更习惯像蓝图编辑器那样用左键拖拽空白区域平移。

### 实现思路
- 左键点击时先检查是否命中 Scope 节点
- 如果没有命中任何节点，则开始平移
- 复用已有的 `bIsPanning` / `PanStartMousePos` / `PanStartOffset`
- `OnMouseButtonUp` 同时响应左键和中键释放

### Prompt

```
【功能】左键拖拽平移画布（类似蓝图编辑器）

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

### 修改 OnMouseButtonDown 函数

把左键部分整体替换为：

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FVector2D LocalPos = MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition());
        FVector2D PanelSize = MyGeometry.GetLocalSize();

        // 先检查是否点击了 Scope 节点
        const FFlowPipeline* Pipeline =
            FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
        if (Pipeline)
        {
            for (const FFlowNode& Node : Pipeline->Nodes)
            {
                FVector2D NodePixel = NodeRelativeToPixel(
                    Node.RelativePosition, PanelSize);
                float HitRadius = 60.0f * CanvasZoom;
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

        // 没有命中节点，开始左键平移
        bIsPanning = true;
        PanStartMousePos = LocalPos;
        PanStartOffset = CanvasOffset;
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }

### 修改 OnMouseButtonUp 函数

把整个函数替换为：

FReply SFlowVisualizerPanel::OnMouseButtonUp(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    if (bIsPanning &&
        (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton ||
         MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton))
    {
        bIsPanning = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}

只修改 OnMouseButtonDown 和 OnMouseButtonUp 函数。
不要修改 OnPaint。
不要修改其他文件。
禁止执行任何终端/PowerShell 命令。
```

### 最终交互操作表

```
┌──────────────────┬──────────────────────��─────┐
│ 操作             │ 效果                        │
├──────────────────┼────────────────────────────┤
│ 左键拖拽空白     │ 平移画布                    │
│ 左键点击 Scope   │ 进入子层级                  │
│ 中键拖拽         │ 平移画布（保留）            │
│ 滚轮上           │ 放大（以鼠标为中心）        │
│ 滚轮下           │ 缩小                        │
│ 右键             │ 返回上级 Scope              │
└──────────────────┴────────────────────────────┘
```

---

## B-Heatmap：频率统计 + 热力图颜色

### 任务背景
节点数量多时，难以直观判断哪些节点触发频繁。
热力图通过颜色和频率数字直观展示节点的"忙碌程度"。

### 实现思路
- 3 秒滑动窗口统计每个节点的触发频率
- 5 级颜色映射：灰 → 绿 → 黄 → 橙 → 红
- 每个节点下方显示频率数字（如 "12.5/s"）
- `UpdateNodeFrequencies()` 在 Tick 中每帧调用
- `GetHeatmapColor()` 静态方法返回对应颜色

### 开发经验教训

> ⚠️ **重要**：热力图功能曾因一个 Prompt 改动过多（3 个文件 6 处修改）
> 导致 Agent 误删 128 行代码（OnPaint 函数被破坏）。
> 最终拆成 6 个小 Prompt 逐步完成，每步只改一处。
> 
> **教训**：对于 OnPaint 这样的大函数（200+ 行），
> 只做"追加插入"，绝不做"替换"。

### Prompt 拆分策略

热力图功能拆成 6 个独立 Prompt，按顺序执行：

| 步骤 | Prompt | 改动范围 | 风险 |
|------|--------|---------|------|
| 1 | H-1a | .h 添加声明 | 极低 |
| 2 | H-1b | .cpp 末尾追加 2 个新函数 | 极低 |
| 3 | H-1c | OnFlowEventReceived 加 1 行 | 极低 |
| 4 | H-1d | Tick 加 1 行 | 极低 |
| 5 | H-1e | OnPaint 插入 1 个代码块 | 中等 |
| 6 | H-1f | ClearVisualization 加 2 行 | 极低 |

### H-1a：头文件添加声明

```
【功能】SFlowVisualizerPanel 添加频率统计声明

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/SFlowVisualizerPanel.h

在已有成员变量区域（NodeLastActiveTime 附近）添加：

    /** 每个节点的信号触发时间戳历史（用于计算频率） */
    TMap<FName, TArray<double>> NodeSignalHistory;

    /** 每个节点的当前频率（次/秒） */
    TMap<FName, float> NodeFrequency;

    /** 频率统计的滑动窗口大小（秒） */
    static constexpr float FrequencyWindowSeconds = 3.0f;

在已有函数声明区域添加：

    /** 更新所有节点的频率统计 */
    void UpdateNodeFrequencies();

    /** 根据频率返回热力图颜色 */
    static FLinearColor GetHeatmapColor(float Frequency);

不要删除任何已有声明和变量。
只修改这一个 .h 文件。
禁止执行任何终端/PowerShell 命令。
```

### H-1b：添加两个新函数（文件末尾追加）

```
【功能】添加 UpdateNodeFrequencies 和 GetHeatmapColor

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

在文件的最末尾（最后一个函数的 } 之后）追加以下两个函数。
不要修改文件中任何已有代码，只在文件末尾追加。

// ============================================================
// 热力图：频率统计
// ============================================================
void SFlowVisualizerPanel::UpdateNodeFrequencies()
{
    double Now = FPlatformTime::Seconds();
    double WindowStart = Now - FrequencyWindowSeconds;

    for (auto& Pair : NodeSignalHistory)
    {
        TArray<double>& History = Pair.Value;

        int32 RemoveCount = 0;
        for (int32 i = 0; i < History.Num(); i++)
        {
            if (History[i] < WindowStart)
                RemoveCount++;
            else
                break;
        }
        if (RemoveCount > 0)
        {
            History.RemoveAt(0, RemoveCount);
        }

        float Freq = (float)History.Num() / FrequencyWindowSeconds;
        NodeFrequency.Add(Pair.Key, Freq);
    }
}

// ============================================================
// 热力图：频率 → 颜色映射
// ============================================================
FLinearColor SFlowVisualizerPanel::GetHeatmapColor(float Frequency)
{
    if (Frequency < 0.1f)
        return FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
    if (Frequency < 10.0f)
        return FLinearColor(0.1f, 0.8f, 0.2f, 1.0f);
    if (Frequency < 30.0f)
        return FLinearColor(0.9f, 0.9f, 0.1f, 1.0f);
    if (Frequency < 60.0f)
        return FLinearColor(0.9f, 0.5f, 0.1f, 1.0f);
    return FLinearColor(1.0f, 0.15f, 0.1f, 1.0f);
}

只在文件末尾追加，不要触碰文件中任何已有行。
禁止执行任何终端/PowerShell 命令。
```

### H-1c：OnFlowEventReceived 记录时间戳

```
【功能】OnFlowEventReceived 记录信号时间戳

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

只修改 OnFlowEventReceived 函数。

在写入 NodeLastActiveTime 的那行代码之后，添加一行：

    NodeSignalHistory.FindOrAdd(Event.NodeID).Add(Event.Timestamp);

只添加这一行。
不要修改其他函数。
不要删除任何已有代码。
禁止执行任何终端/PowerShell 命令。
```

### H-1d：Tick 调用频率更新

```
【功能】Tick 中调用 UpdateNodeFrequencies

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

只修改 Tick 函数。

在 Tick 函数的末尾、Invalidate 调用之前，添加一行：

    UpdateNodeFrequencies();

只添加这一行。
不要修改其他函数。
不要删除任何已有代码。
禁止执行任何终端/PowerShell 命令。
```

### H-1e：OnPaint 绘制频率和热力图

```
【功能】OnPaint 中添加���率标签

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

只修改 OnPaint 函数。

先读取 OnPaint 函数，找到遍历节点的 for 循环中，
IsScopeNode 判断的 if 块结束的那个 } 所在行。

在那个 } 之后、节点 for 循环的下一次迭代之前，
只添加以下代码块：

        // 频率标签
        {
            float Freq = 0.0f;
            if (const float* FoundFreq = NodeFrequency.Find(Node.NodeID))
            {
                Freq = *FoundFreq;
            }
            FString FreqText = FString::Printf(TEXT("%.1f/s"), Freq);
            FLinearColor HeatColor = GetHeatmapColor(Freq);
            FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId + 1,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(NodePixelPos.X - 30.0f, NodePixelPos.Y + 14.0f),
                    FVector2D(80.0f, 16.0f)),
                FText::FromString(FreqText),
                FlowVisStyle::DataValueFont(),
                ESlateDrawEffect::None,
                HeatColor);
        }

重要约束：
- 变量名 Node, NodePixelPos, OutDrawElements, LayerId, AllottedGeometry
  使用 OnPaint 中实际的变量名，如果不同请替换
- 只插入这一个代码块
- 不要删除、移动或修改 OnPaint 中任何已有的代码行
- 不要修改其他函数
- 禁止执行任何终端/PowerShell 命令
```

### H-1f：ClearVisualization 清除频率数据

```
【功能】ClearVisualization 清除频率数据

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

只修改 ClearVisualization 函数。

在已有的 Empty() 调用之后，添加两行：

    NodeSignalHistory.Empty();
    NodeFrequency.Empty();

只添加这两行。
不要修改其他函数。
不要删除任何已有代码。
禁止执行任何终端/PowerShell 命令。
```

### 频率统计原理

```
滑动窗口算法（3 秒窗口）：

时间轴：
  |------- 3秒窗口 -------|
  |-旧信号（移除）-|--有效信号--|   现在
  t-5  t-4  t-3   t-2  t-1    t

NodeSignalHistory["PlayerInput"] = [t-2.1, t-1.5, t-0.8, t-0.2]
                                     ↑ 4个有效信号

频率 = 4 / 3.0 = 1.33/s → 绿色

每帧 Tick 中调用 UpdateNodeFrequencies()：
1. 遍历所有节点的 History
2. 移除 WindowStart 之前的时间戳
3. 计算 Freq = History.Num() / WindowSeconds
4. 写入 NodeFrequency Map
```

### 热力图颜色映射

```
频率(次/秒)    颜色          RGB                     含义
─────────────────────────────────────────────────────────
< 0.1         灰色          (0.4, 0.4, 0.4)        休眠/未触发
0.1 - 10      绿色          (0.1, 0.8, 0.2)        正常
10 - 30       黄色          (0.9, 0.9, 0.1)        较忙
30 - 60       橙色          (0.9, 0.5, 0.1)        很忙
60+           红色          (1.0, 0.15, 0.1)       过热/可能性能问题

实际应用场景：
- Tick 中的信号 → 通常 30-60/s → 橙色/红色
- 输入事件信号 → 通常 1-10/s → 绿色
- 一次性信号（BeginPlay）→ 触发后 3 秒衰减到灰色
```

---

## Part 4 总结

| Prompt | 功能 | 文件改动 | 策略 |
|--------|------|---------|------|
| B-Zoom | 滚轮缩放 | .h + .cpp | 单次 Prompt |
| B-Pan | 左键/中键拖拽 | .cpp | 单次 Prompt |
| H-1a | 热力图声明 | .h | 拆分步骤 1/6 |
| H-1b | 频率+颜色函数 | .cpp 末尾追加 | 拆分步骤 2/6 |
| H-1c | 记录时间戳 | .cpp 加 1 行 | 拆分步骤 3/6 |
| H-1d | Tick 调用 | .cpp 加 1 行 | 拆分步骤 4/6 |
| H-1e | OnPaint 绘制 | .cpp 插入代码块 | 拆分步骤 5/6 |
| H-1f | Clear 清除 | .cpp 加 2 行 | 拆分步骤 6/6 |

### 关键经验

1. **大函数（OnPaint 200+ 行）只做插入，不做替换**
2. **文件末尾追加新函数最安全**（H-1b 策略）
3. **每个 Prompt 只改 1 处，编译验证后再进行下一步**
4. **缩放变换只需改 NodeRelativeToPixel，所有绘制自动适配**

---

*Part 5 将包含修复记录 + 踩坑总结 + Agent 使用经验*