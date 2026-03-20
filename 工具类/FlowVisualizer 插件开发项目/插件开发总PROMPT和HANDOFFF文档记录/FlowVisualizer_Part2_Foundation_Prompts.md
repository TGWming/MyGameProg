# FlowVisualizer 插件开发文档 — Part 2：基础框架 Prompt（P1-P6）

## 开发策略说明

### Prompt 设计原则
1. **每个 Prompt 只改少量文件**（理想 1-2 个）
2. **明确列出文件路径和具体代码**
3. **末尾加安全约束**（禁止删除、禁止执行终端命令）
4. **先编译验证再进入下一步**

### 执行流程
```
编写 Prompt → Agent 执行 → 编译验证 → 通过则下一步 / 失败则修复
```

---

## P1：插件骨架 + Editor Module

### 任务背景
创建 UE 插件的基础文件结构，包含 Runtime 模块和 Editor 模块。
Editor 模块在编辑器菜单注册窗口入口。

### 实现思路
- `.uplugin` 声明两个模块：Runtime + Editor
- Editor 模块继承 `IModuleInterface`
- 在 `Window` 菜单下注册 "Flow Visualizer" 入口
- 点击后 `SpawnTab` 创建 Slate 窗口

### Prompt

```
【创建插件骨架】FlowVisualizer — Editor 工具窗口

## 文件 1: Plugins/FlowVisualizer/FlowVisualizer.uplugin

{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0",
    "FriendlyName": "Flow Visualizer",
    "Description": "Runtime flow signal visualizer for debugging gameplay logic.",
    "Category": "Developer Tools",
    "CreatedBy": "Dev",
    "CanContainContent": false,
    "Modules": [
        {
            "Name": "FlowVisualizer",
            "Type": "Runtime",
            "LoadingPhase": "Default"
        },
        {
            "Name": "FlowVisualizerEditor",
            "Type": "Editor",
            "LoadingPhase": "PostEngineInit"
        }
    ]
}

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizer/FlowVisualizer.Build.cs

using UnrealBuildTool;

public class FlowVisualizer : ModuleRules
{
    public FlowVisualizer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine"
        });
    }
}

## 文件 3: Plugins/FlowVisualizer/Source/FlowVisualizer/Public/FlowVisualizerModule.h

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFlowVisualizerModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

## 文件 4: Plugins/FlowVisualizer/Source/FlowVisualizer/Private/FlowVisualizerModule.cpp

#include "FlowVisualizerModule.h"

#define LOCTEXT_NAMESPACE "FFlowVisualizerModule"

void FFlowVisualizerModule::StartupModule() {}
void FFlowVisualizerModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlowVisualizerModule, FlowVisualizer)

## 文件 5: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/FlowVisualizerEditor.Build.cs

using UnrealBuildTool;

public class FlowVisualizerEditor : ModuleRules
{
    public FlowVisualizerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "EditorStyle",
            "InputCore",
            "LevelEditor",
            "FlowVisualizer"
        });
    }
}

## 文件 6: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/FlowVisualizerEditorModule.h

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFlowVisualizerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedRef<class SDockTab> OnSpawnTab(const class FSpawnTabArgs& Args);
    static const FName FlowVisualizerTabName;
};

## 文件 7: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/FlowVisualizerEditorModule.cpp

#include "FlowVisualizerEditorModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SFlowVisualizerPanel.h"

#define LOCTEXT_NAMESPACE "FFlowVisualizerEditorModule"

const FName FFlowVisualizerEditorModule::FlowVisualizerTabName(TEXT("FlowVisualizer"));

void FFlowVisualizerEditorModule::StartupModule()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        FlowVisualizerTabName,
        FOnSpawnTab::CreateRaw(this, &FFlowVisualizerEditorModule::OnSpawnTab))
        .SetDisplayName(LOCTEXT("TabTitle", "Flow Visualizer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    FLevelEditorModule& LevelEditorModule =
        FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension(
        "Window",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateLambda(
            [](FMenuBuilder& Builder)
            {
                Builder.AddMenuEntry(
                    LOCTEXT("OpenFlowVisualizer", "Flow Visualizer"),
                    LOCTEXT("OpenFlowVisualizerTooltip",
                            "Open the runtime flow visualizer panel"),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([]()
                    {
                        FGlobalTabmanager::Get()->TryInvokeTab(
                            FName("FlowVisualizer"));
                    })));
            }));

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FFlowVisualizerEditorModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FlowVisualizerTabName);
}

TSharedRef<SDockTab> FFlowVisualizerEditorModule::OnSpawnTab(
    const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SFlowVisualizerPanel)
        ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlowVisualizerEditorModule, FlowVisualizerEditor)

禁止执行任何终端/PowerShell 命令。
```

---

## P2：数据结构 + 管线注册表

### 任务背景
定义管线、节点、边的数据结构，以及全局注册表单例。

### 实现思路
- `FFlowPipeline` 包含 `TArray<FFlowNode>` 和 `TArray<FFlowEdge>`
- `FFlowPipelineRegistry` 单例用 `TMap<FName, FFlowPipeline>` 存储
- 提供 `RegisterPipeline` / `FindPipeline` / `AddNodeToPipeline` / `AddEdgeToPipeline`
- `OnRegistryChanged` 多播委托通知 UI 刷新

### Prompt

