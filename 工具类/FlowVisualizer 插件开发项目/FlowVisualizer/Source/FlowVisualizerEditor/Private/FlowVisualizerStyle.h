// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

/**
 * FlowVisStyle — Centralized visual constants for the FlowVisualizer plugin.
 *
 * All colors, font specs, and animation parameters are defined here
 * so that the rest of the plugin references named constants instead of
 * hard-coded magic numbers.
 */
namespace FlowVisStyle
{
	// ------------------------------------------------------------------
	// Colors — Background
	// ------------------------------------------------------------------

	/**
	 * Main canvas background — target on-screen ≈ #0D0D14
	 *
	 * Slate treats FLinearColor as linear-space and applies gamma 2.2
	 * before display.  To land on sRGB ~(13,13,20) we need the *inverse*:
	 *   linear = (sRGB/255)^2.2
	 *   13/255 ≈ 0.051 → 0.051^2.2 ≈ 0.002
	 *   20/255 ≈ 0.078 → 0.078^2.2 ≈ 0.004
	 * After gamma: 0.002^(1/2.2) ≈ 0.05 → RGB ≈ (13,13,15) — near-black.
	 */
	const FLinearColor Background(0.002f, 0.002f, 0.003f, 1.0f);

	/**
	 * Title bar / status bar background — target on-screen ≈ #1A1A25
	 *   26/255 ≈ 0.102 → 0.102^2.2 ≈ 0.008
	 *   37/255 ≈ 0.145 → 0.145^2.2 ≈ 0.016
	 */
	const FLinearColor TitleBarBg(0.008f, 0.008f, 0.012f, 1.0f);

	// ------------------------------------------------------------------
	// Colors — Nodes
	// ------------------------------------------------------------------

	/** Active (recently fired) node text #33CCFF */
	const FLinearColor NodeActive(0.20f, 0.80f, 1.00f, 1.0f);

	/** Inactive / idle node text #666666 */
	const FLinearColor NodeInactive(0.40f, 0.40f, 0.40f, 1.0f);

	/** Data value label below a node #4AFF82 */
	const FLinearColor DataValue(0.29f, 1.00f, 0.51f, 1.0f);

	// ------------------------------------------------------------------
	// Colors — Edges
	// ------------------------------------------------------------------

	/** Default (idle) edge line #888888 */
	const FLinearColor EdgeDefault(0.53f, 0.53f, 0.53f, 1.0f);

	/** Active edge line (signal flowing) #33CCFF */
	const FLinearColor EdgeActive(0.20f, 0.80f, 1.00f, 1.0f);

	/** Flow dot (travelling pulse on edge) #FFFFFF */
	const FLinearColor FlowDot(1.00f, 1.00f, 1.00f, 1.0f);

	// ------------------------------------------------------------------
	// Colors — UI Chrome
	// ------------------------------------------------------------------

	/** Panel border / separator #333340 */
	const FLinearColor Border(0.20f, 0.20f, 0.25f, 1.0f);

	/** Status bar text #999999 */
	const FLinearColor StatusText(0.60f, 0.60f, 0.60f, 1.0f);

	/** Warning / error accent #FF6644 */
	const FLinearColor Warning(1.00f, 0.40f, 0.27f, 1.0f);

	// ------------------------------------------------------------------
	// Colors — Scope Navigation
	// ------------------------------------------------------------------

	/** Scope 节点展开箭头颜色 */
	static FLinearColor ScopeArrowColor() { return FLinearColor(0.3f, 0.8f, 1.0f, 1.0f); }

	/** 面包屑导航文字颜色 */
	static FLinearColor BreadCrumbColor() { return FLinearColor(0.3f, 0.8f, 1.0f, 0.9f); }

	/** Scope 节点边框颜色 */
	static FLinearColor ScopeNodeBorder() { return FLinearColor(0.2f, 0.6f, 0.9f, 0.5f); }

	// ------------------------------------------------------------------
	// Colors — Heatmap (node signal frequency)
	// ------------------------------------------------------------------

	/** 热力图 - 休眠 */
	static FLinearColor HeatDormant() { return FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); }

	/** 热力图 - 正常 */
	static FLinearColor HeatNormal() { return FLinearColor(0.1f, 0.8f, 0.2f, 1.0f); }

	/** 热力图 - 较忙 */
	static FLinearColor HeatBusy() { return FLinearColor(0.9f, 0.9f, 0.1f, 1.0f); }

	/** 热力图 - 很忙 */
	static FLinearColor HeatHeavy() { return FLinearColor(0.9f, 0.5f, 0.1f, 1.0f); }

	/** 热力图 - 过热 */
	static FLinearColor HeatCritical() { return FLinearColor(1.0f, 0.15f, 0.1f, 1.0f); }

	// ------------------------------------------------------------------
	// Colors — FMOD Semantic (used by FMOD node visualization)
	// ------------------------------------------------------------------

	/** FMOD error — bright red #FF4545 — for StartFailed */
	const FLinearColor FMODError(1.00f, 0.27f, 0.27f, 1.0f);

	/** FMOD warning — orange #FFA500 — for Virtualized (Voice Stolen) */
	const FLinearColor FMODWarning(1.00f, 0.65f, 0.00f, 1.0f);

	/** FMOD recovery — green #33E666 — for Restored */
	const FLinearColor FMODRecovery(0.20f, 0.90f, 0.40f, 1.0f);

	/** FMOD lifecycle — light blue #99B3FF — for Created/Started/Stopped */
	const FLinearColor FMODLifecycle(0.60f, 0.70f, 1.00f, 1.0f);

	/** FMOD timeline — gold #FFD94D — for Marker/Beat */
	const FLinearColor FMODTimeline(1.00f, 0.85f, 0.30f, 1.0f);

	// ------------------------------------------------------------------
	// Fonts (implemented in .cpp — FCoreStyle calls are non-trivial)
	// ------------------------------------------------------------------

	/** Roboto Bold 14pt — used for node titles "[Name]" */
	FSlateFontInfo NodeTitleFont();

	/** Roboto Mono 11pt — used for data value labels */
	FSlateFontInfo DataValueFont();

	/** Roboto Mono 10pt — used for the bottom status bar */
	FSlateFontInfo StatusBarFont();

	/** Roboto Bold 12pt — used for the pipeline title in the top bar */
	FSlateFontInfo PipelineTitleFont();

	// ------------------------------------------------------------------
	// Animation / Behaviour Constants
	// ------------------------------------------------------------------

	/** Node highlight fade-out duration in seconds */
	const float HighlightFadeDuration = 1.5f;

	/** Flow dot travel speed in pixels per second */
	const float FlowDotSpeed = 200.0f;

	/** Per-node signal throttle window in milliseconds */
	const float SignalThrottleMs = 100.0f;
}
