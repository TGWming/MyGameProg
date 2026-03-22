# FlowVisualizer — FMOD Bridge + Auto-Capture Hand Off 文档

> **日期**：2026-03-22  
> **用户**：minglikesrunning-png  
> **仓库**：TGWming/MyGameProg  
> **上下文**：FlowVisualizer UE 4.27 编辑器插件，终端风格数据流可视化

---

## 一、今晚完成的工作

### FMOD Bridge（PF.1 ~ PF.10）— Runtime 侧

| Task | 任务名称 | 状态 | 说明 |
|------|---------|------|------|
| PF.1 | Runtime Build.cs 添加 FMOD 可选依赖宏 | ✅ | `WITH_FMOD_STUDIO=0/1` PublicDefinitions |
| PF.2 | FlowFMODBridge.h 空壳头文件 | ✅ | 条件编译双分支，FMOD/no-op |
| PF.3 | FlowFMODBridge.cpp 空壳 + Get() 单例 | ✅ | Meyer's Singleton |
| PF.4 | MonitorEventInstance 注册逻辑 | ✅ | 幂等 + FScopeLock + setCallback |
| PF.5 | EventCallback 静态回调主体 | ✅ | if/else if 链 + DESTROYED 清理 |
| PF.6 | Callback 中 AsyncTask 线程派发 | ✅ | 全部 case → GameThread → FLOW_SIGNAL |
| PF.7 | 手动信号辅助函数 | ✅ | BankLoad/BankUnload/GlobalParameter |
| PF.8 | Style 追加 5 个 FMOD 颜色常量 | ✅ | Error(红)/Warning(橙)/Recovery(绿)/Lifecycle(蓝)/Timeline(金) |
| PF.9 | DrawNode FMOD 节点颜色覆盖 | ✅ | 5 种语义颜色映射 |
| PF.10 | 注册 FMOD Audio System Pipeline | ✅ | 14 节点 13 边，3 行布局 |

### FMOD Auto-Capture（PA.1 ~ PA.8）— Editor 侧

| Task | 任务名称 | 状态 | 说明 |
|------|---------|------|------|
| PA.1 | Editor Build.cs 添加 FMOD 可选依赖 | ✅ | PrivateDefinitions + PrivateDependencyModuleNames |
| PA.2 | FMODAutoCapture.h 空壳头文件 | ✅ | Stub(空)/Full(完整) 双分支 |
| PA.3 | FMODAutoCapture.cpp 空壳 + PIE 绑定 | ✅ | BeginPIE/EndPIE 回调注册 |
| PA.4 | StartCapture — FMOD System 获取 + 全局 Callback | ✅ | IFMODStudioModule → setCallback(POSTUPDATE) + 200ms 定时器 |
| PA.5 | SystemCallback — EventInstance 创建拦截 | ✅ | Bank→Desc→Instance 遍历 + GKnownInstances 去重 + MonitorEventInstance |
| PA.6 | StopCapture — PIE 结束清理 | ✅ | 注销回调 + 停止定时器 + 清理状态 |
| PA.7 | PollGlobalParameters — 全局参数轮询 | ✅ | 快照对比 + FLOW_SIGNAL + 首次不触发洪水 |
| PA.8 | EditorModule 初始化/关闭调用 | ✅ | StartupModule/ShutdownModule 中 #if WITH_FMOD_STUDIO 保护 |

### 修补任务（PX.1）

| Task | 任务名称 | 状态 | 说明 |
|------|---------|------|------|
| PX.1 | EventCallback 补全 4 个缺失回调分支 | ✅ | Restored/SoundPlay/SoundStop/Restarted |

---

## 二、审计结果

两次审计已完成（全功能审计 + 修补验证审计）。

### 全功能审计（PF + PA）

- **14/18** 完全通过
- **4/18** 轻微偏差（均不影响功能）
- **0/18** 缺���
- **主链路完整畅通** ✅

### 修补验证审计（PX.1）

- **4/4** 修补分支正确实现
- **0/14** 死节点（全部节点有信号源）
- **14/14** 颜色覆盖完整
- **现有代码无损** ✅
- **评级：🟢 全部达标**

### 残留的非阻断性轻微偏差