```
【数据层】FlowPipelineTypes + FlowPipelineRegistry

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizer/Public/FlowPipelineTypes.h

#pragma once
#include "CoreMinimal.h"

/** 流程图中的单个节点 */
struct FLOWVISUALIZER_API FFlowNode
{
    FName  NodeID;
    FText  DisplayName;
    FVector2D RelativePosition;   // 0.0~1.0 归一化坐标

    FFlowNode() : RelativePosition(FVector2D::ZeroVector) {}
    FFlowNode(FName InID, FText InName, FVector2D InPos)
        : NodeID(InID), DisplayName(InName), RelativePosition(InPos) {}
};

/** 节点之间的有向边 */
struct FLOWVISUALIZER_API FFlowEdge
{
    FName SourceNodeID;
    FName TargetNodeID;

    FFlowEdge() {}
    FFlowEdge(FName InSrc, FName InDst) : SourceNodeID(InSrc), TargetNodeID(InDst) {}
};

/** 一条完整管线 */
struct FLOWVISUALIZER_API FFlowPipeline
{
    FName  PipelineID;
    FText  DisplayName;
    TArray<FFlowNode> Nodes;
    TArray<FFlowEdge> Edges;
};

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizer/Public/FlowPipelineRegistry.h

#pragma once
#include "CoreMinimal.h"
#include "FlowPipelineTypes.h"

DECLARE_MULTICAST_DELEGATE(FOnRegistryChanged);

class FLOWVISUALIZER_API FFlowPipelineRegistry
{
public:
    static FFlowPipelineRegistry& Get();

    /** 注册一条完整管线 */
    void RegisterPipeline(const FFlowPipeline& Pipeline);

    /** 查找管线（返回 nullptr 表示不存在） */
    const FFlowPipeline* FindPipeline(FName PipelineID) const;

    /** 获取所有管线 ID */
    TArray<FName> GetAllPipelineIDs() const;

    /** 动态添加节点到指定管线（不存在则自动创建管线） */
    void AddNodeToPipeline(FName PipelineID, FName NodeID,
                           FText DisplayName, FVector2D Position);

    /** 动态添加边到指定管线 */
    void AddEdgeToPipeline(FName PipelineID, FName SourceID, FName TargetID);

    /** 注册表变化委托 */
    FOnRegistryChanged OnRegistryChanged;

private:
    TMap<FName, FFlowPipeline> Pipelines;
};

## 文件 3: Plugins/FlowVisualizer/Source/FlowVisualizer/Private/FlowPipelineRegistry.cpp

#include "FlowPipelineRegistry.h"

FFlowPipelineRegistry& FFlowPipelineRegistry::Get()
{
    static FFlowPipelineRegistry Instance;
    return Instance;
}

void FFlowPipelineRegistry::RegisterPipeline(const FFlowPipeline& Pipeline)
{
    Pipelines.Add(Pipeline.PipelineID, Pipeline);
    OnRegistryChanged.Broadcast();
}

const FFlowPipeline* FFlowPipelineRegistry::FindPipeline(FName PipelineID) const
{
    return Pipelines.Find(PipelineID);
}

TArray<FName> FFlowPipelineRegistry::GetAllPipelineIDs() const
{
    TArray<FName> IDs;
    Pipelines.GetKeys(IDs);
    return IDs;
}

void FFlowPipelineRegistry::AddNodeToPipeline(
    FName PipelineID, FName NodeID, FText DisplayName, FVector2D Position)
{
    FFlowPipeline& Pipeline = Pipelines.FindOrAdd(PipelineID);
    if (Pipeline.PipelineID.IsNone())
    {
        Pipeline.PipelineID = PipelineID;
        Pipeline.DisplayName = FText::FromName(PipelineID);
    }

    // 检查是否已存在
    for (const FFlowNode& Node : Pipeline.Nodes)
    {
        if (Node.NodeID == NodeID) return;
    }

    Pipeline.Nodes.Add(FFlowNode(NodeID, DisplayName, Position));
    OnRegistryChanged.Broadcast();
}

void FFlowPipelineRegistry::AddEdgeToPipeline(
    FName PipelineID, FName SourceID, FName TargetID)
{
    FFlowPipeline* Pipeline = Pipelines.Find(PipelineID);
    if (!Pipeline) return;

    // 检查是否已存在
    for (const FFlowEdge& Edge : Pipeline->Edges)
    {
        if (Edge.SourceNodeID == SourceID && Edge.TargetNodeID == TargetID)
            return;
    }

    Pipeline->Edges.Add(FFlowEdge(SourceID, TargetID));
    OnRegistryChanged.Broadcast();
}

禁止执行任何终端/PowerShell 命令。
```

---

## P3：信号追踪器 FFlowTracer

### 任务背景
创建全局信号追踪器，游戏代码调用 `Signal()` 发出信号，
通过委托广播给 UI 层。

### 实现思路
- 单例模式
- `Signal()` 方法创建 `FFlowSignalEvent` 并广播
- 环形缓冲区存储最近 1024 条信号用于回放
- `OnFlowEvent` 多播委托

### Prompt

```
【追踪器】FFlowTracer — 信号广播 + 环形缓冲

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizer/Public/FlowTracer.h

#pragma once
#include "CoreMinimal.h"

/** 信号事件数据 */
struct FLOWVISUALIZER_API FFlowSignalEvent
{
    FName   NodeID;
    FString Data;
    FString Scope;
    double  Timestamp;

    FFlowSignalEvent()
        : Timestamp(0.0) {}
    FFlowSignalEvent(FName InNode, const FString& InData,
                     const FString& InScope, double InTime)
        : NodeID(InNode), Data(InData), Scope(InScope), Timestamp(InTime) {}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlowEvent, const FFlowSignalEvent&);

class FLOWVISUALIZER_API FFlowTracer
{
public:
    static FFlowTracer& Get();

    /** 发出信号 */
    void Signal(FName NodeID,
                const FString& Data = TEXT(""),
                const FString& Scope = TEXT(""));

    /** 获取最近的信号记录 */
    const TArray<FFlowSignalEvent>& GetRecentSignals() const
    { return RingBuffer; }

    /** 信号广播委托 */
    FOnFlowEvent OnFlowEvent;

private:
    TArray<FFlowSignalEvent> RingBuffer;
    int32 BufferIndex = 0;
    static const int32 MaxBufferSize = 1024;
};

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizer/Private/FlowTracer.cpp

#include "FlowTracer.h"
#include "HAL/PlatformTime.h"

FFlowTracer& FFlowTracer::Get()
{
    static FFlowTracer Instance;
    return Instance;
}

void FFlowTracer::Signal(FName NodeID, const FString& Data, const FString& Scope)
{
    FFlowSignalEvent Event(NodeID, Data, Scope, FPlatformTime::Seconds());

    // 环形缓冲
    if (RingBuffer.Num() < MaxBufferSize)
    {
        RingBuffer.Add(Event);
    }
    else
    {
        RingBuffer[BufferIndex] = Event;
        BufferIndex = (BufferIndex + 1) % MaxBufferSize;
    }

    OnFlowEvent.Broadcast(Event);
}

禁止执行任何终端/PowerShell 命令。
```

