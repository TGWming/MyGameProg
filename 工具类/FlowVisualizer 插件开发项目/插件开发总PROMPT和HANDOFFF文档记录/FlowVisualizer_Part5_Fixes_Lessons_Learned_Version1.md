# FlowVisualizer 插件开发文档 — Part 5：修复记录 + 踩坑总结 + Agent 使用经验

## 1. 完整修复记录

### 1.1 修复时间线

| 序号 | 问题 | 原因 | 修复方式 | 涉及文件 |
|------|------|------|---------|---------|
| F1 | EditorStyle 模块引用报错 | UE4.27 模块名差异 | .Build.cs 添加 EditorStyle 依赖 | FlowVisualizerEditor.Build.cs |
| F2 | `IMPLEMENT_MODULE` 链接错误 | Runtime 模块缺少 IMPLEMENT_MODULE | 添加宏到 FlowVisualizerModule.cpp | FlowVisualizerModule.cpp |
| F3 | Slate 菜单不显示 | MenuExtension 挂载点错误 | 改用 "WindowLayout" 或正确的扩展点 | FlowVisualizerEditorModule.cpp |
| F4 | FlowTracer 头文件找不到 | Editor 模块未引用 Runtime 模块 | .Build.cs 添加 FlowVisualizer 依赖 | FlowVisualizerEditor.Build.cs |
| F5 | Blueprint_Auto 下拉框不显示 | 未在 StartupModule 中预注册 | 添加空管线注册 | FlowVisualizerEditorModule.cpp |
| F6 | 蓝图 Flow Signal NodeID 为空 | 用户未在蓝图节点中填写 NodeID | 用户手动填写 "BP_EnemySpawn" | 蓝图编辑器 |
| F7 | 热力图 Prompt 误删 128 行代码 | 单个 Prompt 改动过多 | Git 还原 + 拆分为 6 个小 Prompt | SFlowVisualizerPanel.cpp |
| F8 | 热力图第二次尝试仍编译失败 | Agent 在 OnPaint 中插入代码破坏函数结构 | Git 还原 + 改为文件末尾追加策略 | SFlowVisualizerPanel.cpp |
| F9 | 编译错误：大量未声明标识符 | Agent 把新函数插入到 OnPaint 内部而非外部 | 诊断 + 还原 + 用更精确的插入位置描述 | SFlowVisualizerPanel.cpp |

---

### 1.2 F7 详细分析：热力图误删事件

**事故经过**：

```
第一次尝试（失败）：
├─ 一个 Prompt 要求 Agent 完成 6 处修改（3 个文件）
├─ Agent 在插入 UpdateNodeFrequencies 和 GetHeatmapColor 时
│  误删了 ClearVisualization 之前的 128 行代码
├─ 这 128 行包含了 OnPaint 的后半部分（节点绘制 + 状态栏）
└─ 用户通过截图发现 -128 行异常

紧急处理：
├─ Git 还原到上次编译成功的状态
├─ 诊断当前文件状态（哪些改动保留了、哪些被还原了）
└─ 重新拆分为 6 个独立 Prompt
```

**根本原因**：
Agent 的文本编辑能力在处理大文件（600+ 行）时不稳定，
尤其是在"找到某个位置然后插入"这种操作中，
容易把"插入"变成"替换"，导致大量代码丢失。

**解决策略**：
1. 新函数追加到文件末尾（不在已有函数之间插入）
2. 已有函数只做最小修改（加 1-2 行）
3. 每个 Prompt 只改一处

---

### 1.3 F8 详细分析：第二次热力图编译失败

**事故经过**：

```
第二次尝试（部分成功后失败）：
├─ H-1a（头文件声明）✅ 成功
├─ H-1b（添加两个新函数）— Agent 又出问题
│  ├─ 要求在 ClearVisualization 之前插入
│  ├─ Agent 这次没有误删，但插入位置不对
│  └─ 新函数被插入到了某个函数的内部
├─ 编译报错：大量 "未声明的标识符"
└─ 错误特征："{": 缺少函数标题

分析：
  "缺少函数标题" 说明代码块出现在了函数体内部
  而不是作为独立的��数定义
```

**最终解决**：
改用"文件末尾追加"策略，完全避免在已有代码中间插入。

---

### 1.4 诊断 Prompt 模板

每次出问题后使用的标准诊断模板：

