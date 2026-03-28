// Copyright FlowVisualizer Plugin. All Rights Reserved.

#include "SFlowVisualizerPanel.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "FlowPipelineRegistry.h"
#include "FlowTracer.h"
#include "FlowVisualizerStyle.h"

#define LOCTEXT_NAMESPACE "FlowVisualizerPanel"

void SFlowVisualizerPanel::Construct(const FArguments& InArgs)
{
	bIsPaused = false;
	CurrentPipelineID = NAME_None;
	TestSignalAccumulator = 0.0;
	TestSignalNodeIndex = 0;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Slot 1 — Top toolbar (AutoHeight)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FlowVisStyle::TitleBarBg)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)

				// "Pipeline:" label
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PipelineLabel", "Pipeline:"))
					.ColorAndOpacity(FLinearColor::White)
				]

				// Pipeline combo box
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SAssignNew(PipelineComboBox, SComboBox<TSharedPtr<FName>>)
					.OptionsSource(&PipelineIDList)
					.OnSelectionChanged(this, &SFlowVisualizerPanel::OnPipelineSelected)
					.OnGenerateWidget(this, &SFlowVisualizerPanel::GeneratePipelineComboItem)
					[
						SNew(STextBlock)
						.Text(this, &SFlowVisualizerPanel::GetSelectedPipelineText)
					]
				]

				// Spacer
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[

					SNew(SSpacer)
				]

				// Pause button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() -> FReply
					{
						bIsPaused = !bIsPaused;
						UE_LOG(LogTemp, Log, TEXT("FlowVisualizer: Pause toggled -> %s"), bIsPaused ? TEXT("PAUSED") : TEXT("RESUMED"));
						Invalidate(EInvalidateWidgetReason::Paint);
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SFlowVisualizerPanel::GetPauseButtonText)))
					]
				]

				// Clear button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[

					SNew(SButton)
					.Text(LOCTEXT("ClearButton", "Clear"))
					.OnClicked_Lambda([this]() -> FReply
					{
						FFlowTracer::Get().ClearBuffer();
						ClearVisualization();
						UE_LOG(LogTemp, Log, TEXT("FlowVisualizer: Buffer and visualization cleared"));
						return FReply::Handled();
					})
				]
			]
		]

		// Slot 2 — Main canvas area (fill remaining space)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FlowVisStyle::Background)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.Padding(0.0f)
			[
				SNew(SOverlay)
				// Overlay layers will be added by subsequent tasks (P5.x)
			]
		]
	];

	// Register for registry changes and initialize the pipeline list
	RegistryChangedHandle = FFlowPipelineRegistry::Get().OnRegistryChanged.AddRaw(
		this, &SFlowVisualizerPanel::RefreshPipelineList);

	// Register for flow events from the runtime tracer
	FlowEventHandle = FFlowTracer::Get().OnFlowEvent.AddRaw(
		this, &SFlowVisualizerPanel::OnFlowEventReceived);

	RefreshPipelineList();
}

SFlowVisualizerPanel::~SFlowVisualizerPanel()
{
	FFlowPipelineRegistry::Get().OnRegistryChanged.Remove(RegistryChangedHandle);
	FFlowTracer::Get().OnFlowEvent.Remove(FlowEventHandle);
}

void SFlowVisualizerPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Step 1 — Call parent Tick
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Step 2 — Smooth FPS calculation (preserved from P4.3)
	if (InDeltaTime > 0.0f)
	{
		CachedFPS = FMath::Lerp(CachedFPS, 1.0f / InDeltaTime, 0.1f);
	}

	// ------------------------------------------------------------------
	// Step 2.5 — Built-in test signal emitter
	// Cycles through pipeline nodes every TestSignalInterval seconds,
	// simulating data flowing along the pipeline edges.
	// ------------------------------------------------------------------
	if (!bIsPaused)
	{
		TestSignalAccumulator += static_cast<double>(InDeltaTime);
		if (TestSignalAccumulator >= TestSignalInterval)
		{
			TestSignalAccumulator -= TestSignalInterval;

			const FFlowPipeline* Pipeline = FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
			if (Pipeline != nullptr && Pipeline->Nodes.Num() > 0)
			{
				// Wrap index around if pipeline changed
				if (TestSignalNodeIndex >= Pipeline->Nodes.Num())
				{
					TestSignalNodeIndex = 0;
				}

				const FFlowNode& TargetNode = Pipeline->Nodes[TestSignalNodeIndex];
				FFlowTracer::Get().Signal(TargetNode.NodeID, TEXT("TestData"));

				// Advance to next node in pipeline order
				TestSignalNodeIndex = (TestSignalNodeIndex + 1) % Pipeline->Nodes.Num();
			}
		}
	}

	// Step 3 — Check for active node highlight-fade animations
	const double CurrentTime = FPlatformTime::Seconds();
	bool bHasActiveAnimation = false;
	int32 TempActiveCount = 0;
	double TempMinElapsed = TNumericLimits<double>::Max();

	for (const auto& Pair : NodeLastActiveTime)
	{
		const double Elapsed = CurrentTime - Pair.Value;
		if (Elapsed < static_cast<double>(FlowVisStyle::HighlightFadeDuration))
		{
			bHasActiveAnimation = true;
			TempActiveCount++;
			TempMinElapsed = FMath::Min(TempMinElapsed, Elapsed);
		}
	}

	// Step 4 — Edge animation check (P7.1: check EdgeLastActiveTime for active edges)
	for (const auto& Pair : EdgeLastActiveTime)
	{
		const double Elapsed = CurrentTime - Pair.Value;
		if (Elapsed < static_cast<double>(FlowVisStyle::HighlightFadeDuration))
		{
			bHasActiveAnimation = true;
			break; // confirmed active animation, no need to continue
		}
	}

	// Step 5 — Update statistics
	ActiveSignalCount = TempActiveCount;
	if (TempActiveCount > 0)
	{
		LastSignalElapsed = TempMinElapsed * 1000.0; // convert to milliseconds
	}
	else
	{
		LastSignalElapsed = 0.0;
	}

	// Step 6 — Smart Invalidate: only repaint when animations are running
	UpdateNodeFrequencies();
	if (bHasActiveAnimation)
	{
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

FVector2D SFlowVisualizerPanel::NodeRelativeToPixel(const FVector2D& RelativePos, const FGeometry& Geometry) const
{
	const float CanvasWidth = Geometry.GetLocalSize().X;
	const float CanvasHeight = Geometry.GetLocalSize().Y;

	const float PixelX = RelativePos.X * (CanvasWidth - CanvasMargin * 2) + CanvasMargin;
	const float PixelY = RelativePos.Y * (CanvasHeight - TopBarHeight - StatusBarHeight - CanvasMargin) + TopBarHeight + CanvasMargin * 0.5f;

	FVector2D BasePos(PixelX, PixelY);
	return BasePos * CanvasZoom + CanvasOffset;
}

FVector2D SFlowVisualizerPanel::NodeRelativeToPixel(const FVector2D& RelativePos, const FVector2D& PanelSize) const
{
	const float CanvasWidth = PanelSize.X;
	const float CanvasHeight = PanelSize.Y;

	const float PixelX = RelativePos.X * (CanvasWidth - CanvasMargin * 2) + CanvasMargin;
	const float PixelY = RelativePos.Y * (CanvasHeight - TopBarHeight - StatusBarHeight - CanvasMargin) + TopBarHeight + CanvasMargin * 0.5f;

	FVector2D BasePos(PixelX, PixelY);
	return BasePos * CanvasZoom + CanvasOffset;
}

int32 SFlowVisualizerPanel::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	// ==================================================================
	// Step 1: Let child widgets render FIRST (including SBorder background)
	// This prevents the SBorder's opaque background from covering our
	// custom-drawn content.
	// ==================================================================
	int32 ChildLayerId = SCompoundWidget::OnPaint(
		Args, AllottedGeometry, MyCullingRect, OutDrawElements,
		LayerId, InWidgetStyle, bParentEnabled);

	// All custom drawing uses layers ABOVE child widgets
	int32 CustomBaseLayer = ChildLayerId + 1;

	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	// Current wall-clock time — computed once per frame for consistent highlight fade across all nodes
	const double CurrentTime = FPlatformTime::Seconds();

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	// NOTE: No manual background rectangle needed — the SBorder in Slot 2 already provides it.

	// ------------------------------------------------------------------
	// Fetch current pipeline (shared by edge and node drawing)
	// ------------------------------------------------------------------
	const FFlowPipeline* Pipeline = FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);

	// ------------------------------------------------------------------
	// Layer 1: Edge Drawing — render pipeline edges as lines with arrowheads
	// Drawn BEFORE nodes so that node text appears on top of lines.
	// P7.2: Dynamic highlight color + flow dot animation.
	// ------------------------------------------------------------------
	const int32 EdgeLayerId = CustomBaseLayer;
	const int32 FlowDotLayerId = EdgeLayerId + 1; // flow dots drawn above lines

	if (Pipeline != nullptr)
	{
		static const float EdgeThickness = 2.0f;
		static const float ArrowSize = 8.0f;
		static const float TextHalfHeight = 8.0f;      // Approximate half-height of 14pt Bold text
		static const float FixedTextHalfWidth = 50.0f;  // Fixed estimate for half of "[NodeName]" text width
		static const float EndpointInset = 10.0f;       // Gap between line endpoint and text edge
		static const float DotHalfSize = 3.0f;          // Half of the 6x6 flow dot

		const FPaintGeometry EdgePaintGeometry = AllottedGeometry.ToPaintGeometry();

		for (const FFlowEdge& Edge : Pipeline->Edges)
		{
			// Find source and target nodes by ID
			const FFlowNode* SourceNode = nullptr;
			const FFlowNode* TargetNode = nullptr;

			for (const FFlowNode& Node : Pipeline->Nodes)
			{
				if (Node.NodeID == Edge.SourceNodeID)
				{
					SourceNode = &Node;
				}
				if (Node.NodeID == Edge.TargetNodeID)
				{
					TargetNode = &Node;
				}
				if (SourceNode && TargetNode)
				{
					break;
				}
			}

			// Skip this edge if either endpoint node is missing
			if (!SourceNode || !TargetNode)
			{
				continue;
			}

			// Compute pixel positions of source and target nodes
			const FVector2D SourcePixel = NodeRelativeToPixel(SourceNode->RelativePosition, AllottedGeometry);
			const FVector2D TargetPixel = NodeRelativeToPixel(TargetNode->RelativePosition, AllottedGeometry);

			// Offset endpoints: source exits from right side of text, target enters from left side
			const FVector2D SourcePoint = SourcePixel + FVector2D(FixedTextHalfWidth + EndpointInset, TextHalfHeight);
			const FVector2D TargetPoint = TargetPixel + FVector2D(-EndpointInset, TextHalfHeight);

			// --- P7.2 Step 1: Look up edge activation state ---
			const FEdgeKey EdgeKey(Edge.SourceNodeID, Edge.TargetNodeID);
			const double* EdgeActivePtr = EdgeLastActiveTime.Find(EdgeKey);

			// --- P7.2 Step 2: Compute dynamic line color ---
			FLinearColor LineColor = FlowVisStyle::EdgeDefault;
			float EdgeAlpha = 0.0f; // used later for flow dot
			bool bEdgeAnimating = false;

			if (EdgeActivePtr != nullptr)
			{
				const double EdgeElapsed = CurrentTime - *EdgeActivePtr;
				if (EdgeElapsed < static_cast<double>(FlowVisStyle::HighlightFadeDuration))
				{
					EdgeAlpha = 1.0f - static_cast<float>(EdgeElapsed / static_cast<double>(FlowVisStyle::HighlightFadeDuration));
					LineColor = FMath::Lerp(FlowVisStyle::EdgeDefault, FlowVisStyle::EdgeActive, EdgeAlpha);
					bEdgeAnimating = true;
				}
			}

			// --- P7.2 Step 3: Draw the main line segment (dynamic color) ---
			TArray<FVector2D> LinePoints;
			LinePoints.Add(SourcePoint);
			LinePoints.Add(TargetPoint);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				EdgeLayerId,
				EdgePaintGeometry,
				LinePoints,
				ESlateDrawEffect::None,
				LineColor,
				true,           // bAntialias
				EdgeThickness
			);

			// --- Draw arrowhead at the target end (dynamic color) ---
			const FVector2D Dir = (TargetPoint - SourcePoint).GetSafeNormal();
			const FVector2D Perpendicular(-Dir.Y, Dir.X);

			const FVector2D Wing1 = TargetPoint - Dir * ArrowSize + Perpendicular * ArrowSize * 0.5f;
			const FVector2D Wing2 = TargetPoint - Dir * ArrowSize - Perpendicular * ArrowSize * 0.5f;

			// Arrow wing 1 -> tip
			TArray<FVector2D> ArrowLine1;
			ArrowLine1.Add(Wing1);
			ArrowLine1.Add(TargetPoint);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				EdgeLayerId,
				EdgePaintGeometry,
				ArrowLine1,
				ESlateDrawEffect::None,
				LineColor,
				true,
				EdgeThickness
			);

			// Arrow wing 2 -> tip
			TArray<FVector2D> ArrowLine2;
			ArrowLine2.Add(Wing2);
			ArrowLine2.Add(TargetPoint);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				EdgeLayerId,
				EdgePaintGeometry,
				ArrowLine2,
				ESlateDrawEffect::None,
				LineColor,
				true,
				EdgeThickness
			);

			// --- P7.2 Step 4: Draw flow dot (only on active edges) ---
			if (bEdgeAnimating)
			{
				const float EdgeLength = FVector2D::Distance(SourcePoint, TargetPoint);
				if (EdgeLength > 0.0f)
				{
					const double EdgeElapsed = CurrentTime - *EdgeActivePtr;
					const float Progress = FMath::Fmod(
						static_cast<float>(EdgeElapsed * static_cast<double>(FlowVisStyle::FlowDotSpeed) / static_cast<double>(EdgeLength)),
						1.0f);
					const FVector2D DotPos = FMath::Lerp(SourcePoint, TargetPoint, Progress);

					FLinearColor DotColor = FlowVisStyle::FlowDot;
					DotColor.A = EdgeAlpha;

					const FVector2D DotOrigin = DotPos - FVector2D(DotHalfSize, DotHalfSize);
					const FVector2D DotSize(DotHalfSize * 2.0f, DotHalfSize * 2.0f);

					FSlateDrawElement::MakeBox(
						OutDrawElements,
						FlowDotLayerId,
						AllottedGeometry.ToPaintGeometry(DotOrigin, DotSize),
						WhiteBrush,
						ESlateDrawEffect::None,
						DotColor
					);
				}
			}
		}
	}

	// ------------------------------------------------------------------
	// Layer 2: Node Drawing — render pipeline nodes as terminal-style "[NodeName]"
	// ------------------------------------------------------------------
	int32 NodeLayerId = FlowDotLayerId + 1;

	if (Pipeline != nullptr)
	{
		const FSlateFontInfo NodeFontInfo = FlowVisStyle::NodeTitleFont();

		for (const FFlowNode& Node : Pipeline->Nodes)
		{
			const FVector2D PixelPos = NodeRelativeToPixel(Node.RelativePosition, AllottedGeometry);

			// Terminal-style display: "[NodeName]"
			const FString NodeText = FString::Printf(TEXT("[%s]"), *Node.DisplayName.ToString());

			// --- P6.2: Dynamic highlight fade color ---
			FLinearColor TextColor = FlowVisStyle::NodeInactive;
			const double* LastActivePtr = NodeLastActiveTime.Find(Node.NodeID);
			if (LastActivePtr != nullptr)
			{
				const double Elapsed = CurrentTime - *LastActivePtr;
				if (Elapsed < static_cast<double>(FlowVisStyle::HighlightFadeDuration))
				{
					const float Alpha = 1.0f - static_cast<float>(Elapsed / static_cast<double>(FlowVisStyle::HighlightFadeDuration));

					// --- PF.9: FMOD node semantic color override ---
					FLinearColor ActiveTarget = FlowVisStyle::NodeActive;
					const FString NodeIDStr = Node.NodeID.ToString();
					if (NodeIDStr.StartsWith(TEXT("FMOD_")))
					{
						if (NodeIDStr == TEXT("FMOD_StartFailed"))
						{
							ActiveTarget = FlowVisStyle::FMODError;
						}
						else if (NodeIDStr == TEXT("FMOD_Virtualized"))
						{
							ActiveTarget = FlowVisStyle::FMODWarning;
						}
						else if (NodeIDStr == TEXT("FMOD_Restored"))
						{
							ActiveTarget = FlowVisStyle::FMODRecovery;
						}
						else if (NodeIDStr.Contains(TEXT("Marker")) || NodeIDStr.Contains(TEXT("Beat")))
						{
							ActiveTarget = FlowVisStyle::FMODTimeline;
						}
						else
						{
							ActiveTarget = FlowVisStyle::FMODLifecycle;
						}
					}

					TextColor = FMath::Lerp(FlowVisStyle::NodeInactive, ActiveTarget, Alpha);
				}
			}

			// Create a child geometry at the node's pixel position for text placement
			const FGeometry NodeGeometry = AllottedGeometry.MakeChild(
				FVector2D(200.0f, 30.0f),  // Size — generous width for text
				FSlateLayoutTransform(PixelPos)
			);

			FSlateDrawElement::MakeText(
				OutDrawElements,
				NodeLayerId,
				NodeGeometry.ToPaintGeometry(),
				NodeText,
				NodeFontInfo,
				ESlateDrawEffect::None,
				TextColor
			);

			// --- Scope node expand arrow indicator ---
			if (IsScopeNode(Node.NodeID))
			{
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(
						FVector2D(PixelPos.X + 50.0f, PixelPos.Y - 8.0f),
						FVector2D(20.0f, 20.0f)),
					FText::FromString(TEXT(">")),
					FlowVisStyle::NodeTitleFont(),
					ESlateDrawEffect::None,
					FLinearColor(0.3f, 0.8f, 1.0f, 1.0f)
				);
			}

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
						FVector2D(PixelPos.X - 30.0f, PixelPos.Y + 14.0f),
						FVector2D(80.0f, 16.0f)),
					FText::FromString(FreqText),
					FlowVisStyle::DataValueFont(),
					ESlateDrawEffect::None,
					HeatColor);
			}

			// --- P6.3: Data label drawing below node title ---
			if (LastActivePtr != nullptr)
			{
				const double Elapsed = CurrentTime - *LastActivePtr;
				if (Elapsed < static_cast<double>(FlowVisStyle::HighlightFadeDuration))
				{
					const FString* DataPtr = NodeLastData.Find(Node.NodeID);
					if (DataPtr != nullptr && !DataPtr->IsEmpty())
					{
						// Truncate to 50 characters
						FString DisplayData = DataPtr->Left(50);

						// Split by spaces into fields
						TArray<FString> Fields;
						DisplayData.ParseIntoArray(Fields, TEXT(" "), true);

						// Cap at 4 lines
						const int32 MaxLines = FMath::Min(Fields.Num(), 4);

						// Alpha fades in sync with node highlight
						const float DataAlpha = 1.0f - static_cast<float>(Elapsed / static_cast<double>(FlowVisStyle::HighlightFadeDuration));
						FLinearColor DataColor = FlowVisStyle::DataValue;
						DataColor.A = DataAlpha;

						const FSlateFontInfo DataFontInfo = FlowVisStyle::DataValueFont();
						const float DataStartY = PixelPos.Y + 20.0f; // Node title ~18px + 2px spacing

						for (int32 LineIndex = 0; LineIndex < MaxLines; ++LineIndex)
						{
							const FVector2D DataLinePos(PixelPos.X, DataStartY + LineIndex * 14.0f);

							const FGeometry DataGeometry = AllottedGeometry.MakeChild(
								FVector2D(300.0f, 16.0f),
								FSlateLayoutTransform(DataLinePos)
							);

							FSlateDrawElement::MakeText(
								OutDrawElements,
								NodeLayerId,
								DataGeometry.ToPaintGeometry(),
								Fields[LineIndex],
								DataFontInfo,
								ESlateDrawEffect::None,
								DataColor
							);
						}
					}
				}
			}
		}

		NodeLayerId += 1;
	}

	// ------------------------------------------------------------------
	// Breadcrumb navigation (drawn above status bar when inside a scope)
	// ------------------------------------------------------------------
	if (!CurrentScopePath.IsEmpty())
	{
		FString BreadCrumb = FString::Printf(
			TEXT("< Back | Scope: %s"), *CurrentScopePath);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 5,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(10.0f, 5.0f),
				FVector2D(500.0f, 20.0f)),
			FText::FromString(BreadCrumb),
			FlowVisStyle::StatusBarFont(),
			ESlateDrawEffect::None,
			FLinearColor(0.3f, 0.8f, 1.0f, 0.9f)
		);
	}

	// ------------------------------------------------------------------
	// Layer 3+4: Status Bar — drawn on top of everything else
	// ------------------------------------------------------------------
	const int32 StatusBarLayerId = NodeLayerId + 1;

	// Step 1: Calculate status bar region
	const float StatusBarY = LocalSize.Y - StatusBarHeight;
	const FVector2D StatusBarPos(0.0f, StatusBarY);
	const FVector2D StatusBarSize(LocalSize.X, StatusBarHeight);

	// Step 2: Draw status bar background (Layer 3)
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		StatusBarLayerId,
		AllottedGeometry.ToPaintGeometry(StatusBarPos, StatusBarSize),
		WhiteBrush,
		ESlateDrawEffect::None,
		FlowVisStyle::TitleBarBg
	);

	// Step 3: Assemble status text
	int32 TotalNodeCount = 0;
	{
		const FFlowPipeline* StatusPipeline = FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
		if (StatusPipeline)
		{
			TotalNodeCount = StatusPipeline->Nodes.Num();
		}
	}

	const FString LastStr = (ActiveSignalCount > 0)
		? FString::Printf(TEXT("%dms ago"), FMath::RoundToInt(static_cast<float>(LastSignalElapsed)))
		: TEXT("--");

	const int32 BufferUsed = FFlowTracer::Get().GetBufferUsed();
	const int32 BufferCap = FFlowTracer::Get().GetBufferCapacity();

	FString StatusTextStr = FString::Printf(
		TEXT("Frame: %u | FPS: %d | Active: %d/%d | Last: %s | Buffer: %d/%d"),
		GFrameNumber,
		FMath::RoundToInt(CachedFPS),
		ActiveSignalCount,
		TotalNodeCount,
		*LastStr,
		BufferUsed,
		BufferCap
	);

	if (bIsPaused)
	{
		StatusTextStr += TEXT(" | ");
	}

	// Step 4: Draw status text (Layer 4) — vertically centered, left-padded 10px
	const FSlateFontInfo StatusFontInfo = FlowVisStyle::StatusBarFont();

	const float TextVerticalOffset = (StatusBarHeight - 10.0f) * 0.5f; // approximate centering for 10pt font
	const FVector2D TextPos(10.0f, StatusBarY + TextVerticalOffset);

	FSlateDrawElement::MakeText(
		OutDrawElements,
		StatusBarLayerId + 1,
		AllottedGeometry.ToPaintGeometry(TextPos, FVector2D(LocalSize.X - 10.0f, StatusBarHeight)),
		StatusTextStr,
		StatusFontInfo,
		ESlateDrawEffect::None,
		FlowVisStyle::StatusText
	);

	// Step 5: Draw "PAUSED" indicator in warning color when paused
	if (bIsPaused)
	{
		const FString PausedStr = TEXT("PAUSED");
		// Approximate character width ~7px at 10pt monospace
		const float ApproxStatusTextWidth = StatusTextStr.Len() * 7.0f + 10.0f;
		const FVector2D PausedPos(ApproxStatusTextWidth, StatusBarY + TextVerticalOffset);

		FSlateDrawElement::MakeText(
			OutDrawElements,
			StatusBarLayerId + 1,
			AllottedGeometry.ToPaintGeometry(PausedPos, FVector2D(200.0f, StatusBarHeight)),
			PausedStr,
			StatusFontInfo,
			ESlateDrawEffect::None,
			FlowVisStyle::Warning
		);
	}

	return StatusBarLayerId + 1;
}