---

## P4：C++ 静态埋点（5 条管线，67 个节点）

### 任务背景
在 EditorModule 的 StartupModule 中注册 5 条预定义管线，
覆盖角色移动、相机、战斗、锁定、相机细节等系统。

### 实现思路
- 每条管线在 `StartupModule()` 中通过 `RegisterPipeline()` 注册
- 节点位置使用归一化坐标（0.0~1.0），按行列排列
- 边通过 `FFlowEdge` 定义节点间连接关系
- 在对应的 C++ 游戏代码中调用 `FFlowTracer::Get().Signal(...)` 埋点

### Prompt

```
【埋点注册】5 条管线 — 在 FlowVisualizerEditorModule::StartupModule 中注册

只修改 Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/FlowVisualizerEditorModule.cpp

在文件顶部添加 include：
    #include "FlowPipelineRegistry.h"

在 StartupModule() 函数中，TabSpawner 注册之后、菜单注册之前，
添加以下管线注册代码。

### 管线 1: CharacterMovement（15 个节点）

    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("CharacterMovement");
        P.DisplayName = FText::FromString(TEXT("Character Movement"));

        P.Nodes = {
            { TEXT("PlayerInput"),        FText::FromString(TEXT("Player Input")),          FVector2D(0.06, 0.15) },
            { TEXT("Player1"),            FText::FromString(TEXT("Player Pawn")),            FVector2D(0.20, 0.15) },
            { TEXT("CharBeginPlay"),      FText::FromString(TEXT("Char BeginPlay")),         FVector2D(0.34, 0.15) },
            { TEXT("MovementProcess"),    FText::FromString(TEXT("Movement Process")),       FVector2D(0.50, 0.15) },
            { TEXT("JumpLand"),           FText::FromString(TEXT("Jump / Land")),            FVector2D(0.66, 0.15) },
            { TEXT("MoveInput"),          FText::FromString(TEXT("Move Input")),             FVector2D(0.06, 0.40) },
            { TEXT("AutoRealign"),        FText::FromString(TEXT("Auto Realign")),           FVector2D(0.20, 0.40) },
            { TEXT("SprintToggle"),       FText::FromString(TEXT("Sprint Toggle")),          FVector2D(0.36, 0.40) },
            { TEXT("StaminaSystem"),      FText::FromString(TEXT("Stamina System")),         FVector2D(0.52, 0.40) },
            { TEXT("LockOnInput"),        FText::FromString(TEXT("LockOn Input")),           FVector2D(0.68, 0.40) },
            { TEXT("TargetValidation"),   FText::FromString(TEXT("Target Validation")),      FVector2D(0.06, 0.65) },
            { TEXT("TargetDetection"),    FText::FromString(TEXT("Target Detection")),       FVector2D(0.22, 0.65) },
            { TEXT("StartLockOn"),        FText::FromString(TEXT("Start LockOn")),           FVector2D(0.38, 0.65) },
            { TEXT("UIManager"),          FText::FromString(TEXT("UI Manager")),             FVector2D(0.54, 0.65) },
            { TEXT("UISizeAdapt"),        FText::FromString(TEXT("UI Size Adapt")),          FVector2D(0.70, 0.65) },
        };

        P.Edges = {
            { TEXT("PlayerInput"),      TEXT("Player1") },
            { TEXT("Player1"),          TEXT("CharBeginPlay") },
            { TEXT("CharBeginPlay"),    TEXT("MovementProcess") },
            { TEXT("MovementProcess"),  TEXT("JumpLand") },
            { TEXT("MoveInput"),        TEXT("AutoRealign") },
            { TEXT("SprintToggle"),     TEXT("StaminaSystem") },
            { TEXT("StaminaSystem"),    TEXT("LockOnInput") },
            { TEXT("TargetValidation"), TEXT("TargetDetection") },
            { TEXT("TargetDetection"),  TEXT("StartLockOn") },
            { TEXT("StartLockOn"),      TEXT("UIManager") },
            { TEXT("UIManager"),        TEXT("UISizeAdapt") },
        };

        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

### 管线 2: CameraPipeline（12 个节点）

    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("CameraPipeline");
        P.DisplayName = FText::FromString(TEXT("Camera Pipeline"));

        P.Nodes = {
            { TEXT("CamInput"),          FText::FromString(TEXT("Camera Input")),           FVector2D(0.06, 0.15) },
            { TEXT("CamYawPitch"),       FText::FromString(TEXT("Yaw / Pitch")),            FVector2D(0.22, 0.15) },
            { TEXT("CamCollision"),      FText::FromString(TEXT("Collision")),              FVector2D(0.38, 0.15) },
            { TEXT("CamBoom"),           FText::FromString(TEXT("Spring Arm")),             FVector2D(0.54, 0.15) },
            { TEXT("CamFOV"),            FText::FromString(TEXT("FOV Adjust")),             FVector2D(0.70, 0.15) },
            { TEXT("CamShake"),          FText::FromString(TEXT("Camera Shake")),           FVector2D(0.06, 0.45) },
            { TEXT("CamLockOnTarget"),   FText::FromString(TEXT("LockOn Target")),          FVector2D(0.22, 0.45) },
            { TEXT("CamInterpSpeed"),    FText::FromString(TEXT("Interp Speed")),           FVector2D(0.38, 0.45) },
            { TEXT("CamFinalBlend"),     FText::FromString(TEXT("Final Blend")),            FVector2D(0.54, 0.45) },
            { TEXT("CamOutput"),         FText::FromString(TEXT("Camera Output")),          FVector2D(0.70, 0.45) },
            { TEXT("CamDebug"),          FText::FromString(TEXT("Debug View")),             FVector2D(0.30, 0.75) },
            { TEXT("CamReset"),          FText::FromString(TEXT("Reset Camera")),           FVector2D(0.50, 0.75) },
        };

        P.Edges = {
            { TEXT("CamInput"),        TEXT("CamYawPitch") },
            { TEXT("CamYawPitch"),     TEXT("CamCollision") },
            { TEXT("CamCollision"),    TEXT("CamBoom") },
            { TEXT("CamBoom"),         TEXT("CamFOV") },
            { TEXT("CamShake"),        TEXT("CamLockOnTarget") },
            { TEXT("CamLockOnTarget"), TEXT("CamInterpSpeed") },
            { TEXT("CamInterpSpeed"),  TEXT("CamFinalBlend") },
            { TEXT("CamFinalBlend"),   TEXT("CamOutput") },
            { TEXT("CamFOV"),          TEXT("CamFinalBlend") },
        };

        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

### 管线 3: CombatSystem（14 个节点）

    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("CombatSystem");
        P.DisplayName = FText::FromString(TEXT("Combat System"));

        P.Nodes = {
            { TEXT("AttackInput"),       FText::FromString(TEXT("Attack Input")),           FVector2D(0.06, 0.12) },
            { TEXT("ComboCheck"),        FText::FromString(TEXT("Combo Check")),            FVector2D(0.20, 0.12) },
            { TEXT("ComboWindow"),       FText::FromString(TEXT("Combo Window")),           FVector2D(0.36, 0.12) },
            { TEXT("MontagePlay"),       FText::FromString(TEXT("Montage Play")),           FVector2D(0.52, 0.12) },
            { TEXT("AnimNotify"),        FText::FromString(TEXT("Anim Notify")),            FVector2D(0.68, 0.12) },
            { TEXT("DamageCalc"),        FText::FromString(TEXT("Damage Calc")),            FVector2D(0.06, 0.40) },
            { TEXT("HitDetection"),      FText::FromString(TEXT("Hit Detection")),          FVector2D(0.22, 0.40) },
            { TEXT("HitReaction"),       FText::FromString(TEXT("Hit Reaction")),           FVector2D(0.38, 0.40) },
            { TEXT("DodgeInput"),        FText::FromString(TEXT("Dodge Input")),            FVector2D(0.54, 0.40) },
            { TEXT("DodgeExecute"),      FText::FromString(TEXT("Dodge Execute")),          FVector2D(0.70, 0.40) },
            { TEXT("BlockInput"),        FText::FromString(TEXT("Block Input")),            FVector2D(0.06, 0.68) },
            { TEXT("BlockState"),        FText::FromString(TEXT("Block State")),            FVector2D(0.22, 0.68) },
            { TEXT("ParryWindow"),       FText::FromString(TEXT("Parry Window")),           FVector2D(0.38, 0.68) },
            { TEXT("StaggerApply"),      FText::FromString(TEXT("Stagger Apply")),          FVector2D(0.54, 0.68) },
        };

        P.Edges = {
            { TEXT("AttackInput"),   TEXT("ComboCheck") },
            { TEXT("ComboCheck"),    TEXT("ComboWindow") },
            { TEXT("ComboWindow"),   TEXT("MontagePlay") },
            { TEXT("MontagePlay"),   TEXT("AnimNotify") },
            { TEXT("AnimNotify"),    TEXT("DamageCalc") },
            { TEXT("DamageCalc"),    TEXT("HitDetection") },
            { TEXT("HitDetection"), TEXT("HitReaction") },
            { TEXT("DodgeInput"),   TEXT("DodgeExecute") },
            { TEXT("BlockInput"),   TEXT("BlockState") },
            { TEXT("BlockState"),   TEXT("ParryWindow") },
            { TEXT("ParryWindow"),  TEXT("StaggerApply") },
        };

        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

### 管线 4: LockOnSystem（12 个节点）

    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("LockOnSystem");
        P.DisplayName = FText::FromString(TEXT("Lock-On System"));

        P.Nodes = {
            { TEXT("LockOnToggle"),      FText::FromString(TEXT("LockOn Toggle")),          FVector2D(0.06, 0.15) },
            { TEXT("TargetSearch"),      FText::FromString(TEXT("Target Search")),          FVector2D(0.22, 0.15) },
            { TEXT("TargetScore"),       FText::FromString(TEXT("Target Score")),           FVector2D(0.38, 0.15) },
            { TEXT("TargetSelect"),      FText::FromString(TEXT("Target Select")),          FVector2D(0.54, 0.15) },
            { TEXT("LockOnActivate"),    FText::FromString(TEXT("Activate LockOn")),        FVector2D(0.70, 0.15) },
            { TEXT("SwitchTarget"),      FText::FromString(TEXT("Switch Target")),          FVector2D(0.06, 0.50) },
            { TEXT("TargetLost"),        FText::FromString(TEXT("Target Lost")),            FVector2D(0.22, 0.50) },
            { TEXT("ReTarget"),          FText::FromString(TEXT("Re-Target")),              FVector2D(0.38, 0.50) },
            { TEXT("LockOnDisable"),     FText::FromString(TEXT("Disable LockOn")),         FVector2D(0.54, 0.50) },
            { TEXT("UIProjection"),      FText::FromString(TEXT("UI Projection")),          FVector2D(0.70, 0.50) },
            { TEXT("SubTargetLock"),     FText::FromString(TEXT("Sub-Target Lock")),        FVector2D(0.30, 0.80) },
            { TEXT("CamLockOnSet"),      FText::FromString(TEXT("Cam LockOn Set")),         FVector2D(0.50, 0.80) },
        };

        P.Edges = {
            { TEXT("LockOnToggle"),   TEXT("TargetSearch") },
            { TEXT("TargetSearch"),   TEXT("TargetScore") },
            { TEXT("TargetScore"),    TEXT("TargetSelect") },
            { TEXT("TargetSelect"),   TEXT("LockOnActivate") },
            { TEXT("SwitchTarget"),   TEXT("TargetLost") },
            { TEXT("TargetLost"),     TEXT("ReTarget") },
            { TEXT("ReTarget"),       TEXT("LockOnDisable") },
            { TEXT("LockOnDisable"),  TEXT("UIProjection") },
            { TEXT("UIProjection"),   TEXT("SubTargetLock") },
            { TEXT("SubTargetLock"),  TEXT("CamLockOnSet") },
        };

        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

### 管线 5: CameraDetailPipeline（14 个节点）

    {
        FFlowPipeline P;
        P.PipelineID  = TEXT("CameraDetailPipeline");
        P.DisplayName = FText::FromString(TEXT("Camera Detail Pipeline"));

        P.Nodes = {
            { TEXT("CamStateChange"),      FText::FromString(TEXT("State Change")),         FVector2D(0.06, 0.12) },
            { TEXT("CamLockNotify"),       FText::FromString(TEXT("LockOn Notify")),        FVector2D(0.22, 0.12) },
            { TEXT("CameraInputBlocked"),  FText::FromString(TEXT("Input Blocked")),        FVector2D(0.38, 0.12) },
            { TEXT("CamInputGather"),      FText::FromString(TEXT("Input Gather")),         FVector2D(0.54, 0.12) },
            { TEXT("CamStage1_Input"),     FText::FromString(TEXT("Stage1: Input")),        FVector2D(0.70, 0.12) },
            { TEXT("CamStage2_State"),     FText::FromString(TEXT("Stage2: State")),        FVector2D(0.06, 0.40) },
            { TEXT("CamStage3_Modify"),    FText::FromString(TEXT("Stage3: Modify")),       FVector2D(0.22, 0.40) },
            { TEXT("CamStage4_Collision"), FText::FromString(TEXT("Stage4: Collision")),    FVector2D(0.38, 0.40) },
            { TEXT("CamStage5_Blend"),     FText::FromString(TEXT("Stage5: Blend")),        FVector2D(0.54, 0.40) },
            { TEXT("CamStage6_Apply"),     FText::FromString(TEXT("Stage6: Apply")),        FVector2D(0.70, 0.40) },
            { TEXT("CamTick_Complete"),    FText::FromString(TEXT("Tick Complete")),         FVector2D(0.20, 0.70) },
            { TEXT("CamDebugOverlay"),     FText::FromString(TEXT("Debug Overlay")),        FVector2D(0.40, 0.70) },
            { TEXT("CamPostProcess"),      FText::FromString(TEXT("Post Process")),         FVector2D(0.60, 0.70) },
            { TEXT("CamFinalOutput"),      FText::FromString(TEXT("Final Output")),         FVector2D(0.80, 0.70) },
        };

        P.Edges = {
            { TEXT("CamStateChange"),      TEXT("CamLockNotify") },
            { TEXT("CamLockNotify"),       TEXT("CameraInputBlocked") },
            { TEXT("CameraInputBlocked"),  TEXT("CamInputGather") },
            { TEXT("CamInputGather"),      TEXT("CamStage1_Input") },
            { TEXT("CamStage1_Input"),     TEXT("CamStage2_State") },
            { TEXT("CamStage2_State"),     TEXT("CamStage3_Modify") },
            { TEXT("CamStage3_Modify"),    TEXT("CamStage4_Collision") },
            { TEXT("CamStage4_Collision"), TEXT("CamStage5_Blend") },
            { TEXT("CamStage5_Blend"),     TEXT("CamStage6_Apply") },
            { TEXT("CamStage6_Apply"),     TEXT("CamTick_Complete") },
            { TEXT("CamTick_Complete"),    TEXT("CamDebugOverlay") },
            { TEXT("CamDebugOverlay"),     TEXT("CamPostProcess") },
            { TEXT("CamPostProcess"),      TEXT("CamFinalOutput") },
        };

        FFlowPipelineRegistry::Get().RegisterPipeline(P);
    }

只修改 FlowVisualizerEditorModule.cpp。
不要修改其他文件。
禁止执行任何终端/PowerShell 命令。
```