| # | 内容 | 影响 | 需要修？ |
|---|------|------|---------|
| 1 | EventCallback 用 if/else if 替代 switch | 零影响，风格差异 | 🔵 不需要 |
| 2 | Editor PrivateDefinitions 与 Runtime PublicDefinitions 冗余 | 零影响，同目录探测结果一致 | 🔵 不需要 |
| 3 | PollGlobalParameters `Val=%.3f` vs SignalGlobalParameter `Value=%.4f` 格式不一致 | 仅显示差异 | 🔵 可选 |

---

## 三、当前代码状态

### 文件清单

**Runtime 模块（FlowVisualizerRuntime）：**

```
Plugins/FlowVisualizer/Source/FlowVisualizerRuntime/
├── FlowVisualizerRuntime.Build.cs          ← PF.1 FMOD 宏
├── Public/
│   ├── FlowPipelineTypes.h                 ← P2.1 (已有)
│   ├── FlowTracer.h                        ← P2.2 (已有)
│   ├── FlowTracerMacros.h                  ← P2.3 (已有)
│   ├── FlowPipelineRegistry.h              ← P2.4 (已有)
│   └── FlowFMODBridge.h                    ← PF.2 新建
├── Private/
│   ├── FlowTracer.cpp                      ← P2.2 (已有)
│   ├── FlowPipelineRegistry.cpp            ← P2.4 (已有)
│   ├── FlowVisualizerRuntimeModule.cpp     ← P1.3 (已有)
│   └── FlowFMODBridge.cpp                  ← PF.3~PF.7, PX.1 新建+修补
```

**Editor 模块（FlowVisualizerEditor）：**

```
Plugins/FlowVisualizer/Source/FlowVisualizerEditor/
├── FlowVisualizerEditor.Build.cs           ← PA.1 FMOD 宏
├── Public/
│   ├── SFlowVisualizerPanel.h              ← P3.2+ (已有)
│   ├── FlowVisualizerStyle.h               ← P5.3, PF.8 (已有+追加颜色)
│   └── FMODAutoCapture.h                   ← PA.2 新建
├── Private/
│   ├── SFlowVisualizerPanel.cpp            ← P3.2+, PF.9 (已有+追加颜色覆盖)
│   ├── FlowVisualizerEditorModule.cpp      ← P1.4+, PF.10, PA.8 (已有+追加)
│   ├── FlowVisualizerStyle.cpp             ← P5.3 (已有)
│   └── FMODAutoCapture.cpp                 ← PA.3~PA.7 新建
```

### 关键架构

```
PIE ▶ 启动
  │
  ▼
FFMODAutoCapture::OnBeginPIE()
  → StartCapture()
    → IFMODStudioModule::GetStudioSystem(Runtime)
    → System->setCallback(POSTUPDATE)
    → 启动 200ms PollGlobalParameters 定时器
  │
  ▼ (FMOD Update 周期)
FMODSystemCallback (POSTUPDATE)
  → 遍历 Banks → Descriptions → Instances
  → GKnownInstances 去重
  → AsyncTask(GameThread) → FFlowFMODBridge::MonitorEventInstance()
  │
  ▼ (EventInstance 状态变化)
FFlowFMODBridge::EventCallback
  → 12 种回调类型全覆盖：
    CREATED / STARTED / STOPPED / DESTROYED
    START_FAILED / REAL_TO_VIRTUAL / VIRTUAL_TO_REAL
    SOUND_PLAYED / SOUND_STOPPED / RESTARTED
    TIMELINE_MARKER / TIMELINE_BEAT
  → AsyncTask(GameThread) → FLOW_SIGNAL("FMOD_xxx", ...)
  │
  ▼
FFlowTracer::Get().Signal() → OnFlowEvent 广播
  │
  ▼
SFlowVisualizerPanel::OnFlowEventReceived()
  → NodeLastActiveTime 更新
  → Tick → Invalidate → OnPaint
  → FMOD 节点语义颜色覆盖（红/橙/绿/蓝/金）
  → 面板显示 🎵
  │
  ▼
PIE ⏹ 结束
  → FFMODAutoCapture::OnEndPIE()
  → StopCapture() → 注销回调 + 停止定时器 + 清理
```

