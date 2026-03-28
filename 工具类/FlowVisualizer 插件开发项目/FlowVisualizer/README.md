# FlowVisualizer

A UE 4.27 editor plugin that visualizes C++ runtime data flow in a standalone terminal-style window during PIE sessions.

## Features

- **Standalone Terminal Window** — dedicated editor popup with a dark (#0D0D14) background; zero game-viewport footprint
- **Real-time Node Highlighting** — nodes flash cyan (#33CCFF) on signal, fade to dark grey (#666666) over 1.5 seconds
- **Data Labels** — green (#4AFF82) monospace text displayed alongside active nodes
- **Flowing Light Dots** — 6×6 white squares travel along activated edges
- **Edge Activation** — auto-deduced within a 200 ms window; edges highlight cyan and fade
- **Multi-Pipeline Dropdown** — top-bar combo box switches between registered pipeline topologies
- **Pause / Clear** — toolbar buttons to freeze event intake or reset all visual state and buffers
- **Status Bar** — bottom bar showing Frame, FPS, Active nodes, Last signal, and Buffer usage
- **Window Layout Memory** — position and size persisted across sessions via `GConfig` / `GEditorPerProjectIni`
- **Zero Shipping Overhead** — every `FLOW_SIGNAL` macro compiles to a no-op in Shipping builds

## Module Structure

| Module | Type | Purpose |
|---|---|---|
| **FlowVisualizerRuntime** | Runtime | Data structures, event tracer singleton, convenience macros, pipeline registry |
| **FlowVisualizerEditor** | Editor (UncookedOnly) | Menu entry, SWindow management, Slate canvas rendering, style constants |

### Key Source Files

```
FlowVisualizerRuntime/
  Public/
    FlowPipelineTypes.h      — FFlowEvent, FFlowNode, FFlowEdge, FFlowPipeline
    FlowTracer.h             — FFlowTracer singleton (ring buffer + FCriticalSection + delegate)
    FlowTracerMacros.h       — FLOW_SIGNAL / FLOW_SIGNAL_SIMPLE / FLOW_SIGNAL_FMT
    FlowPipelineRegistry.h   — FFlowPipelineRegistry (register / query pipelines)
  Private/
    FlowTracer.cpp
    FlowPipelineRegistry.cpp
    FlowVisualizerRuntimeModule.cpp

FlowVisualizerEditor/
  Public/
    FlowVisualizerEditorModule.h
    SFlowVisualizerPanel.h
  Private/
    FlowVisualizerEditorModule.cpp
    SFlowVisualizerPanel.cpp
    FlowVisualizerStyle.h / .cpp
```

## Macro API

All macros expand to empty statements in Shipping builds (`#if !UE_BUILD_SHIPPING`).

```cpp
#include "FlowTracerMacros.h"

// Send an event with a data payload string
FLOW_SIGNAL("NodeID", DataString);

// Send a simple event (no data)
FLOW_SIGNAL_SIMPLE("NodeID");

// Send an event with printf-style formatted data
FLOW_SIGNAL_FMT("NodeID", "Dmg=%d Target=%s", Damage, *TargetName);
```

## Pipeline Registration

Define a topology with `FFlowPipeline`, then register it at module startup:

```cpp
#include "FlowPipelineTypes.h"
#include "FlowPipelineRegistry.h"

FFlowPipeline Pipeline;
Pipeline.PipelineID   = FName(TEXT("MyCombat"));
Pipeline.DisplayName  = FText::FromString(TEXT("Combat Pipeline"));

FFlowNode InputNode;
InputNode.NodeID           = FName(TEXT("Input"));
InputNode.DisplayName      = FText::FromString(TEXT("Input"));
InputNode.RelativePosition = FVector2D(0.1f, 0.4f);
Pipeline.Nodes.Add(InputNode);

FFlowNode DamageNode;
DamageNode.NodeID           = FName(TEXT("Damage"));
DamageNode.DisplayName      = FText::FromString(TEXT("Damage"));
DamageNode.RelativePosition = FVector2D(0.5f, 0.4f);
Pipeline.Nodes.Add(DamageNode);

FFlowEdge Edge;
Edge.SourceNodeID = FName(TEXT("Input"));
Edge.TargetNodeID = FName(TEXT("Damage"));
Pipeline.Edges.Add(Edge);

FFlowPipelineRegistry::Get().RegisterPipeline(Pipeline);
```

## Quick Start

1. Copy the `FlowVisualizer` folder into your project's `Plugins/` directory.
2. Add `"FlowVisualizerRuntime"` to your game module's `Build.cs` `PublicDependencyModuleNames`.
3. `#include "FlowTracerMacros.h"` in any source file and place `FLOW_SIGNAL` calls at points of interest.
4. Register one or more `FFlowPipeline` topologies (see above) so the visualizer knows the node layout.
5. In the editor, open **Window → Flow Visualizer** to launch the terminal window, then press **Play** (PIE).

## Visual Spec

| Element | Colour | Detail |
|---|---|---|
| Background | `#0D0D14` | Deep blue-black |
| Node (idle) | `#666666` | Bracketed label `[NodeName]` |
| Node (active) | `#33CCFF` | Cyan flash, 1.5 s fade to idle |
| Data label | `#4AFF82` | Green mono text, visible during highlight |
| Edge (idle) | `#888888` | Line with arrowhead |
| Edge (active) | `#33CCFF` | Cyan highlight, fade to idle |
| Flow dot | `#FFFFFF` | 6×6 square travelling along active edge |

## Built-in Pipelines

- **Demo** — 3 nodes (`Input → Process → Output`), registered by the editor module for quick testing.
- **Soul Project Pipelines** (4) — `CharacterMovement`, `LockOnSystem`, `CameraPipeline`, `CombatSystem`.

## Technical Notes

- **UE Version**: 4.27 (C++14)
- **Thread Safety**: `FFlowTracer` guards its ring buffer with `FCriticalSection`
- **Idle Performance**: canvas `Tick` skips `Invalidate` when no animations are active — zero idle CPU cost
- **Shipping Safety**: all runtime macros compile to no-ops; the editor module is `UncookedOnly`
- **Window Memory**: layout saved to `Saved/Config/Windows/EditorPerProjectUserSettings.ini` under section `[FlowVisualizer.WindowLayout]`

## License

Internal project plugin — not for redistribution.
