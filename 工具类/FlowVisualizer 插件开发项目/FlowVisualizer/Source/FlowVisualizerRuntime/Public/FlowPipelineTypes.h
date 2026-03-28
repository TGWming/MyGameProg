// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * FFlowEvent — A single runtime event captured from the data flow.
 * Represents one signal occurrence with timestamp, frame, source node, and payload.
 */
struct FLOWVISUALIZERRUNTIME_API FFlowEvent
{
	/** Event timestamp, typically from FPlatformTime::Seconds() */
	double Timestamp;

	/** Engine frame number when the event occurred, typically from GFrameNumber */
	uint64 FrameNumber;

	/** ID of the node that triggered this event, e.g. "Movement" */
	FName NodeID;

	/** Free-form data string carried by the event */
	FString DataPayload;

	/** 所属作用域路径，如 "BP_Player/CombatLogic"，空字符串表示顶层 */
	FString Scope;

	FFlowEvent()
		: Timestamp(0.0)
		, FrameNumber(0)
		, NodeID(NAME_None)
	{
	}
};

/**
 * FFlowNode — A node in a flow pipeline definition.
 * Describes a logical processing point with a display name and canvas position.
 */
struct FLOWVISUALIZERRUNTIME_API FFlowNode
{
	/** Unique identifier for the node */
	FName NodeID;

	/** Human-readable display name */
	FText DisplayName;

	/** Relative position on the canvas, each component in the range [0, 1] */
	FVector2D RelativePosition;

	FFlowNode()
		: NodeID(NAME_None)
		, DisplayName()
		, RelativePosition(FVector2D::ZeroVector)
	{
	}
};

/**
 * FFlowEdge — A directed connection between two nodes in a flow pipeline.
 */
struct FLOWVISUALIZERRUNTIME_API FFlowEdge
{
	/** Node ID of the connection source */
	FName SourceNodeID;

	/** Node ID of the connection target */
	FName TargetNodeID;

	FFlowEdge()
		: SourceNodeID(NAME_None)
		, TargetNodeID(NAME_None)
	{
	}
};

/**
 * FFlowPipeline — A complete pipeline definition containing nodes and edges.
 * Represents one logical data-flow graph that can be visualized.
 */
struct FLOWVISUALIZERRUNTIME_API FFlowPipeline
{
	/** Unique identifier for the pipeline */
	FName PipelineID;

	/** Human-readable display name */
	FText DisplayName;

	/** All nodes belonging to this pipeline */
	TArray<FFlowNode> Nodes;

	/** All edges (connections) belonging to this pipeline */
	TArray<FFlowEdge> Edges;

	FFlowPipeline()
		: PipelineID(NAME_None)
		, DisplayName()
	{
	}
};
