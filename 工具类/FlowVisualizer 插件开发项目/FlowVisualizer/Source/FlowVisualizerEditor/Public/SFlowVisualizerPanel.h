// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "FlowPipelineTypes.h"

struct FFlowEvent;

/**
 * FEdgeKey — Lightweight key for identifying a directed edge (Source -> Target).
 * Used as TMap key for EdgeLastActiveTime.
 */
struct FEdgeKey
{
	FName Source;
	FName Target;

	FEdgeKey() : Source(NAME_None), Target(NAME_None) {}
	FEdgeKey(FName InSource, FName InTarget) : Source(InSource), Target(InTarget) {}

	bool operator==(const FEdgeKey& Other) const
	{
		return Source == Other.Source && Target == Other.Target;
	}

	friend uint32 GetTypeHash(const FEdgeKey& Key)
	{
		return HashCombine(GetTypeHash(Key.Source), GetTypeHash(Key.Target));
	}
};

/**
 * SFlowVisualizerPanel
 *
 * Terminal-style panel for visualizing runtime data flow.
 * This is the shell widget; drawing logic will be added in subsequent tasks.
 */
class SFlowVisualizerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFlowVisualizerPanel) {}
	SLATE_END_ARGS()

	/** Constructs the widget. */
	void Construct(const FArguments& InArgs);

	virtual ~SFlowVisualizerPanel();

	// ~ SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	// ~ End SWidget interface

	/** Convert a RelativePosition (0~1) to pixel coordinates on the canvas. */
	FVector2D NodeRelativeToPixel(const FVector2D& RelativePos, const FGeometry& Geometry) const;

	/** Overload accepting panel size directly (FVector2D) instead of FGeometry. */
	FVector2D NodeRelativeToPixel(const FVector2D& RelativePos, const FVector2D& PanelSize) const;

private:
	/** Refresh the pipeline list from the registry and update the combo box data source. */
	void RefreshPipelineList();

	/** Callback when a pipeline is selected from the combo box. */
	void OnPipelineSelected(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);

	/** Generates the widget for each item in the pipeline combo box. */
	TSharedRef<SWidget> GeneratePipelineComboItem(TSharedPtr<FName> Item);

	/** Returns the display text for the currently selected pipeline. */
	FText GetSelectedPipelineText() const;

	/** Currently selected pipeline ID. */
	FName CurrentPipelineID;

	/** Data source for the pipeline combo box (TSharedPtr wrappers for SComboBox). */
	TArray<TSharedPtr<FName>> PipelineIDList;

	/** Currently selected item in the combo box. */
	TSharedPtr<FName> SelectedPipelineItem;

	/** Pause flag — toggled by the Pause button. Full logic in P9.1. */
	bool bIsPaused;

	/** Height of the bottom status bar in pixels. */
	static constexpr float StatusBarHeight = 30.0f;

	/** Canvas margin in pixels — prevents nodes from being drawn at the very edge. */
	static constexpr float CanvasMargin = 40.0f;

	/** Height of the top toolbar bar in pixels — matches the P4.2 top bar. */
	static constexpr float TopBarHeight = 36.0f;

	/** Cached FPS value, smoothed via Lerp to avoid jitter. */
	float CachedFPS = 0.0f;

	/** Handle for the registry changed delegate, used to unsubscribe on destruction. */
	FDelegateHandle RegistryChangedHandle;

	/** Reference to the combo box widget so we can refresh its selection. */
	TSharedPtr<SComboBox<TSharedPtr<FName>>> PipelineComboBox;

	// ------------------------------------------------------------------
	// P6.1 — Flow event delegate connection
	// ------------------------------------------------------------------

	/** Handle for FFlowTracer::OnFlowEvent delegate, must be removed on destruction. */
	FDelegateHandle FlowEventHandle;

	/** Per-node last-activation timestamp (seconds). Read by OnPaint in P6.2. */
	TMap<FName, double> NodeLastActiveTime;

	/** Per-node last data payload string. Read by OnPaint in P6.3. */
	TMap<FName, FString> NodeLastData;

	// ------------------------------------------------------------------
	// Node signal frequency tracking & heatmap
	// ------------------------------------------------------------------

	/** 每个节点的信号触发次数（滑动窗口内） */
	TMap<FName, int32> NodeSignalCount;

	/** 每个节点的信号触发时间戳历史（用于计算频率） */
	TMap<FName, TArray<double>> NodeSignalHistory;

	/** 每个节点的当前频率（次/秒） */
	TMap<FName, float> NodeFrequency;

	/** 频率统计的滑动窗口大小（秒） */
	static constexpr float FrequencyWindowSeconds = 3.0f;

	// ------------------------------------------------------------------
	// Scope navigation state
	// ------------------------------------------------------------------

	/** 当前浏览的 Scope 路径，空 = 顶层 */
	FString CurrentScopePath;

	/** Scope 导航历史栈，用于返回上级 */
	TArray<FString> ScopeHistory;

	/** 记录哪些 NodeID 是 Scope 入口（包含子节点） */
	TSet<FName> ScopeNodes;

	/** 缓存当前 Scope 下应显示的节点列表 */
	TArray<FFlowNode> FilteredNodes;

	/** 缓存当前 Scope 下应显示的边列表 */
	TArray<FFlowEdge> FilteredEdges;

	// ------------------------------------------------------------------
	// P6.4 — Tick-driven animation statistics
	// ------------------------------------------------------------------

	/** Number of nodes currently in highlight-fade animation. Used by status bar. */
	int32 ActiveSignalCount = 0;

	/** Elapsed time (in milliseconds) since the most recent signal. Used by status bar. */
	double LastSignalElapsed = 0.0;

	// ------------------------------------------------------------------
	// P7.1 — Edge activation inference
	// ------------------------------------------------------------------

	/** Per-edge last-activation timestamp (seconds), inferred from node signal proximity. */
	TMap<FEdgeKey, double> EdgeLastActiveTime;

	/** Time window (in seconds) within which two connected node signals infer edge activation. */
	static constexpr double EdgeInferenceWindowSec = 0.2;

	/** Callback bound to FFlowTracer::OnFlowEvent via AddRaw. */
	void OnFlowEventReceived(const FFlowEvent& Event);

	/** Returns the Pause/Resume button label based on current bIsPaused state. */
	FText GetPauseButtonText() const;

	/** Clears all visualization state (node/edge maps, counters) and forces a repaint. */
	void ClearVisualization();

	/** 更新所有节点的频率统计 */
	void UpdateNodeFrequencies();

	/** 根据频率返回热力图颜色 */
	static FLinearColor GetHeatmapColor(float Frequency);

	/** 点击 Scope 节点时进入子层级 */
	void EnterScope(const FString& ScopeName);

	/** 返回上一级 Scope */
	void ExitScope();

	/** 根据 CurrentScopePath 过滤节点和边 */
	void RebuildFilteredView();

	/** 判断一个 NodeID 是否是 Scope 入口 */
	bool IsScopeNode(FName NodeID) const;

	// ------------------------------------------------------------------
	// Test signal emitter — built-in signal source for visual verification
	// ------------------------------------------------------------------

	/** Accumulated time since last test signal emission (seconds). */
	double TestSignalAccumulator = 0.0;

	/** Index of the next node to receive a test signal, cycles through Pipeline->Nodes. */
	int32 TestSignalNodeIndex = 0;

	/** Interval between successive test signal emissions (seconds). */
	static constexpr double TestSignalInterval = 0.8;

	// ------------------------------------------------------------------
	// Canvas zoom & pan state
	// ------------------------------------------------------------------

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
};