---

## P5：Slate 可视化面板（SFlowVisualizerPanel）

### 任务背景
创建 Slate Widget 负责绘制管线的节点和边，
以及管线切换下拉框。

### 实现思路
- 继承 `SCompoundWidget`，实现 `Tick` + `OnPaint`
- `Construct` 中创建 ComboBox + 订阅委托
- `OnPaint` 绘制：背景 → 边（箭头线） → 节点（方括号文字）
- `NodeRelativeToPixel` 将归一化坐标转为像素坐标
- 下拉框切换管线，`RefreshPipelineList` 刷新列表

### Prompt

```
【可视化面板】SFlowVisualizerPanel — 框架 + 静态绘制

## 文件 1: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/FlowVisualizerStyle.h

#pragma once
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

namespace FlowVisStyle
{
    // ---- 颜色 ----
    inline FLinearColor Background()    { return FLinearColor(0.02f, 0.02f, 0.03f, 1.0f); }
    inline FLinearColor NodeInactive()  { return FLinearColor(0.55f, 0.55f, 0.55f, 1.0f); }
    inline FLinearColor NodeActive()    { return FLinearColor(0.0f,  1.0f,  0.6f,  1.0f); }
    inline FLinearColor EdgeInactive()  { return FLinearColor(0.3f,  0.3f,  0.3f,  0.6f); }
    inline FLinearColor EdgeActive()    { return FLinearColor(0.0f,  0.9f,  1.0f,  1.0f); }
    inline FLinearColor ArrowColor()    { return FLinearColor(0.6f,  0.6f,  0.6f,  1.0f); }
    inline FLinearColor StatusBar()     { return FLinearColor(0.08f, 0.08f, 0.10f, 1.0f); }
    inline FLinearColor StatusText()    { return FLinearColor(0.6f,  0.6f,  0.6f,  1.0f); }
    inline FLinearColor DataValueText() { return FLinearColor(0.4f,  0.75f, 1.0f,  1.0f); }
    inline FLinearColor ScopeArrow()    { return FLinearColor(0.9f,  0.7f,  0.2f,  1.0f); }
    inline FLinearColor BreadcrumbText(){ return FLinearColor(0.7f,  0.7f,  0.3f,  1.0f); }

    // ---- 字体 ----
    inline FSlateFontInfo NodeFont()
    {
        return FCoreStyle::GetDefaultFontStyle("Bold", 11);
    }
    inline FSlateFontInfo DataValueFont()
    {
        return FCoreStyle::GetDefaultFontStyle("Regular", 8);
    }
    inline FSlateFontInfo StatusFont()
    {
        return FCoreStyle::GetDefaultFontStyle("Regular", 9);
    }

    // ---- 定数 ----
    inline float EdgeThickness()    { return 1.5f; }
    inline float ActiveEdgeThickness() { return 2.5f; }
    inline float ArrowSize()        { return 8.0f; }
    inline float PulseDecayRate()   { return 2.0f; }
}

## 文件 2: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Public/SFlowVisualizerPanel.h

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "FlowPipelineTypes.h"

class SFlowVisualizerPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SFlowVisualizerPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateDrawElements& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
    // ---- 管线选择 ----
    FName CurrentPipelineID;
    TArray<TSharedPtr<FName>> PipelineIDList;
    TSharedPtr<FName> SelectedPipelineItem;
    TSharedPtr<class SComboBox<TSharedPtr<FName>>> PipelineComboBox;

    void RefreshPipelineList();
    void OnPipelineSelected(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
    FText GetSelectedPipelineText() const;

    TSharedRef<SWidget> GeneratePipelineComboItem(TSharedPtr<FName> InItem);

    // ---- 信号受信 ----
    void OnFlowEventReceived(const struct FFlowSignalEvent& Event);

    // ---- 可视化状态 ----
    TMap<FName, double> NodeLastActiveTime;
    TMap<FName, FString> NodeLastData;
    TMap<FName, double> EdgeLastActiveTime;

    // ---- 委托ハンドル ----
    FDelegateHandle RegistryChangedHandle;
    FDelegateHandle FlowEventHandle;

    // ---- 座標変換 ----
    FVector2D NodeRelativeToPixel(FVector2D RelativePos, FVector2D PanelSize) const;

    // ---- テスト信号 ----
    int32 TestSignalNodeIndex = 0;
    double TestSignalAccumulator = 0.0;

    // ---- 統計 ----
    int32 ActiveSignalCount = 0;
    double LastSignalElapsed = 0.0;
    float  CachedFPS = 0.0f;
};

## 文件 3: Plugins/FlowVisualizer/Source/FlowVisualizerEditor/Private/SFlowVisualizerPanel.cpp

（此文件内容很长，分为以下几个部分）

### 头部 include

#include "SFlowVisualizerPanel.h"
#include "FlowVisualizerStyle.h"
#include "FlowPipelineRegistry.h"
#include "FlowTracer.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Rendering/DrawElements.h"
#include "HAL/PlatformTime.h"

### Construct

void SFlowVisualizerPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 8, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Pipeline:")))
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SAssignNew(PipelineComboBox, SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&PipelineIDList)
                .OnSelectionChanged(this, &SFlowVisualizerPanel::OnPipelineSelected)
                .OnGenerateWidget(this, &SFlowVisualizerPanel::GeneratePipelineComboItem)
                .Content()
                [
                    SNew(STextBlock)
                    .Text(this, &SFlowVisualizerPanel::GetSelectedPipelineText)
                ]
            ]
        ]
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNullWidget::NullWidget  // OnPaint が描画領域全体を使う
        ]
    ];

    // 委托バインド
    RegistryChangedHandle = FFlowPipelineRegistry::Get().OnRegistryChanged.AddRaw(
        this, &SFlowVisualizerPanel::RefreshPipelineList);

    FlowEventHandle = FFlowTracer::Get().OnFlowEvent.AddRaw(
        this, &SFlowVisualizerPanel::OnFlowEventReceived);

    RefreshPipelineList();
}

### 下拉框関連

TSharedRef<SWidget> SFlowVisualizerPanel::GeneratePipelineComboItem(
    TSharedPtr<FName> InItem)
{
    return SNew(STextBlock)
        .Text(FText::FromName(InItem.IsValid() ? *InItem : NAME_None));
}

void SFlowVisualizerPanel::RefreshPipelineList()
{
    TArray<FName> AllIDs = FFlowPipelineRegistry::Get().GetAllPipelineIDs();

    PipelineIDList.Empty();
    for (const FName& ID : AllIDs)
        PipelineIDList.Add(MakeShareable(new FName(ID)));

    bool bCurrentValid = false;
    TSharedPtr<FName> NewSelectedItem;
    for (const TSharedPtr<FName>& Item : PipelineIDList)
    {
        if (Item.IsValid() && *Item == CurrentPipelineID)
        {
            bCurrentValid = true;
            NewSelectedItem = Item;
            break;
        }
    }

    if (!bCurrentValid)
    {
        if (PipelineIDList.Num() > 0)
        {
            NewSelectedItem = PipelineIDList[0];
            CurrentPipelineID = *NewSelectedItem;
        }
        else
        {
            CurrentPipelineID = NAME_None;
        }
    }

    SelectedPipelineItem = NewSelectedItem;
    if (PipelineComboBox.IsValid())
    {
        PipelineComboBox->RefreshOptions();
        PipelineComboBox->SetSelectedItem(SelectedPipelineItem);
    }
}

void SFlowVisualizerPanel::OnPipelineSelected(
    TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
    SelectedPipelineItem = NewSelection;
    if (NewSelection.IsValid())
        CurrentPipelineID = *NewSelection;
    else
        CurrentPipelineID = NAME_None;

    TestSignalNodeIndex = 0;
    TestSignalAccumulator = 0.0;
}

FText SFlowVisualizerPanel::GetSelectedPipelineText() const
{
    if (SelectedPipelineItem.IsValid())
        return FText::FromName(*SelectedPipelineItem);
    return FText::FromString(TEXT("(none)"));
}

### 座標変換

FVector2D SFlowVisualizerPanel::NodeRelativeToPixel(
    FVector2D RelativePos, FVector2D PanelSize) const
{
    return RelativePos * PanelSize;
}

### 信号受信

void SFlowVisualizerPanel::OnFlowEventReceived(const FFlowSignalEvent& Event)
{
    double Now = FPlatformTime::Seconds();
    NodeLastActiveTime.Add(Event.NodeID, Now);
    if (!Event.Data.IsEmpty())
        NodeLastData.Add(Event.NodeID, Event.Data);

    // 辺のアクティベーション
    const FFlowPipeline* Pipeline =
        FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
    if (Pipeline)
    {
        for (const FFlowEdge& Edge : Pipeline->Edges)
        {
            if (Edge.TargetNodeID == Event.NodeID)
            {
                FName EdgeKey = FName(
                    *FString::Printf(TEXT("%s->%s"),
                        *Edge.SourceNodeID.ToString(),
                        *Edge.TargetNodeID.ToString()));
                EdgeLastActiveTime.Add(EdgeKey, Now);
            }
        }
    }
}

### Tick

void SFlowVisualizerPanel::Tick(
    const FGeometry& AllottedGeometry,
    const double InCurrentTime,
    const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    double Now = FPlatformTime::Seconds();
    CachedFPS = (InDeltaTime > 0.0f) ? (1.0f / InDeltaTime) : 0.0f;

    // アクティブカウント
    ActiveSignalCount = 0;
    LastSignalElapsed = 999.0;
    for (const auto& Pair : NodeLastActiveTime)
    {
        double Elapsed = Now - Pair.Value;
        if (Elapsed < 1.0 / FlowVisStyle::PulseDecayRate())
            ActiveSignalCount++;
        if (Elapsed < LastSignalElapsed)
            LastSignalElapsed = Elapsed;
    }

    // テスト信号（自動デモ用）
    const FFlowPipeline* Pipeline =
        FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
    if (Pipeline && Pipeline->Nodes.Num() > 0)
    {
        TestSignalAccumulator += InDeltaTime;
        if (TestSignalAccumulator >= 0.3)
        {
            TestSignalAccumulator = 0.0;
            const FFlowNode& Node =
                Pipeline->Nodes[TestSignalNodeIndex % Pipeline->Nodes.Num()];
            FFlowTracer::Get().Signal(Node.NodeID, TEXT("auto-test"));
            TestSignalNodeIndex++;
        }
    }

    Invalidate(EInvalidateWidgetReason::Paint);
}

### OnPaint

int32 SFlowVisualizerPanel::OnPaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateDrawElements& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    double Now = FPlatformTime::Seconds();

    // ===== 背景 =====
    FSlateDrawElement::MakeBox(
        OutDrawElements, LayerId,
        AllottedGeometry.ToPaintGeometry(),
        WhiteBrush, ESlateDrawEffect::None,
        FlowVisStyle::Background());

    const FFlowPipeline* Pipeline =
        FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
    if (!Pipeline)
        return LayerId;

    // ===== 辺の描画 =====
    for (const FFlowEdge& Edge : Pipeline->Edges)
    {
        FVector2D SrcPos = FVector2D::ZeroVector;
        FVector2D DstPos = FVector2D::ZeroVector;
        bool bFoundSrc = false, bFoundDst = false;

        for (const FFlowNode& Node : Pipeline->Nodes)
        {
            if (Node.NodeID == Edge.SourceNodeID)
            {
                SrcPos = NodeRelativeToPixel(Node.RelativePosition, LocalSize);
                bFoundSrc = true;
            }
            if (Node.NodeID == Edge.TargetNodeID)
            {
                DstPos = NodeRelativeToPixel(Node.RelativePosition, LocalSize);
                bFoundDst = true;
            }
            if (bFoundSrc && bFoundDst) break;
        }

        if (!bFoundSrc || !bFoundDst) continue;

        // 辺色（アクティブ判定）
        FName EdgeKey = FName(*FString::Printf(TEXT("%s->%s"),
            *Edge.SourceNodeID.ToString(),
            *Edge.TargetNodeID.ToString()));

        FLinearColor EdgeColor = FlowVisStyle::EdgeInactive();
        float Thickness = FlowVisStyle::EdgeThickness();

        if (const double* ActiveTime = EdgeLastActiveTime.Find(EdgeKey))
        {
            float Alpha = FMath::Clamp(
                1.0f - (float)(Now - *ActiveTime) * FlowVisStyle::PulseDecayRate(),
                0.0f, 1.0f);
            if (Alpha > 0.0f)
            {
                EdgeColor = FMath::Lerp(FlowVisStyle::EdgeInactive(),
                                        FlowVisStyle::EdgeActive(), Alpha);
                Thickness = FMath::Lerp(FlowVisStyle::EdgeThickness(),
                                        FlowVisStyle::ActiveEdgeThickness(), Alpha);
            }
        }

        // 線
        TArray<FVector2D> LinePoints;
        LinePoints.Add(SrcPos);
        LinePoints.Add(DstPos);
        FSlateDrawElement::MakeLines(
            OutDrawElements, LayerId + 1,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints, ESlateDrawEffect::None,
            EdgeColor, true, Thickness);

        // 矢印
        FVector2D Dir = (DstPos - SrcPos).GetSafeNormal();
        FVector2D Perp(-Dir.Y, Dir.X);
        FVector2D ArrowTip = DstPos - Dir * 12.0f;
        float AS = FlowVisStyle::ArrowSize();

        TArray<FVector2D> Arrow1, Arrow2;
        Arrow1.Add(ArrowTip);
        Arrow1.Add(ArrowTip - Dir * AS + Perp * AS * 0.5f);
        Arrow2.Add(ArrowTip);
        Arrow2.Add(ArrowTip - Dir * AS - Perp * AS * 0.5f);

        FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2,
            AllottedGeometry.ToPaintGeometry(),
            Arrow1, ESlateDrawEffect::None, FlowVisStyle::ArrowColor(), true, 1.5f);
        FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2,
            AllottedGeometry.ToPaintGeometry(),
            Arrow2, ESlateDrawEffect::None, FlowVisStyle::ArrowColor(), true, 1.5f);
    }

    // ===== ノードの描画 =====
    for (const FFlowNode& Node : Pipeline->Nodes)
    {
        FVector2D NodePixelPos = NodeRelativeToPixel(
            Node.RelativePosition, LocalSize);

        // ノード色
        FLinearColor TextColor = FlowVisStyle::NodeInactive();
        if (const double* ActiveTime = NodeLastActiveTime.Find(Node.NodeID))
        {
            float Alpha = FMath::Clamp(
                1.0f - (float)(Now - *ActiveTime) * FlowVisStyle::PulseDecayRate(),
                0.0f, 1.0f);
            if (Alpha > 0.0f)
            {
                TextColor = FMath::Lerp(FlowVisStyle::NodeInactive(),
                                        FlowVisStyle::NodeActive(), Alpha);
            }
        }

        // ノード名
        FString NodeLabel = FString::Printf(TEXT("[%s]"),
            *Node.DisplayName.ToString());

        FSlateDrawElement::MakeText(
            OutDrawElements, LayerId + 3,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(NodePixelPos.X - 60.0f, NodePixelPos.Y - 8.0f),
                FVector2D(120.0f, 20.0f)),
            FText::FromString(NodeLabel),
            FlowVisStyle::NodeFont(),
            ESlateDrawEffect::None,
            TextColor);

        // データ
        if (const FString* Data = NodeLastData.Find(Node.NodeID))
        {
            FSlateDrawElement::MakeText(
                OutDrawElements, LayerId + 3,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(NodePixelPos.X - 50.0f, NodePixelPos.Y + 10.0f),
                    FVector2D(100.0f, 14.0f)),
                FText::FromString(*Data),
                FlowVisStyle::DataValueFont(),
                ESlateDrawEffect::None,
                FlowVisStyle::DataValueText());
        }
    }

    // ===== ステータスバー =====
    const float StatusBarHeight = 24.0f;
    const float StatusBarY = LocalSize.Y - StatusBarHeight;

    FSlateDrawElement::MakeBox(
        OutDrawElements, LayerId + 4,
        AllottedGeometry.ToPaintGeometry(
            FVector2D(0.0f, StatusBarY),
            FVector2D(LocalSize.X, StatusBarHeight)),
        WhiteBrush, ESlateDrawEffect::None,
        FlowVisStyle::StatusBar());

    int32 NodeCount = Pipeline ? Pipeline->Nodes.Num() : 0;
    int32 BufferCount = FFlowTracer::Get().GetRecentSignals().Num();
    FString StatusText = FString::Printf(
        TEXT("Frame: %lld | FPS: %.0f | Active: %d/%d | Last: %.0fms ago | Buffer: %d/%d"),
        GFrameCounter, CachedFPS,
        ActiveSignalCount, NodeCount,
        LastSignalElapsed * 1000.0,
        BufferCount, 1024);

    FSlateDrawElement::MakeText(
        OutDrawElements, LayerId + 5,
        AllottedGeometry.ToPaintGeometry(
            FVector2D(10.0f, StatusBarY + 4.0f),
            FVector2D(LocalSize.X - 20.0f, 16.0f)),
        FText::FromString(StatusText),
        FlowVisStyle::StatusFont(),
        ESlateDrawEffect::None,
        FlowVisStyle::StatusText());

    return LayerId + 6;
}

### OnMouseButtonDown（初期版 — 左クリック空打ち）

FReply SFlowVisualizerPanel::OnMouseButtonDown(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    return FReply::Unhandled();
}

禁止执行任何终端/PowerShell 命令。
```

