// Copyright FlowVisualizer Plugin. All Rights Reserved.

#include "FlowPipelineRegistry.h"

FFlowPipelineRegistry& FFlowPipelineRegistry::Get()
{
	static FFlowPipelineRegistry Instance;
	return Instance;
}

FFlowPipelineRegistry::FFlowPipelineRegistry()
{
}

FFlowPipelineRegistry::~FFlowPipelineRegistry()
{
}

void FFlowPipelineRegistry::RegisterPipeline(const FFlowPipeline& Pipeline)
{
	RegisteredPipelines.Add(Pipeline.PipelineID, Pipeline);
	OnRegistryChanged.Broadcast();
}

void FFlowPipelineRegistry::UnregisterPipeline(FName PipelineID)
{
	if (RegisteredPipelines.Remove(PipelineID) > 0)
	{
		OnRegistryChanged.Broadcast();
	}
}

const FFlowPipeline* FFlowPipelineRegistry::FindPipeline(FName PipelineID) const
{
	return RegisteredPipelines.Find(PipelineID);
}

TArray<FName> FFlowPipelineRegistry::GetAllPipelineIDs() const
{
	TArray<FName> Result;
	RegisteredPipelines.GetKeys(Result);
	return Result;
}

void FFlowPipelineRegistry::EnsurePipelineExists(FName PipelineID, const FText& DisplayName)
{
	if (RegisteredPipelines.Contains(PipelineID))
	{
		return;
	}

	FFlowPipeline NewPipeline;
	NewPipeline.PipelineID = PipelineID;
	NewPipeline.DisplayName = DisplayName.IsEmpty()
		? FText::FromName(PipelineID)
		: DisplayName;

	RegisteredPipelines.Add(PipelineID, NewPipeline);
	OnRegistryChanged.Broadcast();
}

bool FFlowPipelineRegistry::AddNodeToPipeline(FName PipelineID, FName NodeID, const FText& DisplayName, FVector2D RelativePosition)
{
	EnsurePipelineExists(PipelineID);

	FFlowPipeline* Pipeline = RegisteredPipelines.Find(PipelineID);
	if (!Pipeline) return false;

	// Skip if NodeID already exists
	for (const FFlowNode& Node : Pipeline->Nodes)
	{
		if (Node.NodeID == NodeID) return false;
	}

	FFlowNode NewNode;
	NewNode.NodeID = NodeID;
	NewNode.DisplayName = DisplayName;
	NewNode.RelativePosition = RelativePosition;
	Pipeline->Nodes.Add(NewNode);

	OnRegistryChanged.Broadcast();
	return true;
}

bool FFlowPipelineRegistry::AddEdgeToPipeline(FName PipelineID, FName SourceNodeID, FName TargetNodeID)
{
	FFlowPipeline* Pipeline = RegisteredPipelines.Find(PipelineID);
	if (!Pipeline) return false;

	// Skip if edge already exists
	for (const FFlowEdge& Edge : Pipeline->Edges)
	{
		if (Edge.SourceNodeID == SourceNodeID && Edge.TargetNodeID == TargetNodeID)
		{
			return false;
		}
	}

	FFlowEdge NewEdge;
	NewEdge.SourceNodeID = SourceNodeID;
	NewEdge.TargetNodeID = TargetNodeID;
	Pipeline->Edges.Add(NewEdge);

	OnRegistryChanged.Broadcast();
	return true;
}