```
【诊断】确认文件状态

读取 [文件路径]

只报告以下信息：
1. 文件总行数
2. [关键函数] 的起始行号和结束行号
3. [目标特征] 是否存在
4. [另一个特征] 是否存在
...

只读取和报告，不要修改任何文件。
禁止执行任何终端/PowerShell 命令。
```

**使用时机**：
- Agent 执行完 Prompt 后编译失败
- Agent 报告 diff 行数异常（如 -128）
- Git 还原后需要确认当前状态
- 多步 Prompt 中间需要确认前序步骤是否完成

---

## 2. Agent 使用经验总结

### 2.1 Prompt 编写黄金规则

```
┌─────────────────────────────────────────────────────────┐
│                   Prompt 黄金规则                        │
├─────────────────────────────────────────────────────────┤
│ 1. 一个 Prompt 只改一个文件的一处位置                     │
│ 2. 新函数追加到文件末尾，不在中间插入                     │
│ 3. 已有函数只做最小改动（1-3 行）                        │
│ 4. 明确说"不要删除"比说"请保留"更有效                    │
│ 5. 每次改完必须编译验证                                   │
│ 6. 末尾加安全约束（禁止删除、禁止终端命令）              │
│ 7. 大函数（200+ 行）只做追加插入，绝不做替换              │
│ 8. 出问题先诊断再修复，不要盲目重试                      │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Prompt 结构模板

```
【类型标签】简短描述

只修改 [完整文件路径]

只修改 [具体函数名] 函数。

在 [具体锚点描述（某行代码或某个注释之后）]，添加：

    [要添加的代码]

只添加以上代码。
不要修改其他函数。
不要删除任何已有代码。
禁止执行任何终端/PowerShell 命令。
```

### 2.3 风险等级分类

| 操作类型 | 风险 | 说明 |
|---------|------|------|
| 创建新文件 | 极低 | Agent 最擅长的操作 |
| 文件末尾追加函数 | 极低 | 不碰已有代码 |
| 函数内加 1-2 行 | 低 | 锚点明确时很安全 |
| 函数内插入���码块（5-15 行） | 中等 | 需要精确描述插入位置 |
| 替换函数的一部分 | 高 | Agent 可能误删前后代码 |
| 重写整个函数 | 很高 | 几乎必然丢失细节 |
| 一个 Prompt 改多个文件多处 | 极高 | 强烈不推荐 |

### 2.4 出错时的标准处理流程

```
编译失败
    │
    ▼
查看错误信息
    │
    ├─ 少量错误（1-3 个）──→ 针对性修复 Prompt
    │
    ├─ 大量错误（10+）──→ 检查 Agent diff 行数
    │       │
    │       ├─ 删除行数异常（-50+）──→ Git 还原
    │       │       │
    │       │       ▼
    │       │   诊断 Prompt（��认当前状态）
    │       │       │
    │       │       ▼
    │       │   拆分为更小的 Prompt 重新执行
    │       │
    │       └─ 删除行数正常 ──→ 读取文件定位错误
    │
    └─ 未声明标识符 ──→ 检查是否缺少 include 或函数定义在错误位置
```

### 2.5 Git 作为安全网

```
推荐的 Git 工作流：

1. 每个功能模块开始前 commit 一次（作为还原点）
2. 每个 Prompt 执行成功并编译通过后 commit
3. 出问题立即 git checkout 还原到上次成功状态
4. 不要在未 commit 的状态下连续执行多个 Prompt

实际操作：
  git add -A && git commit -m "P5: Panel 基础绘制完成"
  → 执行 P7 Prompt
  → 编译成功
  git add -A && git commit -m "P7: 蓝图函数库完成"
  → 执行 P8 Prompt
  → 编译失败
  git checkout -- .   ← 还原到 P7 完成状态
  → 修改 P8 Prompt 后重新执行
```

---

## 3. 性能考量

### 3.1 当前已知性能点

| 位置 | 操作 | 频率 | 影响 |
|------|------|------|------|
| OnFlowEventReceived | 自动注册检查所有节点 | 每个信号 | O(N) N=节点数 |
| OnFlowEventReceived | AddNodeToPipeline 触发 Broadcast | 每个新节点 | UI 刷新 |
| Tick | UpdateNodeFrequencies 遍历历史 | 每帧 | O(M) M=总历史记录数 |
| Tick | Invalidate 强制重绘 | 每帧 | 全量重绘 |
| OnPaint | 遍历所有节点和边绘制 | 每帧 | O(N+E) |

### 3.2 优化建议（未来可选）

```
优化 1：脏标记代替每帧 Invalidate
  当前：Tick 中无条件调用 Invalidate
  优化：只在有新信号或动画未衰减完时 Invalidate
  收益：空闲时 0 CPU 开销

