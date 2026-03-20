# FlowVisualizer 插件开发文档 — Part 1：架构与背景

## 1. 项目背景

### 1.1 问题
UE4/UE5 项目中，C++ 和蓝图的运行时执行流程难以可视化。开发者需要在多个系统
（角色移动、相机、战斗、锁定等）之间追踪信号流转，传统的日志方式效率低下。

### 1.2 目标
开发一个**编辑器内可视化工具**，能够：
- 实时显示 C++ 和蓝图中埋点信号的流转
- 以**管线（Pipeline）+ 节点（Node）+ 边（Edge）** 的形式展示
- 支持多条管线切换（角色移动、相机、战斗等）
- 支持 Scope 层级嵌套（折叠/展开）
- 支持蓝图信号自动注册（无需预定义管线结构）
- 支持缩放、平移、热力图等可视化增强

### 1.3 技术选型
- **UE 插件类型**：Editor Module（仅编辑器加载）
- **UI 框架**：Slate（`SCompoundWidget`，自定义 `OnPaint`）
- **数据驱动**：`FFlowPipeline` / `FFlowNode` / `FFlowEdge` 结构体
- **事件系统**：`FFlowTracer` 单例广播 + Slate Widget 监听
- **注册表**：`FFlowPipelineRegistry` 单例管理所有管线

---

## 2. 插件文件结构

```
Plugins/FlowVisualizer/
├── FlowVisualizer.uplugin                    # 插件描述文件
├── Source/
│   ├── FlowVisualizer/                       # Runtime 模块（可在游戏中使用）
│   │   ├── Public/
│   │   │   ├── FlowVisualizerModule.h        # Runtime 模块声明
│   │   │   ├── FlowTracer.h                  # 信号追踪器（单例，广播事件）
│   │   │   ├── FlowPipelineTypes.h           # 数据结构定义
│   │   │   ├── FlowPipelineRegistry.h        # 管线注册表（单例）
│   │   │   └── FlowSignalBlueprintLibrary.h  # 蓝图函数库
│   │   ├── Private/
│   │   │   ├── FlowVisualizerModule.cpp
│   │   │   ├── FlowTracer.cpp
│   │   │   ├── FlowPipelineRegistry.cpp
│   │   │   └── FlowSignalBlueprintLibrary.cpp
│   │   └── FlowVisualizer.Build.cs
│   │
│   └── FlowVisualizerEditor/                 # Editor 模块（仅编辑器）
│       ├── Public/
│       │   ├── FlowVisualizerEditorModule.h   # Editor 模块声明
│       │   ├── SFlowVisualizerPanel.h         # 可视化面板 Widget
│       │   └── FlowVisualizerStyle.h          # 样式常量（颜色、字体）
│       ├── Private/
│       │   ├── FlowVisualizerEditorModule.cpp # 注册菜单、管线、窗口
│       │   ├── SFlowVisualizerPanel.cpp       # 核心绘制与交互逻辑
│       │   └── FlowVisualizerStyle.cpp
│       └── FlowVisualizerEditor.Build.cs
```

---

## 3. 核心数据结构

### 3.1 FlowPipelineTypes.h

```cpp
// 单个节点
struct FFlowNode
{
    FName NodeID;                    // 唯一标识，如 "PlayerInput"
    FText DisplayName;               // 显示名称
    FVector2D RelativePosition;      // 相对位置 (0.0~1.0)
};

// 单条边
struct FFlowEdge
{
    FName SourceNodeID;              // 起始节点
    FName TargetNodeID;              // 目标节点
};

// 一条完整管线
struct FFlowPipeline
{
    FName PipelineID;                // 管线ID，如 "CharacterMovement"
    FText DisplayName;               // 显示名称
    TArray<FFlowNode> Nodes;         // 节点列表
    TArray<FFlowEdge> Edges;         // 边列表
};
```

### 3.2 信号事件

```cpp
// FFlowTracer 广播的事件
struct FFlowSignalEvent
{
    FName NodeID;                    // 触发的节点
    FString Data;                    // 附加数据
    FString Scope;                   // 所属作用域
    double Timestamp;                // 时间戳
};
```

---

## 4. 架构流程图

```
┌─────────────────────────────────────────────────────────┐
│                    C++ 游戏代码                          │
│  FFlowTracer::Get().Signal("PlayerInput", "Key=W");     │
└───────────────────────┬─────────────────────────────────┘
                        │
┌───────────────────────┼─────────────────────────────────┐
│                    蓝图代码                               │
│  Flow Signal (NodeID="BP_Attack", Data="Combo=3")       │
│  Flow Scope Begin / End                                  │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│              FFlowTracer (单例)                           │
│  - Signal() → 广播 OnFlowEvent                           │
│  - 环形缓冲区存储最近 1024 条信号                          │
└───────────────────────┬─────────────────────────────────┘
                        │ OnFlowEvent.Broadcast()
                        ▼
┌─────────────────────────────────────────────────────────┐
│         SFlowVisualizerPanel (Slate Widget)              │
│  - OnFlowEventReceived() 接收信号                        │
│  - 更新 NodeLastActiveTime / EdgeLastActiveTime          │
│  - 自动注册未知节点到 Blueprint_Auto                      │
│  - Tick() 每帧更新动画衰减 + 频率统计                     │
│  - OnPaint() 绘制节点、边、箭头、频率标签                  │
└─────────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│          FFlowPipelineRegistry (单例)                     │
│  - RegisterPipeline() 注册管线                            │
│  - FindPipeline() 查询管线                                │
│  - AddNodeToPipeline() 动态添加节点                       │
│  - AddEdgeToPipeline() 动态添加边                         │
│  - OnRegistryChanged 广播 → 刷新下拉框                    │
└─────────────────────────────────────────────────────────┘
```

---

## 5. 功能清单

| 功能 | 实现方式 | Prompt 编号 |
|------|---------|-------------|
| 插件基础框架 | Editor Module + Runtime Module | P1 |
| 数据结构定义 | FFlowPipeline/Node/Edge | P2 |
| 管线注册表 | FFlowPipelineRegistry 单例 | P2 |
| 信号追踪器 | FFlowTracer 单例 + 环形缓冲区 | P3 |
| 67 个 C++ 埋点 | 5 条静态管线 | P4 |
| Slate 可视化面板 | SFlowVisualizerPanel + OnPaint | P5 |
| 实时动画 | Tick 衰减 + 脉冲颜色 | P6 |
| 蓝图函数库 | FlowSignalBlueprintLibrary | P7 |
| Scope 层级嵌套 | EnterScope/ExitScope + 面包屑 | P8 |
| 蓝图自动注册 | Blueprint_Auto 管线 + 时序推断边 | B-Auto |
| 画布缩放平移 | OnMouseWheel + 左键/中键拖拽 | B-Zoom/Pan |
| 热力图 | 频率统计 + 5 级颜色映射 | B-Heatmap |

---

## 6. 开发环境

- **引擎**: UE 4.27（代码兼容 UE5，需调整 .Build.cs）
- **IDE**: Visual Studio / Rider
- **辅助工具**: GitHub Copilot Agent 执行 Prompt
- **版本控制**: Git

---

*Part 2 将包含基础框架的完整 Prompt（P1-P6）*