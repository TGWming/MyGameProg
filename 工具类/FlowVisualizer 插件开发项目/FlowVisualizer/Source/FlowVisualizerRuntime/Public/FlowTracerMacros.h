// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "FlowTracer.h"

/**
 * Convenience macros for instrumenting game code with flow events.
 *
 * In Shipping builds all macros expand to nothing (zero overhead).
 *
 * Usage examples:
 *   FLOW_SIGNAL("Movement", SomeDataFString);
 *   FLOW_SIGNAL_SIMPLE("PlayerInput");
 *   FLOW_SIGNAL_FMT("Combat", "Type=%s Dmg=%d", *TypeStr, Damage);
 */

#if !UE_BUILD_SHIPPING

/**
 * Send a flow event with an explicit data payload (FString).
 *
 * @param NodeID  String literal identifying the node, e.g. "Movement"
 * @param Data    FString expression carrying the event payload
 */
#define FLOW_SIGNAL(NodeID, Data) \
	FFlowTracer::Get().Signal(FName(TEXT(NodeID)), Data)

/**
 * Send a flow event with no data payload.
 *
 * @param NodeID  String literal identifying the node, e.g. "PlayerInput"
 */
#define FLOW_SIGNAL_SIMPLE(NodeID) \
	FFlowTracer::Get().Signal(FName(TEXT(NodeID)))

/**
 * Send a flow event with a printf-style formatted data payload.
 *
 * @param NodeID  String literal identifying the node, e.g. "Combat"
 * @param Fmt     printf-style format string literal, e.g. "Val=%d"
 * @param ...     Format arguments
 */
#define FLOW_SIGNAL_FMT(NodeID, Fmt, ...) \
	FFlowTracer::Get().Signal(FName(TEXT(NodeID)), FString::Printf(TEXT(Fmt), ##__VA_ARGS__))

#else // UE_BUILD_SHIPPING

#define FLOW_SIGNAL(NodeID, Data)          do {} while(0)
#define FLOW_SIGNAL_SIMPLE(NodeID)         do {} while(0)
#define FLOW_SIGNAL_FMT(NodeID, Fmt, ...)  do {} while(0)

#endif // !UE_BUILD_SHIPPING