优化 2：自动注册批量处理
  当前：每个 AddNode/AddEdge 都 Broadcast
  优化：收集一批后统一 Broadcast
  收益：高频注册场景（BeginPlay 一次注册 20 个节点）减少刷新次数

优化 3：频率统计抽样
  当前：每帧遍历所有节点历史
  优化：每 0.5 秒更新一次频率
  收益：减少 80% 的频率计算开销

优化 4：可视区域裁剪
  当前：OnPaint 绘制所有节点（包括画布外的）
  优化：只绘制 CanvasOffset + Zoom 范围内可见的节点
  收益：缩小状态时减少大量绘制调用
```

---

## 4. 后续扩展方向

### 4.1 短期（可用现有架构直接实现）

| 功能 | 实现方式 | 预计 Prompt 数 |
|------|---------|---------------|
| 节点名称颜色跟随热力图 | OnPaint 中 TextColor 改用 GetHeatmapColor | 1 |
| 双击节点显示信号详情弹窗 | 新建 SFlowNodeDetailPopup Widget | 2-3 |
| 导出信号日志到 CSV | 遍历 RingBuffer 写文件 | 1 |
| 搜索/高亮特定节点 | 添加搜索框 + 匹配高亮 | 2 |
| 节点拖拽重排 | OnMouseButtonDown + Move 修改 RelativePosition | 2 |

### 4.2 中期（需要扩展架构）

| 功能 | 实现方式 | 预计 Prompt 数 |
|------|---------|---------------|
| 信号回放（时间轴拖动） | 基于 RingBuffer 的时间切片回放 | 3-4 |
| 多 PIE 实例区分 | 给 Signal 添加 WorldContext 参数 | 2-3 |
| 蓝图节点自动发现 | 扫描 UBlueprint 资产中的 FlowSignal 节点 | 3-4 |
| 管线配置持久化 | 保存/加载管线布局到 JSON | 2-3 |

### 4.3 长期（需要较大重构）

| 功能 | 说明 |
|------|------|
| 网络同步可视化 | 显示 Server/Client 之间的信号同步 |
| 性能 Profiler 集成 | 与 Unreal Insights 联动 |
| 自定义节点 Widget | 替代纯文字绘制，支持图标/进度条 |
| 多人协作标注 | 在节点上添加注释，团队共享 |

---

## 5. 完整文件清单（最终状态）

```
Plugins/FlowVisualizer/
├── FlowVisualizer.uplugin
├── Source/
│   ├── FlowVisualizer/                          # Runtime 模块
│   │   ├── FlowVisualizer.Build.cs
│   │   ├── Public/
│   │   │   ├── FlowVisualizerModule.h
│   │   │   ├── FlowTracer.h                     # 信号追踪器
│   │   │   ├── FlowPipelineTypes.h              # 数据结构
│   │   │   ├── FlowPipelineRegistry.h           # 管线注册表
│   │   │   └── FlowSignalBlueprintLibrary.h     # 蓝图函数库
│   │   └── Private/
│   │       ├── FlowVisualizerModule.cpp
│   │       ├── FlowTracer.cpp
│   │       ├── FlowPipelineRegistry.cpp
│   │       └── FlowSignalBlueprintLibrary.cpp
│   │
│   └── FlowVisualizerEditor/                    # Editor 模块
│       ├── FlowVisualizerEditor.Build.cs
│       ├── Public/
│       │   ├── FlowVisualizerEditorModule.h
│       │   ├── SFlowVisualizerPanel.h            # 636→670+ 行
│       │   └── FlowVisualizerStyle.h             # 样式常量
│       └── Private/
│           ├── FlowVisualizerEditorModule.cpp     # 含 6 条管线注册
│           ├── SFlowVisualizerPanel.cpp           # 核心文件 670+ 行
│           └── FlowVisualizerStyle.cpp
```

### SFlowVisualizerPanel.cpp 函数清单（最终状态）

| 函数 | 行数（约） | 功能 |
|------|-----------|------|
| Construct | 50 | UI 构建 + 委托订阅 |
| GeneratePipelineComboItem | 5 | 下拉框项生成 |
| RefreshPipelineList | 30 | 刷��管线列表 |
| OnPipelineSelected | 10 | 切换管线回调 |
| GetSelectedPipelineText | 5 | 下拉框显示文字 |
| NodeRelativeToPixel | 5 | 坐标变换（含缩放偏移） |
| OnFlowEventReceived | 50 | 信号接收 + 自动注册 |
| Tick | 30 | 动画更新 + 频率统计 |
| OnPaint | 260 | 全量绘制（背景→边→节点→频率→面包屑→状态栏） |
| OnMouseButtonDown | 40 | 左键平移/Scope + 中键平移 + 右键返回 |
| OnMouseButtonUp | 10 | 释放拖拽 |
| OnMouseMove | 10 | 拖拽更新 |
| OnMouseWheel | 15 | 滚轮缩放 |
| EnterScope | 5 | 进入子 Scope |
| ExitScope | 5 | 退出 Scope |
| RebuildFilteredView | 30 | 重建过滤视图 |
| IsScopeNode | 15 | 判断 Scope 节点 |
| ClearVisualization | 15 | 清除所有运行时状态 |
| UpdateNodeFrequencies | 25 | 滑动窗口频率统计 |
| GetHeatmapColor | 10 | 频率→颜色映射 |

---

## 6. SFlowVisualizerPanel.h 成员变量清单（最终状态）

```cpp
// ---- 管线选择 ----
FName CurrentPipelineID;
TArray<TSharedPtr<FName>> PipelineIDList;
TSharedPtr<FName> SelectedPipelineItem;
TSharedPtr<SComboBox<TSharedPtr<FName>>> PipelineComboBox;