void SFlowVisualizerPanel::RefreshPipelineList()
{
	TArray<FName> AllIDs = FFlowPipelineRegistry::Get().GetAllPipelineIDs();

	PipelineIDList.Empty();
	for (const FName& ID : AllIDs)
	{
		PipelineIDList.Add(MakeShareable(new FName(ID)));
	}

	// Check if current selection is still valid
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
			NewSelectedItem = nullptr;
			CurrentPipelineID = NAME_None;
		}
	}

	SelectedPipelineItem = NewSelectedItem;

	// Update combo box selection
	if (PipelineComboBox.IsValid())
	{
		PipelineComboBox->RefreshOptions();
		PipelineComboBox->SetSelectedItem(SelectedPipelineItem);
	}
}

void SFlowVisualizerPanel::OnPipelineSelected(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedPipelineItem = NewSelection;

	if (NewSelection.IsValid())
	{
		CurrentPipelineID = *NewSelection;
		UE_LOG(LogTemp, Log, TEXT("FlowVisualizer: Pipeline selected -> %s"), *CurrentPipelineID.ToString());
	}
	else
	{
		CurrentPipelineID = NAME_None;
	}

	// Reset test signal state when pipeline changes
	TestSignalNodeIndex = 0;
	TestSignalAccumulator = 0.0;
}

