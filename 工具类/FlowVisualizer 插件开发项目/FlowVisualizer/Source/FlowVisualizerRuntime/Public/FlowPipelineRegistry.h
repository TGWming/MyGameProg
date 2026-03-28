// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlowPipelineTypes.h"

/** Delegate broadcast when the pipeline registry changes (register / unregister). */
DECLARE_MULTICAST_DELEGATE(FOnRegistryChanged);

/**
 * FFlowPipelineRegistry — Global singleton registry for flow pipeline definitions.
 *
 * Game code and editor modules register / query pipeline topology through this class.
 * The editor panel listens to OnRegistryChanged to refresh its pipeline list.
 */
class FLOWVISUALIZERRUNTIME_API FFlowPipelineRegistry
{
public:
	/**
	 * Returns the singleton instance (Meyer's Singleton).
	 */
	static FFlowPipelineRegistry& Get();

	/**
	 * Register (or overwrite) a pipeline definition.
	 *
	 * @param Pipeline  The pipeline to register. Pipeline.PipelineID is used as the key.
	 */
	void RegisterPipeline(const FFlowPipeline& Pipeline);

	/**
	 * Remove a previously registered pipeline.
	 *
	 * @param PipelineID  The ID of the pipeline to remove.
	 */
	void UnregisterPipeline(FName PipelineID);

	/**
	 * Look up a pipeline by its ID.
	 *
	 * @param PipelineID  The ID to search for.
	 * @return Pointer to the pipeline if found, nullptr otherwise.
	 */
	const FFlowPipeline* FindPipeline(FName PipelineID) const;

	/**
	 * Get the IDs of all currently registered pipelines.
	 *
	 * @return Array of PipelineID values.
	 */
	TArray<FName> GetAllPipelineIDs() const;

	/**
	 * Ensure a pipeline with the given ID exists; if not, create an empty one.
	 *
	 * @param PipelineID   The ID of the pipeline to ensure.
	 * @param DisplayName  Optional display name; defaults to PipelineID converted to FText.
	 */
	void EnsurePipelineExists(FName PipelineID, const FText& DisplayName = FText::GetEmpty());

	/**
	 * Dynamically add a node to an existing pipeline (skips if NodeID already present).
	 * The pipeline is auto-created via EnsurePipelineExists if it does not yet exist.
	 *
	 * @param PipelineID       The pipeline to add the node to.
	 * @param NodeID           Unique ID for the new node.
	 * @param DisplayName      Human-readable display name.
	 * @param RelativePosition Canvas position with each component in [0, 1].
	 * @return true if a new node was actually added.
	 */
	bool AddNodeToPipeline(FName PipelineID, FName NodeID, const FText& DisplayName, FVector2D RelativePosition);

	/**
	 * Dynamically add a directed edge to an existing pipeline (skips if already present).
	 * The pipeline must already exist; returns false otherwise.
	 *
	 * @param PipelineID    The pipeline to add the edge to.
	 * @param SourceNodeID  Source node of the edge.
	 * @param TargetNodeID  Target node of the edge.
	 * @return true if a new edge was actually added.
	 */
	bool AddEdgeToPipeline(FName PipelineID, FName SourceNodeID, FName TargetNodeID);

	/** Broadcast whenever a pipeline is registered or unregistered. */
	FOnRegistryChanged OnRegistryChanged;

private:
	FFlowPipelineRegistry();
	~FFlowPipelineRegistry();

	// Non-copyable, non-movable
	FFlowPipelineRegistry(const FFlowPipelineRegistry&) = delete;
	FFlowPipelineRegistry& operator=(const FFlowPipelineRegistry&) = delete;
	FFlowPipelineRegistry(FFlowPipelineRegistry&&) = delete;
	FFlowPipelineRegistry& operator=(FFlowPipelineRegistry&&) = delete;

	/** Internal storage keyed by PipelineID. */
	TMap<FName, FFlowPipeline> RegisteredPipelines;
};