// ---- 信号可视化状态 ----
TMap<FName, double> NodeLastActiveTime;
TMap<FName, FString> NodeLastData;
TMap<FName, double> EdgeLastActiveTime;

// ---- 频率统计（热力图） ----
TMap<FName, TArray<double>> NodeSignalHistory;
TMap<FName, float> NodeFrequency;
static constexpr float FrequencyWindowSeconds = 3.0f;

// ---- 自动注册 ----
FName AutoPipelineID = FName(TEXT("Blueprint_Auto"));
int32 AutoNodeCount = 0;
FName LastAutoSignalNodeID = NAME_None;
double LastAutoSignalTime = 0.0;
static constexpr double AutoEdgeTimeWindow = 0.5;

// ---- Scope 层级 ----
TArray<FString> CurrentScopeStack;
TArray<FFlowNode> FilteredNodes;
TArray<FFlowEdge> FilteredEdges;

// ---- 画布缩放平移 ----
float CanvasZoom = 1.0f;
FVector2D CanvasOffset = FVector2D::ZeroVector;
bool bIsPanning = false;
FVector2D PanStartMousePos = FVector2D::ZeroVector;
FVector2D PanStartOffset = FVector2D::ZeroVector;
static constexpr float MinZoom = 0.2f;
static constexpr float MaxZoom = 3.0f;

// ---- 委托句柄 ----
FDelegateHandle RegistryChangedHandle;
FDelegateHandle FlowEventHandle;

// ---- 统计/调试 ----
int32 TestSignalNodeIndex = 0;
double TestSignalAccumulator = 0.0;
int32 ActiveSignalCount = 0;
double LastSignalElapsed = 0.0;
float CachedFPS = 0.0f;
bool bIsPaused = false;
```

---

## 7. 致谢与版本记录

```
FlowVisualizer Plugin v1.0

开发方式：Human + AI Agent 协作
  - 人类：架构设计、Prompt 编写、编译验证、Bug 定位、Git 管理
  - Agent：代码生成、文件编辑、诊断报告

开发周期：1 个对话会话
总 Prompt 数：约 25 个（含修复和诊断）
总文件数：12 个
总代码量：约 2000 行（含注释）

核心功能：
  ✅ C++ 信号追踪（FFlowTracer）
  ✅ 管线注册表（FFlowPipelineRegistry）
  ✅ 5 条预定义管线 + 67 节点
  ✅ Slate ���时可视化面板
  ✅ 蓝图函数库（FlowSignal/ScopeBegin/ScopeEnd）
  ✅ 蓝图信号自动注册（Blueprint_Auto）
  ✅ 画布缩放平移
  ✅ 热力图频率统计
  ✅ Scope 层级嵌套
  ✅ Pause / Clear 控制
```

---

*文档完 — 共 5 个 Part*