TSharedRef<SWidget> SFlowVisualizerPanel::GeneratePipelineComboItem(TSharedPtr<FName> Item)
{
	FString ItemText = Item.IsValid() ? Item->ToString() : TEXT("(None)");
	return SNew(STextBlock).Text(FText::FromString(ItemText));
}

FText SFlowVisualizerPanel::GetSelectedPipelineText() const
{
	if (SelectedPipelineItem.IsValid())
	{
		return FText::FromString(SelectedPipelineItem->ToString());
	}
	return LOCTEXT("NoPipeline", "(None)");
}

void SFlowVisualizerPanel::OnFlowEventReceived(const FFlowEvent& Event)
{
	// ------------------------------------------------------------------
	// Scope detection: intercept __ENTER__ / __EXIT__ meta-signals
	// ------------------------------------------------------------------
	FString NodeIDStr = Event.NodeID.ToString();
	if (NodeIDStr.EndsWith(TEXT(".__ENTER__")))
	{
		FString ScopeName = NodeIDStr.LeftChop(10);
		ScopeNodes.Add(FName(*ScopeName));
		return;
	}
	if (NodeIDStr.EndsWith(TEXT(".__EXIT__")))
	{
		return;
	}

	// Scope path filter: if both the event and the panel have scope paths,
	// discard events that don't belong to the current scope
	if (!Event.Scope.IsEmpty() && !CurrentScopePath.IsEmpty())
	{
		if (!Event.Scope.StartsWith(CurrentScopePath))
		{
			return;
		}
	}

	// Step 1 — Pause check: discard events while paused
	if (bIsPaused)
	{
		return;
	}

	// Step 2 — Sampling throttle: within SignalThrottleMs window, only update data (not timestamp)
	const double* ExistingTime = NodeLastActiveTime.Find(Event.NodeID);
	if (ExistingTime != nullptr)
	{
		const double ElapsedSec = Event.Timestamp - *ExistingTime;
		if (ElapsedSec < static_cast<double>(FlowVisStyle::SignalThrottleMs) / 1000.0)
		{
			// Within throttle window — update data payload only, skip timestamp reset
			NodeLastData.Add(Event.NodeID, Event.DataPayload);
			return;
		}
	}

	// === 自动注册蓝图信号节点 ===
	static const FName AutoPipelineID = TEXT("Blueprint_Auto");
	static int32 AutoNodeCount = 0;
	static FName LastAutoNodeID = NAME_None;
	static double LastAutoSignalTime = 0.0;

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

	// 如果节点不存在于当前管线，自动添加到 Blueprint_Auto
	if (!bNodeExists)
	{
		// 检查是否在 Blueprint_Auto 中已存在
		const FFlowPipeline* AutoPipeline =
			FFlowPipelineRegistry::Get().FindPipeline(AutoPipelineID);
		bool bExistsInAuto = false;
		if (AutoPipeline)
		{
			for (const FFlowNode& Node : AutoPipeline->Nodes)
			{
				if (Node.NodeID == Event.NodeID)
				{
					bExistsInAuto = true;
					break;
				}
			}
		}

		if (!bExistsInAuto)
		{
			// 自动排列：每行 5 个节点，间距均匀
			int32 Col = AutoNodeCount % 5;
			int32 Row = AutoNodeCount / 5;
			FVector2D AutoPos(
				0.1f + Col * 0.2f,
				0.1f + Row * 0.2f
			);

			FFlowPipelineRegistry::Get().AddNodeToPipeline(
				AutoPipelineID,
				Event.NodeID,
				FText::FromName(Event.NodeID),
				AutoPos);

			// 自动推断边：基于时间窗口（500ms 内的连续信号才连边）
			if (LastAutoNodeID != NAME_None && LastAutoNodeID != Event.NodeID)
			{
				double TimeSinceLast = Event.Timestamp - LastAutoSignalTime;
				if (TimeSinceLast < 0.5)
				{
					FFlowPipelineRegistry::Get().AddEdgeToPipeline(
						AutoPipelineID,
						LastAutoNodeID,
						Event.NodeID);
				}
			}

			AutoNodeCount++;
		}

		LastAutoNodeID = Event.NodeID;
		LastAutoSignalTime = Event.Timestamp;
	}
	// === 自动注册结束 ===

	// Step 3 — Normal update: record timestamp and data
	NodeLastActiveTime.Add(Event.NodeID, Event.Timestamp);
	NodeLastData.Add(Event.NodeID, Event.DataPayload);

	// 记录信号时间戳用于频率统计
	NodeSignalHistory.FindOrAdd(Event.NodeID).Add(Event.Timestamp);
	NodeSignalCount.FindOrAdd(Event.NodeID)++;

	// ------------------------------------------------------------------
	// Step 4 — P7.1: Edge activation inference
	// When a target node receives a signal, check whether the source node
	// of each incoming edge also fired within EdgeInferenceWindowSec.
	// ------------------------------------------------------------------
	const FFlowPipeline* Pipeline = FFlowPipelineRegistry::Get().FindPipeline(CurrentPipelineID);
	if (Pipeline != nullptr)
	{
		for (const FFlowEdge& Edge : Pipeline->Edges)
		{
			if (Edge.TargetNodeID == Event.NodeID)
			{
				const double* SourceActivePtr = NodeLastActiveTime.Find(Edge.SourceNodeID);
				if (SourceActivePtr != nullptr)
				{
					const double SourceElapsed = Event.Timestamp - *SourceActivePtr;
					if (SourceElapsed >= 0.0 && SourceElapsed < EdgeInferenceWindowSec)
					{
						FEdgeKey EdgeKey(Edge.SourceNodeID, Edge.TargetNodeID);
						EdgeLastActiveTime.Add(EdgeKey, Event.Timestamp);
					}
				}
			}
		}
	}
}