---

## 四、FMOD Pipeline 拓扑（14 节点 13 边）

```
Row 1 (y=0.15) — 正常生命周期流：
  [Created] → [Started] → [SoundPlay] → [Marker]
                                       → [Beat]

Row 2 (y=0.45) — 停止/重启流：
  [Restarted] → (回到 Started)
  [SoundStop] → [Stopped] → [Destroyed]
                           → (回到 Restarted)

Row 3 (y=0.75) — 错误/警告/工具：
  [StartFailed]   ← (从 Started)
  [Virtualized] → [Restored] → (回到 SoundPlay)
  [BankLoad]      (手动信号)
  [GlobalParam]   (自动轮询 + 手动信号)
```

### 节点颜色语义

| 颜色 | 含义 | 对应节点 |
|------|------|---------|
| 🔴 亮红 FMODError | 播放失败 | StartFailed |
| 🟠 橙色 FMODWarning | 被虚拟化（Voice Stolen） | Virtualized |
| 🟢 绿色 FMODRecovery | 从虚拟化恢复 | Restored |
| 🔵 浅蓝 FMODLifecycle | 正常生命周期 | Created/Started/Stopped/Destroyed/SoundPlay/SoundStop/Restarted/BankLoad/GlobalParam |
| 🟡 金色 FMODTimeline | 时间线事件 | Marker/Beat |

---

## 五、Git 提交建议

```bash
# 如果还没提交 PF+PA：
git add -A
git commit -m "feat: FMOD Bridge (PF.1~10) + Auto-Capture (PA.1~8) - zero-config FMOD visualization"

# 如果 PF+PA 已提交，只提交 PX.1 修补：
git add -A
git commit -m "fix: PX.1 补全 EventCallback 4个缺失回调 (Restored/SoundPlay/SoundStop/Restarted)"
```

---

## 六、下周可继续的方向

| 方向 | 说明 | 难度 |
|------|------|------|
| 🎮 **PIE 实测验证** | 运行游戏 → 打开面板 → 选 FMOD Pipeline → 观察自动亮灯 | 低 |
| 🎨 **节点名称颜色跟随热力图** | 高频节点文字颜色跟热力图走，1 个小 Prompt | 低 |
| 📦 **蓝图埋点实战** | 在角色/敌人蓝图中添加 Flow Signal 节点 | 中 |
| 🔀 **Scope 分组测试** | 用 Flow Scope Begin/End 包裹连招逻辑 | 中 |
| 📊 **其他新功能扩展** | 待讨论 | 待定 |

---

## 七、重要提醒

1. **本文档保存到本地**（防止上下文丢失）
2. **当前代码建议 `git commit`** 作为稳定还原点
3. **下周首次启动建议先 PIE 实测**，验证自动采集链路端到端畅通
4. **无 FMOD 的项目也能编译通过** — `WITH_FMOD_STUDIO=0` 保证零影响

---

## 八、完整项目进��总览

| Phase | 内容 | 任务数 | 状态 |
|-------|------|--------|------|
| P1 | 插件骨架 + 双模块创建 | 4 | ✅ |
| P2 | 数据采集核心（Tracer/Signal/Registry） | 4 | ✅ |
| P3 | Editor 窗口 + Panel 空壳 | 2 | ✅ |
| P4 | 内置管线 + 顶栏 + 底栏 | 3 | ✅ |
| P5 | 终端风格渲染（节点/边/Style） | 3 | ✅ |
| P6 | 事件驱动（Delegate/高亮/数据标签/Tick） | 4 | ✅ |
| P7 | 光点动画 + 状态条动态更新 | 2 | ✅ |
| P8 | Soul 项目埋点 + 管线注册 | 4 | ✅ |
| P9 | 窗口记忆 + Blueprint_Auto + README | 4 | ✅ |
| PF | FMOD Bridge（Runtime 侧） | 10 | ✅ |
| PA | FMOD Auto-Capture（Editor 侧） | 8 | ✅ |
| PX | 审计修补 | 1 | ✅ |
| **合计** | | **49** | **全部完成 ✅** |

---

晚安，下周继续！🔧🎵