> **注意**：P5 是最大的 Prompt，包含整个 Panel 的框架实现。
> 如果 Agent 执行出错，应拆分为头文件和 cpp 文件两个独立 Prompt。

---

## P6：实时动画增强

### 任务背景
P5 已经有基本的脉冲衰减效果。P6 增加更精细的动画：
- 节点数据标签显示
- 边的激活传播（信号到达时点亮入边）
- 状态栏实时统计

### 实现思路
P6 的功能实际上已经合并在 P5 的实现中，包括：
- `NodeLastData` 显示在节点下方
- `EdgeLastActiveTime` 控制边的脉冲颜色
- 状态栏显示 Frame/FPS/Active/Buffer 信息
- `PulseDecayRate` 控制衰减速度

### 说明
P6 原计划为独立 Prompt，但在实际开发中已合并到 P5。
如果需要调整动画参数，只需修改 `FlowVisualizerStyle.h` 中的常量：

```cpp
// 调整这些值改变动画效果
inline float PulseDecayRate()      { return 2.0f; }   // 衰减速度（越大越快消失）
inline float EdgeThickness()       { return 1.5f; }   // 边默认粗细
inline float ActiveEdgeThickness() { return 2.5f; }   // 边激活时粗细
```

---

## Part 2 总结

| Prompt | 文件数 | 核心内容 |
|--------|--------|---------|
| P1 | 7 | 插件骨架 + Editor Module + 菜单注册 |
| P2 | 3 | 数据结构 + 注册表单例 |
| P3 | 2 | 信号追踪器 + 环形缓冲 |
| P4 | 1 | 5 条管线 67 节点注册 |
| P5 | 3 | Slate Panel 完整实现（绘制+交互+动画） |
| P6 | 0 | 已合并到 P5 |

---

*Part 3 将包含蓝图信号 + Scope 层级 + 自动注册的 Prompt*