FText SFlowVisualizerPanel::GetPauseButtonText() const
{
	return bIsPaused
		? LOCTEXT("ResumeButton", "Resume")
		: LOCTEXT("PauseButton", "Pause");
}

void SFlowVisualizerPanel::ClearVisualization()
{
	NodeLastActiveTime.Empty();
	NodeLastData.Empty();
	EdgeLastActiveTime.Empty();
	NodeSignalCount.Empty();
	NodeSignalHistory.Empty();
	NodeFrequency.Empty();
	ActiveSignalCount = 0;
	LastSignalElapsed = 0.0;
	TestSignalNodeIndex = 0;
	TestSignalAccumulator = 0.0;
	Invalidate(EInvalidateWidgetReason::Paint);
}

FReply SFlowVisualizerPanel::OnMouseButtonDown(
	const FGeometry& MyGeometry,
	const FPointerEvent& MouseEvent)
{
	// 中键开始平移
	if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
	{
		bIsPanning = true;
		PanStartMousePos = MyGeometry.AbsoluteToLocal(
			MouseEvent.GetScreenSpacePosition());
		PanStartOffset = CanvasOffset;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

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

	// ...existing code for right mouse button...
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (!CurrentScopePath.IsEmpty())
		{
			ExitScope();
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

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

FReply SFlowVisualizerPanel::OnMouseMove(
	const FGeometry& MyGeometry,
	const FPointerEvent& MouseEvent)
{
	if (bIsPanning)
	{
		FVector2D CurrentMouse = MyGeometry.AbsoluteToLocal(
			MouseEvent.GetScreenSpacePosition());
		CanvasOffset = PanStartOffset + (CurrentMouse - PanStartMousePos);
		Invalidate(EInvalidateWidgetReason::Paint);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

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
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	return FReply::Handled();
}

void SFlowVisualizerPanel::EnterScope(const FString& ScopeName)
{
	ScopeHistory.Add(CurrentScopePath);
	CurrentScopePath = CurrentScopePath.IsEmpty()
		? ScopeName
		: FString::Printf(TEXT("%s/%s"), *CurrentScopePath, *ScopeName);
	RebuildFilteredView();
}

void SFlowVisualizerPanel::ExitScope()
{
	if (ScopeHistory.Num() > 0)
	{
		CurrentScopePath = ScopeHistory.Pop();
		RebuildFilteredView();
	}
}

void SFlowVisualizerPanel::RebuildFilteredView()
{
	ClearVisualization();
}

bool SFlowVisualizerPanel::IsScopeNode(FName NodeID) const
{
	return ScopeNodes.Contains(NodeID);
}

#undef LOCTEXT_NAMESPACE

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
